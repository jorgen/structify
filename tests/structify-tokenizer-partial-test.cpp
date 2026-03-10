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

#include <string>

namespace json_tokenizer_partial_test
{

// Test with a complete single buffer (equivalent to the old partial_1 test
// where the two buffers are concatenated into one contiguous buffer)
const char json_data_combined_1[] = "{   \"foo\": \"bar\","
                                     "   \"color\": \"red\"\n"
                                     "}";

TEST_CASE("check_json_single_buffer_1", "[tokenizer]")
{
  STFY::Error error;
  STFY::Tokenizer tokenizer;
  tokenizer.allowAsciiType(true);
  tokenizer.allowNewLineAsTokenDelimiter(true);
  tokenizer.addData(json_data_combined_1, sizeof(json_data_combined_1));

  STFY::Token token;
  error = tokenizer.nextTokens(&token, 1).second;
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectStart);

  error = tokenizer.nextTokens(&token, 1).second;
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(assert_token(token, STFY::Type::String, "foo", STFY::Type::String, "bar") == 0);

  error = tokenizer.nextTokens(&token, 1).second;
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE((assert_token(token, STFY::Type::String, "color", STFY::Type::String, "red") == 0));

  error = tokenizer.nextTokens(&token, 1).second;
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectEnd);

  error = tokenizer.nextTokens(&token, 1).second;
  REQUIRE(error == STFY::Error::NeedMoreData);
}

// Test with complete buffer containing ascii properties and bool values
const char json_data_combined_2[] = "{  \"foo\": \"bar\","
                                     "   color : \"red\"\n"
                                     "}";

TEST_CASE("check_json_single_buffer_2", "[tokenizer]")
{
  STFY::Error error;
  STFY::Tokenizer tokenizer;
  tokenizer.allowAsciiType(true);
  tokenizer.allowNewLineAsTokenDelimiter(true);
  tokenizer.addData(json_data_combined_2, sizeof(json_data_combined_2));

  STFY::Token token;
  error = tokenizer.nextTokens(&token, 1).second;
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectStart);

  error = tokenizer.nextTokens(&token, 1).second;
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(assert_token(token, STFY::Type::String, "foo", STFY::Type::String, "bar") == 0);

  error = tokenizer.nextTokens(&token, 1).second;
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE((assert_token(token, STFY::Type::Ascii, "color", STFY::Type::String, "red") == 0));

  error = tokenizer.nextTokens(&token, 1).second;
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectEnd);

  error = tokenizer.nextTokens(&token, 1).second;
  REQUIRE(error == STFY::Error::NeedMoreData);
}

// Test with complete buffer containing bool value
const char json_data_combined_3[] = "{  \"foo\": \"bar\","
                                     "   color : true"
                                     "}";

TEST_CASE("check_json_single_buffer_3", "[tokenizer]")
{
  STFY::Error error;
  STFY::Tokenizer tokenizer;
  tokenizer.allowAsciiType(true);
  tokenizer.allowNewLineAsTokenDelimiter(true);
  tokenizer.addData(json_data_combined_3, sizeof(json_data_combined_3));

  STFY::Token token;
  error = tokenizer.nextTokens(&token, 1).second;
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectStart);

  error = tokenizer.nextTokens(&token, 1).second;
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(assert_token(token, STFY::Type::String, "foo", STFY::Type::String, "bar") == 0);

  error = tokenizer.nextTokens(&token, 1).second;
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE((assert_token(token, STFY::Type::Ascii, "color", STFY::Type::Bool, "true") == 0));

  error = tokenizer.nextTokens(&token, 1).second;
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectEnd);

  error = tokenizer.nextTokens(&token, 1).second;
  REQUIRE(error == STFY::Error::NeedMoreData);
}

// Test with complete buffer containing an array
const char json_data_combined_4[] = "{  \"foo\": \"bar\","
                                     "  \"array\": ["
                                     "    \"one\","
                                     "    \"two\","
                                     "    \"three\""
                                     "  ]"
                                     "}";

TEST_CASE("check_json_single_buffer_4", "[tokenizer]")
{
  STFY::Error error;
  STFY::Tokenizer tokenizer;
  tokenizer.allowAsciiType(true);
  tokenizer.allowNewLineAsTokenDelimiter(true);
  tokenizer.addData(json_data_combined_4, sizeof(json_data_combined_4));

  STFY::Token token;
  error = tokenizer.nextTokens(&token, 1).second;
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectStart);

  error = tokenizer.nextTokens(&token, 1).second;
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(assert_token(token, STFY::Type::String, "foo", STFY::Type::String, "bar") == 0);

  error = tokenizer.nextTokens(&token, 1).second;
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE((assert_token(token, STFY::Type::String, "array", STFY::Type::ArrayStart, "[") == 0));

  error = tokenizer.nextTokens(&token, 1).second;
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE((assert_token(token, STFY::Type::Ascii, "", STFY::Type::String, "one") == 0));

  error = tokenizer.nextTokens(&token, 1).second;
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE((assert_token(token, STFY::Type::Ascii, "", STFY::Type::String, "two") == 0));

  error = tokenizer.nextTokens(&token, 1).second;
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE((assert_token(token, STFY::Type::Ascii, "", STFY::Type::String, "three") == 0));

  error = tokenizer.nextTokens(&token, 1).second;
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE((assert_token(token, STFY::Type::Ascii, "", STFY::Type::ArrayEnd, "]") == 0));

  error = tokenizer.nextTokens(&token, 1).second;
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectEnd);

  error = tokenizer.nextTokens(&token, 1).second;
  REQUIRE(error == STFY::Error::NeedMoreData);
}

} // namespace json_tokenizer_partial_test
