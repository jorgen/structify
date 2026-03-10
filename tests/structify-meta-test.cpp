#include "catch2/catch_all.hpp"
#include <structify/structify.h>

namespace
{
const char first_child_with_data_json[] = R"json([ [], [],  [
[],
[],
{
  "this has a member": true
},
[]
], [], []])json";

TEST_CASE("find_first_child_with_data", "[structify][meta]")
{
  STFY::ParseContext pc(first_child_with_data_json);
  STFY::JsonTokens tokens;
  auto error = pc.parseTo(tokens);
  REQUIRE(error == STFY::Error::NoError);
  std::vector<STFY::JsonMeta> meta = STFY::metaForTokens(tokens);
  size_t first_child = STFY::Internal::findFirstChildWithData(meta, 0);
  REQUIRE(first_child == 2);
}
const char first_child_with_data_json_last[] = R"json([ [], [], [],  [
[],
[],
{
  "this has a member": true
},
[]
]])json";

TEST_CASE("find_first_child_with_data_last", "[structify][meta]")
{
  STFY::ParseContext pc(first_child_with_data_json_last);
  STFY::JsonTokens tokens;
  auto error = pc.parseTo(tokens);
  REQUIRE(error == STFY::Error::NoError);
  std::vector<STFY::JsonMeta> meta = STFY::metaForTokens(tokens);
  size_t first_child = STFY::Internal::findFirstChildWithData(meta, 0);
  REQUIRE(first_child == 3);
}
} // namespace
