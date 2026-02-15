#include <json_struct/json_struct.h>

#include "catch2/catch_all.hpp"

namespace yaml_struct_test
{

struct SimpleStruct
{
  std::string name;
  int age;
  float score;
  JS_OBJ(name, age, score);
};

struct NestedInner
{
  std::string street;
  std::string city;
  JS_OBJ(street, city);
};

struct NestedOuter
{
  std::string name;
  NestedInner address;
  JS_OBJ(name, address);
};

struct WithVector
{
  std::string name;
  std::vector<int> scores;
  JS_OBJ(name, scores);
};

struct WithStringVector
{
  std::vector<std::string> items;
  JS_OBJ(items);
};

struct Person
{
  std::string name;
  int age;
  JS_OBJ(name, age);
};

struct WithBooleans
{
  bool enabled;
  bool active;
  bool visible;
  bool debug;
  JS_OBJ(enabled, active, visible, debug);
};

struct WithOptional
{
  std::string name;
  JS::Optional<int> age;
  JS::Optional<std::string> email;
  JS_OBJ(name, age, email);
};

struct DeepNested
{
  struct Level2
  {
    struct Level3
    {
      std::string value;
      JS_OBJ(value);
    };
    Level3 level3;
    JS_OBJ(level3);
  };
  Level2 level2;
  JS_OBJ(level2);
};

struct MixedStruct
{
  std::string name;
  int age;
  bool active;
  std::vector<std::string> hobbies;
  JS_OBJ(name, age, active, hobbies);
};

struct WithFlowValues
{
  std::string name;
  std::vector<int> nums;
  JS_OBJ(name, nums);
};

TEST_CASE("yaml_parse_simple_struct", "[yaml][struct]")
{
  const char yaml[] =
    "name: John\n"
    "age: 30\n"
    "score: 95.5\n";

  JS::ParseContext context;
  context.tokenizer.allowYaml(true);
  context.tokenizer.addData(yaml, sizeof(yaml) - 1);

  SimpleStruct s;
  context.parseTo(s);

  REQUIRE(context.error == JS::Error::NoError);
  REQUIRE(s.name == "John");
  REQUIRE(s.age == 30);
  REQUIRE(s.score == Catch::Approx(95.5f));
}

TEST_CASE("yaml_parse_nested_struct", "[yaml][struct]")
{
  const char yaml[] =
    "name: John\n"
    "address:\n"
    "  street: 123 Main St\n"
    "  city: Springfield\n";

  JS::ParseContext context;
  context.tokenizer.allowYaml(true);
  context.tokenizer.addData(yaml, sizeof(yaml) - 1);

  NestedOuter s;
  context.parseTo(s);

  REQUIRE(context.error == JS::Error::NoError);
  REQUIRE(s.name == "John");
  REQUIRE(s.address.street == "123 Main St");
  REQUIRE(s.address.city == "Springfield");
}

TEST_CASE("yaml_parse_vector_of_ints", "[yaml][struct]")
{
  const char yaml[] =
    "name: test\n"
    "scores:\n"
    "  - 100\n"
    "  - 95\n"
    "  - 88\n";

  JS::ParseContext context;
  context.tokenizer.allowYaml(true);
  context.tokenizer.addData(yaml, sizeof(yaml) - 1);

  WithVector s;
  context.parseTo(s);

  REQUIRE(context.error == JS::Error::NoError);
  REQUIRE(s.name == "test");
  REQUIRE(s.scores.size() == 3);
  REQUIRE(s.scores[0] == 100);
  REQUIRE(s.scores[1] == 95);
  REQUIRE(s.scores[2] == 88);
}

TEST_CASE("yaml_parse_vector_of_strings", "[yaml][struct]")
{
  const char yaml[] =
    "items:\n"
    "  - apple\n"
    "  - banana\n"
    "  - cherry\n";

  JS::ParseContext context;
  context.tokenizer.allowYaml(true);
  context.tokenizer.addData(yaml, sizeof(yaml) - 1);

  WithStringVector s;
  context.parseTo(s);

  REQUIRE(context.error == JS::Error::NoError);
  REQUIRE(s.items.size() == 3);
  REQUIRE(s.items[0] == "apple");
  REQUIRE(s.items[1] == "banana");
  REQUIRE(s.items[2] == "cherry");
}

TEST_CASE("yaml_parse_vector_of_structs", "[yaml][struct]")
{
  const char yaml[] =
    "- name: John\n"
    "  age: 30\n"
    "- name: Jane\n"
    "  age: 25\n"
    "- name: Bob\n"
    "  age: 35\n";

  JS::ParseContext context;
  context.tokenizer.allowYaml(true);
  context.tokenizer.addData(yaml, sizeof(yaml) - 1);

  std::vector<Person> people;
  context.parseTo(people);

  REQUIRE(context.error == JS::Error::NoError);
  REQUIRE(people.size() == 3);
  REQUIRE(people[0].name == "John");
  REQUIRE(people[0].age == 30);
  REQUIRE(people[1].name == "Jane");
  REQUIRE(people[1].age == 25);
  REQUIRE(people[2].name == "Bob");
  REQUIRE(people[2].age == 35);
}

TEST_CASE("yaml_parse_booleans_yes_no", "[yaml][struct]")
{
  const char yaml[] =
    "enabled: yes\n"
    "active: true\n"
    "visible: on\n"
    "debug: no\n";

  JS::ParseContext context;
  context.tokenizer.allowYaml(true);
  context.tokenizer.addData(yaml, sizeof(yaml) - 1);

  WithBooleans s;
  context.parseTo(s);

  REQUIRE(context.error == JS::Error::NoError);
  REQUIRE(s.enabled == true);
  REQUIRE(s.active == true);
  REQUIRE(s.visible == true);
  REQUIRE(s.debug == false);
}

TEST_CASE("yaml_parse_optional_members", "[yaml][struct]")
{
  const char yaml[] =
    "name: John\n"
    "age: 30\n";

  JS::ParseContext context;
  context.tokenizer.allowYaml(true);
  context.tokenizer.addData(yaml, sizeof(yaml) - 1);

  WithOptional s;
  context.parseTo(s);

  REQUIRE(context.error == JS::Error::NoError);
  REQUIRE(s.name == "John");
  REQUIRE(s.age.data == 30);
  // email is not in YAML, should remain default
}

TEST_CASE("yaml_parse_deeply_nested", "[yaml][struct]")
{
  const char yaml[] =
    "level2:\n"
    "  level3:\n"
    "    value: deep\n";

  JS::ParseContext context;
  context.tokenizer.allowYaml(true);
  context.tokenizer.addData(yaml, sizeof(yaml) - 1);

  DeepNested s;
  context.parseTo(s);

  REQUIRE(context.error == JS::Error::NoError);
  REQUIRE(s.level2.level3.value == "deep");
}

TEST_CASE("yaml_parse_mixed_struct", "[yaml][struct]")
{
  const char yaml[] =
    "name: Alice\n"
    "age: 28\n"
    "active: true\n"
    "hobbies:\n"
    "  - reading\n"
    "  - gaming\n"
    "  - cooking\n";

  JS::ParseContext context;
  context.tokenizer.allowYaml(true);
  context.tokenizer.addData(yaml, sizeof(yaml) - 1);

  MixedStruct s;
  context.parseTo(s);

  REQUIRE(context.error == JS::Error::NoError);
  REQUIRE(s.name == "Alice");
  REQUIRE(s.age == 28);
  REQUIRE(s.active == true);
  REQUIRE(s.hobbies.size() == 3);
  REQUIRE(s.hobbies[0] == "reading");
  REQUIRE(s.hobbies[1] == "gaming");
  REQUIRE(s.hobbies[2] == "cooking");
}

TEST_CASE("yaml_parse_with_document_marker", "[yaml][struct]")
{
  const char yaml[] =
    "---\n"
    "name: John\n"
    "age: 30\n"
    "score: 100.0\n";

  JS::ParseContext context;
  context.tokenizer.allowYaml(true);
  context.tokenizer.addData(yaml, sizeof(yaml) - 1);

  SimpleStruct s;
  context.parseTo(s);

  REQUIRE(context.error == JS::Error::NoError);
  REQUIRE(s.name == "John");
  REQUIRE(s.age == 30);
  REQUIRE(s.score == Catch::Approx(100.0f));
}

TEST_CASE("yaml_parse_flow_array_value", "[yaml][struct]")
{
  const char yaml[] =
    "name: test\n"
    "nums: [1, 2, 3]\n";

  JS::ParseContext context;
  context.tokenizer.allowYaml(true);
  context.tokenizer.addData(yaml, sizeof(yaml) - 1);

  WithFlowValues s;
  context.parseTo(s);

  REQUIRE(context.error == JS::Error::NoError);
  REQUIRE(s.name == "test");
  REQUIRE(s.nums.size() == 3);
  REQUIRE(s.nums[0] == 1);
  REQUIRE(s.nums[1] == 2);
  REQUIRE(s.nums[2] == 3);
}

TEST_CASE("yaml_parse_extra_members_ignored", "[yaml][struct]")
{
  const char yaml[] =
    "name: John\n"
    "age: 30\n"
    "score: 95.5\n"
    "extra_field: ignored\n"
    "another: also_ignored\n";

  JS::ParseContext context;
  context.tokenizer.allowYaml(true);
  context.tokenizer.addData(yaml, sizeof(yaml) - 1);

  SimpleStruct s;
  context.parseTo(s);

  REQUIRE(context.error == JS::Error::NoError);
  REQUIRE(s.name == "John");
  REQUIRE(s.age == 30);
  REQUIRE(s.score == Catch::Approx(95.5f));
}

TEST_CASE("yaml_parse_with_comments", "[yaml][struct]")
{
  const char yaml[] =
    "# Configuration\n"
    "name: John # person name\n"
    "age: 30\n"
    "score: 100.0\n";

  JS::ParseContext context;
  context.tokenizer.allowYaml(true);
  context.tokenizer.addData(yaml, sizeof(yaml) - 1);

  SimpleStruct s;
  context.parseTo(s);

  REQUIRE(context.error == JS::Error::NoError);
  REQUIRE(s.name == "John");
  REQUIRE(s.age == 30);
}

TEST_CASE("yaml_parse_quoted_strings", "[yaml][struct]")
{
  const char yaml[] =
    "name: \"John Doe\"\n"
    "age: 30\n"
    "score: 0.0\n";

  JS::ParseContext context;
  context.tokenizer.allowYaml(true);
  context.tokenizer.addData(yaml, sizeof(yaml) - 1);

  SimpleStruct s;
  context.parseTo(s);

  REQUIRE(context.error == JS::Error::NoError);
  REQUIRE(s.name == "John Doe");
}

TEST_CASE("yaml_parse_negative_numbers", "[yaml][struct]")
{
  const char yaml[] =
    "name: test\n"
    "age: -5\n"
    "score: -3.14\n";

  JS::ParseContext context;
  context.tokenizer.allowYaml(true);
  context.tokenizer.addData(yaml, sizeof(yaml) - 1);

  SimpleStruct s;
  context.parseTo(s);

  REQUIRE(context.error == JS::Error::NoError);
  REQUIRE(s.age == -5);
  REQUIRE(s.score == Catch::Approx(-3.14f));
}

TEST_CASE("yaml_parse_literal_block_scalar", "[yaml][struct]")
{
  const char yaml[] =
    "name: |\n"
    "  line one\n"
    "  line two\n"
    "age: 30\n"
    "score: 0.0\n";

  JS::ParseContext context;
  context.tokenizer.allowYaml(true);
  context.tokenizer.addData(yaml, sizeof(yaml) - 1);

  SimpleStruct s;
  context.parseTo(s);

  REQUIRE(context.error == JS::Error::NoError);
  REQUIRE(s.name == "line one\nline two\n");
  REQUIRE(s.age == 30);
}

} // namespace yaml_struct_test
