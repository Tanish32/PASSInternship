// This code stores all the unknown variables connectToFromAWS.ino
#include <pgmspace.h>

#define SECRET
#define THINGNAME "MQTTXnew"

// variables required to initialize the wifi
const char WIFI_SSID[] = "T-HUB2.0";       // change this
const char WIFI_PASSWORD[] = "Innovation"; // change this
// the endpoint of the mqtt server (AWS iot core)
const char AWS_IOT_ENDPOINT[] = "";

// Authorization credentials
//  Amazon Root CA 1 that verifies that the server is AWS and authorized
static const char AWS_CERT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
-----END CERTIFICATE-----
)EOF";

// Device Certificate
static const char AWS_CERT_CRT[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----
-----END CERTIFICATE-----
)KEY";

// Device Private Key
static const char AWS_CERT_PRIVATE[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
-----END RSA PRIVATE KEY-----

)KEY";
