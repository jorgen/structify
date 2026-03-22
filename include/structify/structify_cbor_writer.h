#pragma once
#include "structify_core.h"
#include <vector>
#include <cstdint>
#include <cstring>
#include <cmath>

namespace STFY
{

class CborWriter
{
public:
  CborWriter(std::vector<uint8_t> &output)
    : output_(output)
  {
  }

  bool write(const Token &token)
  {
    // Write the key (name) if present — we're in a map context
    if (token.name.size > 0)
    {
      writeTextString(token.name.data, token.name.size);
    }

    switch (token.value_type)
    {
    case Type::ObjectStart:
      output_.push_back(0xbf); // indefinite-length map
      break;
    case Type::ObjectEnd:
      output_.push_back(0xff); // break
      break;
    case Type::ArrayStart:
      output_.push_back(0x9f); // indefinite-length array
      break;
    case Type::ArrayEnd:
      output_.push_back(0xff); // break
      break;
    case Type::String:
    case Type::Ascii:
      writeTextString(token.value.data, token.value.size);
      break;
    case Type::Number:
      writeNumber(token.value.data, token.value.size);
      break;
    case Type::Bool:
      if (token.value.size == 4 && memcmp(token.value.data, "true", 4) == 0)
        output_.push_back(0xf5); // true
      else
        output_.push_back(0xf4); // false
      break;
    case Type::Null:
      output_.push_back(0xf6); // null
      break;
    default:
      // Verbatim and other types — write as text string
      writeTextString(token.value.data, token.value.size);
      break;
    }
    return true;
  }

  // Direct binary encoding methods (avoid text round-trip)
  void writeUint(uint64_t value)
  {
    writeTypeAndValue(0, value); // major type 0
  }

  void writeNegInt(int64_t value)
  {
    // CBOR negative: major type 1, value = -1 - n
    writeTypeAndValue(1, static_cast<uint64_t>(-1 - value));
  }

  void writeInt(int64_t value)
  {
    if (value >= 0)
      writeUint(static_cast<uint64_t>(value));
    else
      writeNegInt(value);
  }

  void writeDouble(double value)
  {
    output_.push_back(0xfb); // major type 7, additional 27 = float64
    uint64_t bits;
    memcpy(&bits, &value, sizeof(bits));
    for (int i = 7; i >= 0; i--)
      output_.push_back(static_cast<uint8_t>((bits >> (i * 8)) & 0xff));
  }

  void writeFloat(float value)
  {
    output_.push_back(0xfa); // major type 7, additional 26 = float32
    uint32_t bits;
    memcpy(&bits, &value, sizeof(bits));
    for (int i = 3; i >= 0; i--)
      output_.push_back(static_cast<uint8_t>((bits >> (i * 8)) & 0xff));
  }

  void writeBool(bool value)
  {
    output_.push_back(value ? 0xf5 : 0xf4);
  }

  void writeNull()
  {
    output_.push_back(0xf6);
  }

  void writeKey(const char *data, size_t size)
  {
    writeTextString(data, size);
  }

private:
  std::vector<uint8_t> &output_;

  void writeTypeAndValue(uint8_t major_type, uint64_t value)
  {
    uint8_t mt = static_cast<uint8_t>(major_type << 5);
    if (value <= 23)
    {
      output_.push_back(mt | static_cast<uint8_t>(value));
    }
    else if (value <= 0xff)
    {
      output_.push_back(mt | 24);
      output_.push_back(static_cast<uint8_t>(value));
    }
    else if (value <= 0xffff)
    {
      output_.push_back(mt | 25);
      output_.push_back(static_cast<uint8_t>((value >> 8) & 0xff));
      output_.push_back(static_cast<uint8_t>(value & 0xff));
    }
    else if (value <= 0xffffffffULL)
    {
      output_.push_back(mt | 26);
      output_.push_back(static_cast<uint8_t>((value >> 24) & 0xff));
      output_.push_back(static_cast<uint8_t>((value >> 16) & 0xff));
      output_.push_back(static_cast<uint8_t>((value >> 8) & 0xff));
      output_.push_back(static_cast<uint8_t>(value & 0xff));
    }
    else
    {
      output_.push_back(mt | 27);
      for (int i = 7; i >= 0; i--)
        output_.push_back(static_cast<uint8_t>((value >> (i * 8)) & 0xff));
    }
  }

  void writeTextString(const char *data, size_t size)
  {
    writeTypeAndValue(3, size); // major type 3 = text string
    output_.insert(output_.end(), reinterpret_cast<const uint8_t *>(data),
                   reinterpret_cast<const uint8_t *>(data) + size);
  }

  void writeNumber(const char *data, size_t size)
  {
    // Try to parse as integer first
    bool is_negative = false;
    size_t i = 0;
    if (i < size && data[i] == '-')
    {
      is_negative = true;
      i++;
    }

    // Check if this is a pure integer (no dot, no e/E)
    bool is_integer = true;
    for (size_t j = i; j < size; j++)
    {
      if (data[j] == '.' || data[j] == 'e' || data[j] == 'E')
      {
        is_integer = false;
        break;
      }
    }

    if (is_integer && i < size)
    {
      uint64_t value = 0;
      bool overflow = false;
      for (size_t j = i; j < size; j++)
      {
        if (data[j] < '0' || data[j] > '9')
        {
          is_integer = false;
          break;
        }
        uint64_t prev = value;
        value = value * 10 + static_cast<uint64_t>(data[j] - '0');
        if (value < prev)
        {
          overflow = true;
          break;
        }
      }
      if (is_integer && !overflow)
      {
        if (is_negative)
        {
          // CBOR negative: major type 1, value = n - 1
          if (value > 0)
            writeTypeAndValue(1, value - 1);
          else
            writeTypeAndValue(0, 0); // -0 → 0
        }
        else
        {
          writeTypeAndValue(0, value);
        }
        return;
      }
    }

    // Parse as double
    // Use a simple text-to-double parse
    double d = 0;
    bool ok = false;
    {
      char *end = nullptr;
      d = strtod(data, &end);
      ok = (end == data + size);
    }
    if (ok)
    {
      writeDouble(d);
    }
    else
    {
      // Fallback: write as text string
      writeTextString(data, size);
    }
  }
};

} // namespace STFY
