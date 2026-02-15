#include <json_struct/json_struct.h>
#include "tokenizer-test-util.h"

#include "catch2/catch_all.hpp"

namespace yaml_tokenizer_test
{

TEST_CASE("yaml_simple_mapping", "[yaml][tokenizer]")
{
  const char yaml[] = "name: John\nage: 30\n";

  JS::Tokenizer tokenizer;
  tokenizer.allowYaml(true);
  tokenizer.addData(yaml, sizeof(yaml) - 1);

  JS::Token token;
  JS::Error error;

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ObjectStart);

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(assert_token(token, JS::Type::Ascii, "name", JS::Type::Ascii, "John") == 0);

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(assert_token(token, JS::Type::Ascii, "age", JS::Type::Number, "30") == 0);

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ObjectEnd);
}

TEST_CASE("yaml_simple_sequence", "[yaml][tokenizer]")
{
  const char yaml[] = "- apple\n- banana\n- cherry\n";

  JS::Tokenizer tokenizer;
  tokenizer.allowYaml(true);
  tokenizer.addData(yaml, sizeof(yaml) - 1);

  JS::Token token;
  JS::Error error;

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ArrayStart);

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::Ascii);
  REQUIRE(std::string(token.value.data, token.value.size) == "apple");

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::Ascii);
  REQUIRE(std::string(token.value.data, token.value.size) == "banana");

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::Ascii);
  REQUIRE(std::string(token.value.data, token.value.size) == "cherry");

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ArrayEnd);
}

TEST_CASE("yaml_nested_mapping", "[yaml][tokenizer]")
{
  const char yaml[] =
    "person:\n"
    "  name: John\n"
    "  age: 30\n";

  JS::Tokenizer tokenizer;
  tokenizer.allowYaml(true);
  tokenizer.addData(yaml, sizeof(yaml) - 1);

  JS::Token token;
  JS::Error error;

  // Outer ObjectStart
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ObjectStart);

  // person: ObjectStart (property with container value)
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ObjectStart);
  REQUIRE(std::string(token.name.data, token.name.size) == "person");

  // name: John
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(assert_token(token, JS::Type::Ascii, "name", JS::Type::Ascii, "John") == 0);

  // age: 30
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(assert_token(token, JS::Type::Ascii, "age", JS::Type::Number, "30") == 0);

  // Inner ObjectEnd
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ObjectEnd);

  // Outer ObjectEnd
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ObjectEnd);
}

TEST_CASE("yaml_sequence_of_mappings", "[yaml][tokenizer]")
{
  const char yaml[] =
    "- name: John\n"
    "  age: 30\n"
    "- name: Jane\n"
    "  age: 25\n";

  JS::Tokenizer tokenizer;
  tokenizer.allowYaml(true);
  tokenizer.addData(yaml, sizeof(yaml) - 1);

  JS::Token token;
  JS::Error error;

  // ArrayStart
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ArrayStart);

  // First item: ObjectStart
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ObjectStart);

  // name: John
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(assert_token(token, JS::Type::Ascii, "name", JS::Type::Ascii, "John") == 0);

  // age: 30
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(assert_token(token, JS::Type::Ascii, "age", JS::Type::Number, "30") == 0);

  // ObjectEnd (first item)
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ObjectEnd);

  // Second item: ObjectStart
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ObjectStart);

  // name: Jane
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(assert_token(token, JS::Type::Ascii, "name", JS::Type::Ascii, "Jane") == 0);

  // age: 25
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(assert_token(token, JS::Type::Ascii, "age", JS::Type::Number, "25") == 0);

  // ObjectEnd (second item)
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ObjectEnd);

  // ArrayEnd
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ArrayEnd);
}

