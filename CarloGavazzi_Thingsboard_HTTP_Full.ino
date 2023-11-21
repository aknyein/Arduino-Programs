#include <ESP8266WiFi.h>
#include <WiFiClient.h>

const char* ssid = "O&M SLDS";
const char* password = "O&M@Slds12345";

// const char* ssid = "WMS";
// const char* password = "Dscwms123!@#";

//const char* thingsBoardServer = "thingsboard.cloud";
const char* thingsBoardServer = "thingsboard.cloud"; // For demo thingsboard
const int thingsBoardPort = 80;
//const char* accessToken = "qZrS7rP0pKUEh4DFzNOE"; // Replace with your ThingsBoard access token for ESP8266
const char* accessToken = "DU3fS5N2CaI6OylYm3h4"; // Replace with your ThingsBoard access token for ESP8266


IPAddress modbusServerIP(192,168,1,230); // Replace with the Modbus TCP server's IP address
const int modbusServerPort = 502; // Default Modbus TCP port is 502
const int A1TempInputRegisterAddress[]  = {818,842,866,890,878,854,830,902,914,800,3110,3122}; // Modbus Input Register addresses to read
const int A1HumidInputRegisterAddress[] = {820,844,868,892,880,856,832,904,916,802,3112,3124}; // Modbus Input Register addresses to read
const int A2TempInputRegisterAddress[]  = {3134,3146,3158,3170,3182,3194,3206,3218,4190,4202,4214,4226}; // Modbus Input Register addresses to read
const int A2HumidInputRegisterAddress[] = {3136,3148,3160,3172,3184,3196,3208,3220,4192,4204,4216,4228}; // Modbus Input Register addresses to read
const int B1TempInputRegisterAddress[]  = {3230,3242,3254,3266,3278,3290,3302,3314,3326,3338}; // Modbus Input Register addresses to read
const int B1HumidInputRegisterAddress[] = {3232,3244,3256,3268,3280,3292,3304,3316,3328,3340}; // Modbus Input Register addresses to read
const int B2TempInputRegisterAddress[]  = {3350,3362,3374,3386,3398,3410,3422,3434,3446,3458}; // Modbus Input Register addresses to read
const int B2HumidInputRegisterAddress[] = {3352,3364,3376,3388,3400,3412,3424,3436,3448,3460}; // Modbus Input Register addresses to read
const int A1TempInputRegisterNum =12;
const int A2TempInputRegisterNum =12;
const int A1HumidInputRegisterNum =12;
const int A2HumidInputRegisterNum =12;
const int B1TempInputRegisterNum =10;
const int B2TempInputRegisterNum =10;
const int B1HumidInputRegisterNum =10;
const int B2HumidInputRegisterNum =10;

WiFiClient wifiClient;

int failed_count = 0;
uint16_t first_failed_time;
uint16_t total_failed_time;

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  Serial.println("IP address: " + WiFi.localIP().toString());
}

