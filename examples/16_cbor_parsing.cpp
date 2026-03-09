#include <string>
#include <vector>
#include <json_struct/json_struct.h>

// Example: Parsing CBOR binary data into C++ structs.
//
// json_struct's tokenizer supports CBOR (RFC 8949) natively - no external
// library needed. Just enable CBOR mode and parse directly into your structs,
// the same way you would with JSON or YAML.

struct ServerSettings
{
  std::string host;
  int port = 0;
  int workers = 1;

  JS_OBJ(host, port, workers);
};

struct AppConfig
{
  std::string name;
  double version = 0.0;
  ServerSettings server;
  bool debug = false;
  std::vector<std::string> features;

  JS_OBJ(name, version, server, debug, features);
};

int main()
{
  // This is pre-encoded CBOR data equivalent to:
  // {
  //   "name": "my-service",
  //   "version": 2.5 (float64),
  //   "server": {"host": "0.0.0.0", "port": 8080, "workers": 4},
  //   "debug": false,
  //   "features": ["auth", "metrics"]
  // }
  const unsigned char cbor_data[] = {
    0xA5, // map(5)

    // "name"
    0x64, 'n', 'a', 'm', 'e',
    // "my-service"
    0x6A, 'm', 'y', '-', 's', 'e', 'r', 'v', 'i', 'c', 'e',

    // "version"
    0x67, 'v', 'e', 'r', 's', 'i', 'o', 'n',
    // float64(2.5)
    0xFB, 0x40, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    // "server"
    0x66, 's', 'e', 'r', 'v', 'e', 'r',
    // map(3)
    0xA3,
      0x64, 'h', 'o', 's', 't',
      0x67, '0', '.', '0', '.', '0', '.', '0',
      0x64, 'p', 'o', 'r', 't',
      0x19, 0x1F, 0x90, // 8080
      0x67, 'w', 'o', 'r', 'k', 'e', 'r', 's',
      0x04,

    // "debug"
    0x65, 'd', 'e', 'b', 'u', 'g',
    0xF4, // false

    // "features"
    0x68, 'f', 'e', 'a', 't', 'u', 'r', 'e', 's',
    // array(2)
    0x82,
      0x64, 'a', 'u', 't', 'h',
      0x67, 'm', 'e', 't', 'r', 'i', 'c', 's'
  };

  AppConfig config;
  JS::ParseContext context;
  context.tokenizer.allowCbor(true);
  context.tokenizer.addData((const char *)cbor_data, sizeof(cbor_data));

  if (context.parseTo(config) != JS::Error::NoError)
  {
    std::string errorStr = context.makeErrorString();
    fprintf(stderr, "Error parsing CBOR: %s\n", errorStr.c_str());
    return -1;
  }

  fprintf(stdout, "Application: %s v%.1f\n\n", config.name.c_str(), config.version);

  fprintf(stdout, "Server:\n");
  fprintf(stdout, "  Host: %s\n", config.server.host.c_str());
  fprintf(stdout, "  Port: %d\n", config.server.port);
  fprintf(stdout, "  Workers: %d\n\n", config.server.workers);

  fprintf(stdout, "Debug: %s\n\n", config.debug ? "true" : "false");

  fprintf(stdout, "Features:\n");
  for (const auto &feature : config.features)
  {
    fprintf(stdout, "  - %s\n", feature.c_str());
  }

  return 0;
}
