#pragma once
#include "structify_tokenizer.h"
#include "float_tools/float_tools.h"
#include <cstring>
#include <cmath>

// ============================================================================
// CBOR Tokenizer Implementation (RFC 8949)
// ============================================================================

namespace STFY
{

namespace Internal
{
static const char cbor_obj_start[] = "{";
static const char cbor_obj_end[] = "}";
static const char cbor_arr_start[] = "[";
static const char cbor_arr_end[] = "]";
static const char cbor_true_str[] = "true";
static const char cbor_false_str[] = "false";
static const char cbor_null_str[] = "null";

// Base64 encoding table for byte strings
static const char cbor_base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

inline std::string cborBase64Encode(const unsigned char *data, size_t len)
{
  std::string result;
  result.reserve(((len + 2) / 3) * 4);
  for (size_t i = 0; i < len; i += 3)
  {
    unsigned int b = (unsigned int)(data[i]) << 16;
    if (i + 1 < len)
      b |= (unsigned int)(data[i + 1]) << 8;
    if (i + 2 < len)
      b |= (unsigned int)(data[i + 2]);

    result.push_back(cbor_base64_chars[(b >> 18) & 0x3F]);
    result.push_back(cbor_base64_chars[(b >> 12) & 0x3F]);
    result.push_back((i + 1 < len) ? cbor_base64_chars[(b >> 6) & 0x3F] : '=');
    result.push_back((i + 2 < len) ? cbor_base64_chars[b & 0x3F] : '=');
  }
  return result;
}

} // namespace Internal

// -- Owned string helpers (same pattern as YAML) --

inline std::string &Tokenizer::cborOwnString(const std::string &str)
{
  cbor_owned_strings_.push_back(str);
  return cbor_owned_strings_.back();
}

inline std::string &Tokenizer::cborOwnString(const char *data, size_t len)
{
  cbor_owned_strings_.emplace_back(data, len);
  return cbor_owned_strings_.back();
}

inline DataRef Tokenizer::cborOwnedRef(const std::string &str)
{
  std::string &owned = cborOwnString(str);
  return DataRef(owned.data(), owned.size());
}

inline DataRef Tokenizer::cborOwnedRef(const char *data, size_t len)
{
  std::string &owned = cborOwnString(data, len);
  return DataRef(owned.data(), owned.size());
}

// -- Token emission --

inline void Tokenizer::cborEmitAnonymous(Type type, const char *value, size_t value_size)
{
  Token t;
  t.name = DataRef();
  t.name_type = Type::Ascii;
  t.value = DataRef(value, value_size);
  t.value_type = type;
  cbor_tokens_.push_back(t);
}

inline void Tokenizer::cborEmitProperty(const char *name, size_t name_size, Type name_type,
                                        const char *value, size_t value_size, Type value_type)
{
  Token t;
  t.name = DataRef(name, name_size);
  t.name_type = name_type;
  t.value = DataRef(value, value_size);
  t.value_type = value_type;
  cbor_tokens_.push_back(t);
}

// -- Byte-level helpers --

inline Error Tokenizer::cborReadByte(const unsigned char *data, size_t size, size_t &pos, unsigned char &out)
{
  if (pos >= size)
    return Error::CborTruncatedData;
  out = data[pos++];
  return Error::NoError;
}

inline Error Tokenizer::cborReadArgument(const unsigned char *data, size_t size, size_t &pos,
                                         unsigned char additional, uint64_t &value)
{
  if (additional <= 23)
  {
    value = additional;
    return Error::NoError;
  }
  if (additional == 24)
  {
    if (pos >= size)
      return Error::CborTruncatedData;
    value = data[pos++];
    return Error::NoError;
  }
  if (additional == 25)
  {
    if (pos + 2 > size)
      return Error::CborTruncatedData;
    value = ((uint64_t)data[pos] << 8) | (uint64_t)data[pos + 1];
    pos += 2;
    return Error::NoError;
  }
  if (additional == 26)
  {
    if (pos + 4 > size)
      return Error::CborTruncatedData;
    value = ((uint64_t)data[pos] << 24) | ((uint64_t)data[pos + 1] << 16) |
            ((uint64_t)data[pos + 2] << 8) | (uint64_t)data[pos + 3];
    pos += 4;
    return Error::NoError;
  }
  if (additional == 27)
  {
    if (pos + 8 > size)
      return Error::CborTruncatedData;
    value = ((uint64_t)data[pos] << 56) | ((uint64_t)data[pos + 1] << 48) |
            ((uint64_t)data[pos + 2] << 40) | ((uint64_t)data[pos + 3] << 32) |
            ((uint64_t)data[pos + 4] << 24) | ((uint64_t)data[pos + 5] << 16) |
            ((uint64_t)data[pos + 6] << 8) | (uint64_t)data[pos + 7];
    pos += 8;
    return Error::NoError;
  }
  // 28-30 are reserved, 31 is indefinite (handled by caller)
  return Error::CborInvalidEncoding;
}

inline double Tokenizer::cborDecodeFloat16(uint16_t half)
{
  // IEEE 754 half-precision: 1 sign, 5 exponent, 10 mantissa
  int sign = (half >> 15) & 1;
  int exp = (half >> 10) & 0x1F;
  int mant = half & 0x3FF;

  double value;
  if (exp == 0)
  {
    // Subnormal or zero
    value = std::ldexp((double)mant, -24);
  }
  else if (exp == 31)
  {
    // Infinity or NaN
    if (mant == 0)
      value = INFINITY;
    else
      value = NAN;
  }
  else
  {
    // Normalized
    value = std::ldexp((double)(mant + 1024), exp - 25);
  }
  return sign ? -value : value;
}

// -- Number-to-text conversion helpers --

inline DataRef Tokenizer::cborUint64ToText(uint64_t value)
{
  char buf[40];
  int digits_truncated;
  // ft::integer::to_buffer handles uint64_t via the template
  int size = ft::integer::to_buffer(value, buf, sizeof(buf), &digits_truncated);
  if (size <= 0 || digits_truncated)
  {
    return cborOwnedRef("0", 1);
  }
  return cborOwnedRef(buf, (size_t)size);
}

inline DataRef Tokenizer::cborInt64ToText(int64_t value)
{
  char buf[40];
  int digits_truncated;
  int size = ft::integer::to_buffer(value, buf, sizeof(buf), &digits_truncated);
  if (size <= 0 || digits_truncated)
  {
    return cborOwnedRef("0", 1);
  }
  return cborOwnedRef(buf, (size_t)size);
}

inline DataRef Tokenizer::cborDoubleToText(double value)
{
  if (std::isnan(value))
    return DataRef("NaN", 3);
  if (std::isinf(value))
    return value > 0 ? DataRef("Infinity", 8) : DataRef("-Infinity", 9);

  char buf[32];
  int size = ft::ryu::to_buffer(value, buf, sizeof(buf));
  if (size <= 0)
  {
    return cborOwnedRef("0", 1);
  }
  return cborOwnedRef(buf, (size_t)size);
}

// -- Recursive item parser --

inline Error Tokenizer::cborParseItem(const unsigned char *data, size_t size, size_t &pos,
                                      int depth, bool as_property,
                                      const char *prop_name, size_t prop_name_size)
{
  if (depth > 256)
    return Error::CborNestingTooDeep;

  unsigned char initial;
  Error err = cborReadByte(data, size, pos, initial);
  if (err != Error::NoError)
    return err;

  unsigned char major_type = (initial >> 5) & 0x07;
  unsigned char additional = initial & 0x1F;

  switch (major_type)
  {
  case 0: // Unsigned integer
  {
    uint64_t value;
    err = cborReadArgument(data, size, pos, additional, value);
    if (err != Error::NoError)
      return err;
    DataRef text = cborUint64ToText(value);
    if (as_property)
      cborEmitProperty(prop_name, prop_name_size, Type::Ascii, text.data, text.size, Type::Number);
    else
      cborEmitAnonymous(Type::Number, text.data, text.size);
    return Error::NoError;
  }

  case 1: // Negative integer
  {
    uint64_t raw;
    err = cborReadArgument(data, size, pos, additional, raw);
    if (err != Error::NoError)
      return err;
    // CBOR negative int: -1 - raw
    // If raw > INT64_MAX-1, the value can't fit in int64_t. Use the closest representable.
    if (raw > (uint64_t)9223372036854775807ULL)
    {
      // Overflow: emit as -INT64_MAX equivalent string
      DataRef text = cborOwnedRef("-18446744073709551616", 21);
      if (as_property)
        cborEmitProperty(prop_name, prop_name_size, Type::Ascii, text.data, text.size, Type::Number);
      else
        cborEmitAnonymous(Type::Number, text.data, text.size);
    }
    else
    {
      int64_t value = -1 - (int64_t)raw;
      DataRef text = cborInt64ToText(value);
      if (as_property)
        cborEmitProperty(prop_name, prop_name_size, Type::Ascii, text.data, text.size, Type::Number);
      else
        cborEmitAnonymous(Type::Number, text.data, text.size);
    }
    return Error::NoError;
  }

  case 2: // Byte string
  {
    if (additional == 31)
    {
      // Indefinite-length byte string: concatenate chunks
      std::string accumulated;
      while (pos < size)
      {
        if (data[pos] == 0xFF)
        {
          pos++; // consume break
          break;
        }
        unsigned char chunk_initial;
        err = cborReadByte(data, size, pos, chunk_initial);
        if (err != Error::NoError)
          return err;
        if (((chunk_initial >> 5) & 0x07) != 2)
          return Error::CborInvalidEncoding;
        uint64_t chunk_len;
        err = cborReadArgument(data, size, pos, chunk_initial & 0x1F, chunk_len);
        if (err != Error::NoError)
          return err;
        if (pos + chunk_len > size)
          return Error::CborTruncatedData;
        accumulated.append((const char *)data + pos, (size_t)chunk_len);
        pos += (size_t)chunk_len;
      }
      std::string encoded = Internal::cborBase64Encode((const unsigned char *)accumulated.data(), accumulated.size());
      DataRef text = cborOwnedRef(encoded);
      if (as_property)
        cborEmitProperty(prop_name, prop_name_size, Type::Ascii, text.data, text.size, Type::Ascii);
      else
        cborEmitAnonymous(Type::Ascii, text.data, text.size);
    }
    else
    {
      uint64_t len;
      err = cborReadArgument(data, size, pos, additional, len);
      if (err != Error::NoError)
        return err;
      if (pos + len > size)
        return Error::CborTruncatedData;
      std::string encoded = Internal::cborBase64Encode(data + pos, (size_t)len);
      pos += (size_t)len;
      DataRef text = cborOwnedRef(encoded);
      if (as_property)
        cborEmitProperty(prop_name, prop_name_size, Type::Ascii, text.data, text.size, Type::Ascii);
      else
        cborEmitAnonymous(Type::Ascii, text.data, text.size);
    }
    return Error::NoError;
  }

  case 3: // Text string
  {
    if (additional == 31)
    {
      // Indefinite-length text string: concatenate chunks
      std::string accumulated;
      while (pos < size)
      {
        if (data[pos] == 0xFF)
        {
          pos++; // consume break
          break;
        }
        unsigned char chunk_initial;
        err = cborReadByte(data, size, pos, chunk_initial);
        if (err != Error::NoError)
          return err;
        if (((chunk_initial >> 5) & 0x07) != 3)
          return Error::CborInvalidEncoding;
        uint64_t chunk_len;
        err = cborReadArgument(data, size, pos, chunk_initial & 0x1F, chunk_len);
        if (err != Error::NoError)
          return err;
        if (pos + chunk_len > size)
          return Error::CborTruncatedData;
        accumulated.append((const char *)data + pos, (size_t)chunk_len);
        pos += (size_t)chunk_len;
      }
      DataRef text = cborOwnedRef(accumulated);
      if (as_property)
        cborEmitProperty(prop_name, prop_name_size, Type::Ascii, text.data, text.size, Type::String);
      else
        cborEmitAnonymous(Type::String, text.data, text.size);
    }
    else
    {
      uint64_t len;
      err = cborReadArgument(data, size, pos, additional, len);
      if (err != Error::NoError)
        return err;
      if (pos + len > size)
        return Error::CborTruncatedData;
      DataRef text = cborOwnedRef((const char *)data + pos, (size_t)len);
      pos += (size_t)len;
      if (as_property)
        cborEmitProperty(prop_name, prop_name_size, Type::Ascii, text.data, text.size, Type::String);
      else
        cborEmitAnonymous(Type::String, text.data, text.size);
    }
    return Error::NoError;
  }

  case 4: // Array
  {
    if (as_property)
    {
      Token t;
      t.name = DataRef(prop_name, prop_name_size);
      t.name_type = Type::Ascii;
      t.value = DataRef(Internal::cbor_arr_start, 1);
      t.value_type = Type::ArrayStart;
      cbor_tokens_.push_back(t);
    }
    else
    {
      cborEmitAnonymous(Type::ArrayStart, Internal::cbor_arr_start, 1);
    }

    if (additional == 31)
    {
      // Indefinite-length array
      while (pos < size)
      {
        if (data[pos] == 0xFF)
        {
          pos++; // consume break
          break;
        }
        err = cborParseItem(data, size, pos, depth + 1, false, nullptr, 0);
        if (err != Error::NoError)
          return err;
      }
    }
    else
    {
      uint64_t count;
      err = cborReadArgument(data, size, pos, additional, count);
      if (err != Error::NoError)
        return err;
      for (uint64_t i = 0; i < count; i++)
      {
        err = cborParseItem(data, size, pos, depth + 1, false, nullptr, 0);
        if (err != Error::NoError)
          return err;
      }
    }

    cborEmitAnonymous(Type::ArrayEnd, Internal::cbor_arr_end, 1);
    return Error::NoError;
  }

  case 5: // Map
  {
    if (as_property)
    {
      Token t;
      t.name = DataRef(prop_name, prop_name_size);
      t.name_type = Type::Ascii;
      t.value = DataRef(Internal::cbor_obj_start, 1);
      t.value_type = Type::ObjectStart;
      cbor_tokens_.push_back(t);
    }
    else
    {
      cborEmitAnonymous(Type::ObjectStart, Internal::cbor_obj_start, 1);
    }

    if (additional == 31)
    {
      // Indefinite-length map
      while (pos < size)
      {
        if (data[pos] == 0xFF)
        {
          pos++; // consume break
          break;
        }
        // Parse key
        DataRef key_ref;
        err = cborParseMapKey(data, size, pos, depth + 1, key_ref);
        if (err != Error::NoError)
          return err;
        // Parse value as property
        err = cborParseItem(data, size, pos, depth + 1, true, key_ref.data, key_ref.size);
        if (err != Error::NoError)
          return err;
      }
    }
    else
    {
      uint64_t count;
      err = cborReadArgument(data, size, pos, additional, count);
      if (err != Error::NoError)
        return err;
      for (uint64_t i = 0; i < count; i++)
      {
        // Parse key
        DataRef key_ref;
        err = cborParseMapKey(data, size, pos, depth + 1, key_ref);
        if (err != Error::NoError)
          return err;
        // Parse value as property
        err = cborParseItem(data, size, pos, depth + 1, true, key_ref.data, key_ref.size);
        if (err != Error::NoError)
          return err;
      }
    }

    cborEmitAnonymous(Type::ObjectEnd, Internal::cbor_obj_end, 1);
    return Error::NoError;
  }

  case 6: // Tag (semantic tag)
  {
    // Skip the tag number and process the enclosed item
    uint64_t tag_number;
    err = cborReadArgument(data, size, pos, additional, tag_number);
    if (err != Error::NoError)
      return err;
    (void)tag_number; // Phase 1: ignore tag semantics
    return cborParseItem(data, size, pos, depth, as_property, prop_name, prop_name_size);
  }

  case 7: // Simple values and floats
  {
    if (additional <= 19)
    {
      // Unassigned simple values - treat as null
      if (as_property)
        cborEmitProperty(prop_name, prop_name_size, Type::Ascii,
                         Internal::cbor_null_str, 4, Type::Null);
      else
        cborEmitAnonymous(Type::Null, Internal::cbor_null_str, 4);
      return Error::NoError;
    }
    if (additional == 20) // false
    {
      if (as_property)
        cborEmitProperty(prop_name, prop_name_size, Type::Ascii,
                         Internal::cbor_false_str, 5, Type::Bool);
      else
        cborEmitAnonymous(Type::Bool, Internal::cbor_false_str, 5);
      return Error::NoError;
    }
    if (additional == 21) // true
    {
      if (as_property)
        cborEmitProperty(prop_name, prop_name_size, Type::Ascii,
                         Internal::cbor_true_str, 4, Type::Bool);
      else
        cborEmitAnonymous(Type::Bool, Internal::cbor_true_str, 4);
      return Error::NoError;
    }
    if (additional == 22) // null
    {
      if (as_property)
        cborEmitProperty(prop_name, prop_name_size, Type::Ascii,
                         Internal::cbor_null_str, 4, Type::Null);
      else
        cborEmitAnonymous(Type::Null, Internal::cbor_null_str, 4);
      return Error::NoError;
    }
    if (additional == 23) // undefined → treat as null
    {
      if (as_property)
        cborEmitProperty(prop_name, prop_name_size, Type::Ascii,
                         Internal::cbor_null_str, 4, Type::Null);
      else
        cborEmitAnonymous(Type::Null, Internal::cbor_null_str, 4);
      return Error::NoError;
    }
    if (additional == 24) // simple value in next byte
    {
      if (pos >= size)
        return Error::CborTruncatedData;
      pos++; // skip the simple value byte
      if (as_property)
        cborEmitProperty(prop_name, prop_name_size, Type::Ascii,
                         Internal::cbor_null_str, 4, Type::Null);
      else
        cborEmitAnonymous(Type::Null, Internal::cbor_null_str, 4);
      return Error::NoError;
    }
    if (additional == 25) // float16
    {
      if (pos + 2 > size)
        return Error::CborTruncatedData;
      uint16_t half = ((uint16_t)data[pos] << 8) | (uint16_t)data[pos + 1];
      pos += 2;
      double value = cborDecodeFloat16(half);
      DataRef text = cborDoubleToText(value);
      if (as_property)
        cborEmitProperty(prop_name, prop_name_size, Type::Ascii, text.data, text.size, Type::Number);
      else
        cborEmitAnonymous(Type::Number, text.data, text.size);
      return Error::NoError;
    }
    if (additional == 26) // float32
    {
      if (pos + 4 > size)
        return Error::CborTruncatedData;
      uint32_t bits = ((uint32_t)data[pos] << 24) | ((uint32_t)data[pos + 1] << 16) |
                      ((uint32_t)data[pos + 2] << 8) | (uint32_t)data[pos + 3];
      pos += 4;
      float fval;
      memcpy(&fval, &bits, sizeof(fval));
      DataRef text = cborDoubleToText((double)fval);
      if (as_property)
        cborEmitProperty(prop_name, prop_name_size, Type::Ascii, text.data, text.size, Type::Number);
      else
        cborEmitAnonymous(Type::Number, text.data, text.size);
      return Error::NoError;
    }
    if (additional == 27) // float64
    {
      if (pos + 8 > size)
        return Error::CborTruncatedData;
      uint64_t bits = ((uint64_t)data[pos] << 56) | ((uint64_t)data[pos + 1] << 48) |
                      ((uint64_t)data[pos + 2] << 40) | ((uint64_t)data[pos + 3] << 32) |
                      ((uint64_t)data[pos + 4] << 24) | ((uint64_t)data[pos + 5] << 16) |
                      ((uint64_t)data[pos + 6] << 8) | (uint64_t)data[pos + 7];
      pos += 8;
      double dval;
      memcpy(&dval, &bits, sizeof(dval));
      DataRef text = cborDoubleToText(dval);
      if (as_property)
        cborEmitProperty(prop_name, prop_name_size, Type::Ascii, text.data, text.size, Type::Number);
      else
        cborEmitAnonymous(Type::Number, text.data, text.size);
      return Error::NoError;
    }
    if (additional == 31) // break code (should not appear here)
    {
      return Error::CborInvalidEncoding;
    }
    // Reserved simple values (28-30)
    return Error::CborInvalidEncoding;
  }

  default:
    return Error::CborInvalidEncoding;
  }
}

// -- Map key parser: extracts key as text DataRef --

inline Error Tokenizer::cborParseMapKey(const unsigned char *data, size_t size, size_t &pos,
                                        int depth, DataRef &key_out)
{
  if (pos >= size)
    return Error::CborTruncatedData;

  unsigned char initial = data[pos];
  unsigned char major_type = (initial >> 5) & 0x07;
  unsigned char additional = initial & 0x1F;

  if (major_type == 3) // Text string key (most common)
  {
    pos++;
    if (additional == 31)
    {
      // Indefinite-length text string key
      std::string accumulated;
      while (pos < size)
      {
        if (data[pos] == 0xFF)
        {
          pos++;
          break;
        }
        unsigned char chunk_initial;
        Error err = cborReadByte(data, size, pos, chunk_initial);
        if (err != Error::NoError)
          return err;
        if (((chunk_initial >> 5) & 0x07) != 3)
          return Error::CborInvalidEncoding;
        uint64_t chunk_len;
        err = cborReadArgument(data, size, pos, chunk_initial & 0x1F, chunk_len);
        if (err != Error::NoError)
          return err;
        if (pos + chunk_len > size)
          return Error::CborTruncatedData;
        accumulated.append((const char *)data + pos, (size_t)chunk_len);
        pos += (size_t)chunk_len;
      }
      key_out = cborOwnedRef(accumulated);
    }
    else
    {
      uint64_t len;
      Error err = cborReadArgument(data, size, pos, additional, len);
      if (err != Error::NoError)
        return err;
      if (pos + len > size)
        return Error::CborTruncatedData;
      key_out = cborOwnedRef((const char *)data + pos, (size_t)len);
      pos += (size_t)len;
    }
    return Error::NoError;
  }

  if (major_type == 0) // Unsigned int key → convert to text
  {
    pos++;
    uint64_t value;
    Error err = cborReadArgument(data, size, pos, additional, value);
    if (err != Error::NoError)
      return err;
    key_out = cborUint64ToText(value);
    return Error::NoError;
  }

  if (major_type == 1) // Negative int key → convert to text
  {
    pos++;
    uint64_t raw;
    Error err = cborReadArgument(data, size, pos, additional, raw);
    if (err != Error::NoError)
      return err;
    if (raw > (uint64_t)9223372036854775807ULL)
    {
      key_out = cborOwnedRef("-18446744073709551616", 21);
    }
    else
    {
      int64_t value = -1 - (int64_t)raw;
      key_out = cborInt64ToText(value);
    }
    return Error::NoError;
  }

  // For any other type used as key, skip it and use a placeholder
  // This handles bool keys, null keys, etc.
  (void)depth;
  // We need to consume the item but use it as text
  // Fall back: read it as a generic item but capture before/after position
  // For simplicity, convert common simple types
  if (major_type == 7) // Simple/float as key
  {
    pos++;
    if (additional == 20)
    {
      key_out = DataRef(Internal::cbor_false_str, 5);
      return Error::NoError;
    }
    if (additional == 21)
    {
      key_out = DataRef(Internal::cbor_true_str, 4);
      return Error::NoError;
    }
    if (additional == 22 || additional == 23)
    {
      key_out = DataRef(Internal::cbor_null_str, 4);
      return Error::NoError;
    }
    // Float as key - skip float bytes and use text representation
    if (additional == 25)
    {
      if (pos + 2 > size)
        return Error::CborTruncatedData;
      uint16_t half = ((uint16_t)data[pos] << 8) | (uint16_t)data[pos + 1];
      pos += 2;
      double value = cborDecodeFloat16(half);
      key_out = cborDoubleToText(value);
      return Error::NoError;
    }
    if (additional == 26)
    {
      if (pos + 4 > size)
        return Error::CborTruncatedData;
      uint32_t bits = ((uint32_t)data[pos] << 24) | ((uint32_t)data[pos + 1] << 16) |
                      ((uint32_t)data[pos + 2] << 8) | (uint32_t)data[pos + 3];
      pos += 4;
      float fval;
      memcpy(&fval, &bits, sizeof(fval));
      key_out = cborDoubleToText((double)fval);
      return Error::NoError;
    }
    if (additional == 27)
    {
      if (pos + 8 > size)
        return Error::CborTruncatedData;
      uint64_t bits = ((uint64_t)data[pos] << 56) | ((uint64_t)data[pos + 1] << 48) |
                      ((uint64_t)data[pos + 2] << 40) | ((uint64_t)data[pos + 3] << 32) |
                      ((uint64_t)data[pos + 4] << 24) | ((uint64_t)data[pos + 5] << 16) |
                      ((uint64_t)data[pos + 6] << 8) | (uint64_t)data[pos + 7];
      pos += 8;
      double dval;
      memcpy(&dval, &bits, sizeof(dval));
      key_out = cborDoubleToText(dval);
      return Error::NoError;
    }
    return Error::CborInvalidEncoding;
  }

  // Byte string or other complex types as keys: use placeholder
  key_out = cborOwnedRef("_cbor_key_", 10);
  // Skip the item
  size_t saved_count = cbor_tokens_.size();
  Error err = cborParseItem(data, size, pos, depth, false, nullptr, 0);
  // Remove any tokens emitted by the skipped item
  cbor_tokens_.resize(saved_count);
  return err;
}

// -- Top-level entry point --

inline void Tokenizer::allowCbor(bool allow)
{
  allow_cbor = allow;
}

inline Error Tokenizer::cborParseData(const char *data, size_t size)
{
  cbor_tokens_.clear();
  cbor_owned_strings_.clear();
  cbor_owned_strings_.reserve(64);
  cbor_tokens_.reserve(64);

  size_t pos = 0;
  const unsigned char *udata = (const unsigned char *)data;

  Error err = cborParseItem(udata, size, pos, 0, false, nullptr, 0);
  if (err != Error::NoError)
    return err;

  if (!cbor_tokens_.empty())
  {
    parsed_data_vector = &cbor_tokens_;
    cursor_index = 0;
  }
  return Error::NoError;
}

} // namespace STFY
