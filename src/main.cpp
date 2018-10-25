#include <Arduino.h>
#include <ESP8266WiFi.h>      // ESP8266 Board support needed. Add http://arduino.esp8266.com/stable/package_esp8266com_index.json to Borad Manager Urls (Preferences)
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>      // Search and add using Arduino Library Manager
#include <pgmspace.h>
#include <EEPROM.h>
#include <GxEPD2_BW.h>        // Download/Clone and put in Arduino Library https://github.com/ZinggJM/GxEPD2

// FreeFonts from Adafruit_GFX
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/TomThumb.h>

GxEPD2_BW<GxEPD2_270, GxEPD2_270::HEIGHT> display(GxEPD2_270(/*CS=D8*/ 2, /*DC=D3*/ 0, /*RST=D4*/ 4, /*BUSY=D2*/ 5)); // 2.7" b/w 264x176

// Button pins
#define BUTTON1_PIN 10   //10
#define BUTTON2_PIN 12    //12
#define WAKE_PIN 16

int counter = 0;
int row = 0;
int col = 0;

uint8_t ttvPages[2048]; //maximum of 12 pages
int ttvSize = 0;

const char *ssid = "SkyNet";
const char *password = "texttotv";




IPAddress ip(192,168,4,1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);
WiFiServer server(2323);                     // create object

long bgColor = GxEPD_WHITE;
long txtColor = GxEPD_BLACK;


void handleUpload(WiFiClient client);

bool drawScreen = false;

void setup() {
    Serial.begin(9600);

    pinMode(BUTTON1_PIN, INPUT_PULLUP);
    pinMode(BUTTON2_PIN, INPUT_PULLUP);


    pinMode(WAKE_PIN, OUTPUT);
    digitalWrite(WAKE_PIN, HIGH); //immediately set wake pin to HIGH to keep the chip enabled

    display.init();
    display.setRotation(3);

    display.fillScreen(bgColor);
    display.setTextColor(txtColor);

    WiFi.mode(WIFI_AP);
    WiFi.config(ip,gateway,subnet);
    WiFi.softAP(ssid, password, 10);
    drawScreen = true;

    Serial.println();
    Serial.print("Server IP address: ");
    Serial.println(WiFi.softAPIP());
    Serial.println(WiFi.softAPmacAddress());

    server.begin();
}

void loop() {
  if (digitalRead(BUTTON1_PIN) == HIGH) {
    drawScreen = true;
  }

  if (server.hasClient()) {
    WiFiClient client = server.available();
    if (client) handleUpload(client);
  }


  if (drawScreen) {
    do {

      display.fillScreen(bgColor);
      display.setTextColor(txtColor);
      display.fillScreen(GxEPD_WHITE);

      display.setFont(&FreeSans9pt7b);
      display.setCursor(0, 20);
      display.print("AccessPoint: ");
      display.println(ssid);
      display.setCursor(0, 42);
      display.print("Key: ");
      display.println(password);
      display.setFont(NULL);
      display.setCursor(0, 75);
      display.print("Server IP address: ");
      display.println(WiFi.softAPIP());
      display.setCursor(0, 90);
      display.print("Server gateway address: ");
      display.println(WiFi.gatewayIP());
      display.setCursor(0, 105);
      display.print("Server MAC address: ");
      display.println(WiFi.softAPmacAddress());
      display.setCursor(0, 125);

      for (int i = 0; i< 255; i++) display.print((char) i);

      //æ = 145 Æ = 146
      // Ø = 236 ø = 235
      //Å = 143 å = 134

      drawScreen = false;
    } while (display.nextPage());

  }

}


void handleUpload(WiFiClient client) {
  if (client) {
    Serial.println("We have a new client");
    int bytesRead  = 0;
    int charsWritten  =0;

    display.fillScreen(GxEPD_BLACK);
    display.setTextColor(GxEPD_WHITE);
    display.setFont(NULL);
    display.setCursor(0, 10);

    while(client.connected()) {
       int readChar = client.read();
       if (readChar > 0) {
         bytesRead++;
               charsWritten++;

               if (charsWritten == 40) {
                 display.println("");
                 charsWritten = 0;
                 Serial.println();
               }

               if (charsWritten == 0) {
                 display.print("  ");
               }

               if (((int)readChar) == 173) readChar = 62;
               if (((int)readChar) < 32) readChar = 32;
               if (((int)readChar) == 144) readChar == 145;
               if (((int)readChar) == 125) readChar == 235;



               Serial.print("[");
               Serial.print(readChar);
               Serial.print("]");
               Serial.print((char) readChar);

               display.print((char) readChar);

       }
    }
    Serial.println();
    Serial.println("Client went away");
    display.nextPage();
  }
}
