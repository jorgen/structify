/*
 * Copyright © 2019 Jorgen Lind
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

namespace
{

const char json_data1[] = R"json(
{
  "StringNode": "Some test data",
  "NumberNode": 4676.4,
  "BooleanTrue": true,
  "BooleanFalse": false,
  "TestStruct": {
    "SubString": "Some other string",
    "SubNumber": 500,
    "Array": [
      5,
      6,
      3,
      6
    ]
  }
}
)json";
struct TestStructT
{
  std::string SubString;
  int SubNumber;
};
} // namespace
STFY_OBJECT_EXTERNAL(TestStructT, STFY_MEMBER(SubString), STFY_MEMBER(SubNumber))

namespace
{
struct TestStructSub : public TestStructT
{
  std::vector<int> Array;
};
} // namespace
STFY_OBJECT_EXTERNAL_WITH_SUPER(TestStructSub, STFY_SUPER_CLASSES(STFY_SUPER_CLASS(TestStructT)), STFY_MEMBER(Array))

namespace
{
struct JsonData1
{
  std::string StringNode;
  double NumberNode;
  bool BooleanTrue;
  bool BooleanFalse;
  TestStructSub TestStruct;
};
} // namespace
STFY_OBJECT_EXTERNAL(JsonData1, STFY_MEMBER(StringNode), STFY_MEMBER(NumberNode), STFY_MEMBER(BooleanTrue),
                   STFY_MEMBER(BooleanFalse), STFY_MEMBER(TestStruct))

namespace
{
TEST_CASE("structify_external", "[structify]")
{
  STFY::ParseContext context(json_data1);
  JsonData1 data;
  auto error = context.parseTo(data);

  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(data.StringNode == "Some test data");
  REQUIRE(data.TestStruct.SubNumber == 500);
  REQUIRE(data.TestStruct.Array.size() == 4);
  REQUIRE(data.TestStruct.Array[2] == 3);
  REQUIRE(context.error == STFY::Error::NoError);

  std::string json = STFY::serializeStruct(data);
}

} // namespace
