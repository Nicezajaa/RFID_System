#include <SPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <MFRC522.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <ModbusMaster.h>
#include <Arduino.h>

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_I2CDevice.h> //????
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789

// #include <TFT_eSPI.h> 

#define TFT_MOSI 23  // SDA Pin on ESP32
#define TFT_SCLK 18  // SCL Pin on ESP32
#define TFT_CS   13  // Chip select control pin
#define TFT_DC   15  // Data Command control pin
#define TFT_RST  16  // Reset pin (could connect to RST pin)
// Defines SS/SDA PIN and Reset PIN for RFID-RC522.
#define RST_PIN      21
#define SDA_PIN      5
#define MOSI_PIN     23
#define MISO_PIN     19
#define SCK_PIN      18

#define buzzer_pin   4
#define relay_pin    0

//Color rgb565 
//https://rgbcolorpicker.com/565
// https://color.adobe.com/create/color-wheel
//https://developers.line.biz/flex-simulator/

#define ST77XX_BLACKGROUND 0x18e4
#define ST77XX_TGREEN      0x9e6e
#define ST77XX_TBLUE       0x1a3e
#define ST77XX_TSKY        0x775c
#define ST77XX_TLIME       0x1fc8
#define ST77XX_TPURPLE     0xa81e
#define ST77XX_TRED        0xf006 
#define ST77XX_TORENGE     0xecc4
#define ST77XX_TPINK       0xe81b
#define ST77XX_TYELLOW     0xef26


Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

// WiFi credentials
const char* ssid = "Nicejubjub";         // change SSID
const char* password = "Nice2546";       // change password

// Google script ID and required credentials
String GOOGLE_SCRIPT_ID = "AKfycbwsy8iWGfin3Q56820dO_SIB1ZOoNzbIHEi5GiDQfiqvzoTLXJPGPLEn89mXu4jXPCR2g";    // change Gscript ID

// MQTT Broker
const char *mqtt_broker = "db40b13b.ala.asia-southeast1.emqxsl.com";// broker address
const char *topic = "test/cloud"; // define topic 
const char *mqtt_username = "Nice01"; // username for authentication
const char *mqtt_password = "Nice2546";// password for authentication
const int mqtt_port = 8883;// port of MQTT over TLS/SSL

// CA certificate for MQTT Broker
// load DigiCert Global Root CA ca_cert
const char* ca_cert= \
"-----BEGIN CERTIFICATE-----\n"\
"MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh\n" \
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
"d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\n" \
"QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT\n" \
"MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n" \
"b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG\n" \
"9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB\n" \
"CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97\n" \
"nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt\n" \
"43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P\n" \
"T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4\n" \
"gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO\n" \
"BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR\n" \
"TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw\n" \
"DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr\n" \
"hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg\n" \
"06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF\n" \
"PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls\n" \
"YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk\n" \
"CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=" \
"-----END CERTIFICATE-----\n";


// Variable to read data from RFID-RC522.
int readsuccess;
String UID_Result = "";
String username;
String buidy;
char gun[32] = "";

size_t userCount;
String names[10];
String UIDs[10];
String Buids[10];
//Declare Boolean variable
bool Access;

// Create MFRC522 object as "mfrc522" and set SS/SDA PIN and Reset PIN.
MFRC522 mfrc522(SDA_PIN, RST_PIN);
WiFiClientSecure espClient;
PubSubClient client(espClient);


void setup_Wifi(){
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
   if (WiFi.status() == WL_CONNECTED) {
     Serial.println();
     Serial.println("Connected to WiFi");
    } else {
     Serial.println();
     Serial.println("Connection failed. Retrying...");
     setup_Wifi(); 
    }

}


void byteArray_to_string(byte array[], unsigned int len, char buffer[]) {
  for (unsigned int i = 0; i < len; i++) {
    byte nib1 = (array[i] >> 4) & 0x0F;
    byte nib2 = (array[i] >> 0) & 0x0F;
    buffer[i*2+0] = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;
    buffer[i*2+1] = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;
  }
  buffer[len*2] = '\0';
}

int getUID() {
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    byteArray_to_string(mfrc522.uid.uidByte, mfrc522.uid.size, gun);
    UID_Result = gun;
    digitalWrite(buzzer_pin, HIGH);
    delay(300);
    digitalWrite(buzzer_pin, LOW);
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    
    return 1;
  } else {
    return 0; 
  }
}

