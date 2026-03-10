#include "catch2/catch_all.hpp"
#include <structify/structify.h>

#include <stdio.h>

namespace structify_serialize_tuple
{

struct Foo
{
  STFY::Tuple<int, std::string, float> data;
  STFY_OBJECT(STFY_MEMBER(data));
};

const char json[] = R"json(
{
  "data": [
    9876,
    "Tuples are cool",
    3.1415
  ]
}
)json";

TEST_CASE("serialize_tuple", "[structify][tuple]")
{

  Foo out;
  out.data.get<0>() = 12345;
  out.data.get<1>() = "Hello world";
  out.data.get<2>() = 44.50;
  std::string bar = STFY::serializeStruct(out);

  Foo in;
  STFY::ParseContext context(json);
  auto error = context.parseTo(in);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(in.data.get<0>() == 9876);
  REQUIRE(std::string("Tuples are cool") == in.data.get<1>());
  REQUIRE(in.data.get<2>() > 3.14);
  REQUIRE(in.data.get<2>() < 3.15);
}

struct TestInt {
  std::tuple<int32_t> member;
  STFY_OBJ(member);
};

struct TestBool {
  std::tuple<bool> member;
  STFY_OBJ(member);
};

TEST_CASE("bool_tuple", "[structify][tuple]")
{
  TestInt tiStruct;
  STFY::ParseContext intContext(R"({ "member": [5] })");
  REQUIRE(intContext.parseTo(tiStruct) == STFY::Error::NoError);

  TestBool tbStruct;
  STFY::ParseContext boolContext(R"({ "member": [true] })");
  REQUIRE(boolContext.parseTo(tbStruct) == STFY::Error::NoError);

  std::string serializedTbStruct = STFY::serializeStruct(tbStruct);
  REQUIRE(serializedTbStruct.size() != 0);
}

} // namespace structify_serialize_tuple
