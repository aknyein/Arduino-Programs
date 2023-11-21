#include <WiFi.h>
#include <ModbusRTU.h>
#include <HardwareSerial.h>
#include <WiFiClient.h>

const char* ssid = "O&M SLDS";
const char* password = "O&M@Slds12345";

const char* ssidAP2 = "O&M SLDS";
const char* passwordAP2 = "O&M@Slds12345";

const char* ssidAP1 = "WMS";
const char* passwordAP1 = "Dscwms123!@#";

// For 125 KVA Generator
const char* thingsBoardServer1 = "thingsboard.cloud"; // For demo thingsboard
const int thingsBoardPort = 80;
const char* accessToken1 = "DU3fS5N2CaI6OylYm3h4"; // For demo thingsboard

// For 315 KVA Generator
const char* thingsBoardServer2 = "thingsboard.cloud"; // For demo thingsboard
const char* accessToken2 = "DU3fS5N2CaI6OylYm3h4"; // For demo thingsboard


WiFiClient wifiClient;

HardwareSerial SerialModbus(2); // Use UART2 (pins RX2 and TX2) on ESP32
ModbusRTU mb;


// Define the desired register addresses and their names for devices 1 and 2
const uint16_t G125_Volt_RegisterAddresses[] = {20, 21, 22, 23, 24, 25};
const char* G125_Volt_RegisterNames[] = {"G125_Voltage_A", "G125_Voltage_B", "G125_Voltage_C", "G125_Voltage_AB", "G125_Voltage_BC", "G125_Voltage_CA"};
const int G125_Volt_REG_COUNT = sizeof(G125_Volt_RegisterAddresses) / sizeof(G125_Volt_RegisterAddresses[0]);

const uint16_t G125_Amp_RegisterAddresses[] = {26, 27, 28, 29};
const char* G125_Amp_RegisterNames[] = {"G125_Current_A", "G125_Current_B", "G125_Current_C", "G125_Current_Total"};
const int G125_Amp_REG_COUNT = sizeof(G125_Amp_RegisterAddresses) / sizeof(G125_Amp_RegisterAddresses[0]);

const uint16_t G315_Volt_RegisterAddresses[] = {20, 21, 22, 23, 24, 25};
const char* G315_Volt_RegisterNames[] = {"G315_Voltage_A", "G315_Voltage_B", "G315_Voltage_C", "G315_Voltage_AB", "G315_Voltage_BC", "G315_Voltage_CA"};
const int G315_Volt_REG_COUNT = sizeof(G315_Volt_RegisterAddresses) / sizeof(G315_Volt_RegisterAddresses[0]);

const uint16_t G315_Amp_RegisterAddresses[] = {26, 27, 28, 29};
const char* G315_Amp_RegisterNames[] = {"G315_Current_A", "G315_Current_B", "G315_Current_C", "G315_Current_Total"};
const int G315_Amp_REG_COUNT = sizeof(G315_Amp_RegisterAddresses) / sizeof(G315_Amp_RegisterAddresses[0]);


int failed_count = 0;
uint16_t first_failed_time;
uint16_t total_failed_time;
uint16_t G125_Volt_Val[G125_Volt_REG_COUNT];
uint16_t G125_Amp_Val[G125_Amp_REG_COUNT];
uint16_t G125_Power_Val[24];
uint16_t G125_Freq_Val[1];
uint16_t G125_Total_Eng_Com[2];
uint16_t G315_Volt_Val[G315_Volt_REG_COUNT];
uint16_t G315_Amp_Val[G315_Amp_REG_COUNT];
uint16_t G315_Power_Val[24];
uint16_t G315_Freq_Val[1];
uint16_t G315_Total_Eng_Com[2];


bool cb(Modbus::ResultCode event, uint16_t transactionId, void* data){
  if (event != Modbus::EX_SUCCESS){
    Serial.print("Request result: 0x");
    Serial.println(event, HEX);
  }
  return true;
}

float AddTwoReg(uint16_t msb, uint16_t lsb) {
  int32_t rawValue = (msb << 16) | lsb;
  return rawValue;
}

