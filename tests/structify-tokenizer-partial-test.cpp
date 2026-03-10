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

namespace json_tokenizer_partial_test
{
const char json_data_partial_1_1[] = "{";
const char json_data_partial_1_2[] = "   \"foo\": \"bar\","
                                     "   \"color\": \"red\"\n"
                                     "}";

TEST_CASE("check_json_partial_1", "[tokenizer]")
{
  STFY::Error error;
  STFY::Tokenizer tokenizer;
  tokenizer.allowAsciiType(true);
  tokenizer.allowNewLineAsTokenDelimiter(true);
  tokenizer.addData(json_data_partial_1_1, sizeof(json_data_partial_1_1));
  tokenizer.addData(json_data_partial_1_2, sizeof(json_data_partial_1_2));

  STFY::Token token;
  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectStart);

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(assert_token(token, STFY::Type::String, "foo", STFY::Type::String, "bar") == 0);

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE((assert_token(token, STFY::Type::String, "color", STFY::Type::String, "red") == 0));

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectEnd);

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NeedMoreData);
}

const char json_data_partial_2_1[] = "{  \"fo";
const char json_data_partial_2_2[] = "o\": \"bar\","
                                     "   \"color\": \"red\"\n"
                                     "}";

TEST_CASE("check_json_partial_2", "[tokenizer]")
{
  STFY::Error error;
  STFY::Tokenizer tokenizer;
  tokenizer.allowAsciiType(true);
  tokenizer.allowNewLineAsTokenDelimiter(true);
  tokenizer.addData(json_data_partial_2_1, sizeof(json_data_partial_2_1));
  tokenizer.addData(json_data_partial_2_2, sizeof(json_data_partial_2_2));

  STFY::Token token;
  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectStart);

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  std::string foo(token.name.data, token.name.size);
  REQUIRE(assert_token(token, STFY::Type::String, "foo", STFY::Type::String, "bar") == 0);

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE((assert_token(token, STFY::Type::String, "color", STFY::Type::String, "red") == 0));

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectEnd);

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NeedMoreData);
}

const char json_data_partial_3_1[] = "{  \"foo\"";
const char json_data_partial_3_2[] = ": \"bar\","
                                     "   \"color\": \"red\"\n"
                                     "}";

TEST_CASE("check_json_partial_3", "[tokenizer]")
{
  STFY::Error error;
  STFY::Tokenizer tokenizer;
  tokenizer.allowAsciiType(true);
  tokenizer.allowNewLineAsTokenDelimiter(true);
  tokenizer.addData(json_data_partial_3_1, sizeof(json_data_partial_3_1));
  tokenizer.addData(json_data_partial_3_2, sizeof(json_data_partial_3_2));

  STFY::Token token;
  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectStart);

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(assert_token(token, STFY::Type::String, "foo", STFY::Type::String, "bar") == 0);

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE((assert_token(token, STFY::Type::String, "color", STFY::Type::String, "red") == 0));

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectEnd);

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NeedMoreData);
}

const char json_data_partial_4_1[] = "{  \"foo\": \"bar\"";
const char json_data_partial_4_2[] = ","
                                     "   \"color\": \"red\"\n"
                                     "}";

TEST_CASE("check_json_partial_4", "[tokenizer]")
{
  STFY::Error error;
  STFY::Tokenizer tokenizer;
  tokenizer.allowAsciiType(true);
  tokenizer.allowNewLineAsTokenDelimiter(true);
  tokenizer.addData(json_data_partial_4_1, sizeof(json_data_partial_4_1));
  tokenizer.addData(json_data_partial_4_2, sizeof(json_data_partial_4_2));

  STFY::Token token;
  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectStart);

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(assert_token(token, STFY::Type::String, "foo", STFY::Type::String, "bar") == 0);

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE((assert_token(token, STFY::Type::String, "color", STFY::Type::String, "red") == 0));

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectEnd);

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NeedMoreData);
}

const char json_data_partial_5_1[] = "{  \"foo\": \"bar\","
                                     "   col";
const char json_data_partial_5_2[] = "or : \"red\"\n"
                                     "}";

