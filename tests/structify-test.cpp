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
#include <unordered_map>

namespace structify_test
{

static const char json_data1[] = "{\n"
                                 "\"StringNode\": \"Some test data\",\n"
                                 "\"NumberNode\": 4676.4,\n"
                                 "\"BooleanTrue\": true,\n"
                                 "\"BooleanFalse\": false,\n"
                                 "\"TestStruct\": {\n"
                                 "\"SubString\": \"Some other string\",\n"
                                 "\"SubNumber\": 500,\n"
                                 "\"Array\": [\n"
                                 "5,\n"
                                 "6,\n"
                                 "3,\n"
                                 "6\n"
                                 "],\n"
                                 "\"optional_float\": 300,\n"
                                 "\"this_property_does_not_exist\": true\n"
                                 "},\n"
                                 "\"OptionalButWithData\": [ 17.5 ],\n"
                                 "\"subStruct2\": {\n"
                                 "\"Field1\": 4,\n"
                                 "\"Field2\": true\n"
                                 "},\n"
                                 "\"Skipped_sub_object\": {\n"
                                 "\"Field3\": 465\n"
                                 "}\n"
                                 "}\n";

struct SubStruct
{
  std::string SubString;
  int SubNumber = 0;
  std::vector<double> Array;
  STFY::OptionalChecked<float> optional_float;
  STFY::OptionalChecked<double> optional_double;
  STFY::Optional<double> optional_with_value = 4.5;
  STFY_OBJECT(STFY_MEMBER(SubString), STFY_MEMBER(SubNumber), STFY_MEMBER(Array), STFY_MEMBER(optional_float),
            STFY_MEMBER(optional_with_value));
};

static const char sub_struct3_data[] = "{\n"
                                       "\"Field1\": 4,\n"
                                       "\"Field2\": true,\n"
                                       "\"Field3\": \"432\"\n"
                                       "}\n";

struct SubStruct2
{
  float Field1;
  bool Field2;
  STFY_OBJECT(STFY_MEMBER(Field1), STFY_MEMBER_ALIASES(Field2, "hello", "Foobar"));
};

struct SubStruct3 : public SubStruct2
{
  std::string Field3;
  int Field4;
  STFY::Optional<std::string> Field5;
  STFY_OBJECT_WITH_SUPER(STFY_SUPER_CLASSES(STFY_SUPER_CLASS(SubStruct2)), STFY_MEMBER(Field3), STFY_MEMBER(Field4),
                       STFY_MEMBER(Field5));
};

struct JsonData1
{
  std::string StringNode;
  double NumberNode = 0.0;
  bool BooleanTrue = false;
  /*!
   *very special comment for BooleanFalse
   *
   *\json
   *{
   *   json
   *}
   **/
  bool BooleanFalse = false;
  STFY::Optional<int> OptionalInt;
  /// Test structur comment
  SubStruct TestStruct;
  STFY::Optional<std::vector<double>> OptionalButWithData;
  float unassigned_value = 32.f;
  std::unique_ptr<SubStruct2> subStruct2;

  int Field3 = 243;
  std::string NodeWithLiteral = "SOME STRING LITERAL!!!";

  STFY_OBJECT(STFY_MEMBER(StringNode), STFY_MEMBER(NumberNode), STFY_MEMBER(BooleanTrue), STFY_MEMBER(BooleanFalse),
            STFY_MEMBER(OptionalInt), STFY_MEMBER(TestStruct), STFY_MEMBER(OptionalButWithData), STFY_MEMBER(unassigned_value),
            STFY_MEMBER(subStruct2), STFY_MEMBER(Field3), STFY_MEMBER(NodeWithLiteral));
};

TEST_CASE("check_json_tree_nodes_structify")
{
  STFY::ParseContext context(json_data1);
  JsonData1 data;
  auto error = context.parseTo(data);

  data.TestStruct.optional_with_value = 5;
  REQUIRE(data.StringNode == "Some test data");
  if (error != STFY::Error::NoError)
  {
    auto str = context.makeErrorString();
    fprintf(stderr, "Error: %s\n", str.c_str());
  }
  REQUIRE(error == STFY::Error::NoError);

  REQUIRE(data.Field3 == 243);

  std::string json = STFY::serializeStruct(data);
}

static const char json_data2[] = "{\n"
                                 "\"some_int\": 4,\n"
                                 "\"sub_object\": {\n"
                                 "\"more_data\": \"some text\",\n"
                                 "\"a_float\": 1.2,\n"
                                 "\"boolean_member\": false\n"
                                 "}\n"
                                 "}\n";

template <typename T>
struct OuterStruct
{
  int some_int;
  T sub_object;