// Function to send HTTP request and handle the response
void sendHttpRequest() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    
    String url = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_ID + "/exec?read";
    Serial.println("Making a request");
    
    http.begin(url.c_str());
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    int httpCode = http.GET();a
    
    if (httpCode > 0) {
      String payload = http.getString();
      Serial.println("payload:");
      Serial.println(payload);

      // Used deserializeJson to convert JsonData
      Serial.println("deserializeJson start: ");
      StaticJsonDocument<1000> JSONDocument; 
      DeserializationError error = deserializeJson(JSONDocument, payload);
      
      if (error) {
        Serial.print("Failed to deserialize JSON: ");
        Serial.println(error.c_str());
        return;
      }

       userCount = JSONDocument.size();

      Serial.print("Number of Users: ");
      Serial.println(userCount);
      
      for (size_t i = 0; i < userCount; i++) {
        String userKey = "User" + String(i + 1);
        JsonObject user = JSONDocument[userKey];
        
        names[i] = user["Name"].as<String>();
        UIDs[i] = user["UID"].as<String>(); 
        Buids[i] = user["Building"].as<String>();


        // Serial.println(userKey);
        Serial.print("Name: ");
        Serial.print(names[i]);
        Serial.print(", UID: ");
        Serial.print(UIDs[i]);
        Serial.print(", Buiding: ");
        Serial.println(Buids[i]);

      }    

    } 
    else {
      Serial.println("Error on HTTP request");
    }
    http.end();
  } 
  else {
      Serial.println("WiFi Disconnected");
    }
    
}


