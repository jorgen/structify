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

} // namespace