  STFY_OBJECT(STFY_MEMBER(some_int), STFY_MEMBER(sub_object));
};

struct SubObject
{
  std::string more_data;
  float a_float;
  bool boolean_member;

  STFY_OBJECT(STFY_MEMBER(more_data), STFY_MEMBER(a_float), STFY_MEMBER(boolean_member));
};

TEST_CASE("check_json_tree_template", "structify")
{
  STFY::ParseContext context(json_data2);
  OuterStruct<SubObject> data;
  auto error = context.parseTo(data);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(data.sub_object.more_data == "some text");
  std::string json = STFY::serializeStruct(data);
}

TEST_CASE("check_json_tree_subclass", "structify")
{
  STFY::ParseContext context(sub_struct3_data);
  SubStruct3 substruct3;
  auto error = context.parseTo(substruct3);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(substruct3.Field3 == std::string("432"));
}

static const char json_data3[] = "{\n"
                                 "\"SuperSuper\": 5,\n"
                                 "\"Regular\": 42,\n"
                                 "\"Super\": \"This is in the Superclass\"\n"
                                 "}\n";

struct SuperSuperClass
{
  int SuperSuper;
  STFY_OBJECT(STFY_MEMBER(SuperSuper));
};

struct SuperClass : public SuperSuperClass
{
  std::string Super;
  STFY_OBJECT_WITH_SUPER(STFY_SUPER_CLASSES(STFY_SUPER_CLASS(SuperSuperClass)), STFY_MEMBER(Super));
};

struct RegularClass : public SuperClass
{
  int Regular;
  STFY_OBJECT_WITH_SUPER(STFY_SUPER_CLASSES(STFY_SUPER_CLASS(SuperClass)), STFY_MEMBER(Regular));
};

TEST_CASE("check_json_tree_deep_tree", "structify")
{
  STFY::ParseContext context(json_data3);
  RegularClass regular;
  auto error = context.parseTo(regular);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(regular.SuperSuper == 5);
  REQUIRE(regular.Super == "This is in the Superclass");
  REQUIRE(regular.Regular == 42);
}

static const char missing_object_def[] = R"json({
  "first": true,
  "second": "hello world",
    "third": {},
    "fourth": 33
})json";

struct MissingObjectDef
{
  bool first;
  std::string second;
  int fourth;

  STFY_OBJECT(STFY_MEMBER(first), STFY_MEMBER(second), STFY_MEMBER(fourth));
};

TEST_CASE("check_json_missing_object", "structify")
{
  STFY::ParseContext context(missing_object_def);
  MissingObjectDef missing;
  auto error = context.parseTo(missing);
  REQUIRE(error == STFY::Error::NoError);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(missing.fourth == 33);
}

static const char error_in_sub[] = R"json({
  "first": {
    "ffirst": 4,
    "fsecond": {},
    "not_assigned": 555
  },
  "second": "hello world",
  "third": 33
})json";

struct ErrorInSubChild
{
  int ffirst = 0;
  STFY_OBJECT(STFY_MEMBER(ffirst));
};

struct ErrorInSub
{
  ErrorInSubChild first;
  std::string second;
  int third = 0;
  STFY::Optional<int> not_assigned = 999;
  STFY_OBJECT(STFY_MEMBER(first), STFY_MEMBER(second), STFY_MEMBER(third));
};

