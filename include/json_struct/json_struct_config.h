#pragma once

/*
* Copyright © 2020 Jørgen Lind

* Permission to use, copy, modify, distribute, and sell this software and its
* documentation for any purpose is hereby granted without fee, provided that
* the above copyright notice appear in all copies and that both that copyright
* notice and this permission notice appear in supporting documentation, and
* that the name of the copyright holders not be used in advertising or
* publicity pertaining to distribution of the software without specific,
* written prior permission.  The copyright holders make no representations
* about the suitability of this software for any purpose.  It is provided "as
* is" without express or implied warranty.

* THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
* INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
* EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
* CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
* DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
* TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
* OF THIS SOFTWARE.
*/

/*! \file */

/*! \mainpage json_struct
 *
 * json_struct is a set of classes meant for simple and efficient parse,
 * tokenize and validate json.
 *
 * json_struct support parsing json into a stream of tokens using the \ref
 * tokenizer "JS::Tokenizer" api, or parsing json into c++ structures using the
 * \ref js_struct "JS_OBJECT" api.
 */

/*! \page tokenizer Parsing json using JS::Tokenizer
 *
 * Tokenizing json JS::Tokenizer can be used to extract tokens
 * from a json stream.  Tokens does not describe a full object, but only
 * key-value pairs. So a object would be: "some key" and object start. Then the
 * next token would be the first key value pair of that object. Then at some
 * point the object is finished, and an object end value with no key would be
 * the token.
 *
 * Arrays would be expressed in a similar fashion, but the tokens would have no
 * key, and each element in the array would be tokens with only a value
 * specified.
 *
 * The root object would be a token with no key data, but only object or array
 * start
 *
 * A crude example of this is viewed in \ref simple_tokenize.cpp here:
 * \include simple_tokenize.cpp
 *
 * Tokenizing json in this way allows you parse arbitrary large json data.
 * Also the tokenizer has mechanisms for asking for more data, making it easy
 * to stream json data. Using this interface to parse json is a bit verbose and
 * requires the application code to keep some extra state. json_struct also has
 * functionality for parsing json data directly into c++ structures. This is
 * done by adding some metadata to the structure, or by adding a template
 * specialisation of a class.  \ref js_struct "describes this" in more detail.
 */

/*! \example simple_tokenize.cpp
 *
 * This example show very basic usage of how JS::Tokenizer can be used
 */

/*! \example simple_struct.cpp
 *
 * This example shows basic usage of parsing Json directly into structs
 */

/*! \page js_struct Parsing json into C++ structs
 *
 * json_struct makes it very easy to put your json data into c++ structures or
 * take data from c++ structures and generate json.
 *
 * This is best shown with an example: \include simple_struct.cpp
 *
 * The two interesting sections here are the lines are the:
 * \code{.cpp}
 *    JS_OBJECT(JS_MEMBER(key),
 *              JS_MEMBER(number),
 *              JS_MEMBER(boolean));
 * \endcode
 *
 * and
 *
 * \code{.cpp}
 *    JS::ParseContext parseContext(json);
 *    JsonData dataStruct;
 *    parseContext.parseTo(dataStruct);
 * \endcode
 *
 * The JS_OBJECT call inside the JsonData struct will create a nested struct
 * declaration inside the JsonData struct. This nested struct will expose some
 * meta data about the struct, exposing the names of the members at runtime.
 * json_struct can then use this runtime information to populate the struct.
 *
 * Populating the struct is done by first creating a JS::ParseContext. The
 * JS::ParseContext contains a JS::Tokenizer. This tokenizer is what the actual
 * state holder for the parsing. If allowing using '\n' instead of ',' to
 * seperate object and array elements, then this should be set on the
 * JS::Tokenizer.
 *
 * Since types will dictate the schema of the input json, the JS::ParseContext
 * will expose a list containing what members where not populated by the input
 * json, and what member in the input json that did not have any member to
 * populate.
 */



#include <algorithm>
#include <assert.h>
#include <atomic>
#include <cmath>
#include <cstring>
#include <functional>
#include <limits>
#include <memory>
#include <stddef.h>
#include <stdlib.h>
#include <string>
#include <vector>

#ifdef _MSC_VER
#include <intrin.h>
#endif

#if __cplusplus > 199711L || (defined(_MSC_VER) && _MSC_VER > 1800)
#define JS_STD_UNORDERED_MAP 1
#endif
#ifdef JS_STD_UNORDERED_MAP
#include <unordered_map>
#endif

#ifndef JS_STD_OPTIONAL
#if defined(__APPLE__)
#if __clang_major__ > 9 && __cplusplus >= 201703L
#define JS_STD_OPTIONAL 1
#endif
#elif defined(_MSC_VER) && _MSC_VER >= 1910 && _HAS_CXX17
#define JS_STD_OPTIONAL 1
#elif __cplusplus >= 201703L
#define JS_STD_OPTIONAL 1
#endif
#endif

