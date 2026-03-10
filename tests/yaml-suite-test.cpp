#include <structify/structify.h>
#include <catch2/catch_all.hpp>
#include <filesystem>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <cmath>
#include <cstdlib>
#include <algorithm>

namespace fs = std::filesystem;

namespace yaml_suite_test
{

struct TestValue
{
  enum Kind
  {
    Null,
    Bool,
    Number,
    String,
    Array,
    Object
  };
  Kind kind = Null;
  std::string scalar;
  std::vector<TestValue> array_items;
  std::vector<std::pair<std::string, TestValue>> object_members;

  bool operator==(const TestValue &other) const
  {
    if (kind != other.kind)
      return false;
    switch (kind)
    {
    case Null:
      return true;
    case Bool:
    case String:
      return scalar == other.scalar;
    case Number:
      if (scalar == other.scalar)
        return true;
      {
        char *end_a = nullptr;
        char *end_b = nullptr;
        double a = std::strtod(scalar.c_str(), &end_a);
        double b = std::strtod(other.scalar.c_str(), &end_b);
        if (end_a != scalar.c_str() + scalar.size())
          return false;
        if (end_b != other.scalar.c_str() + other.scalar.size())
          return false;
        return a == b;
      }
    case Array:
      return array_items == other.array_items;
    case Object:
      if (object_members.size() != other.object_members.size())
        return false;
      for (size_t i = 0; i < object_members.size(); i++)
      {
        bool found = false;
        for (size_t j = 0; j < other.object_members.size(); j++)
        {
          if (object_members[i].first == other.object_members[j].first &&
              object_members[i].second == other.object_members[j].second)
          {
            found = true;
            break;
          }
        }
        if (!found)
          return false;
      }
      return true;
    }
    return false;
  }

  bool operator!=(const TestValue &other) const
  {
    return !(*this == other);
  }

  static const char *kindName(Kind k)
  {
    switch (k)
    {
    case Null:
      return "Null";
    case Bool:
      return "Bool";
    case Number:
      return "Number";
    case String:
      return "String";
    case Array:
      return "Array";
    case Object:
      return "Object";
    }
    return "?";
  }

