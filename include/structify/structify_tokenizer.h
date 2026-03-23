#pragma once
#include "structify_core.h"

namespace STFY
{
namespace Internal
{
enum Lookup
{
  StrEndOrBackSlash = 1,
  AsciiLetters = 2,
  WhiteSpaceOrNull = 4,
  PlusOrMinus = 8,
  Digits = 16,
  HatUnderscoreAprostoph = 32,
  NumberEnd = 64
};

// Constexpr lookup table for better compiler optimization
static constexpr unsigned char lookup_table[256] = {
    /*0*/ 4,        0,       0,       0,       0,       0,       0,       0,
    /*8*/ 0,        4,       4,       0,       0,       4,       0,       0,
    /*16*/ 0,       0,       0,       0,       0,       0,       0,       0,
    /*24*/ 0,       0,       0,       0,       0,       0,       0,       0,
    /*32*/ 4,       0,       1,       0,       0,       0,       0,       0,
    /*40*/ 0,       0,       0,       8 | 64,  0,       8 | 32 | 64,  32 | 64, 32,
    /*48*/ 16 | 64, 16 | 64, 16 | 64, 16 | 64, 16 | 64, 16 | 64, 16 | 64, 16 | 64,
    /*56*/ 16 | 64, 16 | 64, 0,       0,       0,       0,       0,       0,
    /*64*/ 0,       2,       2,       2,       2,       2 | 64,  2,       2,
    /*72*/ 2,       2,       2,       2,       2,       2,       2,       2,
    /*80*/ 2,       2,       2,       2,       2,       2,       2,       2,
    /*88*/ 2,       2,       2,       0,       1,       0,       32,      32,
    /*96*/ 32,      2,       2,       2,       2,       2 | 64,  2,       2,
    /*104*/ 2,      2,       2,       2,       2,       2,       2,       2,
    /*112*/ 2,      2,       2,       2,       2,       2,       2,       2,
    /*120*/ 2,      2,       2,       0,       0,       0,       0,       0,
    /*128*/ 0,      0,       0,       0,       0,       0,       0,       0,
    /*136*/ 0,      0,       0,       0,       0,       0,       0,       0,
    /*144*/ 0,      0,       0,       0,       0,       0,       0,       0,
    /*152*/ 0,      0,       0,       0,       0,       0,       0,       0,
    /*160*/ 0,      0,       0,       0,       0,       0,       0,       0,
    /*168*/ 0,      0,       0,       0,       0,       0,       0,       0,
    /*176*/ 0,      0,       0,       0,       0,       0,       0,       0,
    /*184*/ 0,      0,       0,       0,       0,       0,       0,       0,
    /*192*/ 0,      0,       0,       0,       0,       0,       0,       0,
    /*200*/ 0,      0,       0,       0,       0,       0,       0,       0,
    /*208*/ 0,      0,       0,       0,       0,       0,       0,       0,
    /*216*/ 0,      0,       0,       0,       0,       0,       0,       0,
    /*224*/ 0,      0,       0,       0,       0,       0,       0,       0,
    /*232*/ 0,      0,       0,       0,       0,       0,       0,       0,
    /*240*/ 0,      0,       0,       0,       0,       0,       0,       0,
    /*248*/ 0,      0,       0,       0,       0,       0,       0,       0};

static inline const unsigned char *lookup()
{
  return lookup_table;
}

#ifdef STRUCTIFY_HAS_SSE2

static inline int bit_scan_forward(unsigned int mask)
{
#ifdef _MSC_VER
  unsigned long index;
  _BitScanForward(&index, mask);
  return int(index);
#else
  return __builtin_ctz(mask);
#endif
}

inline size_t findStringEndSIMD(const char* STRUCTIFY_RESTRICT data, size_t length, bool& is_escaped)
{
  const char* current = data;
  const char* end = data + length;

  const __m128i quote = _mm_set1_epi8('"');
  const __m128i backslash = _mm_set1_epi8('\\');

  while (current + 16 <= end) {
    __m128i chunk = _mm_loadu_si128(reinterpret_cast<const __m128i*>(current));

    __m128i quote_mask = _mm_cmpeq_epi8(chunk, quote);
    __m128i backslash_mask = _mm_cmpeq_epi8(chunk, backslash);
    __m128i combined_mask = _mm_or_si128(quote_mask, backslash_mask);

    int mask = _mm_movemask_epi8(combined_mask);
    if (STRUCTIFY_LIKELY(mask != 0)) {
      int offset = bit_scan_forward(mask);
      current += offset;
      break;
    }
    current += 16;
  }

  while (current < end) {
    if (STRUCTIFY_UNLIKELY(is_escaped)) {
      is_escaped = false;
      current++;
      continue;
    }

    char c = *current;
    if (STRUCTIFY_UNLIKELY(c == '\\')) {
      is_escaped = true;
    } else if (c == '"') {
      return current - data + 1;
    }
    current++;
  }

  return current - data;
}

inline size_t findNumberEndSIMD(const char* STRUCTIFY_RESTRICT data, size_t length)
{
  const char* current = data;
  const char* end = data + length;

#ifdef STRUCTIFY_HAS_SSE4_2
  const __m128i number_chars = _mm_setr_epi8('0','1','2','3','4','5','6','7','8','9',
                                            '.','e','E','+','-',0);

  while (current + 16 <= end) {
    __m128i chunk = _mm_loadu_si128(reinterpret_cast<const __m128i*>(current));

    int result = _mm_cmpistri(number_chars, chunk,
                              _SIDD_CMP_EQUAL_ANY | _SIDD_NEGATIVE_POLARITY);

    if (result != 16) {
      current += result;
      break;
    }
    current += 16;
  }
#endif

  while (current < end) {
    char c = *current;
    if (STRUCTIFY_UNLIKELY(!((c >= '0' && c <= '9') || c == '.' ||
                               c == 'e' || c == 'E' || c == '+' || c == '-'))) {
      break;
    }
    current++;
  }

  return current - data;
}
#endif

#ifdef STRUCTIFY_HAS_NEON
inline size_t findStringEndNEON(const char* STRUCTIFY_RESTRICT data, size_t length, bool& is_escaped)
{
  const char* current = data;
  const char* end = data + length;

  const uint8x16_t quote = vdupq_n_u8('"');
  const uint8x16_t backslash = vdupq_n_u8('\\');

  while (current + 16 <= end) {
    uint8x16_t chunk = vld1q_u8(reinterpret_cast<const uint8_t*>(current));

    uint8x16_t quote_mask = vceqq_u8(chunk, quote);
    uint8x16_t backslash_mask = vceqq_u8(chunk, backslash);
    uint8x16_t combined_mask = vorrq_u8(quote_mask, backslash_mask);

    uint64x2_t combined_u64 = vreinterpretq_u64_u8(combined_mask);
    uint64_t combined_scalar = vgetq_lane_u64(combined_u64, 0) | vgetq_lane_u64(combined_u64, 1);

    if (STRUCTIFY_LIKELY(combined_scalar != 0)) {
      uint32x4_t mask_u32 = vreinterpretq_u32_u8(combined_mask);
      uint32_t mask_bits[4];
      vst1q_u32(mask_bits, mask_u32);

      for (int i = 0; i < 4; i++) {
        if (mask_bits[i] != 0) {
          for (int j = 0; j < 4; j++) {
            if ((mask_bits[i] >> (j * 8)) & 0xFF) {
              current += i * 4 + j;
              goto byte_by_byte;
            }
          }
        }
      }
    }
    current += 16;
  }

byte_by_byte:
  while (current < end) {
    if (STRUCTIFY_UNLIKELY(is_escaped)) {
      is_escaped = false;
      current++;
      continue;
    }

    char c = *current;
    if (STRUCTIFY_UNLIKELY(c == '\\')) {
      is_escaped = true;
    } else if (c == '"') {
      return current - data + 1;
    }
    current++;
  }

  return current - data;
}

inline size_t findNumberEndNEON(const char* STRUCTIFY_RESTRICT data, size_t length)
{
  const char* current = data;
  const char* end = data + length;

  const uint8x16_t zero = vdupq_n_u8('0');
  const uint8x16_t nine = vdupq_n_u8('9');
  const uint8x16_t dot = vdupq_n_u8('.');
  const uint8x16_t plus = vdupq_n_u8('+');
  const uint8x16_t minus = vdupq_n_u8('-');
  const uint8x16_t e_lower = vdupq_n_u8('e');
  const uint8x16_t e_upper = vdupq_n_u8('E');

  while (current + 16 <= end) {
    uint8x16_t chunk = vld1q_u8(reinterpret_cast<const uint8_t*>(current));

    uint8x16_t ge_zero = vcgeq_u8(chunk, zero);
    uint8x16_t le_nine = vcleq_u8(chunk, nine);
    uint8x16_t digits = vandq_u8(ge_zero, le_nine);

    uint8x16_t is_dot = vceqq_u8(chunk, dot);
    uint8x16_t is_plus = vceqq_u8(chunk, plus);
    uint8x16_t is_minus = vceqq_u8(chunk, minus);
    uint8x16_t is_e_lower = vceqq_u8(chunk, e_lower);
    uint8x16_t is_e_upper = vceqq_u8(chunk, e_upper);

    uint8x16_t valid_chars = vorrq_u8(digits, is_dot);
    valid_chars = vorrq_u8(valid_chars, is_plus);
    valid_chars = vorrq_u8(valid_chars, is_minus);
    valid_chars = vorrq_u8(valid_chars, is_e_lower);
    valid_chars = vorrq_u8(valid_chars, is_e_upper);

    uint8x16_t invalid_chars = vmvnq_u8(valid_chars);

    uint64x2_t invalid_u64 = vreinterpretq_u64_u8(invalid_chars);
    uint64_t invalid_scalar = vgetq_lane_u64(invalid_u64, 0) | vgetq_lane_u64(invalid_u64, 1);

    if (STRUCTIFY_LIKELY(invalid_scalar != 0)) {
      uint32x4_t invalid_u32 = vreinterpretq_u32_u8(invalid_chars);
      uint32_t invalid_bits[4];
      vst1q_u32(invalid_bits, invalid_u32);

      for (int i = 0; i < 4; i++) {
        if (invalid_bits[i] != 0) {
          for (int j = 0; j < 4; j++) {
            if ((invalid_bits[i] >> (j * 8)) & 0xFF) {
              current += i * 4 + j;
              return current - data;
            }
          }
        }
      }
    }
    current += 16;
  }

  while (current < end) {
    char c = *current;
    if (STRUCTIFY_UNLIKELY(!((c >= '0' && c <= '9') || c == '.' ||
                               c == 'e' || c == 'E' || c == '+' || c == '-'))) {
      break;
    }
    current++;
  }

  return current - data;
}

inline size_t skipWhitespaceNEON(const char* STRUCTIFY_RESTRICT data, size_t length)
{
  const char* current = data;
  const char* end = data + length;

  const uint8x16_t space = vdupq_n_u8(' ');
  const uint8x16_t tab = vdupq_n_u8('\t');
  const uint8x16_t newline = vdupq_n_u8('\n');
  const uint8x16_t carriage = vdupq_n_u8('\r');

  while (current + 16 <= end) {
    uint8x16_t chunk = vld1q_u8(reinterpret_cast<const uint8_t*>(current));

    uint8x16_t is_space = vceqq_u8(chunk, space);
    uint8x16_t is_tab = vceqq_u8(chunk, tab);
    uint8x16_t is_newline = vceqq_u8(chunk, newline);
    uint8x16_t is_carriage = vceqq_u8(chunk, carriage);

    uint8x16_t whitespace = vorrq_u8(is_space, is_tab);
    whitespace = vorrq_u8(whitespace, is_newline);
    whitespace = vorrq_u8(whitespace, is_carriage);

    uint8x16_t non_whitespace = vmvnq_u8(whitespace);

    uint64x2_t non_ws_u64 = vreinterpretq_u64_u8(non_whitespace);
    uint64_t non_ws_scalar = vgetq_lane_u64(non_ws_u64, 0) | vgetq_lane_u64(non_ws_u64, 1);

    if (STRUCTIFY_LIKELY(non_ws_scalar != 0)) {
      uint32x4_t non_ws_u32 = vreinterpretq_u32_u8(non_whitespace);
      uint32_t non_ws_bits[4];
      vst1q_u32(non_ws_bits, non_ws_u32);

      for (int i = 0; i < 4; i++) {
        if (non_ws_bits[i] != 0) {
          for (int j = 0; j < 4; j++) {
            if ((non_ws_bits[i] >> (j * 8)) & 0xFF) {
              current += i * 4 + j;
              return current - data;
            }
          }
        }
      }
    }
    current += 16;
  }

  while (current < end) {
    char c = *current;
    if (c != ' ' && c != '\t' && c != '\n' && c != '\r') {
      break;
    }
    current++;
  }

  return current - data;
}

inline size_t findAsciiEndNEON(const char* STRUCTIFY_RESTRICT data, size_t length)
{
  const char* current = data;
  const char* end = data + length;

  const uint8x16_t char_A = vdupq_n_u8('A');
  const uint8x16_t char_Z = vdupq_n_u8('Z');
  const uint8x16_t char_a = vdupq_n_u8('a');
  const uint8x16_t char_z = vdupq_n_u8('z');
  const uint8x16_t char_0 = vdupq_n_u8('0');
  const uint8x16_t char_9 = vdupq_n_u8('9');
  const uint8x16_t char_underscore = vdupq_n_u8('_');
  const uint8x16_t char_caret = vdupq_n_u8('^');
  const uint8x16_t char_apostrophe = vdupq_n_u8('`');
  const uint8x16_t char_slash = vdupq_n_u8('/');
  const uint8x16_t char_dot = vdupq_n_u8('.');
  const uint8x16_t char_hyphen = vdupq_n_u8('-');

  while (current + 16 <= end) {
    uint8x16_t chunk = vld1q_u8(reinterpret_cast<const uint8_t*>(current));

    // Check A-Z
    uint8x16_t ge_A = vcgeq_u8(chunk, char_A);
    uint8x16_t le_Z = vcleq_u8(chunk, char_Z);
    uint8x16_t is_upper = vandq_u8(ge_A, le_Z);

    // Check a-z
    uint8x16_t ge_a = vcgeq_u8(chunk, char_a);
    uint8x16_t le_z = vcleq_u8(chunk, char_z);
    uint8x16_t is_lower = vandq_u8(ge_a, le_z);

    // Check 0-9
    uint8x16_t ge_0 = vcgeq_u8(chunk, char_0);
    uint8x16_t le_9 = vcleq_u8(chunk, char_9);
    uint8x16_t is_digit = vandq_u8(ge_0, le_9);

    // Check special chars: _, ^, `, /, ., -
    uint8x16_t is_underscore = vceqq_u8(chunk, char_underscore);
    uint8x16_t is_caret = vceqq_u8(chunk, char_caret);
    uint8x16_t is_apostrophe = vceqq_u8(chunk, char_apostrophe);
    uint8x16_t is_slash = vceqq_u8(chunk, char_slash);
    uint8x16_t is_dot = vceqq_u8(chunk, char_dot);
    uint8x16_t is_hyphen = vceqq_u8(chunk, char_hyphen);

    // Combine all valid characters
    uint8x16_t valid_chars = vorrq_u8(is_upper, is_lower);
    valid_chars = vorrq_u8(valid_chars, is_digit);
    valid_chars = vorrq_u8(valid_chars, is_underscore);
    valid_chars = vorrq_u8(valid_chars, is_caret);
    valid_chars = vorrq_u8(valid_chars, is_apostrophe);
    valid_chars = vorrq_u8(valid_chars, is_slash);
    valid_chars = vorrq_u8(valid_chars, is_dot);
    valid_chars = vorrq_u8(valid_chars, is_hyphen);

    // Find invalid characters
    uint8x16_t invalid_chars = vmvnq_u8(valid_chars);

    uint64x2_t invalid_u64 = vreinterpretq_u64_u8(invalid_chars);
    uint64_t invalid_scalar = vgetq_lane_u64(invalid_u64, 0) | vgetq_lane_u64(invalid_u64, 1);

    if (STRUCTIFY_LIKELY(invalid_scalar != 0)) {
      uint32x4_t invalid_u32 = vreinterpretq_u32_u8(invalid_chars);
      uint32_t invalid_bits[4];
      vst1q_u32(invalid_bits, invalid_u32);

      for (int i = 0; i < 4; i++) {
        if (invalid_bits[i] != 0) {
          for (int j = 0; j < 4; j++) {
            if ((invalid_bits[i] >> (j * 8)) & 0xFF) {
              current += i * 4 + j;
              return current - data;
            }
          }
        }
      }
    }
    current += 16;
  }

  while (current < end) {
    char c = *current;
    if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
          (c >= '0' && c <= '9') || c == '_' || c == '^' || c == '`' || c == '/' || c == '.' || c == '-')) {
      break;
    }
    current++;
  }

