/*
 * Copyright © 2012 Jorgen Lind
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

#include "structify-test-data.h"
#include <structify/structify.h>
#include "tokenizer-test-util.h"

#include "catch2/catch_all.hpp"

namespace json_tokenizer_test
{
TEST_CASE("check_json_with_string_and_ascii", "[tokenizer]")
{
  STFY::Error error;
  STFY::Tokenizer tokenizer;
  tokenizer.allowAsciiType(true);
  tokenizer.allowNewLineAsTokenDelimiter(true);
  tokenizer.addData(json_data1, sizeof(json_data1));

  STFY::Token token;
  size_t count;
  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectStart);

  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(assert_token(token, STFY::Type::String, "foo", STFY::Type::String, "bar") == 0);

  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE((assert_token(token, STFY::Type::String, "color", STFY::Type::String, "red") == 0));

  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE((assert_token(token, STFY::Type::Ascii, "weather", STFY::Type::String, "clear") == 0));

  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE((assert_token(token, STFY::Type::Ascii, "weather1", STFY::Type::String, "clear1") == 0));

  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE((assert_token(token, STFY::Type::Ascii, "ToBeTrue", STFY::Type::Bool, "true") == 0));

  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE((assert_token(token, STFY::Type::Ascii, "HeresANull", STFY::Type::Null, "null") == 0));

  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE((assert_token(token, STFY::Type::Ascii, "ThisIsFalse", STFY::Type::Bool, "false") == 0));

  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE((assert_token(token, STFY::Type::Ascii, "EscapedString", STFY::Type::String, "contains \\\"") == 0));

  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE((assert_token(token, STFY::Type::String, "\\\"EscapedName\\\"", STFY::Type::Bool, "true") == 0));

  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE((assert_token(token, STFY::Type::String, "EscapedProp", STFY::Type::String, "\\\"Hello\\\"") == 0));

  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE((assert_token(token, STFY::Type::Ascii, "ThisIsANumber", STFY::Type::Number, "3.14") == 0));

  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE((assert_token(token, STFY::Type::Ascii, "ThisIsAnObject", STFY::Type::ObjectStart, "{") == 0));

  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE((assert_token(token, STFY::Type::Ascii, "ThisIsASubType", STFY::Type::String, "red") == 0));

  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectEnd);

  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE((assert_token(token, STFY::Type::Ascii, "AnotherProp", STFY::Type::String, "prop") == 0));

  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE((assert_token(token, STFY::Type::Ascii, "ThisIsAnotherObject", STFY::Type::ObjectStart, "{") == 0));

  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE((assert_token(token, STFY::Type::Ascii, "ThisIsAnotherASubType", STFY::Type::String, "blue") == 0));

  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectEnd);

  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE((assert_token(token, STFY::Type::Ascii, "ThisIsAnArray", STFY::Type::ArrayStart, "[") == 0));

  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE((assert_token(token, STFY::Type::Ascii, "", STFY::Type::Number, "12.4") == 0));

  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE((assert_token(token, STFY::Type::Ascii, "", STFY::Type::Number, "3") == 0));

  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE((assert_token(token, STFY::Type::Ascii, "", STFY::Type::Number, "43.2") == 0));

  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ArrayEnd);

  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE((assert_token(token, STFY::Type::Ascii, "ThisIsAnObjectArray", STFY::Type::ArrayStart, "[") == 0));

  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE((assert_token(token, STFY::Type::Ascii, "", STFY::Type::ObjectStart, "{") == 0));

  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE((assert_token(token, STFY::Type::Ascii, "Test1", STFY::Type::String, "Test2") == 0));

  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE((assert_token(token, STFY::Type::Ascii, "Test3", STFY::Type::String, "Test4") == 0));

  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectEnd);

  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE((assert_token(token, STFY::Type::Ascii, "", STFY::Type::ObjectStart, "{") == 0));

  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE((assert_token(token, STFY::Type::Ascii, "Test5", STFY::Type::Bool, "true") == 0));

  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE((assert_token(token, STFY::Type::Ascii, "Test7", STFY::Type::Bool, "false") == 0));

  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectEnd);

  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ArrayEnd);

  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectEnd);

  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NeedMoreData);
}

TEST_CASE("check_json_with_array_and_ascii", "[tokenizer]")
{
  STFY::Error error;
  STFY::Tokenizer tokenizer;
  tokenizer.allowAsciiType(true);
  tokenizer.allowNewLineAsTokenDelimiter(true);
  tokenizer.addData(json_data3, sizeof(json_data3));

  STFY::Token token;
  size_t count;
  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectStart);

  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(std::string("an_array") == std::string(token.name.data, token.name.size));
  REQUIRE(token.value_type == STFY::Type::ArrayStart);

  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(std::string("text_element_one") == std::string(token.value.data, token.value.size));

  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(std::string("text_two") == std::string(token.value.data, token.value.size));

  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(std::string("text_three") == std::string(token.value.data, token.value.size));

  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ArrayEnd);

  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectEnd);
}
TEST_CASE("batch_nextTokens_capacity_greater_than_1", "[tokenizer]")
{
  const char json[] = R"({"a": 1, "b": 2})";
  STFY::Tokenizer tokenizer;
  tokenizer.addData(json, sizeof(json) - 1);

  STFY::Token tokens[16];
  size_t count = 0;
  STFY::Error error = tokenizer.nextTokens(tokens, 16, count);
  // Returns NoError when tokens were written, even if input is now exhausted
  REQUIRE(count == 4);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(tokens[0].value_type == STFY::Type::ObjectStart);
  REQUIRE(tokens[1].value_type == STFY::Type::Number);
  REQUIRE(std::string(tokens[1].name.data, tokens[1].name.size) == "a");
  REQUIRE(tokens[2].value_type == STFY::Type::Number);
  REQUIRE(std::string(tokens[2].name.data, tokens[2].name.size) == "b");
  REQUIRE(tokens[3].value_type == STFY::Type::ObjectEnd);
}

TEST_CASE("batch_nextTokens_exact_capacity", "[tokenizer]")
{
  const char json[] = R"([1, 2, 3])";
  STFY::Tokenizer tokenizer;
  tokenizer.addData(json, sizeof(json) - 1);

  // Request exactly 2 tokens
  STFY::Token tokens[2];
  size_t count = 0;
  STFY::Error error = tokenizer.nextTokens(tokens, 2, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(count == 2);
  REQUIRE(tokens[0].value_type == STFY::Type::ArrayStart);
  REQUIRE(tokens[1].value_type == STFY::Type::Number);

  // Get the rest
  error = tokenizer.nextTokens(tokens, 2, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(count == 2);
  REQUIRE(tokens[0].value_type == STFY::Type::Number);
  REQUIRE(tokens[1].value_type == STFY::Type::Number);

  // Get array end
  error = tokenizer.nextTokens(tokens, 2, count);
  REQUIRE(count == 1);
  REQUIRE(tokens[0].value_type == STFY::Type::ArrayEnd);
}

TEST_CASE("batch_nextTokens_count_output", "[tokenizer]")
{
  const char json[] = R"({"x": true})";
  STFY::Tokenizer tokenizer;
  tokenizer.addData(json, sizeof(json) - 1);

  STFY::Token tokens[8];
  size_t count = 99; // should be overwritten
  STFY::Error error = tokenizer.nextTokens(tokens, 8, count);
  REQUIRE(count == 3); // ObjectStart, "x":true, ObjectEnd
  REQUIRE(error == STFY::Error::NoError); // NoError because tokens were written
  // Next call should return NeedMoreData with count == 0
  error = tokenizer.nextTokens(tokens, 8, count);
  REQUIRE(count == 0);
  REQUIRE(error == STFY::Error::NeedMoreData);
}

TEST_CASE("batch_nextTokens_truncated_input", "[tokenizer]")
{
  // Truncated JSON - missing closing brace
  const char json[] = R"({"key": "val)";
  STFY::Tokenizer tokenizer;
  tokenizer.addData(json, sizeof(json) - 1);

  STFY::Token tokens[8];
  size_t count = 0;
  STFY::Error error = tokenizer.nextTokens(tokens, 8, count);
  // Should get ObjectStart, then NeedMoreData on the truncated string
  // Returns NoError because at least 1 token was written
  REQUIRE(count == 1);
  REQUIRE(tokens[0].value_type == STFY::Type::ObjectStart);
  REQUIRE(error == STFY::Error::NoError);
  // Next call returns NeedMoreData
  error = tokenizer.nextTokens(tokens, 8, count);
  REQUIRE(count == 0);
  REQUIRE(error == STFY::Error::NeedMoreData);
}

TEST_CASE("batch_nextTokens_empty_input", "[tokenizer]")
{
  STFY::Tokenizer tokenizer;
  // No data added

  STFY::Token tokens[4];
  size_t count = 99;
  STFY::Error error = tokenizer.nextTokens(tokens, 4, count);
  REQUIRE(error == STFY::Error::NeedMoreData);
  REQUIRE(count == 0);
}

}// namespace json_tokenizer_test