TEST_CASE("check_json_error_in_sub", "structify")
{
  STFY::ParseContext context(error_in_sub);
  ErrorInSub sub;
  auto error = context.parseTo(sub);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(sub.second == "hello world");
  REQUIRE(sub.third == 33);
  REQUIRE(sub.not_assigned.data == 999);
}

struct JsonObjectTester
{
  std::string field;
  STFY::JsonObject obj;
  int number = 0;

  STFY_OBJECT(STFY_MEMBER(field), STFY_MEMBER(obj), STFY_MEMBER(number));
};

struct JsonObjectOrArrayObjectTester
{
  std::string field;
  STFY::JsonObjectOrArray obj;
  int number = 0;

  STFY_OBJECT(STFY_MEMBER(field), STFY_MEMBER(obj), STFY_MEMBER(number));
};

struct JsonObjectRefTester
{
  std::string field;
  STFY::JsonObjectRef obj;
  int number = 0;

  STFY_OBJECT(STFY_MEMBER(field), STFY_MEMBER(obj), STFY_MEMBER(number));
};

struct JsonObjectOrArrayObjectRefTester
{
  std::string field;
  STFY::JsonObjectOrArrayRef obj;
  int number = 0;

  STFY_OBJECT(STFY_MEMBER(field), STFY_MEMBER(obj), STFY_MEMBER(number));
};

static const char jsonObjectTest[] = R"json({
  "field": "hello",
  "obj": {
    "some_sub_filed": 2,
    "some_sub_array": [ "a", "b", "c"],
    "some_sub_object": { "field": "not hello" }
  },
  "number": 43
})json";

TEST_CASE("check_json_object", "structify")
{
  STFY::ParseContext context(jsonObjectTest);
  JsonObjectTester obj;
  auto error = context.parseTo(obj);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(obj.field == "hello");
  REQUIRE(obj.obj.data.size() > 0);
  REQUIRE(obj.number == 43);

  std::string out = STFY::serializeStruct(obj);
  REQUIRE(out == jsonObjectTest);
}

TEST_CASE("check_json_object_or_array_object", "structify")
{
  STFY::ParseContext context(jsonObjectTest);
  JsonObjectOrArrayObjectTester obj;
  auto error = context.parseTo(obj);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(obj.field == "hello");
  REQUIRE(obj.obj.data.size() > 0);
  REQUIRE(obj.number == 43);

  std::string out = STFY::serializeStruct(obj);
  REQUIRE(out == jsonObjectTest);
}

TEST_CASE("check_json_object_ref", "structify")
{
  STFY::ParseContext context(jsonObjectTest);
  JsonObjectRefTester obj;
  auto error = context.parseTo(obj);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(obj.field == "hello");
  REQUIRE(obj.obj.ref.size > 0);
  REQUIRE(obj.number == 43);

  std::string out = STFY::serializeStruct(obj);
  REQUIRE(out == jsonObjectTest);
}

TEST_CASE("check_json_object_or_array_object_ref", "structify")
{
  STFY::ParseContext context(jsonObjectTest);
  JsonObjectOrArrayObjectRefTester obj;
  auto error = context.parseTo(obj);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(obj.field == "hello");
  REQUIRE(obj.obj.ref.size > 0);
  REQUIRE(obj.number == 43);

  std::string out = STFY::serializeStruct(obj);
  REQUIRE(out == jsonObjectTest);
}

struct JsonArrayTester
{
  std::string string;
  STFY::JsonArray array;
  int number = 0;

  STFY_OBJECT(STFY_MEMBER(string), STFY_MEMBER(array), STFY_MEMBER(number));
};

struct JsonObjectOrArrayArrayTester
{
  std::string string;
  STFY::JsonObjectOrArray array;
  int number = 0;

  STFY_OBJECT(STFY_MEMBER(string), STFY_MEMBER(array), STFY_MEMBER(number));
};

struct JsonArrayRefTester
{
  std::string string;
  STFY::JsonArrayRef array;
  int number = 0;

