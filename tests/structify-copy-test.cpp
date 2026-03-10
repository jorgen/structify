/*
 * Copyright © 2016 Jorgen Lind
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

#include <structify/structify.h>

#include "catch2/catch_all.hpp"

namespace
{
const char json[] = "{\n"
                    "  \"property_one\": 432432,\n"
                    "  \"execute_one\": {\n"
                    "    \"number\": 45,\n"
                    "    \"valid\": \"false\"\n"
                    "  },"
                    "  \"execute_two\": 99,\n"
                    "  \"execute_three\": [\n"
                    "    4,\n"
                    "    6,\n"
                    "    8\n"
                    "  ]\n"
                    "}\n";

struct SubObject
{
  int number;
  bool valid;

  STFY_OBJECT(STFY_MEMBER(number), STFY_MEMBER(valid));
};

void js_validate_json(STFY::Tokenizer &tokenizer)
{
  STFY::Token token;
  STFY::Error error;

  error = tokenizer.nextTokens(&token, 1).second;
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectStart);

  error = tokenizer.nextTokens(&token, 1).second;
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::Number);

  error = tokenizer.nextTokens(&token, 1).second;
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectStart);

  // Save start pointer (the '{' of the sub-object value)
  const char *sub_start = token.value.data;

  while (error == STFY::Error::NoError && token.value_type != STFY::Type::ObjectEnd)
    error = tokenizer.nextTokens(&token, 1).second;

  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectEnd);

  // Compute size from start to end (including the closing '}')
  const char *sub_end = token.value.data + token.value.size;
  size_t sub_size = (size_t)(sub_end - sub_start);
  std::string buffer(sub_start, sub_size);

  while (error == STFY::Error::NoError && token.value_type != STFY::Type::ObjectEnd)
    error = tokenizer.nextTokens(&token, 1).second;

  STFY::ParseContext context(buffer.c_str(), buffer.size());
  SubObject subObj;
  error = context.parseTo(subObj);

  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(subObj.number == 45);
  REQUIRE(subObj.valid == false);
}
TEST_CASE("copy_test_js_copy_full", "[tokenizer]")
{
  STFY::Tokenizer tokenizer;
  tokenizer.addData(json);
  js_validate_json(tokenizer);
}

const char json2[] =
  R"json({
  "test": true,
  "more": {
    "sub_object_prop1": true,
    "sub_object_prop2": 456
  },
  "int_value": 65
})json";

struct Child
{
  bool sub_object_prop1;
  int sub_object_prop2;
  STFY_OBJECT(STFY_MEMBER(sub_object_prop1), STFY_MEMBER(sub_object_prop2));
};

struct Parent
{
  bool test;
  Child more;
  int int_value;
  STFY_OBJECT(STFY_MEMBER(test), STFY_MEMBER(more), STFY_MEMBER(int_value));
};

TEST_CASE("copy_test_js_copy_parsed", "[tokenizer]")
{
  STFY::Tokenizer tokenizer;
  tokenizer.addData(json2);

  STFY::Token token;
  STFY::Error error = STFY::Error::NoError;
  std::vector<STFY::Token> tokens;
  while (error == STFY::Error::NoError)
  {
    error = tokenizer.nextTokens(&token, 1).second;
    tokens.push_back(token);
  }

  STFY::ParseContext context;
  context.tokenizer.addData(&tokens);
  Parent parent;
  error = context.parseTo(parent);

  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(parent.test == true);
  REQUIRE(parent.more.sub_object_prop1 == true);
  REQUIRE(parent.more.sub_object_prop2 == 456);
  REQUIRE(parent.int_value == 65);
}

const char json_token_copy[] = R"json(
{
  "number": 45,
  "valid": false,
  "child": {
    "some_more": "world",
    "another_int": 495
  },
  "more_data": "string data",
  "super_data": "hello"
}
)json";

struct SecondChild
{
  std::string some_more;
  int another_int;
  STFY_OBJECT(STFY_MEMBER(some_more), STFY_MEMBER(another_int));
};
struct SecondParent
{
  int number;
  bool valid;
  STFY::JsonTokens child;
  std::string more_data;
  std::string super_data;

  STFY_OBJECT(STFY_MEMBER(number), STFY_MEMBER(valid), STFY_MEMBER(child), STFY_MEMBER(more_data), STFY_MEMBER(super_data));
};

TEST_CASE("copy_test_js_copy_tokens", "[tokenizer]")
{
  SecondParent parent;
  STFY::ParseContext parseContext(json_token_copy);
  auto error = parseContext.parseTo(parent);

  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(parent.child.data.size() == 4);

  STFY::ParseContext childContext;
  childContext.tokenizer.addData(&parent.child.data);
  SecondChild child;
  error = childContext.parseTo(child);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(child.another_int == 495);
  REQUIRE(child.some_more == "world");
}
} // namespace