void authenticateUser(String UID_Result) {
  Access = false;
  
  for(size_t i = 0; i< userCount; i++){
    if (UID_Result.equals(UIDs[i])){
      Access = true;
      username = names[i];
      buidy = Buids[i];
      break;
      
    }
    else{
      Access = false;
      
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  while (!client.connected()) {
    Serial.println("Reconnecting to MQTT broker...");
    String client_id = "esp8266-client-";
    client_id += String(WiFi.macAddress());
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
        Serial.println("Reconnected to MQTT broker.");
        client.subscribe(topic);
    } else {
        Serial.print("Failed to reconnect to MQTT broker, rc=");
        Serial.print(client.state());
        Serial.println("Retrying in 5 seconds.");
        delay(5000);
    }
  }
}


void setup() {
  pinMode(buzzer_pin, OUTPUT);
  digitalWrite(buzzer_pin, LOW);
  pinMode(relay_pin, OUTPUT);
  digitalWrite(relay_pin, HIGH);

  Serial.begin(115200);
  SPI.begin();      
  mfrc522.PCD_Init(); 
  delay(1000);

  tft.init(240,320, SPI_MODE2);  // Initialize the display
  tft.setRotation(3);  // Rotate the display
  tft.fillScreen(ST77XX_BLACKGROUND);

  setup_Wifi();

  // setup_Wifi();
  sendHttpRequest();
  
  // set root ca cert
  espClient.setCACert(ca_cert);
  
  // connecting to a mqtt broker
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  
    if (!client.connected()) {
    Serial.println("SETUP MQTT NOT CONNECT");
    reconnect();
    Serial.println("SETUP MQTT CONNECTED");
  }else{
  Serial.println("SETUP MQTT CONNECTED----");
  }

  // publish and subscribe
  client.publish("test/esp", "ESP initial published"); // publish to the topic
  Serial.println("Send data to test/esp");
  
  //*1 Buzzer
  digitalWrite(buzzer_pin, HIGH);
  delay(100);
  digitalWrite(buzzer_pin, LOW);
  delay(100);  
  digitalWrite(buzzer_pin, HIGH);
  delay(100);
  digitalWrite(buzzer_pin, LOW);
  delay(100);  
  
  //*2 Display Mock data suscessfull delay(1s)
  tft.setCursor(15, 110);
  tft.setTextColor(ST77XX_TSKY);
  tft.setTextSize(2);
  tft.print(" ");
  tft.println("Mock data suscessfull!!");  

  delay(2000);
  
  tft.setCursor(15, 110);
  tft.setTextColor(ST77XX_BLACKGROUND);
  tft.setTextSize(2);
  tft.print(" ");
  tft.println("Mock data suscessfull!!");  
} 

void loop() {

  if (WiFi.status() == WL_CONNECTED) { 
    client.loop();
  if (!client.connected()) {
    
    //*3 Display Reconnecting to broker
    tft.setCursor(40, 110);
    tft.setTextColor(ST77XX_BLACKGROUND);
    tft.setTextSize(2);
    tft.print("Please tap your Card");
    
    tft.setCursor(30, 110);
    tft.setTextColor(ST77XX_TGREEN);
    tft.setTextSize(2);
    tft.print("Reconnecting to broker");
    
    reconnect(); // Reconnect to broker

    tft.setCursor(30, 110);
    tft.setTextColor(ST77XX_BLACKGROUND);
    tft.setTextSize(2);
    tft.print("Reconnecting to broker");

    tft.setCursor(30, 110);
    tft.setTextColor(ST77XX_TGREEN);
    tft.setTextSize(2);
    tft.print("Reconnected to broker");
    
    tft.setCursor(30, 110);
    tft.setTextColor(ST77XX_BLACKGROUND);
    tft.setTextSize(2);
    tft.print("Reconnected to broker");    
    
  }

    tft.setCursor(40, 110);
    tft.setTextColor(ST77XX_TYELLOW);
    tft.setTextSize(2);
    tft.print("Please tap your Card");

    // delay(3000);
    // tft.setCursor(40, 110);
    // tft.setTextColor(ST77XX_BLACKGROUND);
    // tft.setTextSize(1);
    // tft.print(" ");
    // tft.println("Please tap your Card");
 
    readsuccess = getUID();

    if (readsuccess) {
      Serial.println();
      Serial.print("UID :");
      Serial.println(UID_Result);
      delay(100);

      authenticateUser(UID_Result.c_str())  ;
      if (Access) {
        Serial.println("User Authenticate");
        Serial.print("Name: ");
        Serial.println(username);
        Serial.print("Building: ");
        Serial.println(buidy);

        tft.setCursor(40, 110);
        tft.setTextColor(ST77XX_BLACKGROUND);
        tft.setTextSize(2);
        tft.print("Please tap your Card");


        // Create string in the desired format
        String dataToSend = "Name: " + username + "," + " UID: " + UID_Result + "," + " Building: " + buidy;

          if (client.connected()) {
            client.publish(topic, dataToSend.c_str());

            digitalWrite(relay_pin, LOW); 
            
            tft.setCursor(10, 40);
            tft.setTextColor(ST77XX_TGREEN);
            tft.setTextSize(2);
            tft.print("Status: User Authenticate ");

            tft.setCursor(40, 80);
            tft.setTextColor(ST77XX_TGREEN);
            tft.setTextSize(2);
            tft.print("UID: ");
            tft.print(UID_Result);
        
            tft.setCursor(40, 120);
            tft.setTextColor(ST77XX_TGREEN);
            tft.setTextSize(2);
            tft.print("Name: ");
            tft.print(username);

            tft.setCursor(40, 160);
            tft.setTextColor(ST77XX_TGREEN);
            tft.setTextSize(2);
            tft.print("Building: ");
            tft.print(buidy);

            delay(3000);

            digitalWrite(relay_pin, HIGH);

            tft.setCursor(10, 40);
            tft.setTextColor(ST77XX_BLACKGROUND);
            tft.setTextSize(2);
            tft.print("Status: User Authenticate");

            tft.setCursor(40, 80);
            tft.setTextColor(ST77XX_BLACKGROUND);
            tft.setTextSize(2);
            tft.print("UID: ");
            tft.print(UID_Result);
        
            tft.setCursor(40, 120);
            tft.setTextColor(ST77XX_BLACKGROUND);
            tft.setTextSize(2);
            tft.print("Name: ");
            tft.print(username);

            tft.setCursor(40, 160);
            tft.setTextColor(ST77XX_BLACKGROUND);
            tft.setTextSize(2);
            tft.print("Building: ");
            tft.print(buidy);
            
  
          } 
          else {
            Serial.println("MQTT broker not connected!");
          }
 

        // digitalWrite(relay_pin, LOW);
        // delay(3000);  
        // digitalWrite(relay_pin, HIGH);

      } 
      else {
        Serial.println("Authentication failed");

        tft.setCursor(40, 110);
        tft.setTextColor(ST77XX_BLACKGROUND);
        tft.setTextSize(2);
        tft.print("Please tap your Card");

        tft.setCursor(15, 110);
        tft.setTextColor(ST77XX_TRED);
        tft.setTextSize(2);
        tft.print("User Authenticate Failed");

        delay(3000);

        tft.setCursor(15, 110);
        tft.setTextColor(ST77XX_BLACKGROUND);
        tft.setTextSize(2);
        tft.print("User Authenticate Failed");

      }
    }
  } 
  else {
    // ESP.restart();
    Serial.println("WiFi Connecting");

    tft.setCursor(40, 110);
    tft.setTextColor(ST77XX_BLACKGROUND);
    tft.setTextSize(2);
    tft.print("Please tap your Card");

    //*4 Display reconnecting wifi
    tft.setCursor(15, 110);
    tft.setTextColor(ST77XX_TGREEN);
    tft.setTextSize(2);
    tft.print("Reconnecting wifi...");

    setup_Wifi();

    Serial.println("WiFi Connecting");
    //*4 Display reconnecting wifi
    tft.setCursor(15, 110);
    tft.setTextColor(ST77XX_BLACKGROUND);
    tft.setTextSize(2);
    tft.print("Reconnecting wifi...");
    
    //*5 Display reconnected wifi
    Serial.println("WiFi Reconnecteed");
    tft.setCursor(15, 110);
    tft.setTextColor(ST77XX_TGREEN);
    tft.setTextSize(2);
    tft.print("Reconnected wifi !!");
    
    delay(2000);

    Serial.println("WiFi Reconnecteed");
    tft.setCursor(15, 110);
    tft.setTextColor(ST77XX_BLACKGROUND);
    tft.setTextSize(2);
    tft.print("Reconnected wifi !!");

  }
}