  return current - data;
}

inline size_t skipCommentNEON(const char* STRUCTIFY_RESTRICT data, size_t length)
{
  const char* current = data;
  const char* end = data + length;
  const uint8x16_t newline = vdupq_n_u8('\n');

  while (current + 16 <= end) {
    uint8x16_t chunk = vld1q_u8(reinterpret_cast<const uint8_t*>(current));
    uint8x16_t is_newline = vceqq_u8(chunk, newline);

    uint64x2_t newline_u64 = vreinterpretq_u64_u8(is_newline);
    uint64_t newline_scalar = vgetq_lane_u64(newline_u64, 0) | vgetq_lane_u64(newline_u64, 1);

    if (STRUCTIFY_LIKELY(newline_scalar != 0)) {
      uint32x4_t newline_u32 = vreinterpretq_u32_u8(is_newline);
      uint32_t newline_bits[4];
      vst1q_u32(newline_bits, newline_u32);

      for (int i = 0; i < 4; i++) {
        if (newline_bits[i] != 0) {
          for (int j = 0; j < 4; j++) {
            if ((newline_bits[i] >> (j * 8)) & 0xFF) {
              current += i * 4 + j;
              return current - data + 1;
            }
          }
        }
      }
    }
    current += 16;
  }

  while (current < end) {
    if (*current == '\n') {
      return current - data + 1;
    }
    current++;
  }

  return current - data;
}
#endif

#ifdef STRUCTIFY_HAS_SSE2

inline size_t findAsciiEndSIMD(const char* STRUCTIFY_RESTRICT data, size_t length)
{
  const char* current = data;
  const char* end = data + length;

  const __m128i char_A = _mm_set1_epi8('A');
  const __m128i char_Z = _mm_set1_epi8('Z');
  const __m128i char_a = _mm_set1_epi8('a');
  const __m128i char_z = _mm_set1_epi8('z');
  const __m128i char_0 = _mm_set1_epi8('0');
  const __m128i char_9 = _mm_set1_epi8('9');
  const __m128i char_underscore = _mm_set1_epi8('_');
  const __m128i char_caret = _mm_set1_epi8('^');
  const __m128i char_apostrophe = _mm_set1_epi8('`');
  const __m128i char_slash = _mm_set1_epi8('/');
  const __m128i char_dot = _mm_set1_epi8('.');
  const __m128i char_hyphen = _mm_set1_epi8('-');

  while (current + 16 <= end) {
    __m128i chunk = _mm_loadu_si128(reinterpret_cast<const __m128i*>(current));

    // Check A-Z (unsigned comparison)
    __m128i ge_A = _mm_cmpeq_epi8(_mm_max_epu8(chunk, char_A), chunk);
    __m128i le_Z = _mm_cmpeq_epi8(_mm_min_epu8(chunk, char_Z), chunk);
    __m128i is_upper = _mm_and_si128(ge_A, le_Z);

    // Check a-z
    __m128i ge_a = _mm_cmpeq_epi8(_mm_max_epu8(chunk, char_a), chunk);
    __m128i le_z = _mm_cmpeq_epi8(_mm_min_epu8(chunk, char_z), chunk);
    __m128i is_lower = _mm_and_si128(ge_a, le_z);

    // Check 0-9
    __m128i ge_0 = _mm_cmpeq_epi8(_mm_max_epu8(chunk, char_0), chunk);
    __m128i le_9 = _mm_cmpeq_epi8(_mm_min_epu8(chunk, char_9), chunk);
    __m128i is_digit = _mm_and_si128(ge_0, le_9);

    // Check special chars: _, ^, `, /, ., -
    __m128i is_underscore = _mm_cmpeq_epi8(chunk, char_underscore);
    __m128i is_caret = _mm_cmpeq_epi8(chunk, char_caret);
    __m128i is_apostrophe = _mm_cmpeq_epi8(chunk, char_apostrophe);
    __m128i is_slash = _mm_cmpeq_epi8(chunk, char_slash);
    __m128i is_dot = _mm_cmpeq_epi8(chunk, char_dot);
    __m128i is_hyphen = _mm_cmpeq_epi8(chunk, char_hyphen);

    // Combine all valid characters
    __m128i valid_chars = _mm_or_si128(is_upper, is_lower);
    valid_chars = _mm_or_si128(valid_chars, is_digit);
    valid_chars = _mm_or_si128(valid_chars, is_underscore);
    valid_chars = _mm_or_si128(valid_chars, is_caret);
    valid_chars = _mm_or_si128(valid_chars, is_apostrophe);
    valid_chars = _mm_or_si128(valid_chars, is_slash);
    valid_chars = _mm_or_si128(valid_chars, is_dot);
    valid_chars = _mm_or_si128(valid_chars, is_hyphen);

    int mask = _mm_movemask_epi8(valid_chars);

    if (STRUCTIFY_LIKELY(mask != 0xFFFF)) {
      mask = ~mask & 0xFFFF;
      int offset = bit_scan_forward(mask);
      current += offset;
      return current - data;
    }
    current += 16;
  }

  while (current < end) {
    char c = *current;
    if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
          (c >= '0' && c <= '9') || c == '_' || c == '^' || c == '`' || c == '/' || c == '.' || c == '-')) {
      break;
    }
    current++;
  }

  return current - data;
}

inline size_t skipCommentSIMD(const char* STRUCTIFY_RESTRICT data, size_t length)
{
  const char* current = data;
  const char* end = data + length;
  const __m128i newline = _mm_set1_epi8('\n');

  while (current + 16 <= end) {
    __m128i chunk = _mm_loadu_si128(reinterpret_cast<const __m128i*>(current));
    __m128i cmp = _mm_cmpeq_epi8(chunk, newline);
    int mask = _mm_movemask_epi8(cmp);

    if (STRUCTIFY_LIKELY(mask != 0)) {
      int offset = bit_scan_forward(mask);
      current += offset;
      return current - data + 1;
    }
    current += 16;
  }

  while (current < end) {
    if (*current == '\n') {
      return current - data + 1;
    }
    current++;
  }

  return current - data;
}