uint16_t readInputRegister(int address) {
  WiFiClient client;
  if (!client.connect(modbusServerIP, modbusServerPort)) {
    Serial.println("Connection to Modbus server failed");
    return 0;
  }

  // Compose Modbus TCP read request
  uint8_t request[] = {
    0x00, 0x01, // Transaction Identifier (random)
    0x00, 0x00, // Protocol Identifier
    0x00, 0x06, // Length of remaining bytes
    0x01,       // Unit Identifier (Slave ID)
    0x04,       // Function Code: Read Input Registers
    highByte(address), lowByte(address), // Starting Address
    0x00, 0x01  // Quantity of Registers to Read (1 register)
  };

  client.write(request, sizeof(request));
  delay(50);

  if (client.available()) {
    uint8_t response[12];
    client.readBytes(response, sizeof(response));

    if (response[7] == 0x04 && response[8] == 0x02) {
      uint16_t value = (response[9] << 8) | response[10];
      Serial.println("Modbus response OK");
      return value;
    } else {
      Serial.println("Error: Invalid Modbus response");
    }
  } else {
    Serial.println("Error: No Modbus response");
  }

  client.stop();
  return 0;
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    failed_count=+1;
    if(failed_count > 5) ESP.restart();
    else {
      Serial.println("WiFi not connected. Retry in 5 seconds...");
      delay(5000);
      return;
    }
  }

  float rssi = WiFi.RSSI();
  Serial.print("RSSI : ");
  Serial.println(rssi);

  
  String jsonPayload = "{";
  for (int i = 0; i < A1TempInputRegisterNum; i++) {
    uint16_t value = readInputRegister(A1TempInputRegisterAddress[i]);
    if (value==0) continue;
    float val = (static_cast<float>(value))/10;
    Serial.print("Temperature Zone A 1 ");
    Serial.print(i+1); Serial.print(": ");
    Serial.println(val);

    jsonPayload += "\"Zone-A-1-" + String(i+1) + "Temperature" + "\": " + String(val);
    if (i < A1TempInputRegisterNum) {
      jsonPayload += ",";
    }

    delay(100);
  }


  for (int i = 0; i < A2TempInputRegisterNum; i++) {
    uint16_t value = readInputRegister(A2TempInputRegisterAddress[i]);
    if (value==0) continue;
    float val = (static_cast<float>(value))/10;
    Serial.print("Temperature Zone A 2 ");
    Serial.print(i+1); Serial.print(": ");
    Serial.println(val);

    jsonPayload += "\"Zone-A-2-" + String(i+1) + "Temperature" + "\": " + String(val);
    if (i < A2TempInputRegisterNum) {
      jsonPayload += ",";
    }

    delay(100);
  }


  for (int i = 0; i < B1TempInputRegisterNum; i++) {
    uint16_t value = readInputRegister(B1TempInputRegisterAddress[i]);
    if (value==0) continue;
    float val = (static_cast<float>(value))/10;
    Serial.print("Temperature Zone B 1 ");
    Serial.print(i+1); Serial.print(": ");
    Serial.println(val);

    jsonPayload += "\"Zone-B-1-" + String(i+1) + "Temperature" + "\": " + String(val);
    if (i < B1TempInputRegisterNum) {
      jsonPayload += ",";
    }

    delay(100);
  }


  for (int i = 0; i < B2TempInputRegisterNum; i++) {
    uint16_t value = readInputRegister(B2TempInputRegisterAddress[i]);
    if (value==0) continue;
    float val = (static_cast<float>(value))/10;
    Serial.print("Temperature Zone B 2 ");
    Serial.print(i+1); Serial.print(": ");
    Serial.println(val);

    jsonPayload += "\"Zone-B-2-" + String(i+1) + "Temperature" + "\": " + String(val);
    if (i < B2TempInputRegisterNum) {
      jsonPayload += ",";
    }

    delay(100);
  }

  for (int i = 0; i < A1HumidInputRegisterNum; i++) {
    uint16_t value = readInputRegister(A1HumidInputRegisterAddress[i]);
    if (value==0) continue;
    float val = (static_cast<float>(value))/10;
    Serial.print("Humidity Zone A 1 ");
    Serial.print(i+1); Serial.print(": ");
    Serial.println(val);

    jsonPayload += "\"Zone-A-1-" + String(i+1) + "Humidity" + "\": " + String(val);
    if (i < A1HumidInputRegisterNum) {
      jsonPayload += ",";
    }

    delay(1000);
  }


  for (int i = 0; i < A2HumidInputRegisterNum; i++) {
    uint16_t value = readInputRegister(A2HumidInputRegisterAddress[i]);
    if (value==0) continue;
    float val = (static_cast<float>(value))/10;
    Serial.print("Humidity Zone A 2 ");
    Serial.print(i+1); Serial.print(": ");
    Serial.println(val);

    jsonPayload += "\"Zone-A-2-" + String(i+1) + "Humidity" + "\": " + String(val);
    if (i < A2HumidInputRegisterNum) {
      jsonPayload += ",";
    }

    delay(100);
  }


  for (int i = 0; i < B1HumidInputRegisterNum; i++) {
    uint16_t value = readInputRegister(B1HumidInputRegisterAddress[i]);
    if (value==0) continue;
    float val = (static_cast<float>(value))/10;
    Serial.print("Humidity Zone B 1 ");
    Serial.print(i+1); Serial.print(": ");
    Serial.println(val);

    jsonPayload += "\"Zone-B-1-" + String(i+1) + "Humidity" + "\": " + String(val);
    if (i < B1HumidInputRegisterNum) {
      jsonPayload += ",";
    }

    delay(100);
  }


  for (int i = 0; i < B2HumidInputRegisterNum; i++) {
    uint16_t value = readInputRegister(B2HumidInputRegisterAddress[i]);
    if (value==0) continue;
    float val = (static_cast<float>(value))/10;
    Serial.print("Humidity Zone B 2 ");
    Serial.print(i+1); Serial.print(": ");
    Serial.println(val);

    jsonPayload += "\"Zone-B-2-" + String(i+1) + "Humidity" + "\": " + String(val);
    if (i < B2HumidInputRegisterNum) {
      jsonPayload += ",";
    }

    delay(100);
  }

  jsonPayload += "\"Carlogavazzi ESP-8266 RSSI\": " + String(rssi);

  jsonPayload += "}";


    //Create an HTTP client
    WiFiClient client;
    if (!client.connect(thingsBoardServer, thingsBoardPort)) {
      failed_count+=1; 
      if(failed_count==1) first_failed_time = millis();
      Serial.println("Connection to ThingsBoard server failed");
      delay(5000);
      if (failed_count > 5) {
        total_failed_time = millis()- first_failed_time; 
        if (total_failed_time < 300000) ESP.restart();
      }
      else return;
    }

    //Compose the HTTP POST request
    String url = "/api/v1/" + String(accessToken) + "/telemetry";
    String httpRequest = "POST " + url + " HTTP/1.1\r\n";
    httpRequest += "Host: " + String(thingsBoardServer) + "\r\n";
    httpRequest += "Content-Type: application/json\r\n";
    httpRequest += "Content-Length: " + String(jsonPayload.length()) + "\r\n";
    httpRequest += "Connection: close\r\n\r\n";
    httpRequest += jsonPayload;

    // Send the request
    Serial.println("Sending HTTP POST request...");
    client.print(httpRequest);
    delay(100); // A short delay to allow the server to process the request

    // Read and print the response
    while (client.available()) {
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }

    Serial.println("\nHTTP POST request complete");
    client.stop();
    delay(1000);

}
