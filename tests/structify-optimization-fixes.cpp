/*
 * Regression tests for correctness bugs ported from json_struct's optimization
 * / bug hunt. Structify's tokenizer uses the batched nextTokens() API.
 */

#include <structify/structify.h>

#include "catch2/catch_all.hpp"

#include <cmath>
#include <limits>

namespace
{

// Pull one token at a time through the batched API.
static STFY::Error nextToken(STFY::Tokenizer &tok, STFY::Token &token)
{
  return tok.nextTokens(&token, 1).error;
}

struct IntBox
{
  int x = -1;
  STFY_OBJ(x);
};

TEST_CASE("integer_requires_full_token_consumption", "[structify][number]")
{
  {
    STFY::ParseContext c("{\"x\": 100}");
    IntBox b;
    REQUIRE(c.parseTo(b) == STFY::Error::NoError);
    REQUIRE(b.x == 100);
  }
  {
    STFY::ParseContext c("{\"x\": 1e2}");
    IntBox b;
    REQUIRE(c.parseTo(b) == STFY::Error::NoError);
    REQUIRE(b.x == 100);
  }
  // Malformed number-class tokens (still a single Number token to the tokenizer)
  // must now be rejected instead of silently truncated.
  {
    STFY::ParseContext c("{\"x\": 1e2e3}");
    IntBox b;
    REQUIRE(c.parseTo(b) != STFY::Error::NoError);
  }
  {
    STFY::ParseContext c("{\"x\": 12-3}");
    IntBox b;
    REQUIRE(c.parseTo(b) != STFY::Error::NoError);
  }
}

struct DoubleBox
{
  double d = 0.0;
  STFY_OBJ(d);
};

struct FloatBox
{
  float f = 0.0f;
  STFY_OBJ(f);
};

TEST_CASE("nonfinite_serializes_as_valid_json", "[structify][number]")
{
  {
    DoubleBox b;
    b.d = std::numeric_limits<double>::quiet_NaN();
    std::string out = STFY::serializeStruct(b);
    REQUIRE(out.find("nan") == std::string::npos);
    REQUIRE(out.find("null") != std::string::npos);
    // Output must be structurally valid JSON (previously the bare token "nan").
    STFY::Tokenizer tok;
    tok.addData(out.c_str(), out.size());
    STFY::Token token;
    STFY::Error e = STFY::Error::NoError;
    do
    {
      e = nextToken(tok, token);
    } while (e == STFY::Error::NoError && token.value_type != STFY::Type::ObjectEnd);
    REQUIRE(e == STFY::Error::NoError);
    REQUIRE(token.value_type == STFY::Type::ObjectEnd);
  }
  {
    DoubleBox b;
    b.d = std::numeric_limits<double>::infinity();
    std::string out = STFY::serializeStruct(b);
    REQUIRE(out.find("inf") == std::string::npos);
    REQUIRE(out.find("null") != std::string::npos);
  }
  {
    FloatBox b;
    b.f = -std::numeric_limits<float>::infinity();
    std::string out = STFY::serializeStruct(b);
    REQUIRE(out.find("inf") == std::string::npos);
    REQUIRE(out.find("null") != std::string::npos);
  }
}

TEST_CASE("newline_delimiter_simd_gate", "[tokenizer]")
{
  // Newline-delimited array (no commas), long enough that the >=16/32-byte SIMD
  // whitespace-skip in findTokenEnd triggers. Before the gate fix the SIMD path
  // consumed the newline delimiter and findTokenEnd returned InvalidToken.
  static const char data[] = "[\n"
                             "  \"aaaaaaaaaaaaaaaa\"\n"
                             "  \"bbbbbbbbbbbbbbbb\"\n"
                             "  \"cccccccccccccccc\"\n"
                             "  \"dddddddddddddddd\"\n"
                             "]";
  STFY::Tokenizer tok;
  tok.allowNewLineAsTokenDelimiter(true);
  tok.addData(data, sizeof(data) - 1);

  STFY::Token token;
  STFY::Error e = STFY::Error::NoError;
  int strings = 0;
  do
  {
    e = nextToken(tok, token);
    if (e == STFY::Error::NoError && token.value_type == STFY::Type::String)
      strings++;
  } while (e == STFY::Error::NoError && token.value_type != STFY::Type::ArrayEnd);

  REQUIRE(e == STFY::Error::NoError);
  REQUIRE(strings == 4);
}

// NOTE: json_struct's is_escaped streaming regression test is intentionally NOT
// ported. It required a string to span two separately-added buffers, but
// structify's tokenizer refactor removed multi-buffer streaming (no
// need_more_data_callback / data_list); every string is fully contained in one
// buffer, so is_escaped is always false at string start and the bug cannot be
// triggered here. The snapshot/restore fix in findStringEnd is still applied as a
// defensive measure that preserves parity with json_struct.

struct NestChild
{
  int a = 0;
  STFY_OBJ(a);
};

struct NestParent
{
  int collide = 0;
  NestChild child;
  STFY_OBJ(collide, child);
};

TEST_CASE("nested_unknown_field_strict_stops_scanning", "[structify][error]")
{
  // The child object contains a field ("collide") unknown to NestChild but
  // colliding with a NestParent member name. In strict mode this previously made
  // the parent resume scanning its own members against tokens inside the
  // half-parsed child, silently assigning collide=99 and returning NoError.
  static const char json_data[] = R"json({"child":{"a":1,"collide":99},"collide":7})json";

  STFY::ParseContext context(json_data);
  context.allow_missing_members = false;
  NestParent p;
  auto error = context.parseTo(p);

  REQUIRE(error != STFY::Error::NoError);
  REQUIRE(p.collide != 99);
}

TEST_CASE("nested_unknown_field_allowed_by_default", "[structify][error]")
{
  static const char json_data[] = R"json({"child":{"a":1,"extra":99},"collide":7})json";

  STFY::ParseContext context(json_data);
  NestParent p;
  auto error = context.parseTo(p);

  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(p.collide == 7);
  REQUIRE(p.child.a == 1);
}

struct MultiInt
{
  int a = 0;
  int b = 0;
  int c = 0;
  STFY_OBJ(a, b, c);
};

TEST_CASE("pretty_skip_delimiter_omits_comma", "[serializer]")
{
  MultiInt m;
  m.a = 1;
  m.b = 2;
  m.c = 3;
  STFY::SerializerOptions opts(STFY::SerializerOptions::Pretty);
  opts.skipDelimiter(true);
  std::string out = STFY::serializeStruct(m, opts);
  REQUIRE(out.find(',') == std::string::npos);
}

} // namespace