void setup(){
  Serial.begin(115200);
  SerialModbus.begin(9600, SERIAL_8N1, 16, 17); // UART2 (pins RX2 and TX2)
  mb.begin(&SerialModbus, 5); // Pin D4 (GPIO 4) for RS-485 DE/RE control
  mb.master();
  WiFi.setAutoReconnect(true);
  WiFi.begin(ssid, password);

  uint16_t current_time = 0;
  uint16_t delay_time = 0;

  // int numNetworks = WiFi.scanNetworks();
  // if (numNetworks == 0) {
  //   Serial.println("No WiFi networks found.");
  //   while (1);
  // }

  // // Find the best WiFi network
  // int bestSignal = -100; // Set a low initial value
  // int bestNetworkIdx = -1;

  // for (int i = 0; i < numNetworks; i++) {
  //   int rssi = WiFi.RSSI(i); // Get signal strength (RSSI) for the current network
  //   Serial.print("SSID: ");
  //   Serial.print(WiFi.SSID(i));
  //   Serial.print(" | RSSI: ");
  //   Serial.println(rssi);

  //   // Compare signal strength with the best signal
  //   if (rssi > bestSignal) {
  //     bestSignal = rssi;
  //     bestNetworkIdx = i;
  //   }
  // }

  // // Connect to the preferred network
  // if (bestNetworkIdx != -1) {
  //   if (bestSignal >= -70) {
  //     Serial.println("Connecting to AP1...");
  //     WiFi.begin(ssidAP1, passwordAP1);
  //   } else {
  //     Serial.println("Connecting to AP2...");
  //     WiFi.begin(ssidAP2, passwordAP2);
  //   }
  // }

  current_time = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    delay_time = millis() - current_time;
    if (delay_time < 60000) Serial.print(".");
    else ESP.restart();
  }

  Serial.println("\nWiFi connected");
  Serial.println("IP address: " + WiFi.localIP().toString());
}