inline size_t skipWhitespaceSIMD(const char* STRUCTIFY_RESTRICT data, size_t length)
{
  const char* current = data;
  const char* end = data + length;

  const __m128i space = _mm_set1_epi8(' ');
  const __m128i tab = _mm_set1_epi8('\t');
  const __m128i newline = _mm_set1_epi8('\n');
  const __m128i carriage = _mm_set1_epi8('\r');

  while (current + 16 <= end) {
    __m128i chunk = _mm_loadu_si128(reinterpret_cast<const __m128i*>(current));

    __m128i is_space = _mm_cmpeq_epi8(chunk, space);
    __m128i is_tab = _mm_cmpeq_epi8(chunk, tab);
    __m128i is_newline = _mm_cmpeq_epi8(chunk, newline);
    __m128i is_carriage = _mm_cmpeq_epi8(chunk, carriage);

    __m128i whitespace = _mm_or_si128(is_space, is_tab);
    whitespace = _mm_or_si128(whitespace, is_newline);
    whitespace = _mm_or_si128(whitespace, is_carriage);

    int mask = _mm_movemask_epi8(whitespace);

    if (STRUCTIFY_LIKELY(mask != 0xFFFF)) {
      mask = ~mask & 0xFFFF;
      int offset = bit_scan_forward(mask);
      current += offset;
      return current - data;
    }
    current += 16;
  }

  while (current < end) {
    char c = *current;
    if (c != ' ' && c != '\t' && c != '\n' && c != '\r') {
      break;
    }
    current++;
  }

  return current - data;
}
#endif

#ifdef STRUCTIFY_HAS_AVX2

inline size_t skipWhitespaceAVX2(const char* STRUCTIFY_RESTRICT data, size_t length)
{
  const char* current = data;
  const char* end = data + length;

  const __m256i space = _mm256_set1_epi8(' ');
  const __m256i tab = _mm256_set1_epi8('\t');
  const __m256i newline = _mm256_set1_epi8('\n');
  const __m256i carriage = _mm256_set1_epi8('\r');

  while (current + 32 <= end) {
    __m256i chunk = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(current));

    __m256i is_space = _mm256_cmpeq_epi8(chunk, space);
    __m256i is_tab = _mm256_cmpeq_epi8(chunk, tab);
    __m256i is_newline = _mm256_cmpeq_epi8(chunk, newline);
    __m256i is_carriage = _mm256_cmpeq_epi8(chunk, carriage);

    __m256i whitespace = _mm256_or_si256(is_space, is_tab);
    whitespace = _mm256_or_si256(whitespace, is_newline);
    whitespace = _mm256_or_si256(whitespace, is_carriage);

    int mask = _mm256_movemask_epi8(whitespace);

    if (STRUCTIFY_LIKELY(mask != 0xFFFFFFFF)) {
#ifdef STRUCTIFY_HAS_BMI
      int offset = _tzcnt_u32(~mask);
#else
      mask = ~mask;
      int offset = 0;
      while ((mask & 1) == 0) {
        mask >>= 1;
        offset++;
      }
#endif
      current += offset;
      return current - data;
    }
    current += 32;
  }

  // Fallback for remaining bytes
  while (current < end) {
    char c = *current;
    if (c != ' ' && c != '\t' && c != '\n' && c != '\r') {
      break;
    }
    current++;
  }

  return current - data;
}

inline size_t skipCommentAVX2(const char* STRUCTIFY_RESTRICT data, size_t length)
{
  const char* current = data;
  const char* end = data + length;
  const __m256i newline = _mm256_set1_epi8('\n');

  while (current + 32 <= end) {
    __m256i chunk = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(current));
    __m256i cmp = _mm256_cmpeq_epi8(chunk, newline);
    int mask = _mm256_movemask_epi8(cmp);

    if (STRUCTIFY_LIKELY(mask != 0)) {
#ifdef STRUCTIFY_HAS_BMI
      int offset = _tzcnt_u32(mask);
#else
      int offset = 0;
      while ((mask & (1 << offset)) == 0) {
        offset++;
      }
#endif
      current += offset;
      return current - data + 1;
    }
    current += 32;
  }

  while (current < end) {
    if (*current == '\n') {
      return current - data + 1;
    }
    current++;
  }

  return current - data;
}

inline size_t findStringEndAVX2(const char* STRUCTIFY_RESTRICT data, size_t length, bool& is_escaped)
{
  const char* current = data;
  const char* end = data + length;

  const __m256i quote = _mm256_set1_epi8('"');
  const __m256i backslash = _mm256_set1_epi8('\\');

  while (current + 32 <= end) {
    __m256i chunk = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(current));

    __m256i quote_mask = _mm256_cmpeq_epi8(chunk, quote);
    __m256i backslash_mask = _mm256_cmpeq_epi8(chunk, backslash);
    __m256i combined_mask = _mm256_or_si256(quote_mask, backslash_mask);

    int mask = _mm256_movemask_epi8(combined_mask);
    if (STRUCTIFY_LIKELY(mask != 0)) {
#ifdef STRUCTIFY_HAS_BMI
      int offset = _tzcnt_u32(mask);
#else
      int offset = 0;
      while ((mask & (1 << offset)) == 0) {
        offset++;
      }
#endif
      current += offset;
      break;
    }
    current += 32;
  }

  // Handle escapes and final quote
  while (current < end) {
    if (STRUCTIFY_UNLIKELY(is_escaped)) {
      is_escaped = false;
      current++;
      continue;
    }

    char c = *current;
    if (STRUCTIFY_UNLIKELY(c == '\\')) {
      is_escaped = true;
    } else if (c == '"') {
      return current - data + 1;
    }
    current++;
  }

  return current - data;
}

inline size_t findAsciiEndAVX2(const char* STRUCTIFY_RESTRICT data, size_t length)
{
  const char* current = data;
  const char* end = data + length;

  const __m256i char_A = _mm256_set1_epi8('A');
  const __m256i char_Z = _mm256_set1_epi8('Z');
  const __m256i char_a = _mm256_set1_epi8('a');
  const __m256i char_z = _mm256_set1_epi8('z');
  const __m256i char_0 = _mm256_set1_epi8('0');
  const __m256i char_9 = _mm256_set1_epi8('9');
  const __m256i char_underscore = _mm256_set1_epi8('_');
  const __m256i char_caret = _mm256_set1_epi8('^');
  const __m256i char_apostrophe = _mm256_set1_epi8('`');
  const __m256i char_slash = _mm256_set1_epi8('/');
  const __m256i char_dot = _mm256_set1_epi8('.');
  const __m256i char_hyphen = _mm256_set1_epi8('-');

  while (current + 32 <= end) {
    __m256i chunk = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(current));

    // Check A-Z using unsigned comparison
    __m256i ge_A = _mm256_cmpeq_epi8(_mm256_max_epu8(chunk, char_A), chunk);
    __m256i le_Z = _mm256_cmpeq_epi8(_mm256_min_epu8(chunk, char_Z), chunk);
    __m256i is_upper = _mm256_and_si256(ge_A, le_Z);

    // Check a-z
    __m256i ge_a = _mm256_cmpeq_epi8(_mm256_max_epu8(chunk, char_a), chunk);
    __m256i le_z = _mm256_cmpeq_epi8(_mm256_min_epu8(chunk, char_z), chunk);
    __m256i is_lower = _mm256_and_si256(ge_a, le_z);

    // Check 0-9
    __m256i ge_0 = _mm256_cmpeq_epi8(_mm256_max_epu8(chunk, char_0), chunk);
    __m256i le_9 = _mm256_cmpeq_epi8(_mm256_min_epu8(chunk, char_9), chunk);
    __m256i is_digit = _mm256_and_si256(ge_0, le_9);

    // Check special chars: _, ^, `, /, ., -
    __m256i is_underscore = _mm256_cmpeq_epi8(chunk, char_underscore);
    __m256i is_caret = _mm256_cmpeq_epi8(chunk, char_caret);
    __m256i is_apostrophe = _mm256_cmpeq_epi8(chunk, char_apostrophe);
    __m256i is_slash = _mm256_cmpeq_epi8(chunk, char_slash);
    __m256i is_dot = _mm256_cmpeq_epi8(chunk, char_dot);
    __m256i is_hyphen = _mm256_cmpeq_epi8(chunk, char_hyphen);

    // Combine all valid characters
    __m256i valid_chars = _mm256_or_si256(is_upper, is_lower);
    valid_chars = _mm256_or_si256(valid_chars, is_digit);
    valid_chars = _mm256_or_si256(valid_chars, is_underscore);
    valid_chars = _mm256_or_si256(valid_chars, is_caret);
    valid_chars = _mm256_or_si256(valid_chars, is_apostrophe);
    valid_chars = _mm256_or_si256(valid_chars, is_slash);
    valid_chars = _mm256_or_si256(valid_chars, is_dot);
    valid_chars = _mm256_or_si256(valid_chars, is_hyphen);

    int mask = _mm256_movemask_epi8(valid_chars);

    if (STRUCTIFY_LIKELY(mask != 0xFFFFFFFF)) {
#ifdef STRUCTIFY_HAS_BMI
      int offset = _tzcnt_u32(~mask);
#else
      mask = ~mask;
      int offset = 0;
      while ((mask & (1 << offset)) == 0) {
        offset++;
      }
#endif
      current += offset;
      return current - data;
    }
    current += 32;
  }

  while (current < end) {
    char c = *current;
    if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
          (c >= '0' && c <= '9') || c == '_' || c == '^' || c == '`' || c == '/' || c == '.' || c == '-')) {
      break;
    }
    current++;
  }

  return current - data;
}

#endif

} // namespace Internal

struct GetTokensResult
{
  size_t count;
  Error error;
};

class Tokenizer
{
public:
  Tokenizer();

  void allowAsciiType(bool allow);
  void allowNewLineAsTokenDelimiter(bool allow);
  void allowSuperfluousComma(bool allow);
  void allowComments(bool allow);
  void allowYaml(bool allow);
  void allowCbor(bool allow);

  void addData(const char *data, size_t size);
  template <size_t N>
  void addData(const char (&data)[N]);
  void addData(const std::vector<Token> *parsedData);
  void resetData(const char *data, size_t size, size_t index);
  void resetData(const std::vector<Token> *parsedData, size_t index);

  GetTokensResult nextTokens(Token *tokens, size_t capacity);
  const char *currentPosition() const;

  void pushScope(STFY::Type type);
  void popScope();
  STFY::Error goToEndOfScope(STFY::Token &token);

  std::string makeErrorString() const;
  void setErrorContextConfig(size_t lineContext, size_t rangeContext);
  Error updateErrorContext(Error error, const std::string &custom_message = std::string());
  const Internal::ErrorContext &errorContext() const
  {
    return error_context;
  }

private:
  enum class InTokenState : unsigned char
  {
    FindingName,
    FindingDelimiter,
    FindingData,
    FindingTokenEnd
  };

  enum class InPropertyState : unsigned char
  {
    NoStartFound,
    FindingEnd,
    FoundEnd
  };

  void resetForNewToken();
  void resetForNewValue();
  Error findStringEnd(const DataRef &json_data, size_t *chars_ahead);
  Error findAsciiEnd(const DataRef &json_data, size_t *chars_ahead);
  Error findNumberEnd(const DataRef &json_data, size_t *chars_ahead);
  Error findStartOfNextValue(Type *type, const DataRef &json_data, size_t *chars_ahead);
  Error findDelimiter(const DataRef &json_data, size_t *chars_ahead);
  Error findTokenEnd(const DataRef &json_data, size_t *chars_ahead);
  Error skipComment(const DataRef &json_data, size_t *chars_ahead);
  Error populateFromDataRef(DataRef &data, Type &type, const DataRef &json_data);
  static void populate_anonymous_token(const DataRef &data, Type type, Token &token);
  Error populateNextTokenFromDataRef(Token &next_token, const DataRef &json_data);