  std::string toString(int indent = 0) const
  {
    std::string pad(indent * 2, ' ');
    switch (kind)
    {
    case Null:
      return "null";
    case Bool:
      return scalar;
    case Number:
      return scalar;
    case String:
      return "\"" + scalar + "\"";
    case Array:
    {
      if (array_items.empty())
        return "[]";
      std::string result = "[\n";
      for (size_t i = 0; i < array_items.size(); i++)
      {
        result += pad + "  " + array_items[i].toString(indent + 1);
        if (i + 1 < array_items.size())
          result += ",";
        result += "\n";
      }
      result += pad + "]";
      return result;
    }
    case Object:
    {
      if (object_members.empty())
        return "{}";
      std::string result = "{\n";
      for (size_t i = 0; i < object_members.size(); i++)
      {
        result += pad + "  \"" + object_members[i].first + "\": " + object_members[i].second.toString(indent + 1);
        if (i + 1 < object_members.size())
          result += ",";
        result += "\n";
      }
      result += pad + "}";
      return result;
    }
    }
    return "?";
  }
};

typedef std::map<std::string, TestValue> AnchorMap;

static std::string jsonUnescapeString(const std::string &raw)
{
  std::string result;
  result.reserve(raw.size());
  for (size_t i = 0; i < raw.size(); i++)
  {
    if (raw[i] == '\\' && i + 1 < raw.size())
    {
      switch (raw[i + 1])
      {
      case '"': result += '"'; i++; break;
      case '\\': result += '\\'; i++; break;
      case '/': result += '/'; i++; break;
      case 'b': result += '\b'; i++; break;
      case 'f': result += '\f'; i++; break;
      case 'n': result += '\n'; i++; break;
      case 'r': result += '\r'; i++; break;
      case 't': result += '\t'; i++; break;
      case 'u':
        if (i + 5 < raw.size())
        {
          auto hexval = [](char c) -> int {
            if (c >= '0' && c <= '9') return c - '0';
            if (c >= 'a' && c <= 'f') return c - 'a' + 10;
            if (c >= 'A' && c <= 'F') return c - 'A' + 10;
            return -1;
          };
          int h0 = hexval(raw[i+2]), h1 = hexval(raw[i+3]),
              h2 = hexval(raw[i+4]), h3 = hexval(raw[i+5]);
          if (h0 >= 0 && h1 >= 0 && h2 >= 0 && h3 >= 0)
          {
            uint32_t cp = (uint32_t)((h0 << 12) | (h1 << 8) | (h2 << 4) | h3);
            // Handle surrogate pairs
            if (cp >= 0xD800 && cp <= 0xDBFF && i + 11 < raw.size() &&
                raw[i+6] == '\\' && raw[i+7] == 'u')
            {
              int l0 = hexval(raw[i+8]), l1 = hexval(raw[i+9]),
                  l2 = hexval(raw[i+10]), l3 = hexval(raw[i+11]);
              if (l0 >= 0 && l1 >= 0 && l2 >= 0 && l3 >= 0)
              {
                uint32_t low = (uint32_t)((l0 << 12) | (l1 << 8) | (l2 << 4) | l3);
                if (low >= 0xDC00 && low <= 0xDFFF)
                {
                  cp = 0x10000 + ((cp - 0xD800) << 10) + (low - 0xDC00);
                  i += 6;
                }
              }
            }
            // Encode as UTF-8
            if (cp <= 0x7F) result += (char)cp;
            else if (cp <= 0x7FF) { result += (char)(0xC0 | (cp >> 6)); result += (char)(0x80 | (cp & 0x3F)); }
            else if (cp <= 0xFFFF) { result += (char)(0xE0 | (cp >> 12)); result += (char)(0x80 | ((cp >> 6) & 0x3F)); result += (char)(0x80 | (cp & 0x3F)); }
            else { result += (char)(0xF0 | (cp >> 18)); result += (char)(0x80 | ((cp >> 12) & 0x3F)); result += (char)(0x80 | ((cp >> 6) & 0x3F)); result += (char)(0x80 | (cp & 0x3F)); }
            i += 5;
          }
          else
            result += raw[i];
        }
        else
          result += raw[i];
        break;
      default:
        result += raw[i];
        break;
      }
    }
    else
    {
      result += raw[i];
    }
  }
  return result;
}

static TestValue scalarFromToken(const STFY::Token &token, AnchorMap &anchors, bool json_mode = false)
{
  // Handle alias tokens
  if (token.value_type == STFY::Type::Alias)
  {
    std::string alias_name(token.value.data, token.value.size);
    auto it = anchors.find(alias_name);
    if (it != anchors.end())
      return it->second;
    // Unknown alias - return as string
    TestValue val;
    val.kind = TestValue::String;
    val.scalar = "*" + alias_name;
    return val;
  }

  TestValue val;
  val.scalar = std::string(token.value.data, token.value.size);
  switch (token.value_type)
  {
  case STFY::Type::Null:
    val.kind = TestValue::Null;
    break;
  case STFY::Type::Bool:
    val.kind = TestValue::Bool;
    break;
  case STFY::Type::Number:
    val.kind = TestValue::Number;
    break;
  case STFY::Type::String:
  case STFY::Type::Ascii:
    val.kind = TestValue::String;
    if (json_mode && token.value_type == STFY::Type::String)
      val.scalar = jsonUnescapeString(val.scalar);
    break;
  default:
    val.kind = TestValue::String;
    break;
  }

  // Record anchor if present
  if (token.anchor.size > 0)
  {
    std::string anchor_name(token.anchor.data, token.anchor.size);
    anchors[anchor_name] = val;
  }

  return val;
}

static const int MAX_TOKENS = 10000;

static TestValue parseContainer(STFY::Tokenizer &tok, const STFY::Token &start_token, int &token_count, AnchorMap &anchors, bool json_mode = false)
{
  TestValue val;
  if (start_token.value_type == STFY::Type::ObjectStart)
  {
    val.kind = TestValue::Object;
    while (true)
    {
      if (++token_count > MAX_TOKENS)
        throw std::runtime_error("token limit exceeded");
      STFY::Token inner;
      STFY::Error err = tok.nextTokens(&inner, 1).error;
      if (err != STFY::Error::NoError)
        break;
      if (inner.value_type == STFY::Type::ObjectEnd)
        break;

      std::string key(inner.name.data, inner.name.size);
      if (json_mode && inner.name_type == STFY::Type::String)
        key = jsonUnescapeString(key);

      // Resolve alias keys (when *alias is used as a mapping key, name_type == Alias)
      if (inner.name_type == STFY::Type::Alias && !key.empty())
      {
        auto anchor_it = anchors.find(key);
        if (anchor_it != anchors.end() && anchor_it->second.kind == TestValue::String)
          key = anchor_it->second.scalar;
      }

      // Record key-side anchor (e.g., &a key: value → anchor "a" maps to key value)
      if (inner.name_anchor.size > 0)
      {
        TestValue key_val;
        key_val.kind = TestValue::String;
        key_val.scalar = key;
        anchors[std::string(inner.name_anchor.data, inner.name_anchor.size)] = key_val;
      }

      if (inner.value_type == STFY::Type::Alias)
      {
        val.object_members.push_back({key, scalarFromToken(inner, anchors, json_mode)});
      }
      else if (inner.value_type == STFY::Type::ObjectStart || inner.value_type == STFY::Type::ArrayStart)
      {
        TestValue child = parseContainer(tok, inner, token_count, anchors, json_mode);
        // Record anchor on container if present
        if (inner.anchor.size > 0)
        {
          std::string anchor_name(inner.anchor.data, inner.anchor.size);
          anchors[anchor_name] = child;
        }
        val.object_members.push_back({key, child});
      }
      else
      {
        val.object_members.push_back({key, scalarFromToken(inner, anchors, json_mode)});
      }
    }
  }
  else if (start_token.value_type == STFY::Type::ArrayStart)
  {
    val.kind = TestValue::Array;
    while (true)
    {
      if (++token_count > MAX_TOKENS)
        throw std::runtime_error("token limit exceeded");
      STFY::Token inner;
      STFY::Error err = tok.nextTokens(&inner, 1).error;
      if (err != STFY::Error::NoError)
        break;
      if (inner.value_type == STFY::Type::ArrayEnd)
        break;

      if (inner.value_type == STFY::Type::Alias)
      {
        val.array_items.push_back(scalarFromToken(inner, anchors, json_mode));
      }
      else if (inner.value_type == STFY::Type::ObjectStart || inner.value_type == STFY::Type::ArrayStart)
      {
        TestValue child = parseContainer(tok, inner, token_count, anchors, json_mode);
        if (inner.anchor.size > 0)
        {
          std::string anchor_name(inner.anchor.data, inner.anchor.size);
          anchors[anchor_name] = child;
        }
        val.array_items.push_back(child);
      }
      else
      {
        val.array_items.push_back(scalarFromToken(inner, anchors, json_mode));
      }
    }
  }
  return val;
}

static TestValue tokensToValue(STFY::Tokenizer &tok, bool json_mode = false)
{
  AnchorMap anchors;
  STFY::Token token;
  STFY::Error error = tok.nextTokens(&token, 1).error;
  if (error != STFY::Error::NoError)
  {
    TestValue val;
    val.kind = TestValue::Null;
    return val;
  }
  if (token.value_type == STFY::Type::Alias)
  {
    return scalarFromToken(token, anchors, json_mode);
  }
  if (token.value_type == STFY::Type::ObjectStart || token.value_type == STFY::Type::ArrayStart)
  {
    int token_count = 0;
    TestValue result = parseContainer(tok, token, token_count, anchors, json_mode);
    if (token.anchor.size > 0)
    {
      std::string anchor_name(token.anchor.data, token.anchor.size);
      anchors[anchor_name] = result;
    }
    return result;
  }
  return scalarFromToken(token, anchors, json_mode);
}

static std::string readFile(const fs::path &path)
{
  std::ifstream file(path, std::ios::binary);
  if (!file.is_open())
    return {};
  std::ostringstream ss;
  ss << file.rdbuf();
  return ss.str();
}

static bool isUnsupported(const fs::path &, const std::string &)
{
  // All feature-based skipping is handled via known_failures
  return false;
}

static bool isMultiPart(const fs::path &test_dir)
{
  for (auto &entry : fs::directory_iterator(test_dir))
  {
    if (entry.is_directory())
    {
      std::string name = entry.path().filename().string();
      if (!name.empty() && std::all_of(name.begin(), name.end(), ::isdigit))
        return true;
    }
  }
  return false;
}

struct TestResult
{
  std::string test_id;
  std::string skip_reason;
  bool passed = false;
  std::string failure_detail;
};

TEST_CASE("yaml_test_suite", "[yaml][suite]")
{
  fs::path suite_dir(YAML_TEST_SUITE_DIR);
  REQUIRE(fs::exists(suite_dir));

  // Hardcoded skips: tests that crash the tokenizer (segfault/assert)
  std::set<std::string> tokenizer_crash = {
  };

  // Tests that fail due to known tokenizer limitations:
  // - multi-line plain scalars, block scalar chomping/indentation
  // - flow collections with complex content, implicit keys
  // - YAML 1.1 vs 1.2 boolean differences (yes/no/on/off)
  // - document markers (---/...) in non-trivial positions
  // Remove tests from this set as the tokenizer is improved.
  std::set<std::string> known_failures = {
    "2SXE", // Complex anchor reuse on same line (&a: key: &a value)
    "P76L", // Requires %TAG directive support
  };

  std::set<std::string> extra_skip;
  extra_skip.insert(tokenizer_crash.begin(), tokenizer_crash.end());
  extra_skip.insert(known_failures.begin(), known_failures.end());

  // Collect and sort test directories for deterministic order
  std::vector<fs::directory_entry> test_dirs;
  for (auto &entry : fs::directory_iterator(suite_dir))
  {
    if (!entry.is_directory())
      continue;
    std::string dirname = entry.path().filename().string();
    if (dirname == "tags" || dirname == "name" || dirname == ".git")
      continue;
    test_dirs.push_back(entry);
  }
  std::sort(test_dirs.begin(), test_dirs.end(), [](const fs::directory_entry &a, const fs::directory_entry &b) {
    return a.path().filename() < b.path().filename();
  });

  std::vector<TestResult> results;
  int pass_count = 0;
  int skip_count = 0;
  int fail_count = 0;

  for (auto &entry : test_dirs)
  {
    std::string test_id = entry.path().filename().string();
    TestResult result;
    result.test_id = test_id;

    // Check skip conditions
    if (isUnsupported(suite_dir, test_id))
    {
      result.skip_reason = "unsupported feature";
      skip_count++;
      results.push_back(result);
      continue;
    }
    if (fs::exists(entry.path() / "error"))
    {
      result.skip_reason = "expected error test";
      skip_count++;
      results.push_back(result);
      continue;
    }
    if (!fs::exists(entry.path() / "in.yaml"))
    {
      result.skip_reason = "no in.yaml";
      skip_count++;
      results.push_back(result);
      continue;
    }
    if (!fs::exists(entry.path() / "in.json"))
    {
      result.skip_reason = "no JSON reference";
      skip_count++;
      results.push_back(result);
      continue;
    }
    if (isMultiPart(entry.path()))
    {
      result.skip_reason = "multi-part test";
      skip_count++;
      results.push_back(result);
      continue;
    }
    if (extra_skip.count(test_id))
    {
      result.skip_reason = "known unsupported";
      skip_count++;
      results.push_back(result);
      continue;
    }

    std::string yaml_input = readFile(entry.path() / "in.yaml");
    std::string json_input = readFile(entry.path() / "in.json");

    try
    {
      // Parse YAML
      STFY::Tokenizer yaml_tok;
      yaml_tok.allowYaml(true);
      yaml_tok.addData(yaml_input.c_str(), yaml_input.size());
      TestValue yaml_val = tokensToValue(yaml_tok);

      // Parse JSON reference
      STFY::Tokenizer json_tok;
      json_tok.addData(json_input.c_str(), json_input.size());
      TestValue json_val = tokensToValue(json_tok, true);

      // If JSON parsing returned Null and input isn't empty, try YAML parser
      // (handles bare scalars like "foo" that aren't valid standalone JSON)
      if (json_val.kind == TestValue::Null && !json_input.empty())
      {
        STFY::Tokenizer yaml_json_tok;
        yaml_json_tok.allowYaml(true);
        yaml_json_tok.addData(json_input.c_str(), json_input.size());
        TestValue retry = tokensToValue(yaml_json_tok);
        if (retry.kind != TestValue::Null)
          json_val = retry;
      }

      if (yaml_val == json_val)
      {
        result.passed = true;
        pass_count++;
      }
      else
      {
        fail_count++;
        std::ostringstream detail;
        detail << "Test " << test_id << " FAILED\n";
        detail << "  YAML input:\n    ";
        std::string yaml_preview = yaml_input.substr(0, 200);
        for (auto &c : yaml_preview)
        {
          if (c == '\n')
            detail << "\n    ";
          else
            detail << c;
        }
        if (yaml_input.size() > 200)
          detail << "...";
        detail << "\n";
        if (yaml_val.kind != json_val.kind)
        {
          detail << "  Kind mismatch: YAML=" << TestValue::kindName(yaml_val.kind)
                 << " JSON=" << TestValue::kindName(json_val.kind) << "\n";
        }
        detail << "  YAML result: " << yaml_val.toString() << "\n";
        detail << "  JSON expected: " << json_val.toString() << "\n";
        result.failure_detail = detail.str();
      }
    }
    catch (const std::exception &e)
    {
      fail_count++;
      result.failure_detail = "Test " + test_id + " EXCEPTION: " + e.what() + "\n";
    }
    results.push_back(result);
  }

  // Print results to stderr so they're always visible
  fprintf(stderr, "\n=== YAML Test Suite Results ===\n");
  fprintf(stderr, "%d passed, %d failed, %d skipped out of %d tests\n\n",
          pass_count, fail_count, skip_count, (int)results.size());

  if (fail_count > 0)
  {
    std::vector<std::string> failed_ids;
    for (auto &r : results)
    {
      if (!r.skip_reason.empty() || r.passed)
        continue;
      fprintf(stderr, "%s\n", r.failure_detail.c_str());
      failed_ids.push_back(r.test_id);
    }
    fprintf(stderr, "=== Failed test IDs for extra_skip: ");
    for (size_t i = 0; i < failed_ids.size(); i++)
    {
      if (i > 0)
        fprintf(stderr, ", ");
      fprintf(stderr, "\"%s\"", failed_ids[i].c_str());
    }
    fprintf(stderr, " ===\n");
  }

  INFO("YAML Test Suite Results: " << pass_count << " passed, " << fail_count << " failed, " << skip_count << " skipped out of " << results.size() << " tests");
  CHECK(fail_count == 0);
}

} // namespace yaml_suite_test
