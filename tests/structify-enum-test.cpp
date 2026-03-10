#include "catch2/catch_all.hpp"
#include <structify/structify.h>
#include <stdio.h>

STFY_ENUM(Colors, Red, Green, Blue, Yellow4, Purple)

namespace
{
struct TestEnumParser
{
  Colors colors;

  STFY_OBJECT(STFY_MEMBER(colors));
};
} // namespace

STFY_ENUM_DECLARE_STRING_PARSER(Colors)

namespace
{
const char json[] = R"json({
  "colors": "Green"
})json";

TEST_CASE("check_enum_parser", "[structify][enum]")
{
  STFY::ParseContext pc(json);
  TestEnumParser ep;
  auto error = pc.parseTo(ep);
  REQUIRE(error == STFY::Error::NoError);

  REQUIRE(ep.colors == Colors::Green);

  std::string jsonout = STFY::serializeStruct(ep);
  REQUIRE(jsonout == json);
}

const char json_number[] = R"json({
  "colors": 2
})json";

TEST_CASE("check_enum_number_parser", "[structify][enum]")
{
  STFY::ParseContext pc(json_number);
  TestEnumParser ep;
  auto error = pc.parseTo(ep);

  REQUIRE(error == STFY::Error::NoError);

  REQUIRE(ep.colors == Colors::Blue);
}

namespace FOO
{
namespace BAR
{
STFY_ENUM(Cars, Fiat, VW, BMW, Peugeot, Mazda)
}
} // namespace FOO
namespace One
{
namespace Two
{
struct CarContainer
{
  FOO::BAR::Cars car;

  STFY_OBJECT(STFY_MEMBER(car));
};
} // namespace Two
} // namespace One

} // namespace

STFY_ENUM_NAMESPACE_DECLARE_STRING_PARSER(FOO::BAR, Cars)

namespace
{
const char car_json[] = R"json({
  "car": "BMW"
})json";

TEST_CASE("check_enum_parser_namespace", "[structify][enum]")
{
  STFY::ParseContext pc(car_json);
  One::Two::CarContainer cc;
  auto error = pc.parseTo(cc);
  REQUIRE(error == STFY::Error::NoError);

  REQUIRE(cc.car == FOO::BAR::Cars::BMW);

  std::string jsonout = STFY::serializeStruct(cc);
  REQUIRE(jsonout == car_json);
}

} // namespace