  InTokenState token_state = InTokenState::FindingName;
  InPropertyState property_state = InPropertyState::NoStartFound;
  Type property_type = Type::Error;
  bool is_escaped;
  bool allow_ascii_properties : 1;
  bool allow_new_lines : 1;
  bool allow_superfluous_comma : 1;
  bool allow_comments : 1;
  bool allow_yaml : 1;
  bool allow_cbor : 1;
  bool expecting_prop_or_anonymous_data : 1;
  size_t cursor_index;
  size_t current_data_start;
  size_t line_context;
  size_t line_range_context;
  size_t range_context;
  DataRef data_;
  std::vector<Internal::ScopeCounter> scope_counter;
  std::vector<Type> container_stack;
  const std::vector<Token> *parsed_data_vector;
  Internal::ErrorContext error_context;

  // YAML support
  struct YamlIndentEntry
  {
    int indent;
    Type container_type;
  };
  std::vector<Token> yaml_tokens_;
  std::vector<std::string> yaml_owned_strings_;
  DataRef yaml_pending_anchor_;
  DataRef yaml_pending_tag_;

  Error yamlParseData(const char *data, size_t size);
  std::string &yamlOwnString(const std::string &str);
  std::string &yamlOwnString(const char *data, size_t len);
  DataRef yamlOwnedRef(const std::string &str);
  DataRef yamlOwnedRef(const char *data, size_t len);
  void yamlEmitAnonymous(Type type, const char *value, size_t value_size);
  void yamlEmitProperty(const char *name, size_t name_size, Type name_type,
                        const char *value, size_t value_size, Type value_type);
  void yamlEmitPropertyWithContainerValue(const char *name, size_t name_size, Type name_type,
                                          Type container_type);
  Type yamlClassifyScalar(const char *data, size_t size);
  DataRef yamlNormalizeScalar(const char *data, size_t size, Type type);
  bool yamlIsBlankOrComment(const char *line, size_t len);
  size_t yamlReadLine(const char *data, size_t size, size_t pos, const char **line_start, size_t *line_len);
  size_t yamlMeasureIndent(const char *line, size_t len);
  void yamlStripTrailingComment(const char *data, size_t *len);
  size_t yamlFindColon(const char *data, size_t len);
  Error yamlParseBlock(const char *data, size_t size, size_t &pos,
                       std::vector<YamlIndentEntry> &indent_stack, int parent_indent);
  void yamlHandleKeyValue(const char *data, size_t size, size_t &pos,
                          const char *key, size_t key_len, Type key_type,
                          const char *val, size_t val_len,
                          std::vector<YamlIndentEntry> &indent_stack, int content_indent);
  Error yamlParseFlowObject(const char *data, size_t size, size_t &pos);
  Error yamlParseFlowArray(const char *data, size_t size, size_t &pos);
  Error yamlParseFlowObjectInner(const char *data, size_t size, size_t &pos);
  Error yamlParseFlowArrayInner(const char *data, size_t size, size_t &pos);
  void yamlParseFlowImplicitMapValue(const char *data, size_t size, size_t &pos,
                                     const char *key_data, size_t key_len, Type key_type);
  void yamlParseFlowScalar(const char *data, size_t size, size_t &pos,
                           const char **out_data, size_t *out_len, Type *out_type);
  Error yamlParseMultilineScalar(const char *data, size_t size, size_t &pos, char indicator);
  Error yamlParseMultilineScalarEx(const char *data, size_t size, size_t &pos,
                                   char indicator, int chomp_mode, int explicit_indent,
                                   int context_indent = 0);
  void yamlParseBlockScalarHeader(const char *header, size_t header_len,
                                  char *indicator, int *chomp_mode, int *explicit_indent);
  void yamlSkipFlowWhitespace(const char *data, size_t size, size_t &pos);
  void yamlApplyPendingMetadata(Token &t);
  void yamlExtractMetadata(const char *&content, size_t &len);
  std::string yamlAccumulateMultilinePlainScalar(const char *first_text, size_t first_len,
                                                  const char *data, size_t size, size_t &pos,
                                                  int min_indent);

  // CBOR support
  std::vector<Token> cbor_tokens_;
  std::vector<std::string> cbor_owned_strings_;

  Error cborParseData(const char *data, size_t size);
  std::string &cborOwnString(const std::string &str);
  std::string &cborOwnString(const char *data, size_t len);
  DataRef cborOwnedRef(const std::string &str);
  DataRef cborOwnedRef(const char *data, size_t len);
  void cborEmitAnonymous(Type type, const char *value, size_t value_size);
  void cborEmitProperty(const char *name, size_t name_size, Type name_type,
                        const char *value, size_t value_size, Type value_type);
  Error cborReadByte(const unsigned char *data, size_t size, size_t &pos, unsigned char &out);
  Error cborReadArgument(const unsigned char *data, size_t size, size_t &pos,
                         unsigned char additional, uint64_t &value);
  double cborDecodeFloat16(uint16_t half);
  DataRef cborUint64ToText(uint64_t value);
  DataRef cborInt64ToText(int64_t value);
  DataRef cborDoubleToText(double value);
  Error cborParseItem(const unsigned char *data, size_t size, size_t &pos,
                      int depth, bool as_property,
                      const char *prop_name, size_t prop_name_size);
  Error cborParseMapKey(const unsigned char *data, size_t size, size_t &pos,
                        int depth, DataRef &key_out);
};

namespace Internal
{
template <size_t SIZE>
struct StringLiteral
{
  const char *data;
  enum size_enum
  {
    size = SIZE
  };
};
template <size_t SIZE>
constexpr StringLiteral<SIZE - 1> makeStringLiteral(const char (&literal)[SIZE])
{
  return {literal};
}
}

class SerializerOptions
{
public:
  enum Style : unsigned char
  {
    Pretty,
    Compact,
    Relaxed,
    RelaxedCompact
  };

  SerializerOptions(Style style = Style::Pretty);

  int shiftSize() const;
  void setShiftSize(unsigned char set);

  Style style() const;
  void setStyle(Style style);

  bool convertAsciiToString() const;
  void setConvertAsciiToString(bool set);

  unsigned char depth() const;
  void setDepth(int depth);

  bool trailingComma() const;

  void skipDelimiter(bool skip);

  const std::string &prefix() const;
  const std::string &tokenDelimiter() const;
  const std::string &valueDelimiter() const;
  const std::string &postfix() const;

private:
  uint8_t m_shift_size;
  uint8_t m_depth;
  Style m_style;
  bool m_convert_ascii_to_string;
  bool m_trailing_comma;

  std::string m_prefix;
  std::string m_token_delimiter;
  std::string m_value_delimiter;
  std::string m_postfix;
};

class SerializerBuffer
{
public:
  SerializerBuffer()
    : buffer(nullptr)
    , size(0)
    , used(0)
  {}
  SerializerBuffer(char *buffer, size_t size)
    : buffer(buffer)
    , size(size)
    , used(0)
  {}
  size_t free() const
  {
    return size - used;
  }
  void append(const char *data, size_t size);
  template<size_t SIZE>
  void append(const char *data);
  char *buffer;
  size_t size;
  size_t used;
};

class Serializer
{
public:
  Serializer();
  Serializer(char *buffer, size_t size);

  void setBuffer(char *buffer, size_t size);
  void setOptions(const SerializerOptions &option);
  SerializerOptions options() const
  {
    return m_option;
  }

  bool write(const Token &token);
  bool write(const char *data, size_t size);
  bool write(const std::string &str)
  {
    return write(str.c_str(), str.size());
  }
  template<size_t SIZE>
  inline bool write(const Internal::StringLiteral<SIZE> &strLiteral);

  void setRequestBufferCallback(std::function<void(Serializer &)> callback);
  const SerializerBuffer &currentBuffer() const;

private:
  void askForMoreBuffers();
  void markCurrentSerializerBufferFull();
  bool writeAsString(const DataRef &data);
  bool write(Type type, const DataRef &data);

  std::function<void(Serializer &)> m_request_buffer_callback;
  SerializerBuffer m_current_buffer;

  bool m_first;
  bool m_token_start;
  SerializerOptions m_option;
};

// IMPLEMENTATION

inline Token::Token()
  : name()
  , value()
  , name_type(Type::String)
  , value_type(Type::String)
  , anchor()
  , tag()
{
}

inline Tokenizer::Tokenizer()
  : is_escaped(false)
  , allow_ascii_properties(false)
  , allow_new_lines(false)
  , allow_superfluous_comma(false)
  , allow_comments(false)
  , allow_yaml(false)
  , allow_cbor(false)
  , expecting_prop_or_anonymous_data(false)
  , cursor_index(0)
  , current_data_start(0)
  , line_context(4)
  , line_range_context(256)
  , range_context(38)
  , parsed_data_vector(nullptr)
{
  container_stack.reserve(32);
  scope_counter.reserve(32);
}

inline void Tokenizer::allowAsciiType(bool allow)
{
  allow_ascii_properties = allow;
}

inline void Tokenizer::allowNewLineAsTokenDelimiter(bool allow)
{
  allow_new_lines = allow;
}

inline void Tokenizer::allowSuperfluousComma(bool allow)
{
  allow_superfluous_comma = allow;
}

inline void Tokenizer::allowComments(bool allow)
{
  allow_comments = allow;
}

inline void Tokenizer::allowYaml(bool allow)
{
  allow_yaml = allow;
}

inline void Tokenizer::addData(const char *data, size_t data_size)
{
  if (allow_yaml)
  {
    yamlParseData(data, data_size);
    return;
  }
  if (allow_cbor)
  {
    cborParseData(data, data_size);
    return;
  }
  data_ = DataRef(data, data_size);
}

template <size_t N>
inline void Tokenizer::addData(const char (&data)[N])
{
  data_ = DataRef(data, N - 1);
}

inline void Tokenizer::addData(const std::vector<Token> *parsedData)
{
  assert(parsed_data_vector == 0);
  parsed_data_vector = parsedData;
  cursor_index = 0;
}

inline void Tokenizer::resetData(const char *data, size_t size, size_t index)
{
  data_ = DataRef();
  parsed_data_vector = nullptr;
  cursor_index = index;
  addData(data, size);
  resetForNewToken();
}

inline void Tokenizer::resetData(const std::vector<Token> *parsedData, size_t index)
{
  data_ = DataRef();
  parsed_data_vector = parsedData;
  cursor_index = index;
  resetForNewToken();
}