  STFY_OBJECT(STFY_MEMBER(string), STFY_MEMBER(array), STFY_MEMBER(number));
};

struct JsonObjectOrArrayArrayRefTester
{
  std::string string;
  STFY::JsonObjectOrArrayRef array;
  int number = 0;

  STFY_OBJECT(STFY_MEMBER(string), STFY_MEMBER(array), STFY_MEMBER(number));
};

static const char jsonArrayTest[] = R"json({
  "string": "foo",
  "array": [
    ["a","b","c"],
    {
      "sub object": 44.50
    },
    12345
  ],
  "number": 43
})json";

TEST_CASE("check_json_array", "structify")
{
  STFY::ParseContext context(jsonArrayTest);
  JsonArrayTester obj;
  auto error = context.parseTo(obj);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(obj.string == "foo");
  REQUIRE(obj.array.data.size() > 0);
  REQUIRE(obj.number == 43);

  std::string out = STFY::serializeStruct(obj);
  REQUIRE(out == jsonArrayTest);
}

TEST_CASE("check_json_object_or_array_array", "structify")
{
  STFY::ParseContext context(jsonArrayTest);
  JsonObjectOrArrayArrayTester obj;
  auto error = context.parseTo(obj);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(obj.string == "foo");
  REQUIRE(obj.array.data.size() > 0);
  REQUIRE(obj.number == 43);

  std::string out = STFY::serializeStruct(obj);
  REQUIRE(out == jsonArrayTest);
}

TEST_CASE("check_json_array_ref", "structify")
{
  STFY::ParseContext context(jsonArrayTest);
  JsonArrayRefTester obj;
  auto error = context.parseTo(obj);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(obj.string == "foo");
  REQUIRE(obj.array.ref.size > 0);
  REQUIRE(obj.number == 43);

  std::string out = STFY::serializeStruct(obj);
  REQUIRE(out == jsonArrayTest);
}

TEST_CASE("check_json_object_or_array_array_ref", "structify")
{
  STFY::ParseContext context(jsonArrayTest);
  JsonObjectOrArrayArrayRefTester obj;
  auto error = context.parseTo(obj);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(obj.string == "foo");
  REQUIRE(obj.array.ref.size > 0);
  REQUIRE(obj.number == 43);

  std::string out = STFY::serializeStruct(obj);
  REQUIRE(out == jsonArrayTest);
}

struct JsonMapTest
{
  std::unordered_map<std::string, STFY::JsonTokens> map;

  STFY_OBJECT(STFY_MEMBER(map));
};

TEST_CASE("check_json_map", "structify")
{
#ifdef STFY_UNORDERED_MAP_HANDLER
static const char jsonMapTest[] = R"json({
  "map": {
    "hello": { "some object": 3 },
    "bye": [4]
  }
})json";

  STFY::ParseContext context(jsonMapTest);
  JsonMapTest obj;
  context.parseTo(obj);
  REQUIRE(context.error == STFY::Error::NoError);
#endif
}

struct TypeHandlerTypes
{
  double doubleN;
  float floatN;
  int intN;
  unsigned int uintN;
  int64_t int64N;
  uint64_t uint64N;
  int16_t int16N;
  uint16_t uint16N;
  uint8_t uint8N;
  int8_t int8N;
  char charN;
  signed char scharN;   // Normally same as int8_t
  unsigned char ucharN; // Normally same as uint8_t
  bool boolN;

  STFY_OBJECT(STFY_MEMBER(doubleN), STFY_MEMBER(floatN), STFY_MEMBER(intN), STFY_MEMBER(uintN), STFY_MEMBER(int64N),
            STFY_MEMBER(uint64N), STFY_MEMBER(int16N), STFY_MEMBER(uint16N), STFY_MEMBER(uint8N), STFY_MEMBER(int8N),
            STFY_MEMBER(charN), STFY_MEMBER(scharN), STFY_MEMBER(ucharN), STFY_MEMBER(boolN));
};