TEST_CASE("yaml_scalar_types", "[yaml][tokenizer]")
{
  const char yaml[] =
    "string_val: hello world\n"
    "int_val: 42\n"
    "float_val: 3.14\n"
    "neg_val: -7\n"
    "bool_true: true\n"
    "bool_false: false\n"
    "bool_yes: yes\n"
    "bool_no: no\n"
    "bool_on: on\n"
    "bool_off: off\n"
    "null_val: null\n"
    "null_tilde: ~\n"
    "quoted: \"hello world\"\n"
    "single_quoted: 'hello world'\n";

  JS::Tokenizer tokenizer;
  tokenizer.allowYaml(true);
  tokenizer.addData(yaml, sizeof(yaml) - 1);

  JS::Token token;
  JS::Error error;

  // ObjectStart
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ObjectStart);

  // string_val: hello world (Ascii)
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(assert_token(token, JS::Type::Ascii, "string_val", JS::Type::Ascii, "hello world") == 0);

  // int_val: 42
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(assert_token(token, JS::Type::Ascii, "int_val", JS::Type::Number, "42") == 0);

  // float_val: 3.14
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(assert_token(token, JS::Type::Ascii, "float_val", JS::Type::Number, "3.14") == 0);

  // neg_val: -7
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(assert_token(token, JS::Type::Ascii, "neg_val", JS::Type::Number, "-7") == 0);

  // bool_true: true
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(assert_token(token, JS::Type::Ascii, "bool_true", JS::Type::Bool, "true") == 0);

  // bool_false: false
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(assert_token(token, JS::Type::Ascii, "bool_false", JS::Type::Bool, "false") == 0);

  // bool_yes: yes → true
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(assert_token(token, JS::Type::Ascii, "bool_yes", JS::Type::Bool, "true") == 0);

  // bool_no: no → false
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(assert_token(token, JS::Type::Ascii, "bool_no", JS::Type::Bool, "false") == 0);

  // bool_on: on → true
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(assert_token(token, JS::Type::Ascii, "bool_on", JS::Type::Bool, "true") == 0);

  // bool_off: off → false
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(assert_token(token, JS::Type::Ascii, "bool_off", JS::Type::Bool, "false") == 0);

  // null_val: null
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(assert_token(token, JS::Type::Ascii, "null_val", JS::Type::Null, "null") == 0);

  // null_tilde: ~
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(assert_token(token, JS::Type::Ascii, "null_tilde", JS::Type::Null, "null") == 0);

  // quoted: "hello world"
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(assert_token(token, JS::Type::Ascii, "quoted", JS::Type::String, "hello world") == 0);

  // single_quoted: 'hello world'
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(assert_token(token, JS::Type::Ascii, "single_quoted", JS::Type::String, "hello world") == 0);

  // ObjectEnd
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ObjectEnd);
}

TEST_CASE("yaml_mapping_with_array_value", "[yaml][tokenizer]")
{
  const char yaml[] =
    "name: John\n"
    "hobbies:\n"
    "  - reading\n"
    "  - coding\n";

  JS::Tokenizer tokenizer;
  tokenizer.allowYaml(true);
  tokenizer.addData(yaml, sizeof(yaml) - 1);

  JS::Token token;
  JS::Error error;

  // ObjectStart
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ObjectStart);

  // name: John
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(assert_token(token, JS::Type::Ascii, "name", JS::Type::Ascii, "John") == 0);

  // hobbies: ArrayStart
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ArrayStart);
  REQUIRE(std::string(token.name.data, token.name.size) == "hobbies");

  // reading
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::Ascii);
  REQUIRE(std::string(token.value.data, token.value.size) == "reading");

  // coding
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::Ascii);
  REQUIRE(std::string(token.value.data, token.value.size) == "coding");

  // ArrayEnd
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ArrayEnd);

  // ObjectEnd
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ObjectEnd);
}

TEST_CASE("yaml_flow_object", "[yaml][tokenizer]")
{
  const char yaml[] = "point: {x: 1, y: 2}\n";

  JS::Tokenizer tokenizer;
  tokenizer.allowYaml(true);
  tokenizer.addData(yaml, sizeof(yaml) - 1);

  JS::Token token;
  JS::Error error;

  // ObjectStart (outer)
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ObjectStart);

  // point: ObjectStart (inner)
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ObjectStart);
  REQUIRE(std::string(token.name.data, token.name.size) == "point");

  // x: 1
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(assert_token(token, JS::Type::Ascii, "x", JS::Type::Number, "1") == 0);

  // y: 2
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(assert_token(token, JS::Type::Ascii, "y", JS::Type::Number, "2") == 0);

  // ObjectEnd (inner)
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ObjectEnd);

  // ObjectEnd (outer)
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ObjectEnd);
}

