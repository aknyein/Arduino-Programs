#include <WiFi.h>
#include <PubSubClient.h>
#include <IRremote.h>

// WiFi credentials
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// MQTT broker
const char* mqtt_server = "YOUR_MQTT_BROKER_IP";
const int mqtt_port = 1883;
const char* mqtt_topic = "ac/control";

WiFiClient espClient;
PubSubClient client(espClient);

// IR sending pin
#define IR_SEND_PIN 4

// IR codes for ON and OFF (144 bits LSB first)
uint64_t powerOnRawData[]  = {0x51820000126CB23, 0x4036, 0xC800};
uint64_t powerOffRawData[] = {0x51800000126CB23, 0x4036, 0xA800};

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to WiFi ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void sendIRPowerOn() {
  Serial.println("Sending IR Power ON signal");
  IrSender.sendPulseDistanceWidthFromArray(38, 3500, 1650, 400, 1250, 400, 400, &powerOnRawData[0], 144, PROTOCOL_IS_LSB_FIRST, 0, 0);
}

void sendIRPowerOff() {
  Serial.println("Sending IR Power OFF signal");
  IrSender.sendPulseDistanceWidthFromArray(38, 3550, 1650, 400, 1250, 400, 400, &powerOffRawData[0], 144, PROTOCOL_IS_LSB_FIRST, 0, 0);
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  message.trim();
  Serial.print("Received message: ");
  Serial.println(message);

  if (message == "on") {
    sendIRPowerOn();
  } else if (message == "off") {
    sendIRPowerOff();
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client")) {
      Serial.println("connected");
      client.subscribe(mqtt_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" trying again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  // Initialize IR sender
  IrSender.begin(IR_SEND_PIN);  // No LED feedback
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