static const char jsonTypeHandlerTypes[] = R"json({
  "doubleN": 44.50,
  "floatN": 33.40,
  "intN": -345,
  "uintN": 567,
  "int64N": -1234,
  "uint64N": 987,
  "int16N": -23,
  "uint16N": 45,
  "uint8N": 255,
  "int8N": -127,
  "charN": 123,
  "scharN": -123,
  "ucharN": 234,
  "boolN": true
})json";

TEST_CASE("check_json_type_handler_types", "structify")
{
  STFY::ParseContext context(jsonTypeHandlerTypes);
  TypeHandlerTypes obj;
  auto error = context.parseTo(obj);
  REQUIRE(error == STFY::Error::NoError);
}

struct TypeHandlerIntTypes
{
  int intN;
  unsigned int uintN;
  int64_t int64N;
  uint64_t uint64N;
  int16_t int16N;
  uint16_t uint16N;
  uint8_t uint8N;
  int8_t int8N;
  char charN;
  signed char scharN;   // Normally same as int8_t
  unsigned char ucharN; // Normally same as uint8_t
  bool boolN;

  STFY_OBJECT(STFY_MEMBER(intN), STFY_MEMBER(uintN), STFY_MEMBER(int64N), STFY_MEMBER(uint64N), STFY_MEMBER(int16N),
            STFY_MEMBER(uint16N), STFY_MEMBER(uint8N), STFY_MEMBER(int8N), STFY_MEMBER(charN), STFY_MEMBER(scharN),
            STFY_MEMBER(ucharN), STFY_MEMBER(boolN));
};

static const char jsonTypeHandlerIntTypes[] = R"json({
  "intN": -345,
  "uintN": 567,
  "int64N": -1234,
  "uint64N": 987,
  "int16N": -23,
  "uint16N": 45,
  "uint8N": 255,
  "int8N": -127,
  "charN": 123,
  "scharN": -123,
  "ucharN": 234,
  "boolN": true
})json";

TEST_CASE("check_json_type_handler_integer_types", "structify")
{
  STFY::ParseContext context(jsonTypeHandlerIntTypes);
  TypeHandlerIntTypes obj;
  auto error = context.parseTo(obj);
  REQUIRE(error == STFY::Error::NoError);

  std::string serialized = STFY::serializeStruct(obj);

  STFY::ParseContext context2(serialized);
  TypeHandlerIntTypes obj2;
  error = context2.parseTo(obj2);
  REQUIRE(error == STFY::Error::NoError);

  REQUIRE(obj.intN == -345);
  REQUIRE(obj.uintN == 567);
  REQUIRE(obj.int64N == -1234);
  REQUIRE(obj.uint64N == 987);
  REQUIRE(obj.int16N == -23);
  REQUIRE(obj.uint16N == 45);
  REQUIRE(obj.uint8N == 255);
  REQUIRE(obj.int8N == -127);
  REQUIRE(obj.charN == 123);
  REQUIRE(obj.scharN == -123);
  REQUIRE(obj.ucharN == 234);
  REQUIRE(obj.boolN == true);

  REQUIRE(obj.intN == obj2.intN);
  REQUIRE(obj.uintN == obj2.uintN);
  REQUIRE(obj.int64N == obj2.int64N);
  REQUIRE(obj.uint64N == obj2.uint64N);
  REQUIRE(obj.int16N == obj2.int16N);
  REQUIRE(obj.int16N == obj2.int16N);
  REQUIRE(obj.uint16N == obj2.uint16N);
  REQUIRE(obj.uint8N == obj2.uint8N);
  REQUIRE(obj.int8N == obj2.int8N);
  REQUIRE(obj.charN == obj2.charN);
  REQUIRE(obj.scharN == obj2.scharN);
  REQUIRE(obj.ucharN == obj2.ucharN);
  REQUIRE(obj.boolN == obj2.boolN);
}

struct ArrayTest
{
  double data[3];

  STFY_OBJECT(STFY_MEMBER(data));
};

