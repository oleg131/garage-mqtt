#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecureBearSSL.h>
#include <PubSubClient.h>
#include <time.h>

// ------------------------------------------------------------------
// Certificates
// ------------------------------------------------------------------

// Device certificate
// Copy and paste contents from *-certificate.pem.crt
static const char deviceCertificate[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----
-----END CERTIFICATE-----
)KEY";

// Key files / Private key file
// Copy and paste contents from *-private.pem.key
static const char privateKeyFile[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
-----END RSA PRIVATE KEY-----
)KEY";

// RSA 2048 bit key: Amazon Root CA 1
// Copy and paste contents from AmazonRootCA1.cer
static const char rootCaCertificate[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
-----END CERTIFICATE-----
)EOF";

// ------------------------------------------------------------------
// Configuration
// ------------------------------------------------------------------
const char *WIFI_SSID = "";
const char *WIFI_PASSWORD = "";

const char *AWS_ENDPOINT = "";
const char *MQTT_TOPIC = "";
const char *MQTT_CLIENT_ID = "";

// Hardware
const int OUTPUT_PIN = 5;

// Timers
const unsigned long PIN_DELAY = 5000;
const unsigned long RESTART_DELAY = 3600000;
unsigned long lastTime = 0;

// ------------------------------------------------------------------
// Globals
// ------------------------------------------------------------------
BearSSL::WiFiClientSecure secureClient;
PubSubClient mqttClient(secureClient);
WiFiClient wifiClient;

BearSSL::X509List trustAnchorCertificate(rootCaCertificate);
BearSSL::X509List clientCertificate(deviceCertificate);
BearSSL::PrivateKey clientPrivateKey(privateKeyFile);

// ------------------------------------------------------------------
// Utilities
// ------------------------------------------------------------------
void syncTime()
{
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  Serial.print("[Time] Syncing");
  time_t now = time(nullptr);
  while (now < 1000000000)
  { // wait until valid
    delay(500);
    Serial.print(".");
    now = time(nullptr);
    yield();
  }
  Serial.printf("\n[Time] Synced: %s", ctime(&now));
}

void processAction()
{
  Serial.println("[Action] Triggered");
  digitalWrite(OUTPUT_PIN, HIGH); // Optocoupler
  delay(PIN_DELAY);
  digitalWrite(OUTPUT_PIN, LOW);
}

// ------------------------------------------------------------------
// MQTT callback
// ------------------------------------------------------------------
void receiveMessage(char *topic, byte *payload, unsigned int length)
{
  Serial.printf("[MQTT] Msg on %s (%u bytes): ", topic, length);
  for (unsigned int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Trigger the action
  processAction();
}

void connectMQTT()
{
  Serial.println("[MQTT] Initializing secure client...");

  // Configure MQTT connection
  mqttClient.setServer(AWS_ENDPOINT, 8883);
  mqttClient.setCallback(receiveMessage);

  mqttClient.setKeepAlive(60);

  // Connect to AWS IoT Core
  Serial.printf("[MQTT] Connecting to AWS IoT...");
  String clientId = MQTT_CLIENT_ID;

  while (!mqttClient.connect(clientId.c_str()))
  {
    delay(3000);
    Serial.print(".");
    yield();
  }

  Serial.printf("\n[MQTT] ✅ Connected as %s\n", clientId.c_str());

  // Subscribe to your topic
  if (mqttClient.subscribe(MQTT_TOPIC))
  {
    Serial.printf("[MQTT] Subscribed to topic: %s\n", MQTT_TOPIC);
  }
  else
  {
    Serial.printf("[MQTT] ❌ Failed to subscribe to topic: %s\n", MQTT_TOPIC);
  }
}

// ------------------------------------------------------------------
// Setup
// ------------------------------------------------------------------
void setup()
{
  Serial.begin(115200);
  pinMode(OUTPUT_PIN, OUTPUT);
  digitalWrite(OUTPUT_PIN, LOW);

  Serial.println("[BOOT] Starting...");

  // Connect Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.printf("[WiFi] Connecting to %s", WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    yield(); // ✅ prevents watchdog reset during slow Wi-Fi connect
  }
  Serial.printf("\n[WiFi] Connected! IP: %s\n", WiFi.localIP().toString().c_str());

  // Necessary for MQTT client to authenticate
  syncTime();

  // Configure TLS certificates (needed once per session)
  secureClient.setTrustAnchors(&trustAnchorCertificate);
  secureClient.setClientRSACert(&clientCertificate, &clientPrivateKey);

  // Connect to MQTT
  connectMQTT();

  Serial.println("[BOOT] SETUP COMPLETE");
}

// ------------------------------------------------------------------
// Loop
// ------------------------------------------------------------------
void loop()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("[WiFi] Lost connection, reconnecting...");
    WiFi.reconnect();
    delay(2000);
    return; // Skip rest of loop until Wi-Fi is back
  }

  if (!mqttClient.connected())
  {
    connectMQTT();
  }
  mqttClient.loop();

  unsigned long nowMs = millis();
  if (nowMs >= RESTART_DELAY)
  {
    ESP.restart();
  }
}