#ifdef JS_STD_OPTIONAL
#include <optional>
#endif

#ifdef JS_STD_TIMEPOINT
#include <chrono>
#include <type_traits>
#endif

#ifndef JS_IF_CONSTEXPR
#if __cpp_if_constexpr
#define JS_IF_CONSTEXPR(exp) if constexpr (exp)
#elif defined(_MSC_VER)
#define JS_IF_CONSTEXPR(exp) __pragma(warning(push)) __pragma(warning(disable : 4127)) if (exp) __pragma(warning(pop))
#else
#define JS_IF_CONSTEXPR(exp) if (exp)
#endif
#endif

#if JS_NO_NODISCARD
#define JS_NODISCARD
#else
#if __cplusplus >= 201703L
#define JS_NODISCARD [[nodiscard]]
#else
#define JS_NODISCARD
#endif
#endif

#if defined(min) || defined(max)
#error min or max macro is defined. Make sure these are not defined before including json_struct.h.\
 Use "#define NOMINMAX 1" before including Windows.h
#endif

#define JS_UNUSED(x) (void)(x)

#ifndef JS_DISABLE_SIMD
#if defined(__SSE2__) || defined(_M_X64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2)
#define JSON_STRUCT_HAS_SSE2 1
#include <emmintrin.h>
#endif

#if defined(__SSE4_2__) || (defined(_MSC_VER) && defined(__AVX__))
#define JSON_STRUCT_HAS_SSE4_2 1
#include <nmmintrin.h>
#endif

#if defined(__AVX2__)
#define JSON_STRUCT_HAS_AVX2 1
#include <immintrin.h>
#endif

#if defined(__BMI__) || defined(__BMI2__)
#define JSON_STRUCT_HAS_BMI 1
#include <immintrin.h>
#endif

#if defined(__aarch64__) || defined(_M_ARM64) || defined(__ARM_NEON)
#define JSON_STRUCT_HAS_NEON 1
#include <arm_neon.h>
#endif
#endif


#if defined(__GNUC__) || defined(__clang__)
#define JSON_STRUCT_LIKELY(x) __builtin_expect(!!(x), 1)
#define JSON_STRUCT_UNLIKELY(x) __builtin_expect(!!(x), 0)
#define JSON_STRUCT_FORCE_INLINE __attribute__((always_inline)) inline
#define JSON_STRUCT_COLD __attribute__((cold))
#define JSON_STRUCT_HOT __attribute__((hot))
#define JSON_STRUCT_RESTRICT __restrict__
#elif defined(_MSC_VER)
#define JSON_STRUCT_LIKELY(x) (x)
#define JSON_STRUCT_UNLIKELY(x) (x)
#define JSON_STRUCT_FORCE_INLINE __forceinline
#define JSON_STRUCT_COLD
#define JSON_STRUCT_HOT
#define JSON_STRUCT_RESTRICT __restrict
#else
#define JSON_STRUCT_LIKELY(x) (x)
#define JSON_STRUCT_UNLIKELY(x) (x)
#define JSON_STRUCT_FORCE_INLINE inline
#define JSON_STRUCT_COLD
#define JSON_STRUCT_HOT
#define JSON_STRUCT_RESTRICT
#endif

#if defined(__aarch64__) || defined(_M_ARM64)
#define JSON_STRUCT_PREFETCH(ptr) __builtin_prefetch(ptr, 0, 3); __builtin_prefetch((char*)(ptr) + 64, 0, 3)
#define JSON_STRUCT_PREFETCH_WRITE(ptr) __builtin_prefetch(ptr, 1, 3)
#elif defined(__GNUC__) || defined(__clang__)
#define JSON_STRUCT_PREFETCH(ptr) __builtin_prefetch(ptr, 0, 3)
#define JSON_STRUCT_PREFETCH_WRITE(ptr) __builtin_prefetch(ptr, 1, 3)
#else
#define JSON_STRUCT_PREFETCH(ptr) ((void)0)
#define JSON_STRUCT_PREFETCH_WRITE(ptr) ((void)0)
#endif

#if defined(__aarch64__) || defined(_M_ARM64)
#define JSON_STRUCT_CACHE_LINE_SIZE 64
#define JSON_STRUCT_ALIGN_CACHE __attribute__((aligned(JSON_STRUCT_CACHE_LINE_SIZE)))
#define JSON_STRUCT_MEMORY_BARRIER() __asm__ __volatile__("dsb sy" ::: "memory")
#else
#define JSON_STRUCT_CACHE_LINE_SIZE 64
#define JSON_STRUCT_ALIGN_CACHE __attribute__((aligned(JSON_STRUCT_CACHE_LINE_SIZE)))
#define JSON_STRUCT_MEMORY_BARRIER() __asm__ __volatile__("" ::: "memory")
#endif

#ifndef JS
#define JS JS
#endif