inline GetTokensResult Tokenizer::nextTokens(Token *tokens, size_t capacity)
{
  size_t count = 0;
  assert(!scope_counter.size() ||
         (scope_counter.back().type != STFY::Type::ArrayEnd && scope_counter.back().type != STFY::Type::ObjectEnd));

  error_context.clear();

  while (count < capacity)
  {
    if (scope_counter.size() && scope_counter.back().depth == 0)
    {
      return {count, count > 0 ? Error::NoError : Error::ScopeHasEnded};
    }
    if (parsed_data_vector)
    {
      Token &next_token = tokens[count];
      next_token = (*parsed_data_vector)[cursor_index];
      cursor_index++;
      if (cursor_index == parsed_data_vector->size())
      {
        cursor_index = 0;
        parsed_data_vector = nullptr;
      }
      if (scope_counter.size())
        scope_counter.back().handleType(next_token.value_type);
      count++;
      continue;
    }

    if (STRUCTIFY_UNLIKELY(data_.size == 0))
    {
      return {count, count > 0 ? Error::NoError : Error::NeedMoreData};
    }

    if (count == 0)
      resetForNewToken();

    Token &next_token = tokens[count];
    Error error = populateNextTokenFromDataRef(next_token, data_);

    if (STRUCTIFY_UNLIKELY(error == Error::NeedMoreData))
    {
      return {count, count > 0 ? Error::NoError : Error::NeedMoreData};
    }

    if (STRUCTIFY_UNLIKELY(error != Error::NoError))
    {
      updateErrorContext(error);
      return {count, error};
    }

    if (next_token.value_type == Type::ArrayStart || next_token.value_type == Type::ObjectStart)
      container_stack.push_back(next_token.value_type);
    if (STRUCTIFY_UNLIKELY(next_token.value_type == Type::ArrayEnd))
    {
      if (STRUCTIFY_UNLIKELY(!container_stack.size() || container_stack.back() != STFY::Type::ArrayStart))
      {
        error = Error::UnexpectedArrayEnd;
        updateErrorContext(error);
        return {count, error};
      }
      container_stack.pop_back();
    }
    if (STRUCTIFY_UNLIKELY(next_token.value_type == Type::ObjectEnd))
    {
      if (STRUCTIFY_UNLIKELY(!container_stack.size() || container_stack.back() != STFY::Type::ObjectStart))
      {
        error = Error::UnexpectedObjectEnd;
        updateErrorContext(error);
        return {count, error};
      }
      container_stack.pop_back();
    }
    if (STRUCTIFY_LIKELY(scope_counter.size()))
      scope_counter.back().handleType(next_token.value_type);
    count++;
  }
  return {count, Error::NoError};
}

inline const char *Tokenizer::currentPosition() const
{
  if (parsed_data_vector)
    return reinterpret_cast<const char *>(cursor_index);

  if (data_.size == 0)
    return nullptr;

  return data_.data + cursor_index;
}

inline void Tokenizer::pushScope(STFY::Type type)
{
  scope_counter.push_back({type, 1});
  if (type != Type::ArrayStart && type != Type::ObjectStart)
    scope_counter.back().depth--;
}

inline void Tokenizer::popScope()
{
  assert(scope_counter.size() && scope_counter.back().depth == 0);
  scope_counter.pop_back();
}

inline STFY::Error Tokenizer::goToEndOfScope(STFY::Token &token)
{
  Token buf[32];
  while (scope_counter.back().depth)
  {
    auto result = nextTokens(buf, 32);
    if (result.error != Error::NoError)
      return result.error;
    if (result.count > 0)
      token = buf[result.count - 1];
  }
  return Error::NoError;
}



STRUCTIFY_COLD inline std::string Tokenizer::makeErrorString() const
{
  static_assert(sizeof(Internal::error_strings) / sizeof *Internal::error_strings ==
                  size_t(Error::UserDefinedErrors) + 1,
                "Please add missing error message");

  std::string retString("Error");
  if (error_context.error < Error::UserDefinedErrors)
    retString += std::string(" ") + Internal::error_strings[int(error_context.error)];
  if (error_context.custom_message.size())
    retString += " " + error_context.custom_message;
  retString += std::string(":\n");
  for (size_t i = 0; i < error_context.lines.size(); i++)
  {
    retString += error_context.lines[i] + "\n";
    if (i == error_context.line)
    {
      std::string pointing(error_context.character + 2, ' ');
      pointing[error_context.character] = '^';
      pointing[error_context.character + 1] = '\n';
      retString += pointing;
    }
  }
  return retString;
}

inline void Tokenizer::setErrorContextConfig(size_t lineContext, size_t rangeContext)
{
  line_context = lineContext;
  range_context = rangeContext;
}

STRUCTIFY_FORCE_INLINE void Tokenizer::resetForNewToken()
{
  property_state = InPropertyState::NoStartFound;
  property_type = Type::Error;
  current_data_start = 0;
}

STRUCTIFY_FORCE_INLINE void Tokenizer::resetForNewValue()
{
  property_state = InPropertyState::NoStartFound;
  property_type = Type::Error;
  current_data_start = 0;
}

STRUCTIFY_FORCE_INLINE Error Tokenizer::findStringEnd(const DataRef &json_data, size_t *chars_ahead)
{
#ifdef STRUCTIFY_HAS_AVX2
  if (STRUCTIFY_LIKELY(json_data.size - cursor_index >= 32)) {
    size_t consumed = Internal::findStringEndAVX2(
      json_data.data + cursor_index,
      json_data.size - cursor_index,
      is_escaped);
    if (consumed < json_data.size - cursor_index) {
      *chars_ahead = consumed;
      return Error::NoError;
    }
  }
#elif defined(STRUCTIFY_HAS_NEON)
  if (STRUCTIFY_LIKELY(json_data.size - cursor_index >= 16)) {
    size_t consumed = Internal::findStringEndNEON( json_data.data + cursor_index, json_data.size - cursor_index, is_escaped);
    if (consumed < json_data.size - cursor_index) {
      *chars_ahead = consumed;
      return Error::NoError;
    }
  }
#elif defined(STRUCTIFY_HAS_SSE2)
  if (STRUCTIFY_LIKELY(json_data.size - cursor_index >= 16)) {
    size_t consumed = Internal::findStringEndSIMD(
      json_data.data + cursor_index,
      json_data.size - cursor_index,
      is_escaped);

    if (consumed < json_data.size - cursor_index) {
      *chars_ahead = consumed;
      return Error::NoError;
    }
  }
#endif

  size_t end = cursor_index;
  STRUCTIFY_PREFETCH(json_data.data + end + 64);

  while (STRUCTIFY_LIKELY(end < json_data.size))
  {
    if (STRUCTIFY_UNLIKELY(is_escaped))
    {
      is_escaped = false;
      end++;
      continue;
    }

    const char* search_start = json_data.data + end;
    const char* search_end = json_data.data + json_data.size;
    const char* found = std::find_if(search_start, search_end, [](char c) {
      return Internal::lookup()[(unsigned char)c] == Internal::StrEndOrBackSlash;
    });
    end = found - json_data.data;
    if (STRUCTIFY_UNLIKELY(end >= json_data.size))
      break;
    char c = json_data.data[end];
    if (STRUCTIFY_UNLIKELY(c == '\\'))
    {
      is_escaped = true;
    }
    else if (STRUCTIFY_LIKELY(c == '"'))
    {
      *chars_ahead = end + 1 - cursor_index;
      return Error::NoError;
    }
    end++;
  }
  return Error::NeedMoreData;
}

STRUCTIFY_FORCE_INLINE Error Tokenizer::findAsciiEnd(const DataRef &json_data, size_t *chars_ahead)
{
  assert(property_type == Type::Ascii);
  size_t end = cursor_index;

#ifdef STRUCTIFY_HAS_NEON
  if (STRUCTIFY_LIKELY(json_data.size - cursor_index >= 16)) {
    size_t consumed = Internal::findAsciiEndNEON(
      json_data.data + cursor_index,
      json_data.size - cursor_index);

    if (consumed < json_data.size - cursor_index) {
      *chars_ahead = consumed;
      return Error::NoError;
    }
    end = cursor_index + consumed;
  }
#elif defined(STRUCTIFY_HAS_SSE2)
  if (STRUCTIFY_LIKELY(json_data.size - cursor_index >= 16)) {
    size_t consumed = Internal::findAsciiEndSIMD(
      json_data.data + cursor_index,
      json_data.size - cursor_index);

    if (consumed < json_data.size - cursor_index) {
      *chars_ahead = consumed;
      return Error::NoError;
    }
    end = cursor_index + consumed;
  }
#endif

  STRUCTIFY_PREFETCH(json_data.data + end + 64);

  while (STRUCTIFY_LIKELY(end < json_data.size))
  {
    const char* search_start = json_data.data + end;
    const char* search_end = json_data.data + json_data.size;
    const char* found = std::find_if(search_start, search_end, [](char c) {
      unsigned char lc = Internal::lookup()[(unsigned char)c];
      return !(lc & (Internal::AsciiLetters | Internal::Digits | Internal::HatUnderscoreAprostoph));
    });
    end = found - json_data.data;

    if (end >= json_data.size)
      break;

    char ascii_code = json_data.data[end];
    if ((ascii_code >= 'A' && ascii_code <= 'Z') || (ascii_code >= '^' && ascii_code <= 'z') ||
        (ascii_code >= '0' && ascii_code <= '9'))
    {
      end++;
      continue;
    }
    else if (ascii_code == '\0')
    {
      *chars_ahead = end - cursor_index;
      return Error::NeedMoreData;
    }
    else
    {
      *chars_ahead = end - cursor_index;
      return Error::NoError;
    }
  }
  return Error::NeedMoreData;
}

STRUCTIFY_FORCE_INLINE Error Tokenizer::findNumberEnd(const DataRef &json_data, size_t *chars_ahead)
{
#ifdef STRUCTIFY_HAS_NEON
  if (STRUCTIFY_LIKELY(json_data.size - cursor_index >= 16)) {
    size_t consumed = Internal::findNumberEndNEON(
      json_data.data + cursor_index,
      json_data.size - cursor_index);

    if (consumed > 0 && cursor_index + consumed < json_data.size) {
      *chars_ahead = consumed;
      return Error::NoError;
    }
  }
#elif defined(STRUCTIFY_HAS_SSE2)
  if (STRUCTIFY_LIKELY(json_data.size - cursor_index >= 16)) {
    size_t consumed = Internal::findNumberEndSIMD(
      json_data.data + cursor_index,
      json_data.size - cursor_index);

    if (consumed > 0 && cursor_index + consumed < json_data.size) {
      *chars_ahead = consumed;
      return Error::NoError;
    }
  }
#endif

  size_t end = cursor_index;
  STRUCTIFY_PREFETCH(json_data.data + end + 64);

  const char* search_start = json_data.data + end;
  const char* search_end = json_data.data + json_data.size;
  const char* found = std::find_if(search_start, search_end, [](char c) {
    unsigned char lc = Internal::lookup()[(unsigned char)c];
    return !(lc & Internal::NumberEnd);
  });

  end = found - json_data.data;
  if (end < json_data.size)
  {
    *chars_ahead = end - cursor_index;
    return Error::NoError;
  }
  return Error::NeedMoreData;
}

