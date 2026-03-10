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

const char json_data[] = R"json(
{
  "some_data": 44.50
}
)json";

struct JsonMissingMeta
{
  float missing_meta;
};

struct JsonMissingTypeHandler
{
  double vec[3];
};

struct JsonContainingStructMissingTypeHandler
{
  JsonMissingTypeHandler data;

  STFY_OBJECT(STFY_MEMBER(data));
};

TEST_CASE("structify_error_check_missing_meta", "[structify][error]")
{
  STFY::ParseContext context(json_data);
  JsonMissingMeta missing;
  STFY_UNUSED(context);
  STFY_UNUSED(missing);
  // context.parseTo(missing);
  // std::string out = STFY::serializeStruct(missing);
}

TEST_CASE("structify_error_check_missing_typehandler", "[structify][error]")
{
  STFY::ParseContext context(json_data);
  JsonContainingStructMissingTypeHandler missing;
  STFY_UNUSED(context);
  STFY_UNUSED(missing);
  // context.parseTo(missing);
  // std::string out = STFY::serializeStruct(missing);
}
} // namespace