TEST_CASE("yaml_flow_array", "[yaml][tokenizer]")
{
  const char yaml[] = "colors: [red, green, blue]\n";

  JS::Tokenizer tokenizer;
  tokenizer.allowYaml(true);
  tokenizer.addData(yaml, sizeof(yaml) - 1);

  JS::Token token;
  JS::Error error;

  // ObjectStart
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ObjectStart);

  // colors: ArrayStart
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ArrayStart);
  REQUIRE(std::string(token.name.data, token.name.size) == "colors");

  // red
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(std::string(token.value.data, token.value.size) == "red");

  // green
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(std::string(token.value.data, token.value.size) == "green");

  // blue
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(std::string(token.value.data, token.value.size) == "blue");

  // ArrayEnd
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ArrayEnd);

  // ObjectEnd
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ObjectEnd);
}

TEST_CASE("yaml_comments", "[yaml][tokenizer]")
{
  const char yaml[] =
    "# This is a comment\n"
    "name: John # inline comment\n"
    "age: 30\n";

  JS::Tokenizer tokenizer;
  tokenizer.allowYaml(true);
  tokenizer.addData(yaml, sizeof(yaml) - 1);

  JS::Token token;
  JS::Error error;

  // ObjectStart
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ObjectStart);

  // name: John (comment stripped)
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(assert_token(token, JS::Type::Ascii, "name", JS::Type::Ascii, "John") == 0);

  // age: 30
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(assert_token(token, JS::Type::Ascii, "age", JS::Type::Number, "30") == 0);

  // ObjectEnd
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ObjectEnd);
}

TEST_CASE("yaml_document_markers", "[yaml][tokenizer]")
{
  const char yaml[] =
    "---\n"
    "name: John\n"
    "age: 30\n";

  JS::Tokenizer tokenizer;
  tokenizer.allowYaml(true);
  tokenizer.addData(yaml, sizeof(yaml) - 1);

  JS::Token token;
  JS::Error error;

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ObjectStart);

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(assert_token(token, JS::Type::Ascii, "name", JS::Type::Ascii, "John") == 0);

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(assert_token(token, JS::Type::Ascii, "age", JS::Type::Number, "30") == 0);

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ObjectEnd);
}

TEST_CASE("yaml_empty_value_is_null", "[yaml][tokenizer]")
{
  const char yaml[] =
    "name:\n"
    "value: 42\n";

  JS::Tokenizer tokenizer;
  tokenizer.allowYaml(true);
  tokenizer.addData(yaml, sizeof(yaml) - 1);

  JS::Token token;
  JS::Error error;

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ObjectStart);

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(assert_token(token, JS::Type::Ascii, "name", JS::Type::Null, "null") == 0);

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(assert_token(token, JS::Type::Ascii, "value", JS::Type::Number, "42") == 0);

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ObjectEnd);
}

TEST_CASE("yaml_number_sequence", "[yaml][tokenizer]")
{
  const char yaml[] = "- 1\n- 2\n- 3\n";

  JS::Tokenizer tokenizer;
  tokenizer.allowYaml(true);
  tokenizer.addData(yaml, sizeof(yaml) - 1);

  JS::Token token;
  JS::Error error;

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ArrayStart);

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::Number);
  REQUIRE(std::string(token.value.data, token.value.size) == "1");

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::Number);
  REQUIRE(std::string(token.value.data, token.value.size) == "2");

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::Number);
  REQUIRE(std::string(token.value.data, token.value.size) == "3");

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ArrayEnd);
}

TEST_CASE("yaml_deeply_nested", "[yaml][tokenizer]")
{
  const char yaml[] =
    "level1:\n"
    "  level2:\n"
    "    level3:\n"
    "      value: deep\n";

  JS::Tokenizer tokenizer;
  tokenizer.allowYaml(true);
  tokenizer.addData(yaml, sizeof(yaml) - 1);

  JS::Token token;
  JS::Error error;

  // ObjectStart (root)
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ObjectStart);

  // level1: ObjectStart
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ObjectStart);
  REQUIRE(std::string(token.name.data, token.name.size) == "level1");

  // level2: ObjectStart
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ObjectStart);
  REQUIRE(std::string(token.name.data, token.name.size) == "level2");

  // level3: ObjectStart
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ObjectStart);
  REQUIRE(std::string(token.name.data, token.name.size) == "level3");

  // value: deep
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(assert_token(token, JS::Type::Ascii, "value", JS::Type::Ascii, "deep") == 0);

  // ObjectEnd x4
  for (int i = 0; i < 4; i++)
  {
    error = tokenizer.nextToken(token);
    REQUIRE(error == JS::Error::NoError);
    REQUIRE(token.value_type == JS::Type::ObjectEnd);
  }
}