STRUCTIFY_FORCE_INLINE Error Tokenizer::findStartOfNextValue(Type *type, const DataRef &json_data, size_t *chars_ahead)
{

  assert(property_state == InPropertyState::NoStartFound);

  // Skip whitespace using SIMD if available
  size_t current_pos = cursor_index;
#ifdef STRUCTIFY_HAS_AVX2
  if (STRUCTIFY_LIKELY(json_data.size - current_pos >= 32)) {
    size_t ws_skipped = Internal::skipWhitespaceAVX2(
      json_data.data + current_pos,
      json_data.size - current_pos);
    current_pos += ws_skipped;
  }
#elif defined(STRUCTIFY_HAS_NEON)
  if (STRUCTIFY_LIKELY(json_data.size - current_pos >= 16)) {
    size_t ws_skipped = Internal::skipWhitespaceNEON(
      json_data.data + current_pos,
      json_data.size - current_pos);
    current_pos += ws_skipped;
  }
#elif defined(STRUCTIFY_HAS_SSE2)
  if (STRUCTIFY_LIKELY(json_data.size - current_pos >= 16)) {
    size_t ws_skipped = Internal::skipWhitespaceSIMD(
      json_data.data + current_pos,
      json_data.size - current_pos);
    current_pos += ws_skipped;
  }
#endif

  // Fast path: check first character after whitespace for single-char tokens
  if (STRUCTIFY_LIKELY(current_pos < json_data.size))
  {
    const char c = json_data.data[current_pos];

    // Most common single-character tokens - optimized with direct comparison
    if (STRUCTIFY_LIKELY(c == '"'))
    {
      *type = Type::String;
      *chars_ahead = current_pos - cursor_index;
      return Error::NoError;
    }
    else if (STRUCTIFY_LIKELY(c == '{'))
    {
      *type = Type::ObjectStart;
      *chars_ahead = current_pos - cursor_index;
      return Error::NoError;
    }
    else if (STRUCTIFY_LIKELY(c == '}'))
    {
      *type = Type::ObjectEnd;
      *chars_ahead = current_pos - cursor_index;
      return Error::NoError;
    }
    else if (STRUCTIFY_LIKELY(c == '['))
    {
      *type = Type::ArrayStart;
      *chars_ahead = current_pos - cursor_index;
      return Error::NoError;
    }
    else if (STRUCTIFY_LIKELY(c == ']'))
    {
      *type = Type::ArrayEnd;
      *chars_ahead = current_pos - cursor_index;
      return Error::NoError;
    }
  }

  for (; current_pos < json_data.size; current_pos++)
  {
    const char c = json_data.data[current_pos];
    unsigned char lc = Internal::lookup()[(unsigned char)c];

    if (STRUCTIFY_UNLIKELY(allow_comments && c == '/' && current_pos + 1 < json_data.size && json_data.data[current_pos + 1] == '/'))
    {
      cursor_index = current_pos + 2;
      size_t comment_skip;
      Error comment_error = skipComment(json_data, &comment_skip);
      if (comment_error != Error::NoError)
        return comment_error;

      cursor_index += comment_skip;
      current_pos = cursor_index - 1;
      continue;
    }

    if (STRUCTIFY_LIKELY(c == '"'))
    {
      *type = Type::String;
      *chars_ahead = current_pos - cursor_index;
      return Error::NoError;
    }
    else if (STRUCTIFY_LIKELY(c == '{'))
    {
      *type = Type::ObjectStart;
      *chars_ahead = current_pos - cursor_index;
      return Error::NoError;
    }
    else if (STRUCTIFY_LIKELY(c == '}'))
    {
      *type = Type::ObjectEnd;
      *chars_ahead = current_pos - cursor_index;
      return Error::NoError;
    }
    else if (STRUCTIFY_LIKELY(c == '['))
    {
      *type = Type::ArrayStart;
      *chars_ahead = current_pos - cursor_index;
      return Error::NoError;
    }
    else if (STRUCTIFY_LIKELY(c == ']'))
    {
      *type = Type::ArrayEnd;
      *chars_ahead = current_pos - cursor_index;
      return Error::NoError;
    }
    else if (STRUCTIFY_LIKELY(lc & (Internal::PlusOrMinus | Internal::Digits)))
    {
      *type = Type::Number;
      *chars_ahead = current_pos - cursor_index;
      return Error::NoError;
    }
    else if (STRUCTIFY_LIKELY(lc & Internal::AsciiLetters))
    {
      *type = Type::Ascii;
      *chars_ahead = current_pos - cursor_index;
      return Error::NoError;
    }
    else if (STRUCTIFY_UNLIKELY(lc == 0))
    {
      *chars_ahead = current_pos - cursor_index;
      return Error::EncounteredIllegalChar;
    }
  }
  return Error::NeedMoreData;
}

STRUCTIFY_FORCE_INLINE Error Tokenizer::findDelimiter(const DataRef &json_data, size_t *chars_ahead)
{
  if (STRUCTIFY_UNLIKELY(container_stack.empty()))
    return Error::IllegalPropertyType;

  // Skip whitespace using SIMD if available
  size_t end = cursor_index;
#ifdef STRUCTIFY_HAS_AVX2
  if (STRUCTIFY_LIKELY(json_data.size - end >= 32 && !allow_new_lines)) {
    size_t ws_skipped = Internal::skipWhitespaceAVX2(
      json_data.data + end,
      json_data.size - end);
    end += ws_skipped;
  }
#elif defined(STRUCTIFY_HAS_NEON)
  if (STRUCTIFY_LIKELY(json_data.size - end >= 16 && !allow_new_lines)) {
    size_t ws_skipped = Internal::skipWhitespaceNEON(
      json_data.data + end,
      json_data.size - end);
    end += ws_skipped;
  }
#elif defined(STRUCTIFY_HAS_SSE2)
  if (STRUCTIFY_LIKELY(json_data.size - end >= 16 && !allow_new_lines)) {
    size_t ws_skipped = Internal::skipWhitespaceSIMD(
      json_data.data + end,
      json_data.size - end);
    end += ws_skipped;
  }
#endif

  // Prefetch ahead for better cache utilization
  STRUCTIFY_PREFETCH(json_data.data + end + 64);

  for (; end < json_data.size; end++)
  {
    const char c = json_data.data[end];

    if (STRUCTIFY_UNLIKELY(allow_comments && c == '/' && end + 1 < json_data.size && json_data.data[end + 1] == '/'))
    {
      cursor_index = end + 2;
      size_t comment_skip;
      Error comment_error = skipComment(json_data, &comment_skip);
      if (comment_error != Error::NoError)
        return comment_error;

      cursor_index += comment_skip;
      end = cursor_index - 1;
      continue;
    }

    // Cache container type to avoid multiple back() calls
    Type container_type = container_stack.back();

    if (STRUCTIFY_LIKELY(c == ':'))
    {
      if (STRUCTIFY_UNLIKELY(container_type != Type::ObjectStart))
        return Error::ExpectedDelimiter;
      token_state = InTokenState::FindingData;
      *chars_ahead = end + 1 - cursor_index;
      return Error::NoError;
    }
    else if (STRUCTIFY_LIKELY(c == ',') || STRUCTIFY_UNLIKELY(allow_new_lines && c == '\n'))
    {
      if (STRUCTIFY_UNLIKELY(container_type != Type::ArrayStart))
        return Error::ExpectedDelimiter;
      token_state = InTokenState::FindingName;
      *chars_ahead = end + 1 - cursor_index;
      return Error::NoError;
    }
    else if (STRUCTIFY_LIKELY(c == ']'))
    {
      if (STRUCTIFY_UNLIKELY(container_type != Type::ArrayStart))
        return Error::ExpectedDelimiter;
      token_state = InTokenState::FindingName;
      *chars_ahead = end - cursor_index;
      return Error::NoError;
    }
    else if (STRUCTIFY_UNLIKELY(!(Internal::lookup()[(unsigned char)c] & Internal::WhiteSpaceOrNull)))
    {
      return Error::ExpectedDelimiter;
    }
  }
  return Error::NeedMoreData;
}

STRUCTIFY_FORCE_INLINE Error Tokenizer::findTokenEnd(const DataRef &json_data, size_t *chars_ahead)
{
  if (STRUCTIFY_UNLIKELY(container_stack.empty()))
    return Error::NoError;

  // Skip whitespace using SIMD if available
  size_t end = cursor_index;
#ifdef STRUCTIFY_HAS_AVX2
  if (STRUCTIFY_LIKELY(json_data.size - end >= 32 && !allow_ascii_properties)) {
    size_t ws_skipped = Internal::skipWhitespaceAVX2(
      json_data.data + end,
      json_data.size - end);
    end += ws_skipped;
  }
#elif defined(STRUCTIFY_HAS_NEON)
  if (STRUCTIFY_LIKELY(json_data.size - end >= 16 && !allow_ascii_properties)) {
    size_t ws_skipped = Internal::skipWhitespaceNEON(
      json_data.data + end,
      json_data.size - end);
    end += ws_skipped;
  }
#elif defined(STRUCTIFY_HAS_SSE2)
  if (STRUCTIFY_LIKELY(json_data.size - end >= 16 && !allow_ascii_properties)) {
    size_t ws_skipped = Internal::skipWhitespaceSIMD(
      json_data.data + end,
      json_data.size - end);
    end += ws_skipped;
  }
#endif

  // Prefetch ahead for better cache utilization
  STRUCTIFY_PREFETCH(json_data.data + end + 64);

  for (; end < json_data.size; end++)
  {
    const char c = json_data.data[end];

    if (STRUCTIFY_UNLIKELY(allow_comments && c == '/' && end + 1 < json_data.size && json_data.data[end + 1] == '/'))
    {
      cursor_index = end + 2;
      size_t comment_skip;
      Error comment_error = skipComment(json_data, &comment_skip);
      if (comment_error != Error::NoError)
        return comment_error;

      cursor_index += comment_skip;
      end = cursor_index - 1;
      continue;
    }

    if (STRUCTIFY_LIKELY(c == ','))
    {
      expecting_prop_or_anonymous_data = true;
      *chars_ahead = end + 1 - cursor_index;
      return Error::NoError;
    }
    if (STRUCTIFY_LIKELY(c == ']' || c == '}'))
    {
      *chars_ahead = end - cursor_index;
      return Error::NoError;
    }
    if (STRUCTIFY_UNLIKELY(c == '\n'))
    {
      if (allow_new_lines)
      {
        *chars_ahead = end + 1 - cursor_index;
        return Error::NoError;
      }
    }
    else if (STRUCTIFY_LIKELY(Internal::lookup()[(unsigned char)c] & Internal::WhiteSpaceOrNull))
    {
      continue;
    }
    else
    {
      *chars_ahead = end + 1 - cursor_index;
      return Error::InvalidToken;
    }
  }
  return Error::NeedMoreData;
}

inline Error Tokenizer::skipComment(const DataRef &json_data, size_t *chars_ahead)
{
#ifdef STRUCTIFY_HAS_NEON
  if (STRUCTIFY_LIKELY(json_data.size - cursor_index >= 16 && !allow_new_lines)) {
    size_t consumed = Internal::skipCommentNEON(
      json_data.data + cursor_index,
      json_data.size - cursor_index);

    if (consumed > 0 && cursor_index + consumed <= json_data.size &&
        json_data.data[cursor_index + consumed - 1] == '\n') {
      *chars_ahead = consumed;
      return Error::NoError;
    }
  }
#elif defined(STRUCTIFY_HAS_SSE2)
  if (STRUCTIFY_LIKELY(json_data.size - cursor_index >= 16 && !allow_new_lines)) {
    size_t consumed = Internal::skipCommentSIMD(
      json_data.data + cursor_index,
      json_data.size - cursor_index);

    if (consumed > 0 && cursor_index + consumed <= json_data.size &&
        json_data.data[cursor_index + consumed - 1] == '\n') {
      *chars_ahead = consumed;
      return Error::NoError;
    }
  }
#endif

  const char* start = json_data.data + cursor_index;
  const char* end = json_data.data + json_data.size;
  const char* found = std::find(start, end, '\n');

  if (found != end)
  {
    if (allow_new_lines)
    {
      *chars_ahead = (found - start);
    }
    else
    {
      *chars_ahead = (found - start) + 1;
    }
    return Error::NoError;
  }
  return Error::NeedMoreData;
}

