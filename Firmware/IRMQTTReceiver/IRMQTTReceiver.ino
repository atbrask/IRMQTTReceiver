/* General purpose IR MQTT Receiver
 * 
 * This is basically a mixture of the mqtt_esp8266 example from 
 * PubSubClient, the IRrecvDumpV2 example from IRemoteESP8266, and
 * the JsonGeneratorExample from ArduinoJson.
 * 
 * Notes:
 * - Remember to set MQTT_MAX_PACKET_SIZE to 256 in pubsubclient/src/PubSubClient.h
 * - Remember to set ARDUINOJSON_USE_LONG_LONG to 1 in ArduinoJson/src/ArduinoJson/Configuration.hpp
 */

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <ArduinoJson.h>

// SETTINGS

const char* ssid = "...";
const char* password = "...";
const char* mqtt_server = "...";
const char* mqtt_topic = "home/bedroom/IR/receiver";
const uint16_t RECV_PIN = 14; // Pin D5 on NodeMCU and WeMos D1 Mini
const uint16_t CAPTURE_BUFFER_SIZE = 1024;

// STATE

IRrecv irrecv(RECV_PIN, CAPTURE_BUFFER_SIZE);
WiFiClient espClient;
PubSubClient client(espClient);

void setup()
{
  // Start WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
    delay(500);

  // Start MQTT
  randomSeed(micros());
  client.setServer(mqtt_server, 1883);
  connectMQTT();

  // Start IR library
  irrecv.enableIRIn();
}

void loop()
{
  // Update MQTT connection
  if (!client.connected()) {
    connectMQTT();
  }
  client.loop();

  // Check IR receiver state
  decode_results results;
  irparams_t saveState;
  if (irrecv.decode(&results, &saveState))
  {
    // Publish any captured IR code
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["Encoding"] = getEncodingName(results.decode_type);
    root["Data"] = results.value;
    root["Address"] = results.address;
    root["Command"] = results.command;
    root["Repeat"] = results.repeat;
    char msg[128];
    root.printTo(msg, sizeof(msg));
    client.publish(mqtt_topic, msg);
  }
}

void connectMQTT()
{
  while (!client.connected())
  {
    String clientId = "IRMQTTReceiver-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (!client.connect(clientId.c_str()))
      delay(5000);
  }
}

const char* getEncodingName(decode_type_t type)
{
  switch (type) {
    default:
    case UNKNOWN:      return "UNKNOWN";
    case NEC:          return "NEC";
    case NEC_LIKE:     return "NEC (non-strict)";
    case SONY:         return "SONY";
    case RC5:          return "RC5";
    case RC6:          return "RC6";
    case DISH:         return "DISH";
    case SHARP:        return "SHARP";
    case JVC:          return "JVC";
    case SANYO:        return "SANYO";
    case SANYO_LC7461: return "SANYO_LC7461";
    case MITSUBISHI:   return "MITSUBISHI";
    case SAMSUNG:      return "SAMSUNG";
    case LG:           return "LG";
    case WHYNTER:      return "WHYNTER";
    case AIWA_RC_T501: return "AIWA_RC_T501";
    case PANASONIC:    return "PANASONIC";
    case DENON:        return "DENON";
    case COOLIX:       return "COOLIX";
  }
}

