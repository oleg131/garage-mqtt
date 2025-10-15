# 🔗 Twilio → Airtable → AWS IoT → ESP8266 Workflow

![System Workflow Diagram](./img1.png)

This setup links an **incoming Twilio voice call** to an **Airtable authorization list**, and if the caller is authorized, triggers an **AWS IoT Core** action that activates a **physical clicker** on an **ESP8266** via MQTT.

---

## 🧭 Overview

1. **Twilio Function**

   - Receives an incoming call event.
   - Checks caller’s phone number in **Airtable**.
   - If authorized:
     - Greets the caller by name.
     - Sends their record to an **AWS Lambda webhook**.
   - If not authorized:
     - Plays a rejection message.

2. **AWS Lambda**

   - Receives the webhook request from Twilio.
   - Publishes the event payload to an **AWS IoT MQTT topic**.

3. **ESP8266 Device**
   - Connects securely to **AWS IoT Core** via MQTT.
   - Subscribes to the same MQTT topic.
   - When a message arrives, it triggers an **optocoupler** or **relay output** for a brief pulse (simulating a clicker press).

---

## ⚙️ Twilio Function Setup

**Environment Variables**
| Name | Description |
|------|-------------|
| `AIRTABLE_BASE_ID` | Your Airtable base ID |
| `AIRTABLE_TABLE` | Table name containing authorized numbers |
| `AIRTABLE_TOKEN` | Airtable personal access token |
| `ENDPOINT_URL` | URL of your AWS Lambda webhook |
| `ENDPOINT_API_KEY` | API key for the Lambda endpoint (if required) |

**Logic**

- Uses `axios` to query Airtable:

- Airtable must contain 2 columns with names

  - _Name_
  - _Phone_

- If a record is found:

  - Plays “Hello [Name]. Your phone number is authorized.”
  - Sends record data to AWS Lambda:

- Otherwise plays a rejection message.

---

## ☁️ AWS Lambda Setup

**Purpose:**
Receive webhook POSTs and forward data to AWS IoT Core via MQTT.

**Environment Variables**

| Name            | Description                 |
| --------------- | --------------------------- |
| `MQTT_ENDPOINT` | AWS IoT Data Plane endpoint |
| `MQTT_TOPIC`    | MQTT topic to publish to    |

**Permissions (IAM Policy)**
The Lambda role must allow:

```json
{
  "Effect": "Allow",
  "Action": ["iot:Publish"],
  "Resource": "arn:aws:iot:us-east-1:<account-id>:topic/<topic-name>"
}
```

**Flow**

- Validates request body.
- Publishes to IoT.

---

## 🔌 ESP8266 Firmware Setup

**Purpose:**
Subscribe to the AWS IoT MQTT topic and activate a GPIO pin when a message is received.

**Libraries Used**

- `ESP8266WiFi.h`
- `WiFiClientSecureBearSSL.h`
- `PubSubClient.h`
- `time.h`

**Core Behavior**

- Connects to Wi-Fi.
- Authenticates with AWS IoT using (required by AWS IoT):

  - Root CA
  - Device certificate
  - Private key

- Subscribes to `MQTT_TOPIC`.
- When an MQTT message arrives, runs:

  ```cpp
  void processAction() {
    Serial.println("[Action] Triggered");
    digitalWrite(OUTPUT_PIN, HIGH); // activate relay or optocoupler
    delay(PIN_DELAY);
    digitalWrite(OUTPUT_PIN, LOW);
  }
  ```

**Configuration Variables**

| Name             | Description                        | Example                                           |
| ---------------- | ---------------------------------- | ------------------------------------------------- |
| `WIFI_SSID`      | Your Wi-Fi network name            | `"MyWiFi"`                                        |
| `WIFI_PASSWORD`  | Wi-Fi password                     | `"password123"`                                   |
| `AWS_ENDPOINT`   | AWS IoT Core endpoint              | `"abcd123456789-ats.iot.us-east-1.amazonaws.com"` |
| `MQTT_TOPIC`     | MQTT topic to listen to            | `"mqtt_topic"`                                    |
| `MQTT_CLIENT_ID` | Unique ID for the ESP device       | `"mqtt_client_d"`                                 |
| `OUTPUT_PIN`     | GPIO pin controlling relay/clicker | `5` (D1)                                          |
| `PIN_DELAY`      | Duration (ms) the pin stays HIGH   | `5000`                                            |
| `RESTART_DELAY`  | Periodic reboot interval (ms)      | `3600000`                                         |

**Certificates**
Paste your PEM-formatted certificates in the provided arrays:

```cpp
static const char deviceCertificate[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----
<device cert>
-----END CERTIFICATE-----
)KEY";

static const char privateKeyFile[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
<private key>
-----END RSA PRIVATE KEY-----
)KEY";

static const char rootCaCertificate[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
<Amazon Root CA 1>
-----END CERTIFICATE-----
)EOF";
```

---

## 🧩 End-to-End Flow

```
📞 Caller → Twilio Function → Airtable (verify phone)
       ↳ Authorized → POST to AWS Lambda → Publish to AWS IoT
            ↳ ESP8266 subscribed → Triggers GPIO (clicker)
       ↳ Not authorized → Plays “Sorry, not authorized.”
```

---

## ✅ Summary

| Component           | Responsibility                                    |
| ------------------- | ------------------------------------------------- |
| **Twilio Function** | Verifies caller and forwards record               |
| **Airtable**        | Stores authorized phone numbers                   |
| **AWS Lambda**      | Publishes message to MQTT topic                   |
| **AWS IoT Core**    | Relays MQTT message to subscribed devices         |
| **ESP8266**         | Subscribes to topic and triggers clicker via GPIO |