static const char arrayTestJson[] = R"json({
  "data": [4, 5, 6]
})json";

TEST_CASE("check_json_array_test", "structify")
{
  STFY::ParseContext context(arrayTestJson);
  ArrayTest obj;
  auto error = context.parseTo(obj);
  REQUIRE(error == STFY::Error::NoError);
}

struct SkipTestBase
{
  std::string name;
  int id;

  STFY_OBJECT(STFY_MEMBER(name), STFY_MEMBER(id));
};

struct SkipTestInternalContainer
{
  std::vector<float> items;

  STFY_OBJECT(STFY_MEMBER(items));
};

struct SkipTestNameContainer
{
  std::string name;
  int id;
  SkipTestInternalContainer container;

  STFY_OBJECT(STFY_MEMBER(name), STFY_MEMBER(id), STFY_MEMBER(container));
};

struct SkipTestSubClass : public SkipTestBase
{
  float value;
  std::vector<SkipTestNameContainer> skip_test_list_01;
  std::vector<SkipTestNameContainer> skip_test_list_02;

  STFY_OBJECT_WITH_SUPER(STFY_SUPER_CLASSES(STFY_SUPER_CLASS(SkipTestBase)), STFY_MEMBER(value), STFY_MEMBER(skip_test_list_01),
                       STFY_MEMBER(skip_test_list_02));
};

static const char jsonSkipTest[] = R"json(
{
  "skip_test_list_01": [
    {
      "id": 1,
      "container": {
        "items": []
      },
      "skip_me": [],
      "name": "list01"
    },
    {
      "name": "list02",
      "skip_me": [],
      "container": {
        "items": [1.1, 2.2, 3.3]
      },
      "id": 2
    },
    {
      "skip_me": [],
      "name": "list03",
      "id": 3,
      "container": {
        "items": [0, 1, 2]
      }
    }
  ],
  "skip_test_list_02": [
    {
      "name": "list01",
      "id": 1,
      "container": {
        "items": []
      },
      "skip_me": []
    },
    {
      "name": "list02",
      "skip_me": [],
      "container": {
        "items": []
      },
      "id": 2
    },
    {
      "container": {
        "items": []
      },
      "skip_me": [],
      "name": "list03",
      "id": 3
    }
  ],
  "value": 3.14,
  "name": "base_name",
  "id": 444
}
)json";

TEST_CASE("check_json_skip_test", "structify")
{
  STFY::ParseContext base_context(jsonSkipTest);
  SkipTestBase base;
  auto error = base_context.parseTo(base);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(base.name == "base_name");
  REQUIRE(base.id == 444);

  STFY::ParseContext sub_context(jsonSkipTest);
  SkipTestSubClass sub;
  error = sub_context.parseTo(sub);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(sub.skip_test_list_01[2].name == "list03");
  REQUIRE(sub.skip_test_list_02[1].name == "list02");
  REQUIRE(sub.name == "base_name");
  REQUIRE(sub.id == 444);
}

static const char multi_top_level_json[] = R"json({ a: 1}{a: 2}{a:3})json";
struct MultiTopLevel
{
  int a;
  STFY_OBJECT(STFY_MEMBER(a));
};

TEST_CASE("check_multi_top_level_json", "structify")
{
  STFY::ParseContext pc(multi_top_level_json);
  pc.tokenizer.allowAsciiType(true);
  MultiTopLevel t;
  for (int i = 0; i < 3; i++)
  {
    REQUIRE(pc.tokenizer.currentPosition() < multi_top_level_json + sizeof(multi_top_level_json) - 1);
    auto error = pc.parseTo(t);
    REQUIRE(error == STFY::Error::NoError);
    REQUIRE(t.a == i + 1);
  }
  REQUIRE(pc.tokenizer.currentPosition() == multi_top_level_json + sizeof(multi_top_level_json) - 1);
}

static const char escapedJson[] = R"json({
  "some_text": "more\"_te\\xt",
  "sub_object": {
    "more_data": "so\\me \"text",
    "a_float": 1.2,
    "boolean_member": false
  }
})json";

