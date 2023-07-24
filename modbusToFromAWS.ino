#include "secrets.h"
#include <WiFi.h>
#include <ModbusIP_ESP8266.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <string>

using namespace std;
// Modbus Settings
const int REG = 0;
IPAddress IP_ModbusSERVER(10, 50, 60, 33);
const int number_REG = 8;
ModbusIP mbClient;
uint16_t res[8];

WiFiClientSecure net;
PubSubClient client(net);

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi Connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void connectModbus() {
  mbClient.client(); // Initialize the Modbus client
}

void connectAWS() {
  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.setServer(AWS_IOT_ENDPOINT, 8883);

  // Create a message handler
  client.setCallback(messageHandler);

  Serial.print("Connecting to AWS IoT");

  while (!client.connect(THINGNAME)) {
    Serial.print(".");
    delay(1000);
  }

  if (!client.connected()) {
    Serial.println("AWS IoT Timeout!");
    return;
  }

  // Subscribe to a topic
  client.subscribe("esp32/sub");
  Serial.println("AWS IoT Connected!");
}

void readModbusData() {
  if (mbClient.isConnected(IP_ModbusSERVER)) {
    mbClient.readHreg(IP_ModbusSERVER, REG, res, number_REG); // Read holding registers from Modbus slave server
  } else {
    mbClient.connect(IP_ModbusSERVER); // Connect to the Modbus slave server
  }

  mbClient.task(); // Perform Modbus communication tasks
}

void convertAndPublishData() {
  StaticJsonDocument<256> doc;
  doc["time"] = millis();
  for (int i = 0; i < number_REG; i++) {
    doc[String(i)] = res[i]; // Add Modbus data to the JSON document
  }

  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // Serialize the JSON document to a string

  client.publish("esp32/pub", jsonBuffer); // Publish the JSON message to the "esp32/pub" topic
}
//----------------------------------------------------------------
void messageHandler(char* topic, byte* payload, unsigned int length) {
  Serial.print("Incoming: ");
  Serial.println(topic);

  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, payload); // Deserialize the JSON payload

  if (error) {
    Serial.print("Error parsing JSON: ");
    Serial.println(error.c_str());
    return;
  }
  
  // Assuming doc is the JSON document
  uint16_t numberValue = 0;
  char stringValue[20]; // Adjust the size as per your requirement
  uint16_t boolValue = 0;

  if (doc.containsKey("age")) {
    numberValue = doc["age"].as<uint16_t>();
  }

  if (doc.containsKey("name")) {
    const char* nameValue = doc["name"].as<const char*>();
    strncpy(stringValue, nameValue, sizeof(stringValue) - 1);
    stringValue[sizeof(stringValue) - 1] = '\0'; // Null-terminate the string
  }

  uint16_t arr[5] = {0}; // Initialize the array elements to 0
  for (int i = 0; i < 5 && stringValue[i] != '\0'; i++) {
    arr[i] = stringValue[i];
  }

  if (doc.containsKey("isActive")) {
    boolValue = doc["isActive"].as<uint16_t>();
  }

  if (mbClient.isConnected(IP_ModbusSERVER)) {
    mbClient.writeHreg(IP_ModbusSERVER, number_REG, &numberValue, 1);
    mbClient.writeHreg(IP_ModbusSERVER, number_REG + 1, arr, 5);
    mbClient.writeHreg(IP_ModbusSERVER, number_REG + 6, &boolValue, 1);
  } else {
    mbClient.connect(IP_ModbusSERVER); // Connect to the Modbus slave server
  }
  
  mbClient.task(); // Perform Modbus communication tasks
  Serial.println("Updated Modbus data with received JSON");
}

//----------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  connectWiFi();    // Connect to Wi-Fi
  connectModbus();  // Connect to the Modbus slave server
  connectAWS();     // Connect to AWS IoT Core
}

void loop() {
  client.loop(); // Handle incoming and outgoing MQTT messages

  readModbusData(); // Read data from Modbus slave server
  convertAndPublishData(); // Convert and publish data to AWS IoT Core

  delay(1000);
}
