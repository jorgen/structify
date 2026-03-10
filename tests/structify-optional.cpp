#include <structify/structify.h>
#ifdef STFY_STD_OPTIONAL
#include <optional>
#endif
#include "catch2/catch_all.hpp"

namespace
{
struct SmallStructWithoutOptional
{
  int a = 0;
  float b = 2.2f;
  std::string d;

  STFY_OBJ(a, b, d);
};

#ifdef STFY_STD_OPTIONAL
struct SmallStructStd
{
  int a = 0;
  std::optional<float> b = 2.2f;
  std::optional<std::string> c;
  std::optional<std::string> d;

  STFY_OBJ(a, b, c, d);
};
#endif

const char json[] = R"json(
{
  "a": 1,
  "b": 2.2,
  "c": "hello world"
}
)json";

TEST_CASE("test_optional", "[structify]")
{
  {
    STFY::ParseContext context(json);
    context.allow_unasigned_required_members = false;
    SmallStructWithoutOptional data;
    auto error = context.parseTo(data);
    REQUIRE(error != STFY::Error::NoError);
  }
#ifdef STFY_STD_OPTIONAL
  {
    STFY::ParseContext context(json);
    context.allow_unasigned_required_members = false;
    SmallStructStd data;
    auto error = context.parseTo(data);
    REQUIRE(error == STFY::Error::NoError);
    REQUIRE(context.error == STFY::Error::NoError);
    REQUIRE(data.a == 1);
    REQUIRE(data.b.value() > 2.199);
    REQUIRE(data.b.value() < 2.201);
    REQUIRE(data.c.value() == "hello world");
  }
#endif
}
} // namespace