inline Error Tokenizer::populateFromDataRef(DataRef &data, Type &type, const DataRef &json_data)
{
  size_t diff = 0;
  Error error = Error::NoError;
  data.size = 0;
  data.data = json_data.data + cursor_index;
  if (property_state == InPropertyState::NoStartFound)
  {
    error = findStartOfNextValue(&type, json_data, &diff);
    if (error != Error::NoError)
    {
      type = Type::Error;
      return error;
    }

    data.data = json_data.data + cursor_index + diff;
    current_data_start = cursor_index + diff;
    if (type == Type::String)
    {
      data.data++;
      current_data_start++;
    }
    cursor_index += diff + 1;
    property_type = type;

    if (type == Type::ObjectStart || type == Type::ObjectEnd || type == Type::ArrayStart || type == Type::ArrayEnd)
    {
      data.size = 1;
      property_state = InPropertyState::FoundEnd;
    }
    else
    {
      property_state = InPropertyState::FindingEnd;
    }
  }

  size_t negative_size_adjustment = 0;
  if (STRUCTIFY_LIKELY(property_state == InPropertyState::FindingEnd))
  {
    // Use jump table optimization with likely paths
    if (STRUCTIFY_LIKELY(type == Type::String))
    {
      error = findStringEnd(json_data, &diff);
      negative_size_adjustment = 1;
    }
    else if (STRUCTIFY_LIKELY(type == Type::Number))
    {
      error = findNumberEnd(json_data, &diff);
    }
    else if (STRUCTIFY_LIKELY(type == Type::Ascii))
    {
      error = findAsciiEnd(json_data, &diff);
    }
    else
    {
      return Error::InvalidToken;
    }

    if (error != Error::NoError)
    {
      return error;
    }

    cursor_index += diff;
    data.size = cursor_index - current_data_start - negative_size_adjustment;
    property_state = InPropertyState::FoundEnd;
  }

  return Error::NoError;
}

inline void Tokenizer::populate_anonymous_token(const DataRef &data, Type type, Token &token)
{
  token.name = DataRef();
  token.name_type = Type::Ascii;
  token.value = data;
  token.value_type = type;
}

namespace Internal
{
// Optimized keyword detection using integer comparison for small strings
static STRUCTIFY_FORCE_INLINE Type getType(Type type, const char *data, size_t length)
{
  if (STRUCTIFY_LIKELY(type != Type::Ascii))
    return type;

  // Fast path: use integer comparison instead of memcmp
  // "null" = 4 bytes, "true" = 4 bytes, "false" = 5 bytes
  if (STRUCTIFY_LIKELY(length == 4))
  {
    // Load as uint32_t for single comparison
    uint32_t word;
    memcpy(&word, data, 4);

    // "null" = 0x6c6c756e (little endian)
    constexpr uint32_t null_word = 0x6c6c756e;
    if (word == null_word)
      return Type::Null;

    // "true" = 0x65757274 (little endian)
    constexpr uint32_t true_word = 0x65757274;
    if (word == true_word)
      return Type::Bool;
  }
  else if (STRUCTIFY_UNLIKELY(length == 5))
  {
    // "false" - check first 4 bytes + last byte
    uint32_t word;
    memcpy(&word, data, 4);

    // "fals" = 0x736c6166 (little endian)
    constexpr uint32_t false_word = 0x736c6166;
    if (word == false_word && data[4] == 'e')
      return Type::Bool;
  }
  return Type::Ascii;
}

inline size_t strnlen(const char *data, size_t size)
{
  auto it = std::find(data, data + size, '\0');
  return it - data;
}

} // namespace Internal

inline Error Tokenizer::populateNextTokenFromDataRef(Token &next_token, const DataRef &json_data)
{
  Token tmp_token;
  while (cursor_index < json_data.size)
  {
    size_t diff = 0;
    DataRef data;
    Type type;
    Error error;
    switch (token_state)
    {
    case InTokenState::FindingName:
      type = Type::Error;
      error = populateFromDataRef(data, type, json_data);
      if (error != Error::NoError)
        return error;

      if (type == Type::ObjectEnd || type == Type::ArrayEnd || type == Type::ArrayStart || type == Type::ObjectStart)
      {
        switch (type)
        {
        case Type::ObjectEnd:
        case Type::ArrayEnd:
          if (expecting_prop_or_anonymous_data && !allow_superfluous_comma)
          {
            return Error::ExpectedDataToken;
          }
          populate_anonymous_token(data, type, next_token);
          token_state = InTokenState::FindingTokenEnd;
          resetForNewValue();
          return Error::NoError;

        case Type::ObjectStart:
        case Type::ArrayStart:
          populate_anonymous_token(data, type, next_token);
          expecting_prop_or_anonymous_data = false;
          token_state = InTokenState::FindingName;
          resetForNewValue();
          return Error::NoError;
        default:
          return Error::UnknownError;
        }
      }
      else
      {
        tmp_token.name = data;
      }

      tmp_token.name_type = Internal::getType(type, tmp_token.name.data, tmp_token.name.size);
      token_state = InTokenState::FindingDelimiter;
      resetForNewValue();
      break;

    case InTokenState::FindingDelimiter:
      error = findDelimiter(json_data, &diff);
      if (error != Error::NoError)
        return error;
      cursor_index += diff;
      resetForNewValue();
      expecting_prop_or_anonymous_data = false;
      if (token_state == InTokenState::FindingName)
      {
        populate_anonymous_token(tmp_token.name, tmp_token.name_type, next_token);
        return Error::NoError;
      }
      else
      {
        if (tmp_token.name_type != Type::String)
        {
          if (!allow_ascii_properties || tmp_token.name_type != Type::Ascii)
          {
            return Error::IllegalPropertyName;
          }
        }
      }
      break;

    case InTokenState::FindingData:
      type = Type::Error;
      error = populateFromDataRef(data, type, json_data);
      if (error != Error::NoError)
        return error;

      tmp_token.value = data;
      tmp_token.value_type = Internal::getType(type, tmp_token.value.data, tmp_token.value.size);

      if (tmp_token.value_type == Type::Ascii && !allow_ascii_properties)
        return Error::IllegalDataValue;

      if (type == Type::ObjectStart || type == Type::ArrayStart)
      {
        token_state = InTokenState::FindingName;
      }
      else
      {
        token_state = InTokenState::FindingTokenEnd;
      }
      next_token = tmp_token;
      resetForNewValue();
      return Error::NoError;
    case InTokenState::FindingTokenEnd:
      error = findTokenEnd(json_data, &diff);
      if (error != Error::NoError)
        return error;
      cursor_index += diff;
      token_state = InTokenState::FindingName;
      break;
    }
  }
  return Error::NeedMoreData;
}

namespace Internal
{
struct Lines
{
  size_t start;
  size_t end;
};
} // namespace Internal

STRUCTIFY_COLD inline Error Tokenizer::updateErrorContext(Error error, const std::string &custom_message)
{
  error_context.error = error;
  error_context.custom_message = custom_message;
  if ((!parsed_data_vector || parsed_data_vector->empty()) && data_.size == 0)
    return error;

  const DataRef json_data =
    parsed_data_vector && parsed_data_vector->size()
      ? DataRef(parsed_data_vector->front().value.data,
                size_t(parsed_data_vector->back().value.data - parsed_data_vector->front().value.data))
      : data_;
  int64_t real_cursor_index = parsed_data_vector && parsed_data_vector->size()
                               ? int64_t (parsed_data_vector->at(cursor_index).value.data - json_data.data)
                               : int64_t(cursor_index);
  const int64_t stop_back = real_cursor_index - std::min(int64_t(real_cursor_index), int64_t(line_range_context));
  const int64_t stop_forward = std::min(real_cursor_index + int64_t(line_range_context), int64_t(json_data.size));
  std::vector<Internal::Lines> lines;
  lines.push_back({0, size_t(real_cursor_index)});
  assert(real_cursor_index <= int64_t(json_data.size));
  int64_t lines_back = 0;
  int64_t lines_forward = 0;
  int64_t cursor_back;
  int64_t cursor_forward;
  for (cursor_back = real_cursor_index - 1; cursor_back > stop_back; cursor_back--)
  {
    if (*(json_data.data + cursor_back) == '\n')
    {
      lines.front().start = size_t(cursor_back + 1);
      lines_back++;
      if (lines_back == 1)
        error_context.character = size_t(real_cursor_index - cursor_back);
      if (lines_back == int64_t(line_context))
      {
        lines_back--;
        break;
      }

      lines.insert(lines.begin(), {0, size_t(cursor_back)});
    }
  }
  if (lines.front().start == 0 && cursor_back > 0)
      lines.front().start = size_t(cursor_back);
  bool add_new_line = false;
  for (cursor_forward = real_cursor_index; cursor_forward < stop_forward; cursor_forward++)
  {
    if (add_new_line)
    {
      lines.push_back({size_t(cursor_forward), 0});
      add_new_line = false;
    }
    if (*(json_data.data + cursor_forward) == '\n')
    {
      lines.back().end = size_t(cursor_forward);
      lines_forward++;
      if (lines_forward == int64_t(line_context))
        break;
      add_new_line = true;
    }
  }
  if (lines.back().end == 0)
    lines.back().end = size_t(cursor_forward - 1);

  if (lines.size() > 1)
  {
    error_context.lines.reserve(lines.size());
    for (auto &line : lines)
    {
      error_context.lines.push_back(std::string(json_data.data + line.start, line.end - line.start));
    }
    error_context.line = size_t(lines_back);
  }
  else
  {
    error_context.line = 0;

    int64_t left = real_cursor_index > int64_t(range_context) ? real_cursor_index - int64_t(range_context) : 0;
    int64_t right =
      real_cursor_index + int64_t(range_context) > int64_t(json_data.size) ? int64_t(json_data.size) : real_cursor_index + int64_t(range_context);
    error_context.character = size_t(real_cursor_index - left);
    error_context.lines.push_back(std::string(json_data.data + left, size_t(right - left)));
  }
  return error;
}

static inline STFY::Error reformat(const char *data, size_t size, std::string &out,
                                 const SerializerOptions &options = SerializerOptions())
{
  Tokenizer tokenizer;
  tokenizer.addData(data, size);
  Error error = Error::NoError;

  Serializer serializer;
  serializer.setOptions(options);
  size_t last_pos = 0;
  serializer.setRequestBufferCallback([&out, &last_pos](Serializer &serializer_p) {
    size_t end = out.size();
    out.resize(end * 2);
    serializer_p.setBuffer(&out[0] + end, end);
    last_pos = end;
  });
  if (out.empty())
    out.resize(4096);
  serializer.setBuffer(&out[0], out.size());

  Token buf[32];
  while (true)
  {
    auto result = tokenizer.nextTokens(buf, 32);
    for (size_t i = 0; i < result.count; i++)
      serializer.write(buf[i]);
    if (result.error != Error::NoError)
    {
      error = result.error;
      break;
    }
  }
  out.resize(last_pos + serializer.currentBuffer().used);
  if (error == Error::NeedMoreData)
    return Error::NoError;

  return error;
}
static inline STFY::Error reformat(const std::string &in, std::string &out,
                                 const SerializerOptions &options = SerializerOptions())
{
  return reformat(in.c_str(), in.size(), out, options);
}

// Tuple start
namespace Internal
{
template <size_t...>
struct Sequence
{
  using type = Sequence;
};

template <typename A, typename B>
struct Merge;
template <size_t... Is1, size_t... Is2>
struct Merge<Sequence<Is1...>, Sequence<Is2...>>
{
  using type = Sequence<Is1..., (sizeof...(Is1) + Is2)...>;
};

template <size_t size>
struct GenSequence;
template <>
struct GenSequence<0>
{
  using type = Sequence<>;
};
template <>
struct GenSequence<1>
{
  using type = Sequence<0>;
};
template <size_t size>
struct GenSequence
{
  using type = typename Merge<typename GenSequence<size / size_t(2)>::type,
                              typename GenSequence<size - size / size_t(2)>::type>::type;
};

template <size_t index, typename T>
struct Element
{
  constexpr Element()
    : data()
  {
  }

