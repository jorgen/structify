#include "catch2/catch_all.hpp"
#include <structify/structify.h>

namespace
{
struct SmallStructWithoutNullable
{
  int a;
  float b;

  STFY_OBJECT(STFY_MEMBER(a), STFY_MEMBER(b));
};

struct SmallStruct
{
  int a = 0;
  STFY::Nullable<float> b = 2.2f;

  STFY_OBJECT(STFY_MEMBER(a), STFY_MEMBER(b));
};

struct SmallStructNullableChecked
{
  int a = 0;
  STFY::NullableChecked<float> b = 2.2f;

  STFY_OBJECT(STFY_MEMBER(a), STFY_MEMBER(b));
};

const char json[] = R"json(
{
  "a": 1,
  "b": null
}
)json";

TEST_CASE("test_nullable", "[structify]")
{
  {
    STFY::ParseContext context(json);
    SmallStructWithoutNullable data;
    auto error = context.parseTo(data);
    REQUIRE(error != STFY::Error::NoError);
  }
  {
    STFY::ParseContext context(json);
    SmallStruct data;
    auto error = context.parseTo(data);
    REQUIRE(error == STFY::Error::NoError);
    REQUIRE(data.a == 1);
    REQUIRE(data.b() > 2.199);
    REQUIRE(data.b() < 2.201);
  }
  {
    STFY::ParseContext context(json);
    SmallStructNullableChecked data;
    auto error = context.parseTo(data);
    REQUIRE(error == STFY::Error::NoError);
    REQUIRE(data.a == 1);
    REQUIRE(data.b.null);
    REQUIRE(data.b() > 2.199);
    REQUIRE(data.b() < 2.201);
  }
}
} // namespace