TEST_CASE("yaml_quoted_string_escapes", "[yaml][tokenizer]")
{
  const char yaml[] = "msg: \"hello\\nworld\"\n";

  JS::Tokenizer tokenizer;
  tokenizer.allowYaml(true);
  tokenizer.addData(yaml, sizeof(yaml) - 1);

  JS::Token token;
  JS::Error error;

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ObjectStart);

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(std::string(token.name.data, token.name.size) == "msg");
  REQUIRE(token.value_type == JS::Type::String);
  REQUIRE(std::string(token.value.data, token.value.size) == "hello\nworld");

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ObjectEnd);
}

TEST_CASE("yaml_literal_block_scalar", "[yaml][tokenizer]")
{
  const char yaml[] =
    "description: |\n"
    "  line one\n"
    "  line two\n"
    "  line three\n"
    "other: value\n";

  JS::Tokenizer tokenizer;
  tokenizer.allowYaml(true);
  tokenizer.addData(yaml, sizeof(yaml) - 1);

  JS::Token token;
  JS::Error error;

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ObjectStart);

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(std::string(token.name.data, token.name.size) == "description");
  REQUIRE(token.value_type == JS::Type::String);
  REQUIRE(std::string(token.value.data, token.value.size) == "line one\nline two\nline three\n");

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(assert_token(token, JS::Type::Ascii, "other", JS::Type::Ascii, "value") == 0);

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ObjectEnd);
}

TEST_CASE("yaml_folded_block_scalar", "[yaml][tokenizer]")
{
  const char yaml[] =
    "description: >\n"
    "  line one\n"
    "  line two\n"
    "  line three\n"
    "other: value\n";

  JS::Tokenizer tokenizer;
  tokenizer.allowYaml(true);
  tokenizer.addData(yaml, sizeof(yaml) - 1);

  JS::Token token;
  JS::Error error;

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ObjectStart);

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(std::string(token.name.data, token.name.size) == "description");
  REQUIRE(token.value_type == JS::Type::String);
  REQUIRE(std::string(token.value.data, token.value.size) == "line one line two line three\n");

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(assert_token(token, JS::Type::Ascii, "other", JS::Type::Ascii, "value") == 0);

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ObjectEnd);
}

TEST_CASE("yaml_standalone_flow_object", "[yaml][tokenizer]")
{
  const char yaml[] = "{name: John, age: 30}\n";

  JS::Tokenizer tokenizer;
  tokenizer.allowYaml(true);
  tokenizer.addData(yaml, sizeof(yaml) - 1);

  JS::Token token;
  JS::Error error;

  // Flow objects at top level start with { so they're detected as flow
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ObjectStart);

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(assert_token(token, JS::Type::Ascii, "name", JS::Type::Ascii, "John") == 0);

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(assert_token(token, JS::Type::Ascii, "age", JS::Type::Number, "30") == 0);

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ObjectEnd);
}

TEST_CASE("yaml_standalone_flow_array", "[yaml][tokenizer]")
{
  const char yaml[] = "[1, 2, 3]\n";

  JS::Tokenizer tokenizer;
  tokenizer.allowYaml(true);
  tokenizer.addData(yaml, sizeof(yaml) - 1);

  JS::Token token;
  JS::Error error;

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ArrayStart);

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::Number);
  REQUIRE(std::string(token.value.data, token.value.size) == "1");

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::Number);
  REQUIRE(std::string(token.value.data, token.value.size) == "2");

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::Number);
  REQUIRE(std::string(token.value.data, token.value.size) == "3");

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ArrayEnd);
}

TEST_CASE("yaml_url_value_not_split", "[yaml][tokenizer]")
{
  const char yaml[] = "url: http://example.com\n";

  JS::Tokenizer tokenizer;
  tokenizer.allowYaml(true);
  tokenizer.addData(yaml, sizeof(yaml) - 1);

  JS::Token token;
  JS::Error error;

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ObjectStart);

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(assert_token(token, JS::Type::Ascii, "url", JS::Type::Ascii, "http://example.com") == 0);

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ObjectEnd);
}

