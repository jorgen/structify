#include <string>
#include <vector>
#include <json_struct/json_struct.h>

// Example showcasing relaxed JSON parsing rules suitable for config files:
// - Comments using // syntax
// - Newlines instead of commas
// - Unquoted property names (ASCII mode)
// - Trailing commas

const char config[] = R"config(
{
  // Server configuration
  host: localhost
  port: 8080

  // Database settings
  database: {
    name: myapp_db
    user: admin
    // Password should be set via environment variable
    max_connections: 100
    timeout: 30,  // Trailing comma is OK
  }

  // Feature flags
  features: [
    analytics
    logging
    caching,  // Trailing comma here too
  ]

  // Logging configuration
  log_level: info  // Can be: debug, info, warn, error
  log_file: /var/log/myapp.log
}
)config";

struct DatabaseConfig
{
  std::string name;
  std::string user;
  int max_connections = 10;
  int timeout = 5;

  JS_OBJ(name, user, max_connections, timeout);
};

struct ServerConfig
{
  std::string host;
  int port = 3000;
  DatabaseConfig database;
  std::vector<std::string> features;
  std::string log_level;
  std::string log_file;

  JS_OBJ(host, port, database, features, log_level, log_file);
};

int main()
{
  ServerConfig config_obj;
  JS::ParseContext context(config);

  // Enable relaxed parsing rules for config files
  context.tokenizer.allowAsciiType(true);           // Allow unquoted property names
  context.tokenizer.allowNewLineAsTokenDelimiter(true);  // Newline can replace comma
  context.tokenizer.allowSuperfluousComma(true);    // Allow trailing commas
  context.tokenizer.allowComments(true);            // Allow // comments

  if (context.parseTo(config_obj) != JS::Error::NoError)
  {
    std::string errorStr = context.makeErrorString();
    fprintf(stderr, "Error parsing config: %s\n", errorStr.c_str());
    return -1;
  }

  // Display parsed configuration
  fprintf(stdout, "Server Configuration:\n");
  fprintf(stdout, "  Host: %s\n", config_obj.host.c_str());
  fprintf(stdout, "  Port: %d\n", config_obj.port);
  fprintf(stdout, "\nDatabase:\n");
  fprintf(stdout, "  Name: %s\n", config_obj.database.name.c_str());
  fprintf(stdout, "  User: %s\n", config_obj.database.user.c_str());
  fprintf(stdout, "  Max Connections: %d\n", config_obj.database.max_connections);
  fprintf(stdout, "  Timeout: %d\n", config_obj.database.timeout);
  fprintf(stdout, "\nFeatures:\n");
  for (const auto& feature : config_obj.features)
  {
    fprintf(stdout, "  - %s\n", feature.c_str());
  }
  fprintf(stdout, "\nLogging:\n");
  fprintf(stdout, "  Level: %s\n", config_obj.log_level.c_str());
  fprintf(stdout, "  File: %s\n", config_obj.log_file.c_str());

  return 0;
}
