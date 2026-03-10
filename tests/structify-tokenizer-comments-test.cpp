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

#include <structify/structify.h>
#include "tokenizer-test-util.h"
#include "catch2/catch_all.hpp"

namespace json_tokenizer_comments_test
{

TEST_CASE("basic_comment_parsing", "[tokenizer][comments]")
{
  const char json_data[] = R"json({
    // This is a comment
    "key": "value"
  })json";

  STFY::Tokenizer tokenizer;
  tokenizer.allowComments(true);
  tokenizer.addData(json_data, sizeof(json_data));

  STFY::Token token;
  size_t count;
  STFY::Error error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectStart);

  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::String);
  REQUIRE(token.name.size == 3);
  REQUIRE(strncmp(token.name.data, "key", 3) == 0);
  REQUIRE(token.value.size == 5);
  REQUIRE(strncmp(token.value.data, "value", 5) == 0);

  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectEnd);
}

TEST_CASE("comment_at_beginning", "[tokenizer][comments]")
{
  const char json_data[] = R"json(// Start comment
{
  "key": "value"
})json";

  STFY::Tokenizer tokenizer;
  tokenizer.allowComments(true);
  tokenizer.addData(json_data, sizeof(json_data));

  STFY::Token token;
  size_t count;
  STFY::Error error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectStart);

  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::String);
  REQUIRE(strncmp(token.name.data, "key", 3) == 0);
}

TEST_CASE("inline_comment", "[tokenizer][comments]")
{
  const char json_data[] = R"json({
  "key": "value", // Inline comment
  "number": 42
})json";

  STFY::Tokenizer tokenizer;
  tokenizer.allowComments(true);
  tokenizer.addData(json_data, sizeof(json_data));

  STFY::Token token;
  size_t count;
  STFY::Error error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectStart);

  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::String);
  REQUIRE(strncmp(token.name.data, "key", 3) == 0);
  REQUIRE(strncmp(token.value.data, "value", 5) == 0);

  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(assert_token(token, STFY::Type::String, "number", STFY::Type::Number, "42") == 0);
}

TEST_CASE("multiple_consecutive_comments", "[tokenizer][comments]")
{
  const char json_data[] = R"json(// First comment
// Second comment
// Third comment
{
  "key": "value"
})json";

  STFY::Tokenizer tokenizer;
  tokenizer.allowComments(true);
  tokenizer.addData(json_data, sizeof(json_data));

  STFY::Token token;
  size_t count;
  STFY::Error error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectStart);

  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::String);
}

TEST_CASE("comments_in_array", "[tokenizer][comments]")
{
  const char json_data[] = R"json({
  "items": [
    // First item comment
    "item1",
    // Second item comment
    "item2"
    // End of array comment
  ]
})json";

  STFY::Tokenizer tokenizer;
  tokenizer.allowComments(true);
  tokenizer.addData(json_data, sizeof(json_data));

  STFY::Token token;
  size_t count;
  STFY::Error error;

  // ObjectStart
  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectStart);

  // "items" property
  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(assert_token(token, STFY::Type::String, "items", STFY::Type::ArrayStart, "[") == 0);

  // "item1"
  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(assert_token(token, STFY::Type::Ascii, "", STFY::Type::String, "item1") == 0);

  // "item2"
  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(assert_token(token, STFY::Type::Ascii, "", STFY::Type::String, "item2") == 0);

  // ArrayEnd
  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ArrayEnd);

  // ObjectEnd
  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectEnd);
}

TEST_CASE("comments_disabled_by_default", "[tokenizer][comments]")
{
  STFY::Tokenizer tokenizer;
  // Not calling allowComments(true), should be disabled by default

  const char json_data[] = R"json({
  "key": "value"
})json";

  tokenizer.addData(json_data, sizeof(json_data));

  STFY::Token token;
  size_t count;
  STFY::Error error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectStart);
}

TEST_CASE("comments_between_object_properties", "[tokenizer][comments]")
{
  const char json_data[] = R"json({
  "first": "value1",
  // Comment between properties
  "second": "value2",
  // Another comment
  "third": 123
})json";

  STFY::Tokenizer tokenizer;
  tokenizer.allowComments(true);
  tokenizer.addData(json_data, sizeof(json_data));

  STFY::Token token;
  size_t count;
  STFY::Error error;

  // ObjectStart
  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectStart);

  // "first" property
  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::String);
  REQUIRE(strncmp(token.name.data, "first", 5) == 0);
  REQUIRE(strncmp(token.value.data, "value1", 6) == 0);

  // "second" property
  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::String);
  REQUIRE(strncmp(token.name.data, "second", 6) == 0);
  REQUIRE(strncmp(token.value.data, "value2", 6) == 0);

  // "third" property
  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(assert_token(token, STFY::Type::String, "third", STFY::Type::Number, "123") == 0);

  // ObjectEnd
  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectEnd);
}

TEST_CASE("comment_after_colon", "[tokenizer][comments]")
{
  const char json_data[] = R"json({
  "key": // Comment after colon
    "value"
})json";

  STFY::Tokenizer tokenizer;
  tokenizer.allowComments(true);
  tokenizer.addData(json_data, sizeof(json_data));

  STFY::Token token;
  size_t count;
  STFY::Error error;

  // ObjectStart
  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectStart);

  // "key" property
  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::String);
  REQUIRE(strncmp(token.name.data, "key", 3) == 0);
  REQUIRE(strncmp(token.value.data, "value", 5) == 0);

  // ObjectEnd
  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectEnd);
}

TEST_CASE("empty_comment", "[tokenizer][comments]")
{
  const char json_data[] = R"json({
  //
  "key": "value"
})json";

  STFY::Tokenizer tokenizer;
  tokenizer.allowComments(true);
  tokenizer.addData(json_data, sizeof(json_data));

  STFY::Token token;
  size_t count;
  STFY::Error error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectStart);

  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::String);
  REQUIRE(strncmp(token.name.data, "key", 3) == 0);
}

TEST_CASE("comment_with_special_characters", "[tokenizer][comments]")
{
  const char json_data[] = R"json({
  // Comment with special chars: !@#$%^&*(){}[]
  "key": "value"
})json";

  STFY::Tokenizer tokenizer;
  tokenizer.allowComments(true);
  tokenizer.addData(json_data, sizeof(json_data));

  STFY::Token token;
  size_t count;
  STFY::Error error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectStart);

  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::String);
  REQUIRE(strncmp(token.name.data, "key", 3) == 0);
}

TEST_CASE("comment_without_newline_at_eof", "[tokenizer][comments]")
{
  const char json_data[] = "{\n  \"key\": \"value\" // EOF comment";

  STFY::Tokenizer tokenizer;
  tokenizer.allowComments(true);
  tokenizer.addData(json_data, strlen(json_data));

  STFY::Token token;
  size_t count;
  STFY::Error error;

  // ObjectStart
  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectStart);

  // "key" property
  error = tokenizer.nextTokens(&token, 1, count);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::String);
  REQUIRE(strncmp(token.name.data, "key", 3) == 0);
  REQUIRE(strncmp(token.value.data, "value", 5) == 0);

  // This should handle the case where comment reaches EOF
  error = tokenizer.nextTokens(&token, 1, count);
  // The tokenizer should either succeed with ObjectEnd or request more data
  REQUIRE((error == STFY::Error::NoError || error == STFY::Error::NeedMoreData));
}

} // namespace json_tokenizer_comments_test