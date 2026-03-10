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

#include <cmrc/cmrc.hpp>

CMRC_DECLARE(external_json);

TEST_CASE("test_reformat", "[reformat]")
{
  auto fs = cmrc::external_json::get_filesystem();
  auto generated = fs.open("generated.json");
  std::string pretty;
  STFY::Error error = STFY::reformat(generated.begin(), generated.size(), pretty);
  REQUIRE(error == STFY::Error::NoError);

  std::string compact;
  error =
    STFY::reformat(generated.begin(), generated.size(), compact, STFY::SerializerOptions(STFY::SerializerOptions::Compact));
  REQUIRE(error == STFY::Error::NoError);
}
