# 🛰️ ESP8266 + AWS IoT Firmware

This firmware connects an ESP8266 device to **AWS IoT Core** over a secure TLS (MQTT) connection. It listens for MQTT messages on a topic (e.g., `"garage"`) and triggers a GPIO output action when a message arrives.

---

## ⚙️ Variables to Configure

Before uploading the sketch, fill in the following variables in the source code:

### 🧩 Wi-Fi Configuration

| Variable        | Description                    | Example            | Sensitive? |
| --------------- | ------------------------------ | ------------------ | ---------- |
| `WIFI_SSID`     | Your Wi-Fi network name (SSID) | `"MyHomeWiFi"`     | 🔴 Yes     |
| `WIFI_PASSWORD` | Your Wi-Fi password            | `"supersecret123"` | 🔴 Yes     |

> Used for connecting your ESP8266 to the local network and Internet.

---

### ☁️ AWS IoT Configuration

| Variable         | Description                          | Example                                           | Sensitive?                     |
| ---------------- | ------------------------------------ | ------------------------------------------------- | ------------------------------ |
| `AWS_ENDPOINT`   | Your AWS IoT Core endpoint           | `"abcd123456789-ats.iot.us-east-1.amazonaws.com"` | 🟠 Mildly sensitive            |
| `MQTT_CLIENT_ID` | Unique ID for your device in AWS IoT | `"esp8266_garage_01"`                             | 🟢 Safe (but should be unique) |
| `MQTT_TOPIC`     | MQTT topic to subscribe to           | `"garage"`                                        | 🟢 Usually safe                |

> You can find your **AWS IoT Endpoint** in the AWS Console under:
> **IoT Core → Settings → Endpoint**

---

### 🔐 Certificate and Key Files

| Constant              | Description                      | File Source                            | Sensitive?                 |
| --------------------- | -------------------------------- | -------------------------------------- | -------------------------- |
| `deviceCertificate[]` | Device certificate in PEM format | `*-certificate.pem.crt` (from AWS IoT) | 🔴 Yes                     |
| `privateKeyFile[]`    | Private key in PEM format        | `*-private.pem.key` (from AWS IoT)     | 🔴 **Extremely sensitive** |
| `rootCaCertificate[]` | Amazon Root CA 1                 | `AmazonRootCA1.pem`                    | 🟢 Public                  |

> ⚠️ **Never publish or share your private key or certificate.**
> Treat them like passwords. Store in a separate `secrets.h` file if committing to GitHub.

---

### ⚙️ Optional Timers and Hardware

| Variable        | Description                   | Default   | Notes                    |
| --------------- | ----------------------------- | --------- | ------------------------ |
| `OUTPUT_PIN`    | GPIO pin to toggle on message | `5`       | D1 on NodeMCU (GPIO5)    |
| `PIN_DELAY`     | Duration (ms) pin stays HIGH  | `5000`    | 5 seconds                |
| `RESTART_DELAY` | Periodic reboot interval (ms) | `3600000` | 1 hour (prevent lockups) |

---

## 🧠 Example Setup

```cpp
const char *WIFI_SSID = "HomeWiFi";
const char *WIFI_PASSWORD = "StrongPass123";

const char *AWS_ENDPOINT = "abcd123456789-ats.iot.us-east-1.amazonaws.com";
const char *MQTT_TOPIC = "garage";
const char *MQTT_CLIENT_ID = "esp8266_garage_01";
```

---

## 🪪 Certificates

Paste the contents of each file **exactly as provided by AWS**:

```cpp
static const char deviceCertificate[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----
<your certificate contents>
-----END CERTIFICATE-----
)KEY";

static const char privateKeyFile[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
<your private key contents>
-----END RSA PRIVATE KEY-----
)KEY";

static const char rootCaCertificate[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
<Amazon Root CA 1 contents>
-----END CERTIFICATE-----
)EOF";
```

---

## 🧱 Quick Start Steps

1. **Create a new AWS IoT “Thing”**

   - In AWS Console → _IoT Core → Manage → Things → Create thing_
   - Download the `.crt`, `.key`, and `AmazonRootCA1.pem` files.

2. **Attach an IoT policy**

   - Allow `iot:Connect`, `iot:Publish`, `iot:Subscribe`, and `iot:Receive` for your topic.

3. **Paste credentials into your sketch**

   - Replace placeholders in the `deviceCertificate[]`, `privateKeyFile[]`, and `rootCaCertificate[]` blocks.

4. **Upload to your ESP8266**

   - Select `NodeMCU 1.0` or equivalent board.
   - Use 115200 baud for serial monitor.

5. **Verify**

   - Watch for `✅ Connected` and `Subscribed to topic` messages in the serial console.
