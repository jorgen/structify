/*
 * Copyright © 2017 Jorgen Lind
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

#include <cmrc/cmrc.hpp>

CMRC_DECLARE(external_json);

namespace
{

struct Simple
{
  std::string A;
  bool b;
  int some_longer_name;
  STFY_OBJECT(STFY_MEMBER(A), STFY_MEMBER(b), STFY_MEMBER(some_longer_name));
};

const char expected1[] = R"json({
  "A": "TestString",
  "b": false,
  "some_longer_name": 456
})json";

TEST_CASE("test_serialize_simple", "[structify][serialize]")
{
  Simple simple;
  simple.A = "TestString";
  simple.b = false;
  simple.some_longer_name = 456;

  std::string output = STFY::serializeStruct(simple);
  REQUIRE(output == expected1);
}

struct A
{
  int a;
  STFY_OBJECT(STFY_MEMBER(a));
};

struct B : public A
{
  float b;
  STFY_OBJECT_WITH_SUPER(STFY_SUPER_CLASSES(STFY_SUPER_CLASS(A)), STFY_MEMBER(b));
};

struct D
{
  int d;
  STFY_OBJECT(STFY_MEMBER(d));
};

struct E : public D
{
  double e;
  STFY_OBJECT_WITH_SUPER(STFY_SUPER_CLASSES(STFY_SUPER_CLASS(D)), STFY_MEMBER(e));
};

struct F : public E
{
  int f;
  STFY_OBJECT_WITH_SUPER(STFY_SUPER_CLASSES(STFY_SUPER_CLASS(E)), STFY_MEMBER(f));
};
struct G
{
  std::string g;
  STFY_OBJECT(STFY_MEMBER(g));
};

struct Subclass : public B, public F, public G
{
  int h;
  STFY_OBJECT_WITH_SUPER(STFY_SUPER_CLASSES(STFY_SUPER_CLASS(B), STFY_SUPER_CLASS(F), STFY_SUPER_CLASS(G)), STFY_MEMBER(h));
};

const char expected2[] = R"json({
  "h": 7,
  "g": "OutputString",
  "f": 6,
  "e": 5.5,
  "d": 5,
  "b": 4.5,
  "a": 4
})json";

TEST_CASE("test_serialize_deep", "[structify][serialize]")
{
  Subclass subclass;
  subclass.a = 4;
  subclass.b = 4.5;
  subclass.d = 5;
  subclass.e = 5.5;
  subclass.f = 6;
  subclass.g = "OutputString";
  subclass.h = 7;

  std::string output = STFY::serializeStruct(subclass);
  REQUIRE(output == expected2);
}

struct WithEscapedData
{
  std::string data;
  STFY_OBJECT(STFY_MEMBER(data));
};

const char escaped_expected[] = R"json({
  "data": "escaped \n \" \t string"
})json";

TEST_CASE("serialze_test_escaped_data", "[structify][serialize]")
{
  WithEscapedData escaped;
  escaped.data = "escaped \n \" \t string";
  std::string output = STFY::serializeStruct(escaped);
  REQUIRE(output == escaped_expected);
}

const char expected3[] = R"json({"h":7,"g":"OutputString","f":6,"e":5.5,"d":5,"b":4.5,"a":4})json";
TEST_CASE("serialize_test_compact", "[structify][serialize]")
{
  Subclass subclass;
  subclass.a = 4;
  subclass.b = 4.5;
  subclass.d = 5;
  subclass.e = 5.5;
  subclass.f = 6;
  subclass.g = "OutputString";
  subclass.h = 7;

  std::string output = STFY::serializeStruct(subclass, STFY::SerializerOptions(STFY::SerializerOptions::Compact));
  REQUIRE(output == expected3);
}

TEST_CASE("test_serialize_big", "[structify][serialize]")
{
  auto fs = cmrc::external_json::get_filesystem();
  auto generated = fs.open("generated.json");

  STFY::JsonObjectOrArrayRef objOrArr;
  {
    STFY::ParseContext pc(generated.begin(), generated.size());
    auto error = pc.parseTo(objOrArr);
    REQUIRE(error == STFY::Error::NoError);
  }

  std::string serialized_json = STFY::serializeStruct(objOrArr);

  {
    STFY::ParseContext pc(serialized_json.data(), serialized_json.size());
    auto error = pc.parseTo(objOrArr);
    REQUIRE(error == STFY::Error::NoError);
  }
}

const char empty_string_json[] = R"json({
  "key1": "value1",
  "key2": "",
  "key3": "value3"
})json";

struct empty_string_struct
{
  std::string key1;
  std::string key2;
  std::string key3;

  STFY_OBJ(key1, key2, key3);
};
TEST_CASE("test_serialize_empty_string", "[structify][serialize]")
{
  STFY::JsonTokens tokens;
  STFY::ParseContext context(empty_string_json);
  auto error = context.parseTo(tokens);
  REQUIRE(error == STFY::Error::NoError);

  empty_string_struct empty_struct;
  STFY::ParseContext context2(empty_string_json);
  error = context2.parseTo(empty_struct);
  REQUIRE(error == STFY::Error::NoError);

  std::string out = STFY::serializeStruct(tokens);

  std::string out2 = STFY::serializeStruct(empty_struct);
  REQUIRE(out == out2);
  REQUIRE(out == empty_string_json);
}

// ── Relaxed JSON serialization tests ───────────────────────────────

struct RelaxedSimple
{
  std::string name;
  int age;
  float score;
  STFY_OBJ(name, age, score);
};

struct RelaxedInner
{
  std::string city;
  int zip;
  STFY_OBJ(city, zip);
};

struct RelaxedNested
{
  std::string label;
  RelaxedInner address;
  STFY_OBJ(label, address);
};

struct RelaxedWithArray
{
  std::string tag;
  std::vector<int> values;
  STFY_OBJ(tag, values);
};

struct RelaxedWithBool
{
  bool active;
  std::string status;
  STFY_OBJ(active, status);
};

const char relaxed_simple_expected[] = R"json({
  name: "alice",
  age: 30,
  score: 9.5,
})json";

TEST_CASE("relaxed_serialize_simple", "[structify][serialize][relaxed]")
{
  RelaxedSimple s;
  s.name = "alice";
  s.age = 30;
  s.score = 9.5f;

  std::string output = STFY::serializeStruct(s, STFY::SerializerOptions(STFY::SerializerOptions::Relaxed));
  REQUIRE(output == relaxed_simple_expected);
}

TEST_CASE("relaxed_serialize_unquoted_keys", "[structify][serialize][relaxed]")
{
  RelaxedSimple s;
  s.name = "test";
  s.age = 1;
  s.score = 2.0f;

  std::string output = STFY::serializeStruct(s, STFY::SerializerOptions(STFY::SerializerOptions::Relaxed));

  // Keys should NOT be quoted
  REQUIRE(output.find("\"name\"") == std::string::npos);
  REQUIRE(output.find("\"age\"") == std::string::npos);
  REQUIRE(output.find("\"score\"") == std::string::npos);

  // Keys should appear unquoted with ": "
  REQUIRE(output.find("name: ") != std::string::npos);
  REQUIRE(output.find("age: ") != std::string::npos);
  REQUIRE(output.find("score: ") != std::string::npos);

  // String values should still be quoted
  REQUIRE(output.find("\"test\"") != std::string::npos);
}

TEST_CASE("relaxed_serialize_trailing_comma", "[structify][serialize][relaxed]")
{
  RelaxedSimple s;
  s.name = "bob";
  s.age = 25;
  s.score = 8.0f;

  std::string output = STFY::serializeStruct(s, STFY::SerializerOptions(STFY::SerializerOptions::Relaxed));

  // Last field before closing brace should have trailing comma
  auto last_comma = output.rfind(',');
  auto closing_brace = output.rfind('}');
  REQUIRE(last_comma != std::string::npos);
  REQUIRE(closing_brace != std::string::npos);
  REQUIRE(last_comma < closing_brace);

  // Between last comma and closing brace should be only whitespace
  bool only_whitespace = true;
  for (size_t i = last_comma + 1; i < closing_brace; i++)
  {
    if (output[i] != ' ' && output[i] != '\n' && output[i] != '\r' && output[i] != '\t')
    {
      only_whitespace = false;
      break;
    }
  }
  REQUIRE(only_whitespace);
}

const char relaxed_nested_expected[] = R"json({
  label: "home",
  address: {
    city: "NYC",
    zip: 10001,
  },
})json";

TEST_CASE("relaxed_serialize_nested", "[structify][serialize][relaxed]")
{
  RelaxedNested n;
  n.label = "home";
  n.address.city = "NYC";
  n.address.zip = 10001;

  std::string output = STFY::serializeStruct(n, STFY::SerializerOptions(STFY::SerializerOptions::Relaxed));
  REQUIRE(output == relaxed_nested_expected);
}

const char relaxed_array_expected[] = R"json({
  tag: "nums",
  values: [
    1,
    2,
    3,
  ],
})json";

TEST_CASE("relaxed_serialize_array", "[structify][serialize][relaxed]")
{
  RelaxedWithArray w;
  w.tag = "nums";
  w.values = {1, 2, 3};

  std::string output = STFY::serializeStruct(w, STFY::SerializerOptions(STFY::SerializerOptions::Relaxed));
  REQUIRE(output == relaxed_array_expected);
}

TEST_CASE("relaxed_serialize_empty_array", "[structify][serialize][relaxed]")
{
  RelaxedWithArray w;
  w.tag = "empty";
  w.values = {};

  SECTION("relaxed pretty - no trailing comma inside empty array")
  {
    std::string output = STFY::serializeStruct(w, STFY::SerializerOptions(STFY::SerializerOptions::Relaxed));
    REQUIRE(output.find("[,") == std::string::npos);
  }

  SECTION("relaxed compact - empty array stays compact")
  {
    std::string output = STFY::serializeStruct(w, STFY::SerializerOptions(STFY::SerializerOptions::RelaxedCompact));
    REQUIRE(output.find("[]") != std::string::npos);
    REQUIRE(output.find("[,]") == std::string::npos);
  }
}

TEST_CASE("relaxed_serialize_bool", "[structify][serialize][relaxed]")
{
  RelaxedWithBool b;
  b.active = true;
  b.status = "ok";

  std::string output = STFY::serializeStruct(b, STFY::SerializerOptions(STFY::SerializerOptions::Relaxed));

  REQUIRE(output.find("active: true") != std::string::npos);
  REQUIRE(output.find("status: \"ok\"") != std::string::npos);
}

const char relaxed_compact_expected[] = R"json({name:"alice",age:30,score:9.5,})json";

TEST_CASE("relaxed_compact_serialize", "[structify][serialize][relaxed]")
{
  RelaxedSimple s;
  s.name = "alice";
  s.age = 30;
  s.score = 9.5f;

  std::string output = STFY::serializeStruct(s, STFY::SerializerOptions(STFY::SerializerOptions::RelaxedCompact));
  REQUIRE(output == relaxed_compact_expected);
}

TEST_CASE("relaxed_compact_no_whitespace", "[structify][serialize][relaxed]")
{
  RelaxedSimple s;
  s.name = "x";
  s.age = 1;
  s.score = 2.0f;

  std::string output = STFY::serializeStruct(s, STFY::SerializerOptions(STFY::SerializerOptions::RelaxedCompact));

  // No spaces around colons
  REQUIRE(output.find(": ") == std::string::npos);
  // No newlines
  REQUIRE(output.find('\n') == std::string::npos);
}

TEST_CASE("relaxed_roundtrip", "[structify][serialize][relaxed]")
{
  RelaxedSimple original;
  original.name = "roundtrip";
  original.age = 42;
  original.score = 3.14f;

  std::string relaxed = STFY::serializeStruct(original, STFY::SerializerOptions(STFY::SerializerOptions::Relaxed));

  STFY::ParseContext ctx;
  ctx.tokenizer.allowAsciiType(true);
  ctx.tokenizer.allowSuperfluousComma(true);
  ctx.tokenizer.addData(relaxed.c_str(), relaxed.size());

  RelaxedSimple parsed;
  auto error = ctx.parseTo(parsed);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(parsed.name == original.name);
  REQUIRE(parsed.age == original.age);
  REQUIRE(parsed.score == original.score);
}

TEST_CASE("relaxed_compact_roundtrip", "[structify][serialize][relaxed]")
{
  RelaxedNested original;
  original.label = "test";
  original.address.city = "London";
  original.address.zip = 12345;

  std::string relaxed = STFY::serializeStruct(original, STFY::SerializerOptions(STFY::SerializerOptions::RelaxedCompact));

  STFY::ParseContext ctx;
  ctx.tokenizer.allowAsciiType(true);
  ctx.tokenizer.allowSuperfluousComma(true);
  ctx.tokenizer.addData(relaxed.c_str(), relaxed.size());

  RelaxedNested parsed;
  auto error = ctx.parseTo(parsed);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(parsed.label == original.label);
  REQUIRE(parsed.address.city == original.address.city);
  REQUIRE(parsed.address.zip == original.address.zip);
}

TEST_CASE("pretty_and_compact_unchanged", "[structify][serialize][relaxed]")
{
  RelaxedSimple s;
  s.name = "check";
  s.age = 99;
  s.score = 1.0f;

  SECTION("pretty still quotes keys")
  {
    std::string output = STFY::serializeStruct(s, STFY::SerializerOptions(STFY::SerializerOptions::Pretty));
    REQUIRE(output.find("\"name\"") != std::string::npos);
    REQUIRE(output.find("\"age\"") != std::string::npos);

    // No trailing comma — last field value is between last comma and closing brace
    auto last_comma = output.rfind(',');
    auto closing = output.rfind('}');
    REQUIRE(last_comma < closing);
    bool has_content = false;
    for (size_t i = last_comma + 1; i < closing; i++)
    {
      if (output[i] != ' ' && output[i] != '\n' && output[i] != '\r' && output[i] != '\t')
      {
        has_content = true;
        break;
      }
    }
    REQUIRE(has_content);
  }

  SECTION("compact still quotes keys")
  {
    std::string output = STFY::serializeStruct(s, STFY::SerializerOptions(STFY::SerializerOptions::Compact));
    REQUIRE(output.find("\"name\"") != std::string::npos);
    REQUIRE(output.find("\"age\"") != std::string::npos);
    // No trailing comma before }
    auto closing = output.rfind('}');
    REQUIRE(closing > 0);
    REQUIRE(output[closing - 1] != ',');
  }
}

} // namespace