  constexpr Element(const T &t)
    : data(t)
  {
  }
  using type = T;
  T data;
};

template <typename A, typename... Bs>
struct TupleImpl;

template <size_t... indices, typename... Ts>
struct TupleImpl<Sequence<indices...>, Ts...> : public Element<indices, Ts>...
{
  constexpr TupleImpl()
    : Element<indices, Ts>()...
  {
  }

  constexpr TupleImpl(Ts... args)
    : Element<indices, Ts>(args)...
  {
  }
};
} // namespace Internal

template <size_t I, typename... Ts>
struct TypeAt
{
  template <typename T>
  static Internal::Element<I, T> deduce(Internal::Element<I, T>);

  using tuple_impl = Internal::TupleImpl<typename Internal::GenSequence<sizeof...(Ts)>::type, Ts...>;
  using element = decltype(deduce(tuple_impl()));
  using type = typename element::type;
};

template <typename... Ts>
struct Tuple
{
  constexpr Tuple()
    : impl()
  {
  }

  constexpr Tuple(Ts... args)
    : impl(args...)
  {
  }

  using Seq = typename Internal::GenSequence<sizeof...(Ts)>::type;
  Internal::TupleImpl<Seq, Ts...> impl;
  static constexpr const size_t size = sizeof...(Ts);

  template <size_t Index>
  constexpr const typename TypeAt<Index, Ts...>::type &get() const
  {
    return static_cast<const typename TypeAt<Index, Ts...>::element &>(impl).data;
  }

  template <size_t Index>
  typename TypeAt<Index, Ts...>::type &get()
  {
    return static_cast<typename TypeAt<Index, Ts...>::element &>(impl).data;
  }
};

/// \private
template <size_t I, typename... Ts>
struct TypeAt<I, const Tuple<Ts...>>
{
  template <typename T>
  static Internal::Element<I, T> deduce(Internal::Element<I, T>);

  using tuple_impl = Internal::TupleImpl<typename Internal::GenSequence<sizeof...(Ts)>::type, Ts...>;
  using element = decltype(deduce(tuple_impl()));
  using type = typename element::type;
};

/// \private
template <size_t I, typename... Ts>
struct TypeAt<I, Tuple<Ts...>>
{
  template <typename T>
  static Internal::Element<I, T> deduce(Internal::Element<I, T>);

  using tuple_impl = Internal::TupleImpl<typename Internal::GenSequence<sizeof...(Ts)>::type, Ts...>;
  using element = decltype(deduce(tuple_impl()));
  using type = typename element::type;
};

/*!  \private
 */
template <>
struct Tuple<>
{
  static constexpr const size_t size = 0;
};

template <typename... Ts>
constexpr Tuple<Ts...> makeTuple(Ts... args)
{
  return Tuple<Ts...>(args...);
}
// Tuple end

inline SerializerOptions::SerializerOptions(Style style)

  : m_shift_size(style == Compact || style == RelaxedCompact ? 0 : 2)
  , m_depth(0)
  , m_style(style)
  , m_convert_ascii_to_string(style == Pretty || style == Compact)
  , m_trailing_comma(style == Relaxed || style == RelaxedCompact)
  , m_token_delimiter(",")
  , m_value_delimiter(style == Pretty || style == Relaxed ? ": " : ":")
  , m_postfix(style == Pretty || style == Relaxed ? "\n" : "")
{
}

inline int SerializerOptions::shiftSize() const
{
  return m_shift_size;
}

inline void SerializerOptions::setShiftSize(unsigned char set)
{
  m_shift_size = set;
}

inline unsigned char SerializerOptions::depth() const
{
  return m_depth;
}

inline SerializerOptions::Style SerializerOptions::style() const
{
  return m_style;
}

inline bool SerializerOptions::convertAsciiToString() const
{
  return m_convert_ascii_to_string;
}

inline void SerializerOptions::setConvertAsciiToString(bool set)
{
  m_convert_ascii_to_string = set;
}

inline bool SerializerOptions::trailingComma() const
{
  return m_trailing_comma;
}

inline void SerializerOptions::setStyle(Style style)
{
  m_style = style;
  m_postfix = m_style == Pretty ? std::string("\n") : std::string("");
  m_value_delimiter = m_style == Pretty ? std::string(" : ") : std::string(":");
  setDepth(m_depth);
}

inline void SerializerOptions::skipDelimiter(bool skip)
{
  if (skip)
    m_token_delimiter = "";
  else
    m_token_delimiter = ",";
}

inline void SerializerOptions::setDepth(int depth)
{
  m_depth = (unsigned char)depth;
  m_prefix = m_style == Pretty ? std::string(depth * size_t(m_shift_size), ' ') : std::string();
}

inline const std::string &SerializerOptions::prefix() const
{
  return m_prefix;
}
inline const std::string &SerializerOptions::tokenDelimiter() const
{
  return m_token_delimiter;
}
inline const std::string &SerializerOptions::valueDelimiter() const
{
  return m_value_delimiter;
}
inline const std::string &SerializerOptions::postfix() const
{
  return m_postfix;
}

inline void SerializerBuffer::append(const char *data, size_t data_size)
{
  assert(used + data_size <= size);
  memcpy(buffer + used, data, data_size);
  used += data_size;
}

template<size_t SIZE>
inline void SerializerBuffer::append(const char *data)
{
  assert(used + SIZE <= size);
  memcpy(buffer + used, data, SIZE);
  used += SIZE;
}

inline Serializer::Serializer()
  : m_first(true)
  , m_token_start(true)
{
}

inline Serializer::Serializer(char *buffer, size_t size)
  : m_current_buffer(buffer,size)
  , m_first(true)
  , m_token_start(true)

{
}

inline void Serializer::setBuffer(char *buffer, size_t size)
{
  m_current_buffer = SerializerBuffer(buffer, size);
}

inline void Serializer::setOptions(const SerializerOptions &option)
{
  m_option = option;
}


inline bool Serializer::write(const Token &in_token)
{
  auto begining_literals = makeTuple( STFY::Internal::makeStringLiteral("\n  "),
                                      Internal::makeStringLiteral("\n    "),
                                      Internal::makeStringLiteral("\n      "),
                                      Internal::makeStringLiteral("\n        "),
                                      Internal::makeStringLiteral("\n          "),
                                      Internal::makeStringLiteral(",\n  "),
                                      Internal::makeStringLiteral(",\n    "),
                                      Internal::makeStringLiteral(",\n      "),
                                      Internal::makeStringLiteral(",\n        "),
                                      Internal::makeStringLiteral(",\n          "));
  //auto begining_literals_compat = makeTuple( Internal::makeStringLiteral(",\""));
  const Token &token = in_token;

  bool isEnd = token.value_type == Type::ObjectEnd || token.value_type == Type::ArrayEnd;
  if (isEnd)
  {
    if (m_option.depth() <= 0)
    {
      return false;
    }
    m_option.setDepth(m_option.depth() - 1);
  }

  bool shortcut_front = false;
  if (m_option.shiftSize() == 2 && !m_first)
  {
    if (!m_token_start && (!isEnd || m_option.trailingComma()))
    {
      if (m_option.depth() == 1)
        shortcut_front = write(begining_literals.get<5>());
      else if (m_option.depth() == 2)
        shortcut_front = write(begining_literals.get<6>());
      else if (m_option.depth() == 3)
        shortcut_front = write(begining_literals.get<7>());
      else if (m_option.depth() == 4)
        shortcut_front = write(begining_literals.get<8>());
      else if (m_option.depth() == 5)
        shortcut_front = write(begining_literals.get<9>());
    }
    else
    {
      if (m_option.depth() == 1)
        shortcut_front = write(begining_literals.get<0>());
      else if (m_option.depth() == 2)
        shortcut_front = write(begining_literals.get<1>());
      else if (m_option.depth() == 3)
        shortcut_front = write(begining_literals.get<2>());
      else if (m_option.depth() == 4)
        shortcut_front = write(begining_literals.get<3>());
      else if (m_option.depth() == 5)
        shortcut_front = write(begining_literals.get<4>());

    }
  }

  if (!shortcut_front)
  {
    if (!m_token_start)
    {
      if (!isEnd || m_option.trailingComma())
      {
        if (!m_option.tokenDelimiter().empty())
        {
          if (!write(Internal::makeStringLiteral(",")))
            return false;
        }
      }
    }

    if (m_first)
    {
      m_first = false;
    }
    else
    {
      if (!m_option.postfix().empty())
        if (!write(m_option.postfix()))
          return false;
    }


    if (!m_option.prefix().empty())
      if (!write(m_option.prefix()))
        return false;

  }
  if (token.name.size)
  {
    if (!write(token.name_type, token.name))
      return false;

    if (m_option.style() == SerializerOptions::Pretty || m_option.style() == SerializerOptions::Relaxed)
    {
      if (!write(Internal::makeStringLiteral(": ")))
        return false;
    }
    else
    {
      if (!write(Internal::makeStringLiteral(":")))
        return false;
    }
  }

  if (!write(token.value_type, token.value))
    return false;

  m_token_start = (token.value_type == Type::ObjectStart || token.value_type == Type::ArrayStart);
  if (m_token_start)
  {
    m_option.setDepth(m_option.depth() + 1);
  }
  return true;
}

inline void Serializer::setRequestBufferCallback(std::function<void(Serializer &)> callback)
{
  m_request_buffer_callback = callback;
}

inline const SerializerBuffer &Serializer::currentBuffer() const
{
  return m_current_buffer;
}

inline void Serializer::askForMoreBuffers()
{
  if (m_request_buffer_callback)
    m_request_buffer_callback(*this);
}

inline void Serializer::markCurrentSerializerBufferFull()
{
  m_current_buffer = SerializerBuffer();
  askForMoreBuffers();
}

inline bool Serializer::writeAsString(const DataRef &data)
{
  bool written;
  written = write(Internal::makeStringLiteral("\""));
  if (!written)
    return false;

  written = write(data.data, data.size);
  if (!written)
    return false;

  written = write(Internal::makeStringLiteral("\""));

  return written;
}

inline bool Serializer::write(Type type, const DataRef &data)
{
  bool written;
  switch (type)
  {
  case Type::String:
    written = writeAsString(data);
    break;
  case Type::Ascii:
    if (m_option.convertAsciiToString())
      written = writeAsString(data);
    else
      written = write(data.data, data.size);
    break;
  case Type::Null:
    written = write("null", 4);
    break;
  default:
    written = write(data.data, data.size);
    break;
  }
  return written;
}

inline bool Serializer::write(const char *data, size_t size)
{
  if (!size)
    return true;
  size_t written = 0;
  while (written < size)
  {
    size_t free = m_current_buffer.free();
    if (free == 0)
    {
      markCurrentSerializerBufferFull();
      if (!m_current_buffer.free())
        return false;
      continue;
    }
    size_t to_write = std::min(size - written, free);
    m_current_buffer.append(data + written, to_write);
    written += to_write;
  }
  return written == size;
}

template<size_t SIZE>
inline bool Serializer::write(const Internal::StringLiteral<SIZE> &strLiteral)
{
  if (m_current_buffer.free() < SIZE)
    return write(strLiteral.data, SIZE);

  m_current_buffer.append<SIZE>(strLiteral.data);
  return true;
}


} // namespace STFY
