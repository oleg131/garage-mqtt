import { IoTDataPlaneClient, PublishCommand } from "@aws-sdk/client-iot-data-plane";

const iotData = new IoTDataPlaneClient({
  region: "us-east-1",
  endpoint: process.env.MQTT_ENDPOINT,
});

export const handler = async (event) => {
  console.log("Webhook event:", event);

  // Validate that the body exists and is not empty
  if (!event.body) {
    console.warn("❌ Missing request body");
    return {
      statusCode: 400, // Bad Request
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({
        success: false,
        error: "Missing request body",
      }),
    };
  }

  // Publish to your MQTT topic
  const mqttParams = {
    topic: process.env.MQTT_TOPIC,
    qos: 0,
    payload: event.body,
  };

  try {
    await iotData.send(new PublishCommand(mqttParams));
    console.log("Published to MQTT:", mqttParams.topic);
  } catch (err) {
    console.error("Failed to publish:", err);
    throw err;
  }

  return {
    statusCode: 200,
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ success: true, data: event.body }),
  };
};