TEST_CASE("check_json_partial_5", "[tokenizer]")
{
  STFY::Error error;
  STFY::Tokenizer tokenizer;
  tokenizer.allowAsciiType(true);
  tokenizer.allowNewLineAsTokenDelimiter(true);
  tokenizer.addData(json_data_partial_5_1, sizeof(json_data_partial_5_1));
  tokenizer.addData(json_data_partial_5_2, sizeof(json_data_partial_5_2));

  STFY::Token token;
  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectStart);

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(assert_token(token, STFY::Type::String, "foo", STFY::Type::String, "bar") == 0);

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE((assert_token(token, STFY::Type::Ascii, "color", STFY::Type::String, "red") == 0));

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectEnd);

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NeedMoreData);
}

const char json_data_partial_6_1[] = "{  \"foo\": \"bar\","
                                     "   color : tr";
const char json_data_partial_6_2[] = "ue"
                                     "}";

TEST_CASE("check_json_partial_6", "[tokenizer]")
{
  STFY::Error error;
  STFY::Tokenizer tokenizer;
  tokenizer.allowAsciiType(true);
  tokenizer.allowNewLineAsTokenDelimiter(true);
  tokenizer.addData(json_data_partial_6_1, sizeof(json_data_partial_6_1));
  tokenizer.addData(json_data_partial_6_2, sizeof(json_data_partial_6_2));

  STFY::Token token;
  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectStart);

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(assert_token(token, STFY::Type::String, "foo", STFY::Type::String, "bar") == 0);

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE((assert_token(token, STFY::Type::Ascii, "color", STFY::Type::Bool, "true") == 0));

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectEnd);

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NeedMoreData);
}

const char json_data_partial_7_1[] = "{  \"foo\": \"bar\","
                                     "   color : true";
const char json_data_partial_7_2[] = "}";

TEST_CASE("check_json_partial_7", "[tokenizer]")
{
  STFY::Error error;
  STFY::Tokenizer tokenizer;
  tokenizer.allowAsciiType(true);
  tokenizer.allowNewLineAsTokenDelimiter(true);
  tokenizer.addData(json_data_partial_7_1, sizeof(json_data_partial_7_1));
  tokenizer.addData(json_data_partial_7_2, sizeof(json_data_partial_7_2));

  STFY::Token token;
  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectStart);

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(assert_token(token, STFY::Type::String, "foo", STFY::Type::String, "bar") == 0);

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE((assert_token(token, STFY::Type::Ascii, "color", STFY::Type::Bool, "true") == 0));

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectEnd);

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NeedMoreData);
}

const char json_data_partial_8_1[] = "{  \"foo\": \"bar\","
                                     "  \"array\": ["
                                     "    \"one\","
                                     "    \"two\",";
const char json_data_partial_8_2[] = "    \"three\""
                                     "  ]"
                                     "}";

TEST_CASE("check_json_partial_8", "[tokenizer]")
{
  STFY::Error error;
  STFY::Tokenizer tokenizer;
  tokenizer.allowAsciiType(true);
  tokenizer.allowNewLineAsTokenDelimiter(true);
  tokenizer.addData(json_data_partial_8_1, sizeof(json_data_partial_8_1));
  tokenizer.addData(json_data_partial_8_2, sizeof(json_data_partial_8_2));

  STFY::Token token;
  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectStart);

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(assert_token(token, STFY::Type::String, "foo", STFY::Type::String, "bar") == 0);

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE((assert_token(token, STFY::Type::String, "array", STFY::Type::ArrayStart, "[") == 0));

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE((assert_token(token, STFY::Type::Ascii, "", STFY::Type::String, "one") == 0));

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE((assert_token(token, STFY::Type::Ascii, "", STFY::Type::String, "two") == 0));

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE((assert_token(token, STFY::Type::Ascii, "", STFY::Type::String, "three") == 0));

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE((assert_token(token, STFY::Type::Ascii, "", STFY::Type::ArrayEnd, "]") == 0));

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectEnd);

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NeedMoreData);
}

} // namespace json_tokenizer_partial_test