void loop(){

  // if (WiFi.status() != WL_CONNECTED) {
  //   Serial.println("WiFi not connected. Retry in 5 seconds...");
  //   delay(5000);
  //   return;
  // }

  float rssi = WiFi.RSSI();
  Serial.print("RSSI : ");
  Serial.println(rssi);

  if (!mb.slave()) {
    
    Serial.println("Reading Meter 1");
    // Read data from Meter1 Volt registers
    mb.readHreg(1, G125_Volt_RegisterAddresses[0], G125_Volt_Val, G125_Volt_REG_COUNT, cb);
    while(mb.slave()) {
      mb.task();
      delay(1000);
    }
    // Construct the JSON payload for Meter1 Volt
    String jsonPayload1 = "{";
    for (int i = 0; i < G125_Volt_REG_COUNT; i++) {
      jsonPayload1 += "\"" + String(G125_Volt_RegisterNames[i]) + "\": " + String(G125_Volt_Val[i]/10);
      if (i < G125_Volt_REG_COUNT) {
        jsonPayload1 += ",";
      }
    }

    // Read data from Meter1 Amp registers
    mb.readHreg(1, G125_Amp_RegisterAddresses[0], G125_Amp_Val, G125_Amp_REG_COUNT, cb);
    while(mb.slave()) {
      mb.task();
      delay(1000);
    }
    // Construct the JSON payload for Meter1 Amp
    for (int i = 0; i < G125_Amp_REG_COUNT; i++) {
      jsonPayload1 += "\"" + String(G125_Amp_RegisterNames[i]) + "\": " + String(G125_Amp_Val[i]/5);
      if (i < G125_Amp_REG_COUNT) {
        jsonPayload1 += ",";
      }
    }    

    // Read data from Meter1 Freq registers
    mb.readHreg(1, 59, G125_Freq_Val, 1, cb);
    while(mb.slave()){
      mb.task();
      delay(100);
    }
    float G125_Freq = (static_cast<float>(G125_Freq_Val[0]))/100;

    // Construct the JSON payload for Meter1 Frequency
    jsonPayload1 += "\"G125_Frequency\": " + String(G125_Freq) + ",";

    // Read data from Meter1 Total Energy Comsumption registers
    mb.readHreg(1, 60, G125_Total_Eng_Com, 2, cb);
    while(mb.slave()){
      mb.task();
      delay(100);
    }
    float G125_Total_Energy_Com = static_cast<float>((AddTwoReg(G125_Total_Eng_Com[0],G125_Total_Eng_Com[1]))/5);

    // Construct the JSON payload for Meter1 Total Energy Comsumption
    jsonPayload1 += "\"G125_Total_Energy_Comsumption\": " + String(G125_Total_Energy_Com) + ",";
 

    // Read data from Meter1 Freq registers
    mb.readHreg(1, 30, G125_Power_Val, 24, cb);
    while(mb.slave()){
      mb.task();
      delay(100);
    }
    float G125_Active_PowerA = (static_cast<float>(AddTwoReg(G125_Power_Val[0],G125_Power_Val[1])))/50;
    float G125_Active_PowerB = (static_cast<float>(AddTwoReg(G125_Power_Val[2],G125_Power_Val[3])))/50;
    float G125_Active_PowerC = (static_cast<float>(AddTwoReg(G125_Power_Val[4],G125_Power_Val[5])))/50;
    float G125_Total_Active_Power = (static_cast<float>(AddTwoReg(G125_Power_Val[6],G125_Power_Val[7])))/50;
    float G125_Reactive_PowerA = (static_cast<float>(AddTwoReg(G125_Power_Val[8],G125_Power_Val[9])))/50;
    float G125_Reactive_PowerB = (static_cast<float>(AddTwoReg(G125_Power_Val[10],G125_Power_Val[11])))/50;
    float G125_Reactive_PowerC = (static_cast<float>(AddTwoReg(G125_Power_Val[12],G125_Power_Val[13])))/50;
    float G125_Total_Reactive_Power = (static_cast<float>(AddTwoReg(G125_Power_Val[14],G125_Power_Val[15])))/50;
    float G125_Apparent_PowerA = (static_cast<float>(AddTwoReg(G125_Power_Val[16],G125_Power_Val[17])))/50;
    float G125_Apparent_PowerB = (static_cast<float>(AddTwoReg(G125_Power_Val[18],G125_Power_Val[19])))/50;
    float G125_Apparent_PowerC = (static_cast<float>(AddTwoReg(G125_Power_Val[20],G125_Power_Val[21])))/50;
    float G125_Total_Apparent_Power = (static_cast<float>(AddTwoReg(G125_Power_Val[22],G125_Power_Val[23])))/50;

    // Construct the JSON payload for Meter1 Power
    jsonPayload1 += "\"G125_Active_PowerA\": " + String(G125_Active_PowerA) + ",";
    jsonPayload1 += "\"G125_Active_PowerB\": " + String(G125_Active_PowerB) + ",";
    jsonPayload1 += "\"G125_Active_PowerC\": " + String(G125_Active_PowerC) + ",";
    jsonPayload1 += "\"G125_Total_Active_Power\": " + String(G125_Total_Active_Power) + ",";
    jsonPayload1 += "\"G125_Reactive_PowerA\": " + String(G125_Reactive_PowerA) + ",";
    jsonPayload1 += "\"G125_Reactive_PowerB\": " + String(G125_Reactive_PowerB) + ",";
    jsonPayload1 += "\"G125_Reactive_PowerC\": " + String(G125_Reactive_PowerC) + ",";
    jsonPayload1 += "\"G125_Total_Reactive_Power\": " + String(G125_Total_Reactive_Power) + ",";
    jsonPayload1 += "\"G125_Apparent_PowerA\": " + String(G125_Apparent_PowerA) + ",";
    jsonPayload1 += "\"G125_Apparent_PowerB\": " + String(G125_Apparent_PowerB) + ",";
    jsonPayload1 += "\"G125_Apparent_PowerC\": " + String(G125_Apparent_PowerC) + ",";
    jsonPayload1 += "\"G125_Total_Apparent_Power\": " + String(G125_Total_Apparent_Power);
    jsonPayload1 += "}";

//*****************************************************************************************************    
    //Create an HTTP client
    WiFiClient client1;
    if (!client1.connect(thingsBoardServer1, thingsBoardPort)) {
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
    String url1 = "/api/v1/" + String(accessToken1) + "/telemetry";
    String httpRequest1 = "POST " + url1 + " HTTP/1.1\r\n";
    httpRequest1 += "Host: " + String(thingsBoardServer1) + "\r\n";
    httpRequest1 += "Content-Type: application/json\r\n";
    httpRequest1 += "Content-Length: " + String(jsonPayload1.length()) + "\r\n";
    httpRequest1 += "Connection: close\r\n\r\n";
    httpRequest1 += jsonPayload1;

    // Send the request
    Serial.println("Sending HTTP POST request...");
    client1.print(httpRequest1);
    delay(100); // A short delay to allow the server to process the request

    // Read and print the response
    while (client1.available()) {
      String line = client1.readStringUntil('\r');
      Serial.print(line);
    }

    Serial.println("\nHTTP POST request complete");
    client1.stop();
    delay(1000);



//*********************************************************************************************************    
    Serial.println("Reading Meter 2");
    // Read data from Meter1 Volt registers
    mb.readHreg(2, G315_Volt_RegisterAddresses[0], G315_Volt_Val, G315_Volt_REG_COUNT, cb);
    while(mb.slave()) {
      mb.task();
      delay(1000);
    }
    // Construct the JSON payload for Meter2 Volt
    String jsonPayload2 = "{";
    for (int i = 0; i < G315_Volt_REG_COUNT; i++) {
      jsonPayload2 += "\"" + String(G315_Volt_RegisterNames[i]) + "\": " + String(G315_Volt_Val[i]/10);
      if (i < G315_Volt_REG_COUNT) {
        jsonPayload2 += ",";
      }
    }

    // Read data from Meter2 Amp registers
    mb.readHreg(2, G315_Amp_RegisterAddresses[0], G315_Amp_Val, G315_Amp_REG_COUNT, cb);
    while(mb.slave()) {
      mb.task();
      delay(1000);
    }
    // Construct the JSON payload for Meter2 Amp
    for (int i = 0; i < G315_Amp_REG_COUNT; i++) {
      jsonPayload2 += "\"" + String(G315_Amp_RegisterNames[i]) + "\": " + String(G315_Amp_Val[i]);
      if (i < G315_Amp_REG_COUNT) {
        jsonPayload2 += ",";
      }
    }    

    // Read data from Meter2 Freq registers
    mb.readHreg(2, 59, G315_Freq_Val, 1, cb);
    while(mb.slave()){
      mb.task();
      delay(100);
    }
    float G315_Freq = (static_cast<float>(G315_Freq_Val[0]))/100;

    // Construct the JSON payload for Meter2 Frequency
    jsonPayload2 += "\"G315_Frequency\": " + String(G315_Freq) + ",";
 
    // Read data from Meter2 Total Energy Comsumption registers
    mb.readHreg(2, 60, G315_Total_Eng_Com, 2, cb);
    while(mb.slave()){
      mb.task();
      delay(100);
    }
    float G315_Total_Energy_Com = static_cast<float>(AddTwoReg(G315_Total_Eng_Com[0],G315_Total_Eng_Com[1]));

    // Construct the JSON payload for Meter1 Total Energy Comsumption
    jsonPayload2 += "\"G315_Total_Energy_Comsumption\": " + String(G315_Total_Energy_Com) + ",";

    // Read data from Meter2 Freq registers
    mb.readHreg(2, 30, G315_Power_Val, 24, cb);
    while(mb.slave()){
      mb.task();
      delay(100);
    }
    float G315_Active_PowerA = (static_cast<float>(AddTwoReg(G315_Power_Val[0],G315_Power_Val[1])))/10;
    float G315_Active_PowerB = (static_cast<float>(AddTwoReg(G315_Power_Val[2],G315_Power_Val[3])))/10;
    float G315_Active_PowerC = (static_cast<float>(AddTwoReg(G315_Power_Val[4],G315_Power_Val[5])))/10;
    float G315_Total_Active_Power = (static_cast<float>(AddTwoReg(G315_Power_Val[6],G315_Power_Val[7])))/10;
    float G315_Reactive_PowerA = (static_cast<float>(AddTwoReg(G315_Power_Val[8],G315_Power_Val[9])))/10;
    float G315_Reactive_PowerB = (static_cast<float>(AddTwoReg(G315_Power_Val[10],G315_Power_Val[11])))/10;
    float G315_Reactive_PowerC = (static_cast<float>(AddTwoReg(G315_Power_Val[12],G315_Power_Val[13])))/10;
    float G315_Total_Reactive_Power = (static_cast<float>(AddTwoReg(G315_Power_Val[14],G315_Power_Val[15])))/10;
    float G315_Apparent_PowerA = (static_cast<float>(AddTwoReg(G315_Power_Val[16],G315_Power_Val[17])))/10;
    float G315_Apparent_PowerB = (static_cast<float>(AddTwoReg(G315_Power_Val[18],G315_Power_Val[19])))/10;
    float G315_Apparent_PowerC = (static_cast<float>(AddTwoReg(G315_Power_Val[20],G315_Power_Val[21])))/10;
    float G315_Total_Apparent_Power = (static_cast<float>(AddTwoReg(G315_Power_Val[22],G315_Power_Val[23])))/10;

    // Construct the JSON payload for Meter2 Power
    jsonPayload2 += "\"G315_Active_PowerA\": " + String(G315_Active_PowerA) + ",";
    jsonPayload2 += "\"G315_Active_PowerB\": " + String(G315_Active_PowerB) + ",";
    jsonPayload2 += "\"G315_Active_PowerC\": " + String(G315_Active_PowerC) + ",";
    jsonPayload2 += "\"G315_Total_Active_Power\": " + String(G315_Total_Active_Power) + ",";
    jsonPayload2 += "\"G315_Reactive_PowerA\": " + String(G315_Reactive_PowerA) + ",";
    jsonPayload2 += "\"G315_Reactive_PowerB\": " + String(G315_Reactive_PowerB) + ",";
    jsonPayload2 += "\"G315_Reactive_PowerC\": " + String(G315_Reactive_PowerC) + ",";
    jsonPayload2 += "\"G315_Total_Reactive_Power\": " + String(G315_Total_Reactive_Power) + ",";
    jsonPayload2 += "\"G315_Apparent_PowerA\": " + String(G315_Apparent_PowerA) + ",";
    jsonPayload2 += "\"G315_Apparent_PowerB\": " + String(G315_Apparent_PowerB) + ",";
    jsonPayload2 += "\"G315_Apparent_PowerC\": " + String(G315_Apparent_PowerC) + ",";
    jsonPayload2 += "\"G315_Total_Apparent_Power\": " + String(G315_Total_Apparent_Power) + ",";

    jsonPayload2 += "\"Generators ESP-32 RSSI\": " + String(rssi);

    jsonPayload2 += "}";

//*****************************************************************************************************    
    //Create an HTTP client
    WiFiClient client2;
    if (!client2.connect(thingsBoardServer2, thingsBoardPort)) {
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
    String url2 = "/api/v1/" + String(accessToken2) + "/telemetry";
    String httpRequest2 = "POST " + url2 + " HTTP/1.1\r\n";
    httpRequest2 += "Host: " + String(thingsBoardServer2) + "\r\n";
    httpRequest2 += "Content-Type: application/json\r\n";
    httpRequest2 += "Content-Length: " + String(jsonPayload2.length()) + "\r\n";
    httpRequest2 += "Connection: close\r\n\r\n";
    httpRequest2 += jsonPayload2;

    // Send the request
    Serial.println("Sending HTTP POST request...");
    client2.print(httpRequest2);
    delay(100); // A short delay to allow the server to process the request

    // Read and print the response
    while (client2.available()) {
      String line = client2.readStringUntil('\r');
      Serial.print(line);
    }

    Serial.println("\nHTTP POST request complete");
    client2.stop();

  // Wait for a few seconds before sending the next data
  delay(5000);
  }
}