template <typename T>
struct EscapedOuterStruct
{
  std::string some_text;
  T sub_object;

  STFY_OBJECT(STFY_MEMBER(some_text), STFY_MEMBER(sub_object));
};

struct EscapedSubObject
{
  std::string more_data;
  float a_float;
  bool boolean_member;

  STFY_OBJECT(STFY_MEMBER(more_data), STFY_MEMBER(a_float), STFY_MEMBER(boolean_member));
};

TEST_CASE("check_json_escaped", "structify")
{
  STFY::ParseContext context(escapedJson);
  EscapedOuterStruct<EscapedSubObject> data;
  auto error = context.parseTo(data);
  REQUIRE(error == STFY::Error::NoError);
  std::string equals("more\"_te\\xt");
  REQUIRE(data.some_text == equals);
  std::string json = STFY::serializeStruct(data);
}

struct OutsideMeta
{
  std::string data;
  float a;
};

} // namespace structify_test

STFY_OBJECT_EXTERNAL(structify_test::OutsideMeta, STFY_MEMBER(data), STFY_MEMBER(a))

namespace structify_test
{

static const char outside_json[] = R"json({
  "data": "this is some text",
  "a": 44.5
})json";

TEST_CASE("check_json_meta_outside", "structify")
{
  STFY::ParseContext context(outside_json);
  OutsideMeta data;
  auto error = context.parseTo(data);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(data.data == "this is some text");
  REQUIRE(data.a == 44.5);
}

struct FloatingPointAtTheEnd
{
  bool enabled;
  float value;

  STFY_OBJECT(STFY_MEMBER(enabled), STFY_MEMBER(value));
};

static const char short_floating_at_the_end[] = R"json({
  "enabled": true,
  "value": 0.5
})json";

TEST_CASE("check_short_floating_point", "structify")
{
  STFY::ParseContext context(short_floating_at_the_end);
  FloatingPointAtTheEnd data;
  auto error = context.parseTo(data);
  REQUIRE(error == STFY::Error::NoError);
}

struct InvalidFloating
{
  bool enabled;
  float value;
  STFY_OBJECT(STFY_MEMBER(enabled), STFY_MEMBER(value));
};

static const char invalid_floating[] = R"json({
  "enabled": true,
  "value": 0.5.e..-.E.5
})json";

TEST_CASE("check_invalid_floating_point", "structify")
{
  STFY::ParseContext context(invalid_floating);
  FloatingPointAtTheEnd data;
  auto error = context.parseTo(data);
  REQUIRE(error == STFY::Error::FailedToParseFloat);
}

static const char moreEscapedJsonAtEnd[] = R"json({
  "some_text": "more\n",
  "some_other": "tests\"",
  "pure_escape": "\n",
  "strange_escape": "foo\s",
  "pure_strange_escape": "\k",
  "empty_string": ""
  }
})json";

struct MoreEscapedStruct
{
  std::string some_text;
  std::string some_other;
  std::string pure_escape;
  std::string strange_escape;
  std::string pure_strange_escape;
  std::string empty_string;
  STFY_OBJECT(STFY_MEMBER(some_text), STFY_MEMBER(some_other), STFY_MEMBER(pure_escape), STFY_MEMBER(strange_escape),
            STFY_MEMBER(pure_strange_escape), STFY_MEMBER(empty_string));
};

TEST_CASE("check_json_escaped_end", "structify")
{
  STFY::ParseContext context(moreEscapedJsonAtEnd);
  MoreEscapedStruct data;
  auto error = context.parseTo(data);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(data.some_text == std::string("more\n"));
  REQUIRE(data.some_other == std::string("tests\""));
  REQUIRE(data.pure_escape == std::string("\n"));
  REQUIRE(data.strange_escape == std::string("foo\\s"));
  REQUIRE(data.pure_strange_escape == std::string("\\k"));
  std::string json = STFY::serializeStruct(data);
}

} // namespace structify_test