TEST_CASE("yaml_nested_array_in_object", "[yaml][tokenizer]")
{
  const char yaml[] =
    "person:\n"
    "  name: John\n"
    "  scores:\n"
    "    - 100\n"
    "    - 95\n"
    "    - 88\n"
    "  active: true\n";

  JS::Tokenizer tokenizer;
  tokenizer.allowYaml(true);
  tokenizer.addData(yaml, sizeof(yaml) - 1);

  JS::Token token;
  JS::Error error;

  // Root ObjectStart
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ObjectStart);

  // person: ObjectStart
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ObjectStart);
  REQUIRE(std::string(token.name.data, token.name.size) == "person");

  // name: John
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(assert_token(token, JS::Type::Ascii, "name", JS::Type::Ascii, "John") == 0);

  // scores: ArrayStart
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ArrayStart);
  REQUIRE(std::string(token.name.data, token.name.size) == "scores");

  // 100, 95, 88
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::Number);
  REQUIRE(std::string(token.value.data, token.value.size) == "100");

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(std::string(token.value.data, token.value.size) == "95");

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(std::string(token.value.data, token.value.size) == "88");

  // ArrayEnd
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ArrayEnd);

  // active: true
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(assert_token(token, JS::Type::Ascii, "active", JS::Type::Bool, "true") == 0);

  // Inner ObjectEnd
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ObjectEnd);

  // Outer ObjectEnd
  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ObjectEnd);
}

TEST_CASE("yaml_case_insensitive_booleans", "[yaml][tokenizer]")
{
  const char yaml[] =
    "a: True\n"
    "b: FALSE\n"
    "c: Yes\n"
    "d: NO\n"
    "e: On\n"
    "f: OFF\n";

  JS::Tokenizer tokenizer;
  tokenizer.allowYaml(true);
  tokenizer.addData(yaml, sizeof(yaml) - 1);

  JS::Token token;
  JS::Error error;

  error = tokenizer.nextToken(token); // ObjectStart
  REQUIRE(error == JS::Error::NoError);

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(assert_token(token, JS::Type::Ascii, "a", JS::Type::Bool, "true") == 0);

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(assert_token(token, JS::Type::Ascii, "b", JS::Type::Bool, "false") == 0);

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(assert_token(token, JS::Type::Ascii, "c", JS::Type::Bool, "true") == 0);

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(assert_token(token, JS::Type::Ascii, "d", JS::Type::Bool, "false") == 0);

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(assert_token(token, JS::Type::Ascii, "e", JS::Type::Bool, "true") == 0);

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(assert_token(token, JS::Type::Ascii, "f", JS::Type::Bool, "false") == 0);

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ObjectEnd);
}

TEST_CASE("yaml_single_quote_escape", "[yaml][tokenizer]")
{
  const char yaml[] = "msg: 'it''s a test'\n";

  JS::Tokenizer tokenizer;
  tokenizer.allowYaml(true);
  tokenizer.addData(yaml, sizeof(yaml) - 1);

  JS::Token token;
  JS::Error error;

  error = tokenizer.nextToken(token); // ObjectStart
  REQUIRE(error == JS::Error::NoError);

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(std::string(token.name.data, token.name.size) == "msg");
  REQUIRE(token.value_type == JS::Type::String);
  REQUIRE(std::string(token.value.data, token.value.size) == "it's a test");

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ObjectEnd);
}

TEST_CASE("yaml_empty_document", "[yaml][tokenizer]")
{
  const char yaml[] = "";

  JS::Tokenizer tokenizer;
  tokenizer.allowYaml(true);
  tokenizer.addData(yaml, 0);

  // With empty input, there should be no tokens
  REQUIRE(tokenizer.currentPosition() == nullptr);
}

TEST_CASE("yaml_sequence_of_numbers", "[yaml][tokenizer]")
{
  const char yaml[] =
    "- 1.5\n"
    "- -2.3\n"
    "- 0\n"
    "- 1e10\n";

  JS::Tokenizer tokenizer;
  tokenizer.allowYaml(true);
  tokenizer.addData(yaml, sizeof(yaml) - 1);

  JS::Token token;
  JS::Error error;

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ArrayStart);

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::Number);
  REQUIRE(std::string(token.value.data, token.value.size) == "1.5");

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::Number);
  REQUIRE(std::string(token.value.data, token.value.size) == "-2.3");

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::Number);
  REQUIRE(std::string(token.value.data, token.value.size) == "0");

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::Number);
  REQUIRE(std::string(token.value.data, token.value.size) == "1e10");

  error = tokenizer.nextToken(token);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(token.value_type == JS::Type::ArrayEnd);
}

} // namespace yaml_tokenizer_test
