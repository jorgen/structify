#pragma once
#include "json_struct_tokenizer.h"

// ============================================================================
// YAML Tokenizer Implementation
// ============================================================================

namespace JS
{

namespace Internal
{
static const char yaml_obj_start[] = "{";
static const char yaml_obj_end[] = "}";
static const char yaml_arr_start[] = "[";
static const char yaml_arr_end[] = "]";
static const char yaml_true_str[] = "true";
static const char yaml_false_str[] = "false";
static const char yaml_null_str[] = "null";
} // namespace Internal

inline std::string &Tokenizer::yamlOwnString(const std::string &str)
{
  yaml_owned_strings_.push_back(str);
  return yaml_owned_strings_.back();
}

inline std::string &Tokenizer::yamlOwnString(const char *data, size_t len)
{
  yaml_owned_strings_.emplace_back(data, len);
  return yaml_owned_strings_.back();
}

inline DataRef Tokenizer::yamlOwnedRef(const std::string &str)
{
  std::string &owned = yamlOwnString(str);
  return DataRef(owned.data(), owned.size());
}

inline DataRef Tokenizer::yamlOwnedRef(const char *data, size_t len)
{
  std::string &owned = yamlOwnString(data, len);
  return DataRef(owned.data(), owned.size());
}

inline void Tokenizer::yamlApplyPendingMetadata(Token &t)
{
  if (yaml_pending_anchor_.size > 0)
  {
    t.anchor = yaml_pending_anchor_;
    yaml_pending_anchor_ = DataRef();
  }
  if (yaml_pending_tag_.size > 0)
  {
    t.tag = yaml_pending_tag_;
    // Apply standard YAML tag type overrides for scalar values
    if (t.value_type != Type::ObjectStart && t.value_type != Type::ObjectEnd &&
        t.value_type != Type::ArrayStart && t.value_type != Type::ArrayEnd)
    {
      const char *tag = yaml_pending_tag_.data;
      size_t tag_len = yaml_pending_tag_.size;
      // Bare '!' (non-specific tag) → treat as string
      if (tag_len == 1 && tag[0] == '!')
      {
        if (t.value_type == Type::Null || t.value_type == Type::Number ||
            t.value_type == Type::Bool)
          t.value_type = Type::String;
      }
      // Check for !!type tags (stored as "!!str", "!!int", etc.)
      else if (tag_len >= 5 && tag[0] == '!' && tag[1] == '!')
      {
        if (tag_len == 5 && tag[2] == 's' && tag[3] == 't' && tag[4] == 'r')
        {
          t.value_type = Type::String; // !!str
          // !!str on null value → empty string
          if (t.value.size == 4 && t.value.data[0] == 'n' && t.value.data[1] == 'u' &&
              t.value.data[2] == 'l' && t.value.data[3] == 'l')
          {
            t.value = DataRef("", 0);
          }
        }
        else if (tag_len == 5 && tag[2] == 'i' && tag[3] == 'n' && tag[4] == 't')
          t.value_type = Type::Number; // !!int
        else if (tag_len == 7 && tag[2] == 'f' && tag[3] == 'l' && tag[4] == 'o' &&
                 tag[5] == 'a' && tag[6] == 't')
          t.value_type = Type::Number; // !!float
        else if (tag_len == 6 && tag[2] == 'b' && tag[3] == 'o' && tag[4] == 'o' &&
                 tag[5] == 'l')
          t.value_type = Type::Bool; // !!bool
        else if (tag_len == 6 && tag[2] == 'n' && tag[3] == 'u' && tag[4] == 'l' &&
                 tag[5] == 'l')
          t.value_type = Type::Null; // !!null
      }
    }
    yaml_pending_tag_ = DataRef();
  }
}

inline void Tokenizer::yamlExtractMetadata(const char *&content, size_t &len)
{
  // Extract &anchor and !tag prefixes from content, setting pending metadata
  while (len > 0)
  {
    if (content[0] == '&')
    {
      // Anchor: &name followed by whitespace or end
      size_t end = 1;
      while (end < len && content[end] != ' ' && content[end] != '\t' &&
             content[end] != '\n' && content[end] != '\r' &&
             content[end] != ',' && content[end] != '}' && content[end] != ']')
        end++;
      if (end > 1)
      {
        yaml_pending_anchor_ = yamlOwnedRef(content + 1, end - 1);
        content += end;
        len -= end;
        // Skip whitespace after anchor
        while (len > 0 && (*content == ' ' || *content == '\t'))
        {
          content++;
          len--;
        }
        continue;
      }
    }
    else if (content[0] == '!')
    {
      // Tag: !, !tag, !!type, or !<uri>
      size_t end = 1;
      if (end < len && content[end] == '!')
        end++; // !!type
      if (end < len && content[end] == '<')
      {
        // Verbatim tag !<uri>
        while (end < len && content[end] != '>')
          end++;
        if (end < len)
          end++; // skip >
      }
      else
      {
        while (end < len && content[end] != ' ' && content[end] != '\t' &&
               content[end] != '\n' && content[end] != '\r' &&
               content[end] != ',' && content[end] != '}' && content[end] != ']')
          end++;
      }
      // end == 1 means bare '!' (non-specific tag) - still valid
      if (end >= 1)
      {
        // For bare '!' (non-specific tag), store "!" to distinguish from "no tag"
        // For others, store tag content after first '!' (e.g., "!str" for !!str)
        yaml_pending_tag_ = yamlOwnedRef(content, end);
        content += end;
        len -= end;
        // Skip whitespace after tag
        while (len > 0 && (*content == ' ' || *content == '\t'))
        {
          content++;
          len--;
        }
        continue;
      }
    }
    break;
  }
}

inline void Tokenizer::yamlEmitAnonymous(Type type, const char *value, size_t value_size)
{
  Token t;
  t.name = DataRef();
  t.name_type = Type::Ascii;
  t.value = DataRef(value, value_size);
  t.value_type = type;
  yamlApplyPendingMetadata(t);
  yaml_tokens_.push_back(t);
}

inline void Tokenizer::yamlEmitProperty(const char *name, size_t name_size, Type name_type,
                                        const char *value, size_t value_size, Type value_type)
{
  Token t;
  t.name = DataRef(name, name_size);
  t.name_type = name_type;
  t.value = DataRef(value, value_size);
  t.value_type = value_type;
  yamlApplyPendingMetadata(t);
  yaml_tokens_.push_back(t);
}

inline void Tokenizer::yamlEmitPropertyWithContainerValue(const char *name, size_t name_size,
                                                          Type name_type, Type container_type)
{
  Token t;
  t.name = DataRef(name, name_size);
  t.name_type = name_type;
  if (container_type == Type::ObjectStart)
  {
    t.value = DataRef(Internal::yaml_obj_start, 1);
    t.value_type = Type::ObjectStart;
  }
  else
  {
    t.value = DataRef(Internal::yaml_arr_start, 1);
    t.value_type = Type::ArrayStart;
  }
  yamlApplyPendingMetadata(t);
  yaml_tokens_.push_back(t);
}

inline size_t Tokenizer::yamlReadLine(const char *data, size_t size, size_t pos,
                                      const char **line_start, size_t *line_len)
{
  *line_start = data + pos;
  size_t end = pos;
  while (end < size && data[end] != '\n')
    end++;
  *line_len = end - pos;
  // Strip trailing \r
  if (*line_len > 0 && (*line_start)[*line_len - 1] == '\r')
    (*line_len)--;
  // Advance past \n
  if (end < size)
    end++;
  return end;
}

inline size_t Tokenizer::yamlMeasureIndent(const char *line, size_t len)
{
  size_t indent = 0;
  while (indent < len && (line[indent] == ' ' || line[indent] == '\t'))
    indent++;
  return indent;
}

inline bool Tokenizer::yamlIsBlankOrComment(const char *line, size_t len)
{
  size_t i = 0;
  while (i < len && (line[i] == ' ' || line[i] == '\t' || line[i] == '\r'))
    i++;
  return i >= len || line[i] == '#';
}

inline void Tokenizer::yamlStripTrailingComment(const char *data, size_t *len)
{
  bool in_double_quote = false;
  bool in_single_quote = false;
  for (size_t i = 0; i < *len; i++)
  {
    if (in_double_quote)
    {
      if (data[i] == '\\' && i + 1 < *len)
        i++; // skip escaped char
      else if (data[i] == '"')
        in_double_quote = false;
    }
    else if (in_single_quote)
    {
      if (data[i] == '\'' && i + 1 < *len && data[i + 1] == '\'')
        i++; // skip escaped single quote
      else if (data[i] == '\'')
        in_single_quote = false;
    }
    else
    {
      if (data[i] == '"')
        in_double_quote = true;
      else if (data[i] == '\'')
        in_single_quote = true;
      else if (data[i] == '#' && i > 0 && (data[i - 1] == ' ' || data[i - 1] == '\t'))
      {
        *len = i - 1;
        // Strip trailing whitespace before comment
        while (*len > 0 && (data[*len - 1] == ' ' || data[*len - 1] == '\t'))
          (*len)--;
        return;
      }
    }
  }
}

inline size_t Tokenizer::yamlFindColon(const char *data, size_t len)
{
  bool in_double_quote = false;
  bool in_single_quote = false;
  for (size_t i = 0; i < len; i++)
  {
    if (in_double_quote)
    {
      if (data[i] == '\\' && i + 1 < len)
        i++;
      else if (data[i] == '"')
        in_double_quote = false;
    }
    else if (in_single_quote)
    {
      if (data[i] == '\'' && i + 1 < len && data[i + 1] == '\'')
        i++;
      else if (data[i] == '\'')
        in_single_quote = false;
    }
    else
    {
      if (data[i] == '"' && i == 0)
        in_double_quote = true;
      else if (data[i] == '\'' && i == 0)
        in_single_quote = true;
      else if (data[i] == ':')
      {
        // Colon followed by space, or colon at end of string
        if (i + 1 >= len || data[i + 1] == ' ' || data[i + 1] == '\t')
          return i;
      }
    }
  }
  return (size_t)-1;
}

inline Type Tokenizer::yamlClassifyScalar(const char *data, size_t size)
{
  if (size == 0)
    return Type::Null;

  // Check for quoted strings
  if (size >= 2 && data[0] == '"' && data[size - 1] == '"')
    return Type::String;
  if (size >= 2 && data[0] == '\'' && data[size - 1] == '\'')
    return Type::String;

  // Check for null
  if (size == 1 && data[0] == '~')
    return Type::Null;
  if (size == 4 && (memcmp(data, "null", 4) == 0 || memcmp(data, "Null", 4) == 0 || memcmp(data, "NULL", 4) == 0))
    return Type::Null;

  // Check for boolean
  if (size == 4 && (memcmp(data, "true", 4) == 0 || memcmp(data, "True", 4) == 0 || memcmp(data, "TRUE", 4) == 0))
    return Type::Bool;
  if (size == 5 &&
      (memcmp(data, "false", 5) == 0 || memcmp(data, "False", 5) == 0 || memcmp(data, "FALSE", 5) == 0))
    return Type::Bool;
  if (size == 3 && (memcmp(data, "yes", 3) == 0 || memcmp(data, "Yes", 3) == 0 || memcmp(data, "YES", 3) == 0))
    return Type::Bool;
  if (size == 2 && (memcmp(data, "no", 2) == 0 || memcmp(data, "No", 2) == 0 || memcmp(data, "NO", 2) == 0))
    return Type::Bool;
  if (size == 2 && (memcmp(data, "on", 2) == 0 || memcmp(data, "On", 2) == 0 || memcmp(data, "ON", 2) == 0))
    return Type::Bool;
  if (size == 3 && (memcmp(data, "off", 3) == 0 || memcmp(data, "Off", 3) == 0 || memcmp(data, "OFF", 3) == 0))
    return Type::Bool;

  // Check for hex number (0x...)
  if (size >= 3 && data[0] == '0' && (data[1] == 'x' || data[1] == 'X'))
  {
    size_t i = 2;
    while (i < size && ((data[i] >= '0' && data[i] <= '9') || (data[i] >= 'a' && data[i] <= 'f') ||
                         (data[i] >= 'A' && data[i] <= 'F')))
      i++;
    if (i == size && i > 2)
      return Type::Number;
  }

  // Check for octal number (0o...)
  if (size >= 3 && data[0] == '0' && (data[1] == 'o' || data[1] == 'O'))
  {
    size_t i = 2;
    while (i < size && data[i] >= '0' && data[i] <= '7')
      i++;
    if (i == size && i > 2)
      return Type::Number;
  }

  // Check for number
  {
    size_t i = 0;
    if (i < size && (data[i] == '+' || data[i] == '-'))
      i++;
    if (i < size && data[i] >= '0' && data[i] <= '9')
    {
      while (i < size && data[i] >= '0' && data[i] <= '9')
        i++;
      if (i < size && data[i] == '.')
      {
        i++;
        while (i < size && data[i] >= '0' && data[i] <= '9')
          i++;
      }
      if (i < size && (data[i] == 'e' || data[i] == 'E'))
      {
        i++;
        if (i < size && (data[i] == '+' || data[i] == '-'))
          i++;
        while (i < size && data[i] >= '0' && data[i] <= '9')
          i++;
      }
      if (i == size)
        return Type::Number;
    }
    // Also handle ".5" style numbers
    if (size >= 2 && data[0] == '.' && data[1] >= '0' && data[1] <= '9')
    {
      i = 1;
      while (i < size && data[i] >= '0' && data[i] <= '9')
        i++;
      if (i == size)
        return Type::Number;
    }
  }

  return Type::Ascii;
}

static int yamlHexVal(char c)
{
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  return -1;
}

static void yamlAppendUtf8(std::string &out, uint32_t cp)
{
  if (cp <= 0x7F)
  {
    out += (char)cp;
  }
  else if (cp <= 0x7FF)
  {
    out += (char)(0xC0 | (cp >> 6));
    out += (char)(0x80 | (cp & 0x3F));
  }
  else if (cp <= 0xFFFF)
  {
    out += (char)(0xE0 | (cp >> 12));
    out += (char)(0x80 | ((cp >> 6) & 0x3F));
    out += (char)(0x80 | (cp & 0x3F));
  }
  else if (cp <= 0x10FFFF)
  {
    out += (char)(0xF0 | (cp >> 18));
    out += (char)(0x80 | ((cp >> 12) & 0x3F));
    out += (char)(0x80 | ((cp >> 6) & 0x3F));
    out += (char)(0x80 | (cp & 0x3F));
  }
}

// Helper: fold newlines in quoted strings. Processes a sequence of newlines starting at inner[i].
// Returns the new index (pointing at last consumed character).
static size_t yamlFoldQuotedNewlines(const char *inner, size_t inner_len, size_t i, std::string &result)
{
  // Trim trailing whitespace before the newline
  while (!result.empty() && (result.back() == ' ' || result.back() == '\t'))
    result.pop_back();

  // Count consecutive line breaks (including intervening whitespace-only lines)
  int newline_count = 0;
  while (i < inner_len && (inner[i] == '\n' || inner[i] == '\r'))
  {
    // Skip \r\n pair
    if (inner[i] == '\r' && i + 1 < inner_len && inner[i + 1] == '\n')
      i++;
    newline_count++;
    i++;
    // Skip whitespace (indentation of next line)
    while (i < inner_len && (inner[i] == ' ' || inner[i] == '\t'))
      i++;
  }

  if (newline_count >= 2)
  {
    // n line breaks -> n-1 literal newlines
    for (int j = 0; j < newline_count - 1; j++)
      result += '\n';
  }
  else
  {
    // Single line break -> space
    result += ' ';
  }

  return i - 1; // -1 because the for loop will increment
}

// Accumulate a multiline plain scalar. first_text/first_len is the text of the first line.
// data/size/pos are the full buffer (pos is after the first line).
// min_indent is the minimum indent for continuation lines (lines must have indent > min_indent).
// Returns the accumulated string with proper folding.
inline std::string Tokenizer::yamlAccumulateMultilinePlainScalar(
    const char *first_text, size_t first_len,
    const char *data, size_t size, size_t &pos, int min_indent)
{
  std::string result(first_text, first_len);
  // Strip trailing whitespace from first line
  while (!result.empty() && (result.back() == ' ' || result.back() == '\t'))
    result.pop_back();

  int blank_count = 0;

  while (pos < size)
  {
    size_t save_pos = pos;
    const char *line_start;
    size_t line_len;
    size_t next_pos = yamlReadLine(data, size, pos, &line_start, &line_len);

    // Check for blank line
    if (yamlIsBlankOrComment(line_start, line_len))
    {
      // But only count blank (not comment) lines for folding purposes
      bool is_pure_blank = true;
      for (size_t bi = 0; bi < line_len; bi++)
      {
        if (line_start[bi] != ' ' && line_start[bi] != '\t' && line_start[bi] != '\r')
        {
          is_pure_blank = false;
          break;
        }
      }
      if (is_pure_blank)
        blank_count++;
      else
      {
        // Comment line terminates plain scalar
        pos = save_pos;
        break;
      }
      pos = next_pos;
      continue;
    }

    // Check for document markers at column 0
    if (line_len >= 3)
    {
      if ((line_start[0] == '-' && line_start[1] == '-' && line_start[2] == '-') ||
          (line_start[0] == '.' && line_start[1] == '.' && line_start[2] == '.'))
      {
        if (line_len == 3 || line_start[3] == ' ' || line_start[3] == '\t' || line_start[3] == '\r')
        {
          pos = save_pos;
          break;
        }
      }
    }

    int indent = (int)yamlMeasureIndent(line_start, line_len);
    if (indent <= min_indent)
    {
      pos = save_pos;
      break;
    }

    const char *content = line_start + indent;
    size_t content_len = line_len - indent;
    // Strip trailing whitespace/\r
    while (content_len > 0 && (content[content_len - 1] == ' ' || content[content_len - 1] == '\t' ||
           content[content_len - 1] == '\r'))
      content_len--;

    // Stop at indicators that start new structures
    // Note: "- " is NOT checked here because in a plain scalar's multiline continuation,
    // "- " at deeper indent is literal text, not a sequence indicator.
    if (content_len >= 1 && (content[0] == '|' || content[0] == '>'))
    {
      pos = save_pos;
      break;
    }
    if (content_len >= 1 && (content[0] == '{' || content[0] == '['))
    {
      pos = save_pos;
      break;
    }
    // Check if this line is a mapping key (has an unquoted colon)
    size_t colon_pos = yamlFindColon(content, content_len);
    if (colon_pos != (size_t)-1)
    {
      pos = save_pos;
      break;
    }

    // Strip trailing comment from content
    yamlStripTrailingComment(content, &content_len);

    // Fold: blank lines → newlines, single line break → space
    if (blank_count > 0)
    {
      for (int b = 0; b < blank_count; b++)
        result += '\n';
      blank_count = 0;
    }
    else
    {
      result += ' ';
    }
    result.append(content, content_len);
    pos = next_pos;
  }

  return result;
}

// Find the end of a quoted string starting at data[start] (which is " or ').
// Returns position after the closing quote, or (size_t)-1 if not found.
static size_t yamlFindQuoteEnd(const char *data, size_t size, size_t start)
{
  char quote = data[start];
  size_t i = start + 1;
  while (i < size)
  {
    if (data[i] == quote)
    {
      if (quote == '\'' && i + 1 < size && data[i + 1] == '\'')
      {
        i += 2;
        continue;
      }
      return i + 1;
    }
    if (quote == '"' && data[i] == '\\' && i + 1 < size)
    {
      i += 2;
      continue;
    }
    i++;
  }
  return (size_t)-1;
}

inline DataRef Tokenizer::yamlNormalizeScalar(const char *data, size_t size, Type type)
{
  if (type == Type::Null)
  {
    return DataRef(Internal::yaml_null_str, 4);
  }
  if (type == Type::Bool)
  {
    // Determine truthiness
    bool is_true = false;
    if (size == 4 && (memcmp(data, "true", 4) == 0 || memcmp(data, "True", 4) == 0 || memcmp(data, "TRUE", 4) == 0))
      is_true = true;
    else if (size == 3 &&
             (memcmp(data, "yes", 3) == 0 || memcmp(data, "Yes", 3) == 0 || memcmp(data, "YES", 3) == 0))
      is_true = true;
    else if (size == 2 &&
             (memcmp(data, "on", 2) == 0 || memcmp(data, "On", 2) == 0 || memcmp(data, "ON", 2) == 0))
      is_true = true;

    if (is_true)
      return DataRef(Internal::yaml_true_str, 4);
    else
      return DataRef(Internal::yaml_false_str, 5);
  }
  if (type == Type::Number)
  {
    // Convert hex (0x...) and octal (0o...) to decimal for JSON compatibility
    if (size >= 3 && data[0] == '0' && (data[1] == 'x' || data[1] == 'X'))
    {
      unsigned long long val = 0;
      for (size_t i = 2; i < size; i++)
      {
        val *= 16;
        if (data[i] >= '0' && data[i] <= '9')
          val += (unsigned long long)(data[i] - '0');
        else if (data[i] >= 'a' && data[i] <= 'f')
          val += (unsigned long long)(data[i] - 'a' + 10);
        else if (data[i] >= 'A' && data[i] <= 'F')
          val += (unsigned long long)(data[i] - 'A' + 10);
      }
      char buf[32];
      int len = snprintf(buf, sizeof(buf), "%llu", val);
      return yamlOwnedRef(buf, (size_t)len);
    }
    if (size >= 3 && data[0] == '0' && (data[1] == 'o' || data[1] == 'O'))
    {
      unsigned long long val = 0;
      for (size_t i = 2; i < size; i++)
        val = val * 8 + (unsigned long long)(data[i] - '0');
      char buf[32];
      int len = snprintf(buf, sizeof(buf), "%llu", val);
      return yamlOwnedRef(buf, (size_t)len);
    }
    return DataRef(data, size);
  }
  if (type == Type::String)
  {
    // Strip quotes and handle escapes
    if (size >= 2 && data[0] == '"' && data[size - 1] == '"')
    {
      const char *inner = data + 1;
      size_t inner_len = size - 2;
      // Check if any escape sequences or literal newlines exist
      bool needs_processing = false;
      for (size_t i = 0; i < inner_len; i++)
      {
        if (inner[i] == '\\' || inner[i] == '\n' || inner[i] == '\r')
        {
          needs_processing = true;
          break;
        }
      }
      if (needs_processing)
      {
        std::string result;
        result.reserve(inner_len);
        for (size_t i = 0; i < inner_len; i++)
        {
          if (inner[i] == '\\' && i + 1 < inner_len)
          {
            i++;
            switch (inner[i])
            {
            case 'n':
              result += '\n';
              break;
            case 't':
              result += '\t';
              break;
            case 'r':
              result += '\r';
              break;
            case '\\':
              result += '\\';
              break;
            case '"':
              result += '"';
              break;
            case '/':
              result += '/';
              break;
            case '0':
              result += '\0';
              break;
            case ' ':
              result += ' ';
              break;
            case 'a':
              result += '\a';
              break;
            case 'b':
              result += '\b';
              break;
            case 'v':
              result += '\v';
              break;
            case 'e':
              result += '\x1B';
              break;
            case '_':
              yamlAppendUtf8(result, 0xA0);
              break; // NBSP
            case 'N':
              yamlAppendUtf8(result, 0x85);
              break; // NEL
            case 'L':
              yamlAppendUtf8(result, 0x2028);
              break; // LS
            case 'P':
              yamlAppendUtf8(result, 0x2029);
              break; // PS
            case 'x':
            {
              // \xNN - 2 hex digits
              uint32_t cp = 0;
              for (int h = 0; h < 2 && i + 1 < inner_len; h++)
              {
                int v = yamlHexVal(inner[i + 1]);
                if (v < 0)
                  break;
                cp = (cp << 4) | (uint32_t)v;
                i++;
              }
              yamlAppendUtf8(result, cp);
              break;
            }
            case 'u':
            {
              // \uNNNN - 4 hex digits
              uint32_t cp = 0;
              for (int h = 0; h < 4 && i + 1 < inner_len; h++)
              {
                int v = yamlHexVal(inner[i + 1]);
                if (v < 0)
                  break;
                cp = (cp << 4) | (uint32_t)v;
                i++;
              }
              yamlAppendUtf8(result, cp);
              break;
            }
            case 'U':
            {
              // \UNNNNNNNN - 8 hex digits
              uint32_t cp = 0;
              for (int h = 0; h < 8 && i + 1 < inner_len; h++)
              {
                int v = yamlHexVal(inner[i + 1]);
                if (v < 0)
                  break;
                cp = (cp << 4) | (uint32_t)v;
                i++;
              }
              yamlAppendUtf8(result, cp);
              break;
            }
            case '\r':
              // Escaped \r\n or bare \r - line joining (eliminate break + leading whitespace on next line)
              if (i + 1 < inner_len && inner[i + 1] == '\n')
                i++;
              while (i + 1 < inner_len && (inner[i + 1] == ' ' || inner[i + 1] == '\t'))
                i++;
              break;
            case '\n':
              // Escaped newline - line joining (eliminate break + leading whitespace on next line)
              while (i + 1 < inner_len && (inner[i + 1] == ' ' || inner[i + 1] == '\t'))
                i++;
              break;
            default:
              result += '\\';
              result += inner[i];
              break;
            }
          }
          else if (inner[i] == '\n' || inner[i] == '\r')
          {
            i = yamlFoldQuotedNewlines(inner, inner_len, i, result);
          }
          else
          {
            result += inner[i];
          }
        }
        return yamlOwnedRef(result);
      }
      else
      {
        return DataRef(inner, inner_len);
      }
    }
    else if (size >= 2 && data[0] == '\'' && data[size - 1] == '\'')
    {
      const char *inner = data + 1;
      size_t inner_len = size - 2;
      // In single-quoted YAML, '' is escape for '. Also fold multi-line.
      bool needs_processing = false;
      for (size_t i = 0; i < inner_len; i++)
      {
        if ((inner[i] == '\'' && i + 1 < inner_len && inner[i + 1] == '\'') ||
            inner[i] == '\n' || inner[i] == '\r')
        {
          needs_processing = true;
          break;
        }
      }
      if (needs_processing)
      {
        std::string result;
        result.reserve(inner_len);
        for (size_t i = 0; i < inner_len; i++)
        {
          if (inner[i] == '\'' && i + 1 < inner_len && inner[i + 1] == '\'')
          {
            result += '\'';
            i++;
          }
          else if (inner[i] == '\n' || inner[i] == '\r')
          {
            i = yamlFoldQuotedNewlines(inner, inner_len, i, result);
          }
          else
          {
            result += inner[i];
          }
        }
        return yamlOwnedRef(result);
      }
      else
      {
        return DataRef(inner, inner_len);
      }
    }
  }
  // For Number and Ascii, return as-is pointing into source
  return DataRef(data, size);
}

inline void Tokenizer::yamlSkipFlowWhitespace(const char *data, size_t size, size_t &pos)
{
  while (pos < size)
  {
    if (data[pos] == ' ' || data[pos] == '\t' || data[pos] == '\r' || data[pos] == '\n')
    {
      pos++;
    }
    else if (data[pos] == '#')
    {
      // Skip comment to end of line
      while (pos < size && data[pos] != '\n')
        pos++;
    }
    else
    {
      break;
    }
  }
}

inline void Tokenizer::yamlParseFlowScalar(const char *data, size_t size, size_t &pos,
                                           const char **out_data, size_t *out_len, Type *out_type)
{
  yamlSkipFlowWhitespace(data, size, pos);
  if (pos >= size)
  {
    *out_data = nullptr;
    *out_len = 0;
    *out_type = Type::Error;
    return;
  }

  if (data[pos] == '"')
  {
    // Double-quoted string
    size_t start = pos;
    pos++; // skip opening quote
    while (pos < size)
    {
      if (data[pos] == '\\' && pos + 1 < size)
        pos += 2;
      else if (data[pos] == '"')
      {
        pos++; // skip closing quote
        break;
      }
      else
        pos++;
    }
    *out_data = data + start;
    *out_len = pos - start;
    *out_type = Type::String;
  }
  else if (data[pos] == '\'')
  {
    // Single-quoted string
    size_t start = pos;
    pos++; // skip opening quote
    while (pos < size)
    {
      if (data[pos] == '\'' && pos + 1 < size && data[pos + 1] == '\'')
        pos += 2;
      else if (data[pos] == '\'')
      {
        pos++; // skip closing quote
        break;
      }
      else
        pos++;
    }
    *out_data = data + start;
    *out_len = pos - start;
    *out_type = Type::String;
  }
  else
  {
    // Unquoted scalar - terminated by , } ] (not newline - multiline allowed in flow)
    // Per YAML spec, ':' only terminates when followed by whitespace, flow indicator, or end of input
    size_t start = pos;
    std::string folded;
    bool multiline = false;
    while (pos < size && data[pos] != ',' && data[pos] != '}' && data[pos] != ']')
    {
      if (data[pos] == ':')
      {
        if (pos + 1 >= size || data[pos + 1] == ' ' || data[pos + 1] == '\t' ||
            data[pos + 1] == ',' || data[pos + 1] == '{' || data[pos + 1] == '}' ||
            data[pos + 1] == '[' || data[pos + 1] == ']' ||
            data[pos + 1] == '\n' || data[pos + 1] == '\r')
          break;
      }
      if (data[pos] == '#' && pos > start && data[pos - 1] == ' ')
        break;
      if (data[pos] == '\n' || data[pos] == '\r')
      {
        // Fold the current text and continue on next line
        if (!multiline)
        {
          // Copy what we have so far into the folded string
          folded.assign(data + start, pos - start);
          // Trim trailing whitespace
          while (!folded.empty() && (folded.back() == ' ' || folded.back() == '\t'))
            folded.pop_back();
          multiline = true;
        }
        // Skip whitespace/newlines/comments
        yamlSkipFlowWhitespace(data, size, pos);
        // Check if next char is a flow terminator
        if (pos >= size || data[pos] == ',' || data[pos] == '}' || data[pos] == ']')
          break;
        // Check if next char is ':' indicating a mapping key
        if (data[pos] == ':' &&
            (pos + 1 >= size || data[pos + 1] == ' ' || data[pos + 1] == '\t' ||
             data[pos + 1] == ',' || data[pos + 1] == '{' || data[pos + 1] == '}' ||
             data[pos + 1] == '[' || data[pos + 1] == ']' ||
             data[pos + 1] == '\n' || data[pos + 1] == '\r'))
          break;
        folded += ' '; // fold newline to space
        // Continue scanning from current pos (start of next logical content)
        size_t line_start = pos;
        while (pos < size && data[pos] != ',' && data[pos] != '}' && data[pos] != ']' &&
               data[pos] != '\n' && data[pos] != '\r')
        {
          if (data[pos] == ':')
          {
            if (pos + 1 >= size || data[pos + 1] == ' ' || data[pos + 1] == '\t' ||
                data[pos + 1] == ',' || data[pos + 1] == '{' || data[pos + 1] == '}' ||
                data[pos + 1] == '[' || data[pos + 1] == ']' ||
                data[pos + 1] == '\n' || data[pos + 1] == '\r')
              break;
          }
          if (data[pos] == '#' && pos > line_start && data[pos - 1] == ' ')
            break;
          pos++;
        }
        size_t seg_len = pos - line_start;
        while (seg_len > 0 && (data[line_start + seg_len - 1] == ' ' || data[line_start + seg_len - 1] == '\t'))
          seg_len--;
        folded.append(data + line_start, seg_len);
        continue; // re-check for more newlines
      }
      pos++;
    }

    if (multiline)
    {
      // Trim trailing whitespace from folded result
      while (!folded.empty() && (folded.back() == ' ' || folded.back() == '\t'))
        folded.pop_back();
      DataRef owned = yamlOwnedRef(folded);
      *out_data = owned.data;
      *out_len = owned.size;
      *out_type = yamlClassifyScalar(*out_data, *out_len);
    }
    else
    {
      *out_len = pos - start;
      // Trim trailing whitespace
      while (*out_len > 0 && (data[start + *out_len - 1] == ' ' || data[start + *out_len - 1] == '\t'))
        (*out_len)--;
      *out_data = data + start;
      *out_type = yamlClassifyScalar(*out_data, *out_len);
    }
  }
}

inline Error Tokenizer::yamlParseFlowObjectInner(const char *data, size_t size, size_t &pos)
{
  // Parse contents of a flow object (between { and })
  // pos should be right after '{'
  yamlSkipFlowWhitespace(data, size, pos);
  while (pos < size && data[pos] != '}')
  {
    // Extract metadata (anchors, tags) from key position
    while (pos < size && (data[pos] == '&' || data[pos] == '!'))
    {
      const char *meta_start = data + pos;
      size_t meta_remaining = size - pos;
      yamlExtractMetadata(meta_start, meta_remaining);
      pos = (size_t)(meta_start - data);
      yamlSkipFlowWhitespace(data, size, pos);
    }

    // Parse key
    const char *key_data;
    size_t key_len;
    Type key_type;
    yamlParseFlowScalar(data, size, pos, &key_data, &key_len, &key_type);

    // Handle quoted key - normalize to handle escape sequences and multiline folding
    Type emit_key_type = Type::Ascii;
    const char *emit_key_data = key_data;
    size_t emit_key_len = key_len;
    if (key_type == Type::String && key_len >= 2)
    {
      DataRef kr = yamlNormalizeScalar(key_data, key_len, Type::String);
      emit_key_data = kr.data;
      emit_key_len = kr.size;
      emit_key_type = Type::String;
    }

    yamlSkipFlowWhitespace(data, size, pos);
    if (pos < size && data[pos] == ':')
    {
      pos++; // skip colon
      yamlSkipFlowWhitespace(data, size, pos);
    }

    // Extract metadata (anchors, tags) from value position
    while (pos < size && (data[pos] == '&' || data[pos] == '!'))
    {
      const char *meta_start = data + pos;
      size_t meta_remaining = size - pos;
      yamlExtractMetadata(meta_start, meta_remaining);
      pos = (size_t)(meta_start - data);
      yamlSkipFlowWhitespace(data, size, pos);
    }

    // Parse value
    if (pos < size && data[pos] == '*')
    {
      // Alias as value
      pos++; // skip *
      size_t start = pos;
      while (pos < size && data[pos] != ',' && data[pos] != '}' && data[pos] != ']' &&
             data[pos] != ' ' && data[pos] != '\t' && data[pos] != '\n' && data[pos] != '\r')
        pos++;
      DataRef alias_name = yamlOwnedRef(data + start, pos - start);
      yamlEmitProperty(emit_key_data, emit_key_len, emit_key_type, alias_name.data, alias_name.size, Type::Alias);
    }
    else if (pos < size && data[pos] == '{')
    {
      yamlEmitPropertyWithContainerValue(emit_key_data, emit_key_len, emit_key_type, Type::ObjectStart);
      pos++; // skip {
      yamlParseFlowObjectInner(data, size, pos);
      if (pos < size && data[pos] == '}')
        pos++;
    }
    else if (pos < size && data[pos] == '[')
    {
      yamlEmitPropertyWithContainerValue(emit_key_data, emit_key_len, emit_key_type, Type::ArrayStart);
      pos++; // skip [
      yamlParseFlowArrayInner(data, size, pos);
      if (pos < size && data[pos] == ']')
        pos++;
    }
    else
    {
      const char *val_data;
      size_t val_len;
      Type val_type;
      yamlParseFlowScalar(data, size, pos, &val_data, &val_len, &val_type);
      DataRef val_ref = yamlNormalizeScalar(val_data, val_len, val_type);
      yamlEmitProperty(emit_key_data, emit_key_len, emit_key_type, val_ref.data, val_ref.size, val_type);
    }

    yamlSkipFlowWhitespace(data, size, pos);
    if (pos < size && data[pos] == ',')
    {
      pos++;
      yamlSkipFlowWhitespace(data, size, pos);
    }
  }
  yamlEmitAnonymous(Type::ObjectEnd, Internal::yaml_obj_end, 1);
  return Error::NoError;
}

inline void Tokenizer::yamlParseFlowImplicitMapValue(const char *data, size_t size, size_t &pos,
                                                      const char *key_data, size_t key_len, Type key_type)
{
  // Emit an implicit mapping inside a flow sequence: {key: value}
  Type emit_key_type = Type::Ascii;
  const char *emit_key_data = key_data;
  size_t emit_key_len = key_len;
  if (key_type == Type::String && key_len >= 2)
  {
    DataRef kr = yamlNormalizeScalar(key_data, key_len, Type::String);
    emit_key_data = kr.data;
    emit_key_len = kr.size;
    emit_key_type = Type::String;
  }

  yamlEmitAnonymous(Type::ObjectStart, Internal::yaml_obj_start, 1);
  pos++; // skip ':'
  yamlSkipFlowWhitespace(data, size, pos);

  // Extract metadata (anchors, tags) from value
  while (pos < size && (data[pos] == '&' || data[pos] == '!'))
  {
    const char *meta_start = data + pos;
    size_t meta_remaining = size - pos;
    yamlExtractMetadata(meta_start, meta_remaining);
    pos = (size_t)(meta_start - data);
    yamlSkipFlowWhitespace(data, size, pos);
  }

  if (pos >= size || data[pos] == ',' || data[pos] == ']' || data[pos] == '}')
  {
    yamlEmitProperty(emit_key_data, emit_key_len, emit_key_type,
                     Internal::yaml_null_str, 4, Type::Null);
  }
  else if (data[pos] == '*')
  {
    // Alias as value
    pos++; // skip *
    size_t start = pos;
    while (pos < size && data[pos] != ',' && data[pos] != '}' && data[pos] != ']' &&
           data[pos] != ' ' && data[pos] != '\t' && data[pos] != '\n' && data[pos] != '\r')
      pos++;
    DataRef alias_name = yamlOwnedRef(data + start, pos - start);
    yamlEmitProperty(emit_key_data, emit_key_len, emit_key_type,
                     alias_name.data, alias_name.size, Type::Alias);
  }
  else if (data[pos] == '{')
  {
    yamlEmitPropertyWithContainerValue(emit_key_data, emit_key_len, emit_key_type, Type::ObjectStart);
    pos++;
    yamlParseFlowObjectInner(data, size, pos);
    if (pos < size && data[pos] == '}')
      pos++;
  }
  else if (data[pos] == '[')
  {
    yamlEmitPropertyWithContainerValue(emit_key_data, emit_key_len, emit_key_type, Type::ArrayStart);
    pos++;
    yamlParseFlowArrayInner(data, size, pos);
    if (pos < size && data[pos] == ']')
      pos++;
  }
  else
  {
    const char *v_data;
    size_t v_len;
    Type v_type;
    yamlParseFlowScalar(data, size, pos, &v_data, &v_len, &v_type);
    DataRef v_ref = yamlNormalizeScalar(v_data, v_len, v_type);
    yamlEmitProperty(emit_key_data, emit_key_len, emit_key_type,
                     v_ref.data, v_ref.size, v_type);
  }
  yamlEmitAnonymous(Type::ObjectEnd, Internal::yaml_obj_end, 1);
}

inline Error Tokenizer::yamlParseFlowArrayInner(const char *data, size_t size, size_t &pos)
{
  // Parse contents of a flow array (between [ and ])
  yamlSkipFlowWhitespace(data, size, pos);
  while (pos < size && data[pos] != ']')
  {
    // Extract metadata (anchors, tags) before value
    while (pos < size && (data[pos] == '&' || data[pos] == '!'))
    {
      const char *meta_start = data + pos;
      size_t meta_remaining = size - pos;
      yamlExtractMetadata(meta_start, meta_remaining);
      pos = (size_t)(meta_start - data);
      yamlSkipFlowWhitespace(data, size, pos);
    }

    if (pos < size && data[pos] == '*')
    {
      // Alias in flow array
      pos++; // skip *
      size_t start = pos;
      while (pos < size && data[pos] != ',' && data[pos] != '}' && data[pos] != ']' &&
             data[pos] != ' ' && data[pos] != '\t' && data[pos] != '\n' && data[pos] != '\r')
        pos++;
      DataRef alias_name = yamlOwnedRef(data + start, pos - start);
      yamlEmitAnonymous(Type::Alias, alias_name.data, alias_name.size);
    }
    else if (pos < size && data[pos] == '{')
    {
      yamlEmitAnonymous(Type::ObjectStart, Internal::yaml_obj_start, 1);
      pos++; // skip {
      yamlParseFlowObjectInner(data, size, pos);
      if (pos < size && data[pos] == '}')
        pos++;

      // Check if this flow object is a key in an implicit mapping
      yamlSkipFlowWhitespace(data, size, pos);
      if (pos < size && data[pos] == ':')
      {
        // Flow object as key of implicit mapping - skip the value for now
        // (complex key support is limited)
        pos++; // skip ':'
        yamlSkipFlowWhitespace(data, size, pos);
        if (pos < size && data[pos] != ',' && data[pos] != ']')
        {
          const char *v_data;
          size_t v_len;
          Type v_type;
          yamlParseFlowScalar(data, size, pos, &v_data, &v_len, &v_type);
          // Discard the value - we already emitted the object without wrapping
        }
      }
    }
    else if (pos < size && data[pos] == '[')
    {
      yamlEmitAnonymous(Type::ArrayStart, Internal::yaml_arr_start, 1);
      pos++; // skip [
      yamlParseFlowArrayInner(data, size, pos);
      if (pos < size && data[pos] == ']')
        pos++;
    }
    else if (pos < size && data[pos] == ':' &&
             (pos + 1 >= size || data[pos + 1] == ' ' || data[pos + 1] == '\t' ||
              data[pos + 1] == ',' || data[pos + 1] == '}' || data[pos + 1] == ']' ||
              data[pos + 1] == '\n' || data[pos + 1] == '\r'))
    {
      // Empty key implicit mapping: [: value]
      static const char empty_key[] = "";
      yamlParseFlowImplicitMapValue(data, size, pos, empty_key, 0, Type::Ascii);
    }
    else if (pos < size && data[pos] == '?' &&
             (pos + 1 >= size || data[pos + 1] == ' ' || data[pos + 1] == '\t' ||
              data[pos + 1] == '\n' || data[pos + 1] == '\r'))
    {
      // Explicit key in flow sequence: [? key : value]
      pos++; // skip ?
      yamlSkipFlowWhitespace(data, size, pos);
      const char *key_data;
      size_t key_len;
      Type key_type;
      yamlParseFlowScalar(data, size, pos, &key_data, &key_len, &key_type);
      yamlSkipFlowWhitespace(data, size, pos);
      // Expect ':'
      if (pos < size && data[pos] == ':')
      {
        yamlParseFlowImplicitMapValue(data, size, pos, key_data, key_len, key_type);
      }
      else
      {
        // ? key without : value - emit as mapping with null value
        yamlEmitAnonymous(Type::ObjectStart, Internal::yaml_obj_start, 1);
        Type emit_key_type = Type::Ascii;
        const char *emit_key_data = key_data;
        size_t emit_key_len = key_len;
        if (key_type == Type::String && key_len >= 2)
        {
          DataRef kr = yamlNormalizeScalar(key_data, key_len, Type::String);
          emit_key_data = kr.data;
          emit_key_len = kr.size;
          emit_key_type = Type::String;
        }
        yamlEmitProperty(emit_key_data, emit_key_len, emit_key_type,
                         Internal::yaml_null_str, 4, Type::Null);
        yamlEmitAnonymous(Type::ObjectEnd, Internal::yaml_obj_end, 1);
      }
    }
    else
    {
      const char *val_data;
      size_t val_len;
      Type val_type;
      yamlParseFlowScalar(data, size, pos, &val_data, &val_len, &val_type);

      // Check for implicit mapping (scalar followed by ':')
      yamlSkipFlowWhitespace(data, size, pos);
      if (pos < size && data[pos] == ':')
      {
        yamlParseFlowImplicitMapValue(data, size, pos, val_data, val_len, val_type);
      }
      else
      {
        DataRef val_ref = yamlNormalizeScalar(val_data, val_len, val_type);
        yamlEmitAnonymous(val_type, val_ref.data, val_ref.size);
      }
    }

    yamlSkipFlowWhitespace(data, size, pos);
    if (pos < size && data[pos] == ',')
    {
      pos++;
      yamlSkipFlowWhitespace(data, size, pos);
    }
  }
  yamlEmitAnonymous(Type::ArrayEnd, Internal::yaml_arr_end, 1);
  return Error::NoError;
}

inline Error Tokenizer::yamlParseFlowObject(const char *data, size_t size, size_t &pos)
{
  // pos points to '{'
  yamlEmitAnonymous(Type::ObjectStart, Internal::yaml_obj_start, 1);
  pos++; // skip {
  yamlParseFlowObjectInner(data, size, pos);
  if (pos < size && data[pos] == '}')
    pos++;
  return Error::NoError;
}

inline Error Tokenizer::yamlParseFlowArray(const char *data, size_t size, size_t &pos)
{
  // pos points to '['
  yamlEmitAnonymous(Type::ArrayStart, Internal::yaml_arr_start, 1);
  pos++; // skip [
  yamlParseFlowArrayInner(data, size, pos);
  if (pos < size && data[pos] == ']')
    pos++;
  return Error::NoError;
}

inline void Tokenizer::yamlParseBlockScalarHeader(const char *header, size_t header_len,
                                                   char *indicator, int *chomp_mode, int *explicit_indent)
{
  *indicator = header[0]; // '|' or '>'
  *chomp_mode = 0;        // clip (default)
  *explicit_indent = -1;  // auto-detect
  for (size_t i = 1; i < header_len; i++)
  {
    if (header[i] == '-')
      *chomp_mode = 1; // strip
    else if (header[i] == '+')
      *chomp_mode = 2; // keep
    else if (header[i] >= '1' && header[i] <= '9')
      *explicit_indent = header[i] - '0';
  }
}

inline Error Tokenizer::yamlParseMultilineScalar(const char *data, size_t size, size_t &pos, char indicator)
{
  return yamlParseMultilineScalarEx(data, size, pos, indicator, 0, -1);
}

inline Error Tokenizer::yamlParseMultilineScalarEx(const char *data, size_t size, size_t &pos,
                                                    char indicator, int chomp_mode, int explicit_indent,
                                                    int context_indent)
{
  // chomp_mode: 0=clip (default), 1=strip (-), 2=keep (+)
  // explicit_indent: -1=auto-detect, >0=explicit indent width (relative to context_indent)
  // context_indent: indentation level of the parent context
  // pos should be at the line AFTER the indicator line
  int block_indent = explicit_indent > 0 ? (context_indent + explicit_indent) : -1;

  // Pre-scan to find block_indent when auto-detecting, so we can correctly
  // classify whitespace-only lines that have content beyond block_indent
  if (block_indent < 0)
  {
    size_t scan_pos = pos;
    while (scan_pos < size)
    {
      const char *sl;
      size_t sll;
      size_t snp = yamlReadLine(data, size, scan_pos, &sl, &sll);
      bool sb = true;
      for (size_t si = 0; si < sll; si++)
      {
        if (sl[si] != ' ' && sl[si] != '\t' && sl[si] != '\r')
        {
          sb = false;
          break;
        }
      }
      if (!sb)
      {
        int si = (int)yamlMeasureIndent(sl, sll);
        // Check for document markers
        if (sll >= 3 && ((sl[0] == '-' && sl[1] == '-' && sl[2] == '-') ||
                         (sl[0] == '.' && sl[1] == '.' && sl[2] == '.')) &&
            (sll == 3 || sl[3] == ' ' || sl[3] == '\t' || sl[3] == '\r'))
          break;
        if (si > context_indent)
          block_indent = si;
        break;
      }
      scan_pos = snp;
    }
  }

  std::string result;
  int trailing_newlines = 0;
  bool prev_more_indented = false;

  while (pos < size)
  {
    const char *line_start;
    size_t line_len;
    size_t save_pos = pos;
    size_t next_pos = yamlReadLine(data, size, pos, &line_start, &line_len);

    // Check if blank line (only whitespace)
    bool is_blank = true;
    for (size_t i = 0; i < line_len; i++)
    {
      if (line_start[i] != ' ' && line_start[i] != '\t' && line_start[i] != '\r')
      {
        is_blank = false;
        break;
      }
    }

    // A whitespace-only line that extends beyond block_indent has content (whitespace content)
    if (is_blank && block_indent >= 0)
    {
      size_t stripped = line_len;
      while (stripped > 0 && line_start[stripped - 1] == '\r')
        stripped--;
      if ((int)stripped > block_indent)
        is_blank = false;
    }

    if (is_blank)
    {
      trailing_newlines++;
      pos = next_pos;
      continue;
    }

    int indent = (int)yamlMeasureIndent(line_start, line_len);

    // Check for document markers at column 0
    if (line_len >= 3)
    {
      if ((line_start[0] == '-' && line_start[1] == '-' && line_start[2] == '-') ||
          (line_start[0] == '.' && line_start[1] == '.' && line_start[2] == '.'))
      {
        if (line_len == 3 || line_start[3] == ' ' || line_start[3] == '\t' || line_start[3] == '\r')
        {
          pos = save_pos;
          break;
        }
      }
    }

    if (block_indent < 0)
    {
      // Auto-detect: content must be deeper than context
      if (indent <= context_indent)
      {
        pos = save_pos;
        break;
      }
      block_indent = indent;
    }

    if (indent < block_indent)
    {
      // End of block scalar, put line back
      pos = save_pos;
      break;
    }

    pos = next_pos;

    const char *content = line_start + block_indent;
    size_t content_len = line_len - block_indent;
    // Strip trailing \r
    while (content_len > 0 && content[content_len - 1] == '\r')
      content_len--;

    bool cur_more_indented = (indent > block_indent);

    if (!result.empty())
    {
      if (trailing_newlines > 0)
      {
        // Blank lines: in literal mode or when adjacent to more-indented lines,
        // the line break before blank lines is preserved (trailing_newlines + 1).
        // In folded mode between normal lines, the line break is consumed
        // by the blank line (trailing_newlines only).
        if (indicator == '|' || prev_more_indented || cur_more_indented)
        {
          for (int i = 0; i <= trailing_newlines; i++)
            result += '\n';
        }
        else
        {
          for (int i = 0; i < trailing_newlines; i++)
            result += '\n';
        }
        trailing_newlines = 0;
      }
      else if (indicator == '|')
      {
        result += '\n';
      }
      else
      {
        // Folded mode: more-indented lines preserve newlines before/after
        if (prev_more_indented || cur_more_indented)
          result += '\n';
        else
          result += ' ';
      }
    }
    else
    {
      // Leading blank lines - preserve them
      if (trailing_newlines > 0)
      {
        for (int i = 0; i < trailing_newlines; i++)
          result += '\n';
      }
      trailing_newlines = 0;
    }

    prev_more_indented = cur_more_indented;
    result.append(content, content_len);
  }

  // Apply chomping
  if (result.empty())
  {
    // Empty block scalar
    if (chomp_mode == 2)
      result = "\n"; // keep: at least one trailing newline
    // strip and clip on empty: empty string
  }
  else
  {
    if (chomp_mode == 1)
    {
      // Strip: remove all trailing newlines (trailing_newlines is already not appended)
    }
    else if (chomp_mode == 2)
    {
      // Keep: keep all trailing newlines (add final one plus any pending)
      for (int i = 0; i < trailing_newlines; i++)
        result += '\n';
      result += '\n';
    }
    else
    {
      // Clip (default): add exactly one trailing newline
      result += '\n';
    }
  }

  DataRef ref = yamlOwnedRef(result);
  yamlEmitAnonymous(Type::String, ref.data, ref.size);
  return Error::NoError;
}

inline void Tokenizer::yamlHandleKeyValue(const char *data, size_t size, size_t &pos,
                                          const char *key, size_t key_len, Type key_type,
                                          const char *val, size_t val_len,
                                          std::vector<YamlIndentEntry> &indent_stack, int content_indent)
{
  // Trim trailing whitespace from value
  while (val_len > 0 && (val[val_len - 1] == ' ' || val[val_len - 1] == '\t'))
    val_len--;

  // Extract YAML metadata (anchors, tags) from value
  yamlExtractMetadata(val, val_len);

  // Check for alias
  if (val_len > 0 && val[0] == '*')
  {
    DataRef alias_name = yamlOwnedRef(val + 1, val_len - 1);
    yamlEmitProperty(key, key_len, key_type, alias_name.data, alias_name.size, Type::Alias);
    return;
  }

  // Check for compact sequence value (e.g., "key: - item1\n  - item2")
  if (val_len >= 2 && val[0] == '-' && (val[1] == ' ' || val[1] == '\t'))
  {
    // Emit property with array container value
    yamlEmitPropertyWithContainerValue(key, key_len, key_type, Type::ArrayStart);
    // Compute the actual column of the '-' to match subsequent items at same indent
    const char *line_start = val;
    while (line_start > data && line_start[-1] != '\n')
      line_start--;
    int dash_indent = (int)(val - line_start);
    indent_stack.push_back({dash_indent, Type::ArrayStart});

    // Process first item content after "- "
    const char *item = val + 2;
    size_t item_len = val_len - 2;
    while (item_len > 0 && (*item == ' ' || *item == '\t'))
    {
      item++;
      item_len--;
    }
    // Strip trailing whitespace
    while (item_len > 0 && (item[item_len - 1] == ' ' || item[item_len - 1] == '\t'))
      item_len--;

    if (item_len == 0)
    {
      yamlEmitAnonymous(Type::Null, Internal::yaml_null_str, 4);
    }
    else
    {
      yamlExtractMetadata(item, item_len);
      // Check for nested sequence
      if (item_len >= 2 && item[0] == '-' && (item[1] == ' ' || item[1] == '\t'))
      {
        // Nested compact sequence within compact sequence
        int nested_indent = dash_indent + (int)(item - val);
        yamlEmitAnonymous(Type::ArrayStart, Internal::yaml_arr_start, 1);
        indent_stack.push_back({nested_indent, Type::ArrayStart});
        // Process innermost item
        const char *inner = item + 2;
        size_t inner_len = item_len - 2;
        while (inner_len > 0 && (*inner == ' ' || *inner == '\t'))
        {
          inner++;
          inner_len--;
        }
        while (inner_len > 0 && (inner[inner_len - 1] == ' ' || inner[inner_len - 1] == '\t'))
          inner_len--;
        if (inner_len > 0)
        {
          yamlExtractMetadata(inner, inner_len);
          Type vt = yamlClassifyScalar(inner, inner_len);
          DataRef vr = yamlNormalizeScalar(inner, inner_len, vt);
          yamlEmitAnonymous(vt, vr.data, vr.size);
        }
        else
        {
          yamlEmitAnonymous(Type::Null, Internal::yaml_null_str, 4);
        }
      }
      else
      {
        Type vt = yamlClassifyScalar(item, item_len);
        DataRef vr = yamlNormalizeScalar(item, item_len, vt);
        yamlEmitAnonymous(vt, vr.data, vr.size);
      }
    }
    // Let yamlParseBlock handle subsequent lines
    yamlParseBlock(data, size, pos, indent_stack, dash_indent - 1);
    return;
  }

  if (val_len == 0)
  {
    // Empty value - check next lines for nested content
    size_t peek_pos = pos;
    const char *peek_line = nullptr;
    size_t peek_len = 0;
    bool found = false;
    while (peek_pos < size)
    {
      size_t np = yamlReadLine(data, size, peek_pos, &peek_line, &peek_len);
      if (!yamlIsBlankOrComment(peek_line, peek_len))
      {
        found = true;
        break;
      }
      peek_pos = np;
    }
    if (found)
    {
      int peek_indent = (int)yamlMeasureIndent(peek_line, peek_len);
      // In YAML, a sequence at the same indent as its mapping key is the key's value
      // e.g., "key:\n- item1\n- item2" means key maps to [item1, item2]
      bool same_indent_sequence = false;
      if (peek_indent == content_indent)
      {
        const char *pi_content = peek_line + peek_indent;
        size_t pi_content_len = peek_len - peek_indent;
        if (pi_content_len >= 2 && pi_content[0] == '-' && (pi_content[1] == ' ' || pi_content[1] == '\t'))
          same_indent_sequence = true;
        else if (pi_content_len == 1 && pi_content[0] == '-')
          same_indent_sequence = true;
      }
      if (peek_indent > content_indent || same_indent_sequence)
      {
        const char *peek_content = peek_line + peek_indent;
        size_t peek_content_len = peek_len - peek_indent;
        // Strip trailing \r
        while (peek_content_len > 0 && peek_content[peek_content_len - 1] == '\r')
          peek_content_len--;

        // Strip metadata (anchors/tags) from peek content for classification
        const char *peek_stripped = peek_content;
        size_t peek_stripped_len = peek_content_len;
        // Don't actually set metadata - just skip past & and ! prefixes for classification
        while (peek_stripped_len > 0 && (peek_stripped[0] == '&' || peek_stripped[0] == '!'))
        {
          size_t end = 1;
          if (peek_stripped[0] == '!' && end < peek_stripped_len && peek_stripped[end] == '!')
            end++;
          while (end < peek_stripped_len && peek_stripped[end] != ' ' && peek_stripped[end] != '\t')
            end++;
          peek_stripped += end;
          peek_stripped_len -= end;
          while (peek_stripped_len > 0 && (*peek_stripped == ' ' || *peek_stripped == '\t'))
          {
            peek_stripped++;
            peek_stripped_len--;
          }
        }

        // If peeked content was only metadata (e.g., bare &anchor line), peek further
        int container_indent = peek_indent;
        if (peek_stripped_len == 0)
        {
          // The peeked line contains only metadata (anchors/tags).
          // Extract metadata from it now and advance pos past it so yamlParseBlock
          // won't see it and prematurely close the container.
          const char *meta_ptr = peek_content;
          size_t meta_len = peek_content_len;
          yamlExtractMetadata(meta_ptr, meta_len);

          // Skip past the current peek line first (peek_pos points to its start)
          const char *tmp_l;
          size_t tmp_ll;
          size_t further_pos = yamlReadLine(data, size, peek_pos, &tmp_l, &tmp_ll);
          pos = further_pos; // Advance pos past the metadata-only line

          while (further_pos < size)
          {
            size_t np2 = yamlReadLine(data, size, further_pos, &peek_line, &peek_len);
            if (!yamlIsBlankOrComment(peek_line, peek_len))
            {
              peek_indent = (int)yamlMeasureIndent(peek_line, peek_len);
              peek_content = peek_line + peek_indent;
              peek_content_len = peek_len - peek_indent;
              while (peek_content_len > 0 && peek_content[peek_content_len - 1] == '\r')
                peek_content_len--;
              // Re-strip metadata from this further line
              peek_stripped = peek_content;
              peek_stripped_len = peek_content_len;
              while (peek_stripped_len > 0 && (peek_stripped[0] == '&' || peek_stripped[0] == '!'))
              {
                size_t e2 = 1;
                if (peek_stripped[0] == '!' && e2 < peek_stripped_len && peek_stripped[e2] == '!')
                  e2++;
                while (e2 < peek_stripped_len && peek_stripped[e2] != ' ' && peek_stripped[e2] != '\t')
                  e2++;
                peek_stripped += e2;
                peek_stripped_len -= e2;
                while (peek_stripped_len > 0 && (*peek_stripped == ' ' || *peek_stripped == '\t'))
                {
                  peek_stripped++;
                  peek_stripped_len--;
                }
              }
              break;
            }
            further_pos = np2;
          }
          // Use content line's indent for the container
          container_indent = peek_indent;
        }

        if (peek_content_len >= 2 && peek_content[0] == '-' &&
            (peek_content[1] == ' ' || peek_content[1] == '\t'))
        {
          yamlEmitPropertyWithContainerValue(key, key_len, key_type, Type::ArrayStart);
          indent_stack.push_back({container_indent, Type::ArrayStart});
        }
        else if (peek_content_len == 1 && peek_content[0] == '-')
        {
          yamlEmitPropertyWithContainerValue(key, key_len, key_type, Type::ArrayStart);
          indent_stack.push_back({container_indent, Type::ArrayStart});
        }
        else if (peek_stripped_len > 0 && yamlFindColon(peek_stripped, peek_stripped_len) != (size_t)-1)
        {
          yamlEmitPropertyWithContainerValue(key, key_len, key_type, Type::ObjectStart);
          indent_stack.push_back({container_indent, Type::ObjectStart});
        }
        else if (peek_stripped_len > 0 && peek_stripped[0] == '?' &&
                 (peek_stripped_len == 1 || peek_stripped[1] == ' ' || peek_stripped[1] == '\t'))
        {
          // Explicit key indicator - this is a mapping
          yamlEmitPropertyWithContainerValue(key, key_len, key_type, Type::ObjectStart);
          indent_stack.push_back({container_indent, Type::ObjectStart});
        }
        else if (peek_stripped_len > 0 && (peek_stripped[0] == '{' || peek_stripped[0] == '['))
        {
          // Flow collection follows
          yamlEmitPropertyWithContainerValue(key, key_len, key_type,
            peek_stripped[0] == '{' ? Type::ObjectStart : Type::ArrayStart);
          indent_stack.push_back({container_indent,
            peek_stripped[0] == '{' ? Type::ObjectStart : Type::ArrayStart});
        }
        else if (peek_stripped_len > 0 && peek_stripped[0] == '-' &&
                 (peek_stripped_len == 1 || peek_stripped[1] == ' ' || peek_stripped[1] == '\t'))
        {
          yamlEmitPropertyWithContainerValue(key, key_len, key_type, Type::ArrayStart);
          indent_stack.push_back({container_indent, Type::ArrayStart});
        }
        else if (peek_stripped_len > 0 && (peek_stripped[0] == '|' || peek_stripped[0] == '>'))
        {
          // Block scalar indicator after metadata on separate line(s)
          char indicator;
          int chomp_mode, explicit_indent_val;
          yamlParseBlockScalarHeader(peek_stripped, peek_stripped_len, &indicator, &chomp_mode, &explicit_indent_val);
          // Advance pos to after the block scalar header line
          // (pos may already be past metadata lines from the further-peek above)
          const char *skip_l;
          size_t skip_ll;
          size_t header_end = yamlReadLine(data, size, pos, &skip_l, &skip_ll);
          // Skip blank/metadata lines to find the header line
          while (header_end < size)
          {
            if (!yamlIsBlankOrComment(skip_l, skip_ll))
            {
              size_t si = yamlMeasureIndent(skip_l, skip_ll);
              const char *sc = skip_l + si;
              size_t scl = skip_ll - si;
              // Strip metadata
              yamlExtractMetadata(sc, scl);
              if (scl > 0 && (sc[0] == '|' || sc[0] == '>'))
              {
                pos = header_end; // Advance past the header line
                break;
              }
            }
            header_end = yamlReadLine(data, size, header_end, &skip_l, &skip_ll);
          }
          size_t save_tokens = yaml_tokens_.size();
          yamlParseMultilineScalarEx(data, size, pos, indicator, chomp_mode, explicit_indent_val, content_indent);
          if (yaml_tokens_.size() > save_tokens)
          {
            Token &last = yaml_tokens_.back();
            last.name = DataRef(key, key_len);
            last.name_type = key_type;
          }
          return;
        }
        else
        {
          // Bare scalar value - emit as property, not as container
          // Parse the block to get the scalar value
          size_t save_token_count = yaml_tokens_.size();
          yamlParseBlock(data, size, pos, indent_stack, content_indent);
          // The block parser should have emitted tokens. Check if we got
          // a single anonymous scalar - if so, convert it to a named property.
          if (yaml_tokens_.size() > save_token_count)
          {
            Token &first_new = yaml_tokens_[save_token_count];
            if (first_new.name.size == 0 && first_new.value_type != Type::ObjectStart &&
                first_new.value_type != Type::ObjectEnd && first_new.value_type != Type::ArrayStart &&
                first_new.value_type != Type::ArrayEnd)
            {
              first_new.name = DataRef(key, key_len);
              first_new.name_type = key_type;
            }
          }
          return;
        }
        yamlParseBlock(data, size, pos, indent_stack, content_indent);
        return;
      }
    }
    // Null value
    yamlEmitProperty(key, key_len, key_type, Internal::yaml_null_str, 4, Type::Null);
  }
  else if (val[0] == '{')
  {
    // Flow object value
    yamlEmitPropertyWithContainerValue(key, key_len, key_type, Type::ObjectStart);
    size_t flow_pos = (size_t)(val - data);
    flow_pos++; // skip {
    yamlParseFlowObjectInner(data, size, flow_pos);
    if (flow_pos < size && data[flow_pos] == '}')
      flow_pos++;
    if (flow_pos > pos)
      pos = flow_pos;
  }
  else if (val[0] == '[')
  {
    // Flow array value
    yamlEmitPropertyWithContainerValue(key, key_len, key_type, Type::ArrayStart);
    size_t flow_pos = (size_t)(val - data);
    flow_pos++; // skip [
    yamlParseFlowArrayInner(data, size, flow_pos);
    if (flow_pos < size && data[flow_pos] == ']')
      flow_pos++;
    if (flow_pos > pos)
      pos = flow_pos;
  }
  else if ((val_len >= 1) && (val[0] == '|' || val[0] == '>'))
  {
    // Multi-line scalar with optional chomp/indent modifiers
    char indicator;
    int chomp_mode, explicit_indent;
    yamlParseBlockScalarHeader(val, val_len, &indicator, &chomp_mode, &explicit_indent);
    // Emit as named property after parsing the multiline content
    size_t save_tokens = yaml_tokens_.size();
    yamlParseMultilineScalarEx(data, size, pos, indicator, chomp_mode, explicit_indent, content_indent);
    // The multiline parser emitted an anonymous token, convert it to a named property
    if (yaml_tokens_.size() > save_tokens)
    {
      Token &last = yaml_tokens_.back();
      last.name = DataRef(key, key_len);
      last.name_type = key_type;
    }
  }
  else
  {
    // Inline scalar value - check for multiline plain scalar continuation
    bool is_quoted = (val_len >= 2 && ((val[0] == '"' && val[val_len - 1] == '"') ||
                                        (val[0] == '\'' && val[val_len - 1] == '\'')));
    bool is_unclosed_quote = (!is_quoted && val_len >= 1 && (val[0] == '"' || val[0] == '\''));
    if (is_unclosed_quote)
    {
      // Multi-line quoted string - find the closing quote across lines
      size_t quote_start = (size_t)(val - data);
      size_t quote_end = yamlFindQuoteEnd(data, size, quote_start);
      if (quote_end != (size_t)-1)
      {
        const char *full_str = data + quote_start;
        size_t full_len = quote_end - quote_start;
        DataRef val_ref = yamlNormalizeScalar(full_str, full_len, Type::String);
        yamlEmitProperty(key, key_len, key_type, val_ref.data, val_ref.size, Type::String);
        pos = quote_end;
        // Skip to end of line
        while (pos < size && data[pos] != '\n')
          pos++;
        if (pos < size)
          pos++;
      }
      else
      {
        Type val_type = yamlClassifyScalar(val, val_len);
        DataRef val_ref = yamlNormalizeScalar(val, val_len, val_type);
        yamlEmitProperty(key, key_len, key_type, val_ref.data, val_ref.size, val_type);
      }
    }
    else if (!is_quoted && val_len > 0 && val[0] != '*')
    {
      // Plain scalar - try to accumulate multiline continuation
      std::string accumulated = yamlAccumulateMultilinePlainScalar(val, val_len, data, size, pos, content_indent);
      DataRef owned = yamlOwnedRef(accumulated);
      Type val_type = yamlClassifyScalar(owned.data, owned.size);
      DataRef val_ref = yamlNormalizeScalar(owned.data, owned.size, val_type);
      yamlEmitProperty(key, key_len, key_type, val_ref.data, val_ref.size, val_type);
    }
    else
    {
      Type val_type = yamlClassifyScalar(val, val_len);
      DataRef val_ref = yamlNormalizeScalar(val, val_len, val_type);
      yamlEmitProperty(key, key_len, key_type, val_ref.data, val_ref.size, val_type);
    }
  }
}

inline Error Tokenizer::yamlParseBlock(const char *data, size_t size, size_t &pos,
                                       std::vector<YamlIndentEntry> &indent_stack, int parent_indent)
{
  while (pos < size)
  {
    size_t save_pos = pos;
    const char *line_start;
    size_t line_len;
    size_t next_pos = yamlReadLine(data, size, pos, &line_start, &line_len);

    if (yamlIsBlankOrComment(line_start, line_len))
    {
      pos = next_pos;
      continue;
    }

    // Check for document markers (must be exactly '---' or '...' at column 0,
    // followed by whitespace, end of line, or end of input)
    if (line_len >= 3 && line_start[0] == '-' && line_start[1] == '-' && line_start[2] == '-')
    {
      if (line_len == 3 || line_start[3] == ' ' || line_start[3] == '\t' || line_start[3] == '\r')
      {
        pos = save_pos;
        return Error::NoError;
      }
    }
    if (line_len >= 3 && line_start[0] == '.' && line_start[1] == '.' && line_start[2] == '.')
    {
      if (line_len == 3 || line_start[3] == ' ' || line_start[3] == '\t' || line_start[3] == '\r')
      {
        pos = save_pos;
        return Error::NoError;
      }
    }

    int indent = (int)yamlMeasureIndent(line_start, line_len);

    if (indent <= parent_indent)
    {
      pos = save_pos;
      return Error::NoError;
    }

    pos = next_pos;

    const char *content = line_start + indent;
    size_t content_len = line_len - indent;

    // Strip trailing whitespace
    while (content_len > 0 &&
           (content[content_len - 1] == ' ' || content[content_len - 1] == '\t' || content[content_len - 1] == '\r'))
      content_len--;

    // Strip trailing comment
    yamlStripTrailingComment(content, &content_len);

    // Close containers that are deeper than current indent
    while (!indent_stack.empty() && indent_stack.back().indent > indent)
    {
      if (indent_stack.back().container_type == Type::ArrayStart)
        yamlEmitAnonymous(Type::ArrayEnd, Internal::yaml_arr_end, 1);
      else
        yamlEmitAnonymous(Type::ObjectEnd, Internal::yaml_obj_end, 1);
      indent_stack.pop_back();
    }

    // Determine line type
    bool is_sequence_item = false;
    if (content_len >= 2 && content[0] == '-' && (content[1] == ' ' || content[1] == '\t'))
      is_sequence_item = true;
    else if (content_len == 1 && content[0] == '-')
      is_sequence_item = true;

    if (is_sequence_item)
    {
      // Close non-array container at same indent
      if (!indent_stack.empty() && indent_stack.back().indent == indent &&
          indent_stack.back().container_type != Type::ArrayStart)
      {
        yamlEmitAnonymous(Type::ObjectEnd, Internal::yaml_obj_end, 1);
        indent_stack.pop_back();
      }

      // Open array if needed
      if (indent_stack.empty() || indent_stack.back().indent != indent ||
          indent_stack.back().container_type != Type::ArrayStart)
      {
        yamlEmitAnonymous(Type::ArrayStart, Internal::yaml_arr_start, 1);
        indent_stack.push_back({indent, Type::ArrayStart});
      }

      // Content after "- "
      const char *item_content = content + 2;
      size_t item_content_len = (content_len > 2) ? content_len - 2 : 0;
      if (content_len == 1)
      {
        item_content = content + 1;
        item_content_len = 0;
      }

      // Trim leading whitespace
      while (item_content_len > 0 && (*item_content == ' ' || *item_content == '\t'))
      {
        item_content++;
        item_content_len--;
      }

      // Extract YAML metadata (anchors, tags) from sequence item
      yamlExtractMetadata(item_content, item_content_len);

      // Check for alias (possibly as mapping key)
      if (item_content_len > 0 && item_content[0] == '*')
      {
        size_t alias_colon = yamlFindColon(item_content, item_content_len);
        if (alias_colon != (size_t)-1)
        {
          // Alias as mapping key within sequence item (e.g., - *alias : value)
          int item_indent = indent + (int)(item_content - (line_start + indent));
          if (item_indent <= indent)
            item_indent = indent + 2;
          yamlEmitAnonymous(Type::ObjectStart, Internal::yaml_obj_start, 1);
          indent_stack.push_back({item_indent, Type::ObjectStart});

          const char *akey = item_content + 1;
          size_t akey_len = alias_colon - 1;
          while (akey_len > 0 && (akey[akey_len - 1] == ' ' || akey[akey_len - 1] == '\t'))
            akey_len--;

          const char *aval = item_content + alias_colon + 1;
          size_t aval_len = item_content_len - alias_colon - 1;
          while (aval_len > 0 && (*aval == ' ' || *aval == '\t'))
          {
            aval++;
            aval_len--;
          }
          yamlHandleKeyValue(data, size, pos, akey, akey_len, Type::Alias, aval, aval_len, indent_stack, item_indent);
        }
        else
        {
          DataRef alias_name = yamlOwnedRef(item_content + 1, item_content_len - 1);
          yamlEmitAnonymous(Type::Alias, alias_name.data, alias_name.size);
        }
      }
      else if (item_content_len == 0)
      {
        // Empty item - check for nested content
        size_t peek_pos = pos;
        const char *peek_line = nullptr;
        size_t peek_len = 0;
        bool found = false;
        while (peek_pos < size)
        {
          size_t np = yamlReadLine(data, size, peek_pos, &peek_line, &peek_len);
          if (!yamlIsBlankOrComment(peek_line, peek_len))
          {
            found = true;
            break;
          }
          peek_pos = np;
        }
        if (found)
        {
          int peek_indent = (int)yamlMeasureIndent(peek_line, peek_len);
          if (peek_indent > indent)
          {
            yamlParseBlock(data, size, pos, indent_stack, indent);
          }
          else
          {
            yamlEmitAnonymous(Type::Null, Internal::yaml_null_str, 4);
          }
        }
        else
        {
          yamlEmitAnonymous(Type::Null, Internal::yaml_null_str, 4);
        }
      }
      else if (item_content[0] == '{')
      {
        size_t flow_pos = (size_t)(item_content - data);
        yamlParseFlowObject(data, size, flow_pos);
        if (flow_pos > pos)
          pos = flow_pos;
      }
      else if (item_content[0] == '[')
      {
        size_t flow_pos = (size_t)(item_content - data);
        yamlParseFlowArray(data, size, flow_pos);
        if (flow_pos > pos)
          pos = flow_pos;
      }
      else if (item_content[0] == '|' || item_content[0] == '>')
      {
        char indicator;
        int chomp_mode, explicit_indent;
        yamlParseBlockScalarHeader(item_content, item_content_len, &indicator, &chomp_mode, &explicit_indent);
        yamlParseMultilineScalarEx(data, size, pos, indicator, chomp_mode, explicit_indent, indent);
      }
      else if ((item_content_len >= 2 && item_content[0] == '-' &&
                (item_content[1] == ' ' || item_content[1] == '\t')) ||
               (item_content_len == 1 && item_content[0] == '-'))
      {
        // Nested sequence item (e.g., "- - value" or "- - - value")
        // Peel off nested "- " prefixes, creating arrays for each level
        while ((item_content_len >= 2 && item_content[0] == '-' &&
                (item_content[1] == ' ' || item_content[1] == '\t')) ||
               (item_content_len == 1 && item_content[0] == '-'))
        {
          int nested_indent = indent + (int)(item_content - (line_start + indent));
          if (nested_indent <= indent)
            nested_indent = indent + 2;
          yamlEmitAnonymous(Type::ArrayStart, Internal::yaml_arr_start, 1);
          indent_stack.push_back({nested_indent, Type::ArrayStart});
          indent = nested_indent;
          if (item_content_len == 1)
          {
            item_content++;
            item_content_len = 0;
          }
          else
          {
            item_content += 2;
            item_content_len -= 2;
          }
          while (item_content_len > 0 && (*item_content == ' ' || *item_content == '\t'))
          {
            item_content++;
            item_content_len--;
          }
          yamlExtractMetadata(item_content, item_content_len);
        }
        // Now item_content is the innermost value
        if (item_content_len == 0)
        {
          // Check for nested content on next lines
          size_t peek_pos2 = pos;
          const char *peek_line2 = nullptr;
          size_t peek_len2 = 0;
          bool found2 = false;
          while (peek_pos2 < size)
          {
            size_t np2 = yamlReadLine(data, size, peek_pos2, &peek_line2, &peek_len2);
            if (!yamlIsBlankOrComment(peek_line2, peek_len2))
            {
              found2 = true;
              break;
            }
            peek_pos2 = np2;
          }
          if (found2 && (int)yamlMeasureIndent(peek_line2, peek_len2) > indent)
            yamlParseBlock(data, size, pos, indent_stack, indent);
          else
            yamlEmitAnonymous(Type::Null, Internal::yaml_null_str, 4);
        }
        else if (item_content[0] == '{')
        {
          size_t flow_pos = (size_t)(item_content - data);
          yamlParseFlowObject(data, size, flow_pos);
          if (flow_pos > pos)
            pos = flow_pos;
        }
        else if (item_content[0] == '[')
        {
          size_t flow_pos = (size_t)(item_content - data);
          yamlParseFlowArray(data, size, flow_pos);
          if (flow_pos > pos)
            pos = flow_pos;
        }
        else
        {
          size_t colon_pos2 = yamlFindColon(item_content, item_content_len);
          if (colon_pos2 != (size_t)-1)
          {
            int item_indent2 = indent;
            yamlEmitAnonymous(Type::ObjectStart, Internal::yaml_obj_start, 1);
            indent_stack.push_back({item_indent2, Type::ObjectStart});
            const char *k = item_content;
            size_t kl = colon_pos2;
            while (kl > 0 && (k[kl - 1] == ' ' || k[kl - 1] == '\t'))
              kl--;
            const char *v = item_content + colon_pos2 + 1;
            size_t vl = item_content_len - colon_pos2 - 1;
            while (vl > 0 && (*v == ' ' || *v == '\t'))
            {
              v++;
              vl--;
            }
            yamlHandleKeyValue(data, size, pos, k, kl, Type::Ascii, v, vl, indent_stack, item_indent2);
          }
          else
          {
            Type vt = yamlClassifyScalar(item_content, item_content_len);
            DataRef vr = yamlNormalizeScalar(item_content, item_content_len, vt);
            yamlEmitAnonymous(vt, vr.data, vr.size);
          }
        }
      }
      else
      {
        // Check if item is a mapping (key: value)
        size_t colon_pos = yamlFindColon(item_content, item_content_len);
        if (colon_pos != (size_t)-1)
        {
          // Sequence item is a mapping
          int item_indent = indent + (int)(item_content - (line_start + indent));
          if (item_indent <= indent)
            item_indent = indent + 2;

          yamlEmitAnonymous(Type::ObjectStart, Internal::yaml_obj_start, 1);
          indent_stack.push_back({item_indent, Type::ObjectStart});

          // Parse key
          const char *key = item_content;
          size_t key_len = colon_pos;
          while (key_len > 0 && (key[key_len - 1] == ' ' || key[key_len - 1] == '\t'))
            key_len--;

          Type key_type = Type::Ascii;
          if (key_len >= 2 && ((key[0] == '"' && key[key_len - 1] == '"') ||
                               (key[0] == '\'' && key[key_len - 1] == '\'')))
          {
            DataRef kr = yamlNormalizeScalar(key, key_len, Type::String);
            key = kr.data;
            key_len = kr.size;
            key_type = Type::String;
          }

          // Parse value
          const char *val = item_content + colon_pos + 1;
          size_t val_len = item_content_len - colon_pos - 1;
          while (val_len > 0 && (*val == ' ' || *val == '\t'))
          {
            val++;
            val_len--;
          }

          yamlHandleKeyValue(data, size, pos, key, key_len, key_type, val, val_len, indent_stack, item_indent);
        }
        else
        {
          // Simple scalar as sequence item (metadata already extracted above)
          // Try multiline plain scalar accumulation
          bool is_quoted = (item_content_len >= 2 && ((item_content[0] == '"' && item_content[item_content_len - 1] == '"') ||
                                                       (item_content[0] == '\'' && item_content[item_content_len - 1] == '\'')));
          if (!is_quoted && item_content_len > 0 && item_content[0] != '*')
          {
            std::string accumulated = yamlAccumulateMultilinePlainScalar(item_content, item_content_len, data, size, pos, indent);
            DataRef owned = yamlOwnedRef(accumulated);
            Type vt = yamlClassifyScalar(owned.data, owned.size);
            DataRef vr = yamlNormalizeScalar(owned.data, owned.size, vt);
            yamlEmitAnonymous(vt, vr.data, vr.size);
          }
          else
          {
            Type vt = yamlClassifyScalar(item_content, item_content_len);
            DataRef vr = yamlNormalizeScalar(item_content, item_content_len, vt);
            yamlEmitAnonymous(vt, vr.data, vr.size);
          }
        }
      }
    }
    else if (content_len > 0 && content[0] == '!')
    {
      // Tag prefix - extract metadata then handle remaining content
      const char *meta_content = content;
      size_t meta_len = content_len;
      yamlExtractMetadata(meta_content, meta_len);

      if (meta_len == 0)
      {
        // Just a tag (and possibly anchor) with no content on this line.
        // The metadata applies to the next token - continue to next line.
        continue;
      }
      else if (meta_content[0] == '*')
      {
        DataRef alias_name = yamlOwnedRef(meta_content + 1, meta_len - 1);
        yamlEmitAnonymous(Type::Alias, alias_name.data, alias_name.size);
      }
      else if (meta_content[0] == '{')
      {
        size_t flow_pos = (size_t)(meta_content - data);
        yamlParseFlowObject(data, size, flow_pos);
        if (flow_pos > pos)
          pos = flow_pos;
      }
      else if (meta_content[0] == '[')
      {
        size_t flow_pos = (size_t)(meta_content - data);
        yamlParseFlowArray(data, size, flow_pos);
        if (flow_pos > pos)
          pos = flow_pos;
      }
      else
      {
        size_t colon_pos = yamlFindColon(meta_content, meta_len);
        if (colon_pos != (size_t)-1)
        {
          // Save key-side anchor before container setup may consume it
          DataRef saved_name_anchor = yaml_pending_anchor_;
          yaml_pending_anchor_ = DataRef();

          if (!indent_stack.empty() && indent_stack.back().indent == indent &&
              indent_stack.back().container_type != Type::ObjectStart)
          {
            yamlEmitAnonymous(Type::ArrayEnd, Internal::yaml_arr_end, 1);
            indent_stack.pop_back();
          }
          if (indent_stack.empty() || indent_stack.back().indent != indent ||
              indent_stack.back().container_type != Type::ObjectStart)
          {
            yamlEmitAnonymous(Type::ObjectStart, Internal::yaml_obj_start, 1);
            indent_stack.push_back({indent, Type::ObjectStart});
          }

          const char *key = meta_content;
          size_t key_len = colon_pos;
          while (key_len > 0 && (key[key_len - 1] == ' ' || key[key_len - 1] == '\t'))
            key_len--;
          Type key_type = Type::Ascii;
          if (key_len >= 2 && ((key[0] == '"' && key[key_len - 1] == '"') ||
                               (key[0] == '\'' && key[key_len - 1] == '\'')))
          {
            DataRef kr = yamlNormalizeScalar(key, key_len, Type::String);
            key = kr.data;
            key_len = kr.size;
            key_type = Type::String;
          }

          const char *val = meta_content + colon_pos + 1;
          size_t val_len = meta_len - colon_pos - 1;
          while (val_len > 0 && (*val == ' ' || *val == '\t'))
          {
            val++;
            val_len--;
          }
          yamlHandleKeyValue(data, size, pos, key, key_len, key_type, val, val_len, indent_stack, indent);
          // Apply key-side anchor to the last emitted property token
          if (saved_name_anchor.size > 0 && !yaml_tokens_.empty())
            yaml_tokens_.back().name_anchor = saved_name_anchor;
        }
        else
        {
          Type vt = yamlClassifyScalar(meta_content, meta_len);
          DataRef vr = yamlNormalizeScalar(meta_content, meta_len, vt);
          yamlEmitAnonymous(vt, vr.data, vr.size);
        }
      }
    }
    else if (content_len > 0 && content[0] == '{')
    {
      // Standalone flow object
      size_t flow_pos = (size_t)(content - data);
      yamlParseFlowObject(data, size, flow_pos);
      if (flow_pos > pos)
        pos = flow_pos;
    }
    else if (content_len > 0 && content[0] == '[')
    {
      // Standalone flow array
      size_t flow_pos = (size_t)(content - data);
      yamlParseFlowArray(data, size, flow_pos);
      if (flow_pos > pos)
        pos = flow_pos;
    }
    else if (content_len > 0 && content[0] == '&')
    {
      // Extract metadata from content that starts with anchor
      const char *meta_content = content;
      size_t meta_len = content_len;
      yamlExtractMetadata(meta_content, meta_len);

      if (meta_len == 0)
      {
        // Just an anchor with no value on this line - the anchor applies to the next token
        // Don't emit anything, just continue to next line
        continue;
      }
      else if (meta_len > 0 && meta_content[0] == '*')
      {
        DataRef alias_name = yamlOwnedRef(meta_content + 1, meta_len - 1);
        yamlEmitAnonymous(Type::Alias, alias_name.data, alias_name.size);
      }
      else if (meta_content[0] == '{')
      {
        size_t flow_pos = (size_t)(meta_content - data);
        yamlParseFlowObject(data, size, flow_pos);
        if (flow_pos > pos)
          pos = flow_pos;
      }
      else if (meta_content[0] == '[')
      {
        size_t flow_pos = (size_t)(meta_content - data);
        yamlParseFlowArray(data, size, flow_pos);
        if (flow_pos > pos)
          pos = flow_pos;
      }
      else
      {
        // Could be a mapping or bare scalar after anchor
        size_t colon_pos = yamlFindColon(meta_content, meta_len);
        if (colon_pos != (size_t)-1)
        {
          // Mapping entry with anchor on key line
          // Save key-side anchor before container setup may consume it
          DataRef saved_name_anchor = yaml_pending_anchor_;
          yaml_pending_anchor_ = DataRef();

          if (!indent_stack.empty() && indent_stack.back().indent == indent &&
              indent_stack.back().container_type != Type::ObjectStart)
          {
            yamlEmitAnonymous(Type::ArrayEnd, Internal::yaml_arr_end, 1);
            indent_stack.pop_back();
          }
          if (indent_stack.empty() || indent_stack.back().indent != indent ||
              indent_stack.back().container_type != Type::ObjectStart)
          {
            yamlEmitAnonymous(Type::ObjectStart, Internal::yaml_obj_start, 1);
            indent_stack.push_back({indent, Type::ObjectStart});
          }

          const char *key = meta_content;
          size_t key_len = colon_pos;
          while (key_len > 0 && (key[key_len - 1] == ' ' || key[key_len - 1] == '\t'))
            key_len--;
          Type key_type = Type::Ascii;
          if (key_len >= 2 && ((key[0] == '"' && key[key_len - 1] == '"') ||
                               (key[0] == '\'' && key[key_len - 1] == '\'')))
          {
            DataRef kr = yamlNormalizeScalar(key, key_len, Type::String);
            key = kr.data;
            key_len = kr.size;
            key_type = Type::String;
          }

          const char *val = meta_content + colon_pos + 1;
          size_t val_len = meta_len - colon_pos - 1;
          while (val_len > 0 && (*val == ' ' || *val == '\t'))
          {
            val++;
            val_len--;
          }
          yamlHandleKeyValue(data, size, pos, key, key_len, key_type, val, val_len, indent_stack, indent);
          // Apply key-side anchor to the last emitted property token
          if (saved_name_anchor.size > 0 && !yaml_tokens_.empty())
            yaml_tokens_.back().name_anchor = saved_name_anchor;
        }
        else
        {
          Type vt = yamlClassifyScalar(meta_content, meta_len);
          DataRef vr = yamlNormalizeScalar(meta_content, meta_len, vt);
          yamlEmitAnonymous(vt, vr.data, vr.size);
        }
      }
    }
    else if (content_len > 0 && content[0] == '*')
    {
      // Check if alias is used as a mapping key (e.g., *alias : value)
      size_t colon_pos = yamlFindColon(content, content_len);
      if (colon_pos != (size_t)-1)
      {
        // Alias as mapping key
        if (!indent_stack.empty() && indent_stack.back().indent == indent &&
            indent_stack.back().container_type != Type::ObjectStart)
        {
          yamlEmitAnonymous(Type::ArrayEnd, Internal::yaml_arr_end, 1);
          indent_stack.pop_back();
        }
        if (indent_stack.empty() || indent_stack.back().indent != indent ||
            indent_stack.back().container_type != Type::ObjectStart)
        {
          yamlEmitAnonymous(Type::ObjectStart, Internal::yaml_obj_start, 1);
          indent_stack.push_back({indent, Type::ObjectStart});
        }

        // The key is the alias name (resolved by the consumer)
        const char *key = content + 1;
        size_t key_len = colon_pos - 1;
        while (key_len > 0 && (key[key_len - 1] == ' ' || key[key_len - 1] == '\t'))
          key_len--;

        const char *val = content + colon_pos + 1;
        size_t val_len = content_len - colon_pos - 1;
        while (val_len > 0 && (*val == ' ' || *val == '\t'))
        {
          val++;
          val_len--;
        }
        // Use Alias as key_type so consumers know to resolve the key from anchors
        yamlHandleKeyValue(data, size, pos, key, key_len, Type::Alias, val, val_len, indent_stack, indent);
      }
      else
      {
        // Bare alias at block level
        DataRef alias_name = yamlOwnedRef(content + 1, content_len - 1);
        yamlEmitAnonymous(Type::Alias, alias_name.data, alias_name.size);
      }
    }
    else if (content_len >= 1 && content[0] == '?' &&
             (content_len == 1 || content[1] == ' ' || content[1] == '\t'))
    {
      // Explicit key indicator
      // Ensure we're in an object container
      if (!indent_stack.empty() && indent_stack.back().indent == indent &&
          indent_stack.back().container_type != Type::ObjectStart)
      {
        yamlEmitAnonymous(Type::ArrayEnd, Internal::yaml_arr_end, 1);
        indent_stack.pop_back();
      }
      if (indent_stack.empty() || indent_stack.back().indent != indent ||
          indent_stack.back().container_type != Type::ObjectStart)
      {
        yamlEmitAnonymous(Type::ObjectStart, Internal::yaml_obj_start, 1);
        indent_stack.push_back({indent, Type::ObjectStart});
      }

      // Extract key content after "? "
      const char *ekey = content + 1;
      size_t ekey_len = content_len - 1;
      while (ekey_len > 0 && (*ekey == ' ' || *ekey == '\t'))
      {
        ekey++;
        ekey_len--;
      }

      // Extract metadata from key
      yamlExtractMetadata(ekey, ekey_len);

      // Handle key type
      Type ekey_type = Type::Ascii;
      if (ekey_len >= 2 && ((ekey[0] == '"' && ekey[ekey_len - 1] == '"') ||
                             (ekey[0] == '\'' && ekey[ekey_len - 1] == '\'')))
      {
        DataRef kr = yamlNormalizeScalar(ekey, ekey_len, Type::String);
        ekey = kr.data;
        ekey_len = kr.size;
        ekey_type = Type::String;
      }
      else if (ekey_len > 0 && (ekey[0] == '|' || ekey[0] == '>'))
      {
        // Block scalar as key - parse it
        char indicator;
        int chomp_mode, explicit_indent_val;
        yamlParseBlockScalarHeader(ekey, ekey_len, &indicator, &chomp_mode, &explicit_indent_val);
        yamlParseMultilineScalarEx(data, size, pos, indicator, chomp_mode, explicit_indent_val, indent);
        // The scalar was emitted as an anonymous token - retrieve it and use as key
        if (!yaml_tokens_.empty())
        {
          Token &last = yaml_tokens_.back();
          ekey = last.value.data;
          ekey_len = last.value.size;
          ekey_type = last.value_type;
          yaml_tokens_.pop_back();
        }
      }
      else if (ekey_len > 0)
      {
        // Try multiline plain scalar accumulation for the key
        std::string accumulated = yamlAccumulateMultilinePlainScalar(ekey, ekey_len, data, size, pos, indent);
        DataRef owned = yamlOwnedRef(accumulated);
        ekey = owned.data;
        ekey_len = owned.size;
      }

      // Peek ahead for ": value" line
      size_t peek_pos = pos;
      const char *peek_line = nullptr;
      size_t peek_len = 0;
      bool has_value_line = false;
      while (peek_pos < size)
      {
        size_t np = yamlReadLine(data, size, peek_pos, &peek_line, &peek_len);
        if (!yamlIsBlankOrComment(peek_line, peek_len))
        {
          int peek_indent = (int)yamlMeasureIndent(peek_line, peek_len);
          const char *peek_content = peek_line + peek_indent;
          size_t peek_content_len = peek_len - peek_indent;
          if (peek_indent == indent && peek_content_len >= 1 && peek_content[0] == ':' &&
              (peek_content_len == 1 || peek_content[1] == ' ' || peek_content[1] == '\t'))
          {
            has_value_line = true;
            pos = np;
            // Extract value content after ": "
            const char *eval = peek_content + 1;
            size_t eval_len = peek_content_len - 1;
            while (eval_len > 0 && (*eval == ' ' || *eval == '\t'))
            {
              eval++;
              eval_len--;
            }
            // Strip trailing whitespace
            while (eval_len > 0 && (eval[eval_len - 1] == ' ' || eval[eval_len - 1] == '\t' ||
                   eval[eval_len - 1] == '\r'))
              eval_len--;
            yamlStripTrailingComment(eval, &eval_len);
            yamlHandleKeyValue(data, size, pos, ekey, ekey_len, ekey_type, eval, eval_len, indent_stack, indent);
          }
          break;
        }
        peek_pos = np;
      }
      if (!has_value_line)
      {
        // No value line - emit key with null value
        yamlHandleKeyValue(data, size, pos, ekey, ekey_len, ekey_type, nullptr, 0, indent_stack, indent);
      }
    }
    else if (content_len >= 2 && content[0] == ':' && (content[1] == ' ' || content[1] == '\t'))
    {
      // Bare ": value" - value for a pending explicit key (shouldn't normally reach here since
      // it's consumed in the ? handler above, but handle gracefully)
      // Just skip it - if we get here something is off
    }
    else if (content_len >= 1 && (content[0] == '|' || content[0] == '>'))
    {
      // Block scalar indicator at block level
      char indicator;
      int chomp_mode, explicit_indent;
      yamlParseBlockScalarHeader(content, content_len, &indicator, &chomp_mode, &explicit_indent);
      yamlParseMultilineScalarEx(data, size, pos, indicator, chomp_mode, explicit_indent, indent);
    }
    else
    {
      // Mapping entry or bare scalar
      size_t colon_pos = yamlFindColon(content, content_len);
      if (colon_pos != (size_t)-1)
      {
        // Mapping entry
        // Close non-object container at same indent
        if (!indent_stack.empty() && indent_stack.back().indent == indent &&
            indent_stack.back().container_type != Type::ObjectStart)
        {
          yamlEmitAnonymous(Type::ArrayEnd, Internal::yaml_arr_end, 1);
          indent_stack.pop_back();
        }

        // Open object if needed
        if (indent_stack.empty() || indent_stack.back().indent != indent ||
            indent_stack.back().container_type != Type::ObjectStart)
        {
          yamlEmitAnonymous(Type::ObjectStart, Internal::yaml_obj_start, 1);
          indent_stack.push_back({indent, Type::ObjectStart});
        }

        // Parse key
        const char *key = content;
        size_t key_len = colon_pos;
        while (key_len > 0 && (key[key_len - 1] == ' ' || key[key_len - 1] == '\t'))
          key_len--;

        // Extract metadata from key (e.g., &anchor key: value)
        yamlExtractMetadata(key, key_len);

        Type key_type = Type::Ascii;
        if (key_len >= 2 && ((key[0] == '"' && key[key_len - 1] == '"') ||
                             (key[0] == '\'' && key[key_len - 1] == '\'')))
        {
          DataRef kr = yamlNormalizeScalar(key, key_len, Type::String);
          key = kr.data;
          key_len = kr.size;
          key_type = Type::String;
        }

        // Parse value
        const char *val = content + colon_pos + 1;
        size_t val_len = content_len - colon_pos - 1;
        while (val_len > 0 && (*val == ' ' || *val == '\t'))
        {
          val++;
          val_len--;
        }

        yamlHandleKeyValue(data, size, pos, key, key_len, key_type, val, val_len, indent_stack, indent);
      }
      else
      {
        // Bare scalar (not inside a sequence, not a mapping)
        // This could be a standalone document value
        const char *sc = content;
        size_t sc_len = content_len;
        yamlExtractMetadata(sc, sc_len);
        if (sc_len > 0 && sc[0] == '*')
        {
          DataRef alias_name = yamlOwnedRef(sc + 1, sc_len - 1);
          yamlEmitAnonymous(Type::Alias, alias_name.data, alias_name.size);
        }
        else
        {
          // Try multiline plain scalar accumulation for bare document scalars
          bool is_quoted = (sc_len >= 2 && ((sc[0] == '"' && sc[sc_len - 1] == '"') ||
                                             (sc[0] == '\'' && sc[sc_len - 1] == '\'')));
          bool is_unclosed_quote = (!is_quoted && sc_len >= 1 && (sc[0] == '"' || sc[0] == '\''));
          if (is_unclosed_quote)
          {
            size_t quote_start = (size_t)(sc - data);
            size_t quote_end = yamlFindQuoteEnd(data, size, quote_start);
            if (quote_end != (size_t)-1)
            {
              const char *full_str = data + quote_start;
              size_t full_len = quote_end - quote_start;
              DataRef vr = yamlNormalizeScalar(full_str, full_len, Type::String);
              yamlEmitAnonymous(Type::String, vr.data, vr.size);
              pos = quote_end;
              while (pos < size && data[pos] != '\n')
                pos++;
              if (pos < size)
                pos++;
            }
            else
            {
              Type first_type = yamlClassifyScalar(sc, sc_len);
              DataRef vr = yamlNormalizeScalar(sc, sc_len, first_type);
              yamlEmitAnonymous(first_type, vr.data, vr.size);
            }
          }
          else
          {
            Type first_type = yamlClassifyScalar(sc, sc_len);
            if (!is_quoted && sc_len > 0 && first_type == Type::Ascii)
            {
              std::string accumulated = yamlAccumulateMultilinePlainScalar(sc, sc_len, data, size, pos, parent_indent);
              DataRef owned = yamlOwnedRef(accumulated);
              Type vt = yamlClassifyScalar(owned.data, owned.size);
              DataRef vr = yamlNormalizeScalar(owned.data, owned.size, vt);
              yamlEmitAnonymous(vt, vr.data, vr.size);
            }
            else
            {
              DataRef vr = yamlNormalizeScalar(sc, sc_len, first_type);
              yamlEmitAnonymous(first_type, vr.data, vr.size);
            }
          }
        }
      }
    }
  }
  return Error::NoError;
}

inline Error Tokenizer::yamlParseData(const char *data, size_t size)
{
  yaml_tokens_.clear();
  yaml_owned_strings_.clear();
  yaml_owned_strings_.reserve(64);
  yaml_tokens_.reserve(64);

  size_t pos = 0;

  // Skip UTF-8 BOM
  if (size >= 3 && (unsigned char)data[0] == 0xEF && (unsigned char)data[1] == 0xBB &&
      (unsigned char)data[2] == 0xBF)
    pos = 3;

  // Skip leading blank lines and comment lines (but preserve indentation of first content line)
  while (pos < size)
  {
    size_t save = pos;
    const char *ls;
    size_t ll;
    size_t np = yamlReadLine(data, size, pos, &ls, &ll);
    if (yamlIsBlankOrComment(ls, ll))
    {
      pos = np;
    }
    else
    {
      pos = save;
      break;
    }
  }

  // Skip directives (%YAML, %TAG) and interspersed comments/whitespace
  while (pos < size && (data[pos] == '%' || data[pos] == '#'))
  {
    while (pos < size && data[pos] != '\n')
      pos++;
    if (pos < size)
      pos++;
    // Skip blank lines after directive
    while (pos < size)
    {
      size_t save = pos;
      const char *ls;
      size_t ll;
      size_t np = yamlReadLine(data, size, pos, &ls, &ll);
      if (yamlIsBlankOrComment(ls, ll))
        pos = np;
      else
      {
        pos = save;
        break;
      }
    }
  }

  // Skip document end marker "..." at start
  if (pos + 3 <= size && data[pos] == '.' && data[pos + 1] == '.' && data[pos + 2] == '.')
  {
    if (pos + 3 >= size || data[pos + 3] == '\n' || data[pos + 3] == '\r' ||
        data[pos + 3] == ' ' || data[pos + 3] == '\t')
    {
      // Document end with no content - nothing to parse
      if (!yaml_tokens_.empty())
      {
        parsed_data_vector = &yaml_tokens_;
        cursor_index = 0;
      }
      return Error::NoError;
    }
  }

  // Handle document start marker "---"
  bool doc_has_inline_value = false;
  if (pos + 3 <= size && data[pos] == '-' && data[pos + 1] == '-' && data[pos + 2] == '-')
  {
    // Check if it's really a document marker (not something like ---word)
    if (pos + 3 >= size || data[pos + 3] == '\n' || data[pos + 3] == '\r' ||
        data[pos + 3] == ' ' || data[pos + 3] == '\t')
    {
      pos += 3;
      // Skip whitespace on same line (not newlines)
      while (pos < size && (data[pos] == ' ' || data[pos] == '\t'))
        pos++;
      // Check for inline content after ---
      if (pos < size && data[pos] != '\n' && data[pos] != '\r' && data[pos] != '#')
      {
        doc_has_inline_value = true;
      }
      else
      {
        // Skip rest of line (comment or empty)
        while (pos < size && data[pos] != '\n')
          pos++;
        if (pos < size)
          pos++; // skip \n
      }
    }
  }

  if (doc_has_inline_value)
  {
    // Content directly after --- on the same line
    const char *content_start = data + pos;
    size_t line_end = pos;
    while (line_end < size && data[line_end] != '\n')
      line_end++;
    size_t content_len = line_end - pos;
    // Strip trailing whitespace
    while (content_len > 0 && (content_start[content_len - 1] == ' ' ||
           content_start[content_len - 1] == '\t' || content_start[content_len - 1] == '\r'))
      content_len--;
    yamlStripTrailingComment(content_start, &content_len);

    // Extract metadata (anchors, tags) from inline content
    yamlExtractMetadata(content_start, content_len);

    if (content_len > 0 && (content_start[0] == '|' || content_start[0] == '>'))
    {
      char indicator;
      int chomp_mode, explicit_indent;
      yamlParseBlockScalarHeader(content_start, content_len, &indicator, &chomp_mode, &explicit_indent);
      pos = line_end;
      if (pos < size)
        pos++; // skip \n
      yamlParseMultilineScalarEx(data, size, pos, indicator, chomp_mode, explicit_indent, -1);
    }
    else if (content_len > 0 && content_start[0] == '{')
    {
      size_t flow_pos = (size_t)(content_start - data);
      yamlParseFlowObject(data, size, flow_pos);
    }
    else if (content_len > 0 && content_start[0] == '[')
    {
      size_t flow_pos = (size_t)(content_start - data);
      yamlParseFlowArray(data, size, flow_pos);
    }
    else if (content_len > 0 && (content_start[0] == '"' || content_start[0] == '\''))
    {
      // Multi-line quoted string - find end across lines
      size_t quote_start = (size_t)(content_start - data);
      size_t quote_end = yamlFindQuoteEnd(data, size, quote_start);
      if (quote_end != (size_t)-1)
      {
        const char *full_str = data + quote_start;
        size_t full_len = quote_end - quote_start;
        Type vt = Type::String;
        DataRef vr = yamlNormalizeScalar(full_str, full_len, vt);
        yamlEmitAnonymous(vt, vr.data, vr.size);
        pos = quote_end;
        // Skip to end of line
        while (pos < size && data[pos] != '\n')
          pos++;
        if (pos < size)
          pos++;
      }
      else
      {
        Type vt = yamlClassifyScalar(content_start, content_len);
        DataRef vr = yamlNormalizeScalar(content_start, content_len, vt);
        yamlEmitAnonymous(vt, vr.data, vr.size);
      }
    }
    else if (content_len > 0)
    {
      Type vt = yamlClassifyScalar(content_start, content_len);
      DataRef vr = yamlNormalizeScalar(content_start, content_len, vt);
      yamlEmitAnonymous(vt, vr.data, vr.size);
    }

    if (content_len == 0)
    {
      // Content was just anchor/tag metadata - skip to next line and parse as block
      pos = line_end;
      if (pos < size)
        pos++; // skip \n
      std::vector<YamlIndentEntry> indent_stack;
      yamlParseBlock(data, size, pos, indent_stack, -1);
      while (!indent_stack.empty())
      {
        if (indent_stack.back().container_type == Type::ArrayStart)
          yamlEmitAnonymous(Type::ArrayEnd, Internal::yaml_arr_end, 1);
        else
          yamlEmitAnonymous(Type::ObjectEnd, Internal::yaml_obj_end, 1);
        indent_stack.pop_back();
      }
    }
  }
  else
  {
    std::vector<YamlIndentEntry> indent_stack;
    yamlParseBlock(data, size, pos, indent_stack, -1);

    // Close all remaining open containers
    while (!indent_stack.empty())
    {
      if (indent_stack.back().container_type == Type::ArrayStart)
        yamlEmitAnonymous(Type::ArrayEnd, Internal::yaml_arr_end, 1);
      else
        yamlEmitAnonymous(Type::ObjectEnd, Internal::yaml_obj_end, 1);
      indent_stack.pop_back();
    }
  }

  if (!yaml_tokens_.empty())
  {
    parsed_data_vector = &yaml_tokens_;
    cursor_index = 0;
  }
  return Error::NoError;
}

} // namespace JS
