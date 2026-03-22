#include <string>
#include <vector>
#include <structify/structify.h>

// Example: Serializing C++ structs to CBOR binary format.
//
// structify can serialize structs to CBOR (RFC 8949), producing
// compact binary output. This complements the existing CBOR parsing
// support for full round-trip: CBOR -> C++ struct -> CBOR.

struct Sensor
{
  std::string id;
  double temperature = 0.0;
  int humidity = 0;
  bool active = false;

  STFY_OBJ(id, temperature, humidity, active);
};

struct SensorReport
{
  std::string station;
  int timestamp = 0;
  std::vector<Sensor> sensors;

  STFY_OBJ(station, timestamp, sensors);
};

int main()
{
  // Build a sensor report
  SensorReport report;
  report.station = "weather-01";
  report.timestamp = 1700000000;

  Sensor s1;
  s1.id = "temp-north";
  s1.temperature = 22.5;
  s1.humidity = 65;
  s1.active = true;
  report.sensors.push_back(s1);

  Sensor s2;
  s2.id = "temp-south";
  s2.temperature = 18.3;
  s2.humidity = 72;
  s2.active = true;
  report.sensors.push_back(s2);

  // Serialize to CBOR
  std::vector<uint8_t> cbor = STFY::serializeStructCbor(report);
  fprintf(stdout, "CBOR size: %zu bytes\n", cbor.size());

  // Print hex dump
  fprintf(stdout, "CBOR hex: ");
  for (size_t i = 0; i < cbor.size(); i++)
  {
    fprintf(stdout, "%02x", cbor[i]);
    if ((i + 1) % 32 == 0)
      fprintf(stdout, "\n          ");
  }
  fprintf(stdout, "\n\n");

  // Compare with JSON size
  std::string json = STFY::serializeStruct(report);
  fprintf(stdout, "JSON size: %zu bytes\n", json.size());
  fprintf(stdout, "CBOR is %.0f%% the size of JSON\n\n", 100.0 * cbor.size() / json.size());

  // Round-trip: parse the CBOR back into a new struct
  SensorReport parsed;
  STFY::ParseContext context;
  context.tokenizer.allowCbor(true);
  context.tokenizer.addData(reinterpret_cast<const char *>(cbor.data()), cbor.size());
  context.allow_missing_members = true;
  context.allow_unasigned_required_members = true;
  auto error = context.parseTo(parsed);

  if (error != STFY::Error::NoError)
  {
    fprintf(stderr, "Round-trip parse error: %s\n", context.makeErrorString().c_str());
    return 1;
  }

  fprintf(stdout, "=== Round-trip verified ===\n");
  fprintf(stdout, "Station: %s\n", parsed.station.c_str());
  fprintf(stdout, "Timestamp: %d\n", parsed.timestamp);
  for (const auto &sensor : parsed.sensors)
  {
    fprintf(stdout, "  Sensor %s: temp=%.1f, humidity=%d, active=%s\n", sensor.id.c_str(), sensor.temperature,
            sensor.humidity, sensor.active ? "true" : "false");
  }

  return 0;
}
