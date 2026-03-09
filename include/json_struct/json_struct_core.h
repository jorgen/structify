#pragma once
#include "json_struct_config.h"

namespace JS
{
/*!
 *  \brief Pointer to data
 *
 *  DataRef is used to refere to some data inside a json string. It holds the
 *  start posisition of the data, and its size.
 */
struct DataRef
{
  /*!
   * Constructs a null Dataref pointing to "" with size 0.
   */
  constexpr explicit DataRef()
    : data("")
    , size(0)
  {
  }

  /*!
   * Constructs a DataRef pointing to data and size.
   * \param data points to start of data.
   * \param size size of data.
   */
  constexpr explicit DataRef(const char *data, size_t size)
    : data(data)
    , size(size)
  {
  }

  /*!  Cobstructs a DataRef pointing to an array. This will \b NOT look for
   * the null terminator, but just initialize the DataRef to the size of the
   * array - 1. This function is intended to be used with string literals.
   * \param data  start of the data.
   */
  template <size_t N>
  constexpr explicit DataRef(const char (&data)[N])
    : data(data)
    , size(N - 1)
  {
  }

  explicit DataRef(const std::string &str)
    : data(&str[0])
    , size(str.size())
  {
  }

  explicit DataRef(const char *data)
    : data(data)
    , size(strlen(data))
  {
  }

  const char *data;
  size_t size;
};

enum class Type : unsigned char
{
  Error,
  String,
  Ascii,
  Number,
  ObjectStart,   // ObjectEnd must equal ObjectStart + 1 (see ScopeCounter::handleType)
  ObjectEnd,
  ArrayStart,    // ArrayEnd must equal ArrayStart + 1 (see ScopeCounter::handleType)
  ArrayEnd,
  Bool,
  Null,
  Verbatim,
  // YAML-specific types (appended after Verbatim to preserve ScopeCounter invariant):
  Alias,         // *ref - reference to an anchor
  Directive,     // %YAML, %TAG directives
  DocumentStart, // ---
  DocumentEnd,   // ...
};

struct Token
{
  Token();

  DataRef name;
  DataRef value;
  Type name_type;
  Type value_type;
  // YAML metadata (empty when not applicable):
  DataRef anchor;       // anchor name (without &), set on the value side
  DataRef name_anchor;  // anchor name (without &), set on the key/name side
  DataRef tag;          // tag string (without !), set on tagged values
};

enum class Error : unsigned char
{
  NoError,
  NeedMoreData,
  InvalidToken,
  ExpectedPropertyName,
  ExpectedDelimiter,
  ExpectedDataToken,
  ExpectedObjectStart,
  ExpectedObjectEnd,
  ExpectedArrayStart,
  ExpectedArrayEnd,
  UnexpectedArrayEnd,
  UnexpectedObjectEnd,
  IllegalPropertyName,
  IllegalPropertyType,
  IllegalDataValue,
  EncounteredIllegalChar,
  NodeNotFound,
  MissingPropertyMember,
  MissingFunction,
  FailedToParseBoolean,
  FailedToParseDouble,
  FailedToParseFloat,
  FailedToParseInt,
  UnassignedRequiredMember,
  NonContigiousMemory,
  ScopeHasEnded,
  KeyNotFound,
  DuplicateInSet,
  UnknownError,
  CborTruncatedData,
  CborInvalidEncoding,
  CborNestingTooDeep,
  UserDefinedErrors
};

namespace Internal
{
class ErrorContext
{
public:
  size_t line = 0;
  size_t character = 0;
  Error error = Error::NoError;
  std::string custom_message;
  std::vector<std::string> lines;

  void clear()
  {
    line = 0;
    character = 0;
    error = Error::NoError;
    lines.clear();
  }
};

} // namespace Internal

namespace Internal
{
struct ScopeCounter
{
  JS::Type type;
  uint16_t depth;
  inline void handleType(JS::Type in_type)
  {
    if (type == JS::Type::ArrayStart || type == JS::Type::ObjectStart)
    {
      if (in_type == type)
        depth++;
      else if (in_type == JS::Type(static_cast<int>(type) + 1))
        depth--;
    }
    else
    {
      depth--;
    }
  }
};
} // namespace Internal

namespace Internal
{
static const char *error_strings[] = {
  "NoError",
  "NeedMoreData",
  "InvalidToken",
  "ExpectedPropertyName",
  "ExpectedDelimiter",
  "ExpectedDataToken",
  "ExpectedObjectStart",
  "ExpectedObjectEnd",
  "ExpectedArrayStart",
  "ExpectedArrayEnd",
  "UnexpectedArrayEnd",
  "UnexpectedObjectEnd",
  "IllegalPropertyName",
  "IllegalPropertyType",
  "IllegalDataValue",
  "EncounteredIllegalChar",
  "NodeNotFound",
  "MissingPropertyMember",
  "MissingFunction",
  "FailedToParseBoolean",
  "FailedToParseDouble",
  "FailedToParseFloat",
  "FailedToParseInt",
  "UnassignedRequiredMember",
  "NonContigiousMemory",
  "ScopeHasEnded",
  "KeyNotFound",
  "DuplicateInSet",
  "UnknownError",
  "CborTruncatedData",
  "CborInvalidEncoding",
  "CborNestingTooDeep",
  "UserDefinedErrors",
};
}

} // namespace JS
