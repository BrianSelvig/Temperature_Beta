#define SSD1306_NO_SPLASH //no adafruit splash screen


//#include <OneWire.h>
#include <ThingSpeak.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_AHTX0.h>

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels
#define YELLOW_STRIP 14
#define BUFFER 8
#define FIRST_LINE 6 + YELLOW_STRIP  //The height in pixels for the first strip of screen colored yellow so that the first line starts in the 'second line'.
#define SECOND_LINE display.height() / 2 + 10
#define FIRST_COLUMN 0
#define SECOND_COLUMN display.width() / 2 + 10
#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define XLENGTH 128
#define QUAD 16

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//AHT sensor
Adafruit_AHTX0 aht;

//Wifi client
WiFiClient client;

// Define
int returncode = 0;
float tempF;
float TempFAHT;
float HAHT;  //Farenheit
float tmin = 50;
float tmax = 100;
float hmin = 50;
float hmax = 100;
int precision = 12;  // Precision of the sensor
int SuccessDelay = 600;
int FailureDelay = 240;
int DelayModifier = 100;

// Thingspeak Channel and API key
unsigned long myChannelNumber = 2407634;         // change this to your channel number
const char* myWriteAPIKey = "7RULZHQA528ANR8Z";  // change this to your channels write API key

// WiFi parameters to be configured
const char* ssid = "HomeSprinkler";   // Write here your router's username
const char* password = "8021880218";  // Write here your router's passward

void setup() {
  Serial.begin(115200);

  //Start the Display
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }
  display.cp437(true);
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.display();
  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  //WiFi Connect
  WiFi.begin(ssid, password);
  // while wifi not connected yet, print '.'
  // then after it connected, get out of the loop
  /*Serial.println();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  */
  
  //print a new line, then print WiFi connected and the IP address
  display.setCursor(0, YELLOW_STRIP);
  Serial.println("WiFi connected: ");
  // Print the IP address
  Serial.println(WiFi.localIP());

  //AHT Sensor setup
  if (!aht.begin()) {
    Serial.println("Could not find AHT? Check wiring");
    while (1) delay(10);
  }
  Serial.println("AHT10 or AHT20 found");
  aht.getTemperatureSensor();
  delay(100);
  ThingSpeak.begin(client);
}

void loop() {
  gdmtemp();
  //*Set THingspeak Fields
  ThingSpeak.setField(3, TempFAHT);
  ThingSpeak.setField(4, HAHT);
  returncode = 0;
  returncode = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  Serial.print("the first attempt raw return code is ");
  Serial.println(returncode);
  gdmtemp();

  //return code for rate limiting  at Thingspeak
  if (returncode == 210) {
    Serial.print("the return code is ");
    Serial.println(returncode);
    for (int t = 0; t <= FailureDelay; t += 1) {
      gdmtemp();
      delay(DelayModifier);
    }
    delay(DelayModifier);
    returncode = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  }
  if (returncode == 200) {
    Serial.print("the return code is ");
    Serial.println(returncode);
    for (int t = 0; t <= SuccessDelay; t += 1) {
      gdmtemp();
      delay(DelayModifier);
    }
    delay(DelayModifier);
    returncode = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  }
  if (returncode != 200) {
    Serial.print("the return code is ");
    Serial.println(returncode);
    Serial.println("If it failed this time it has failed on this loop");
    for (int t = 0; t <= FailureDelay; t += 1) {
      gdmtemp();
      delay(DelayModifier);
    }
    delay(DelayModifier);
    returncode = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  }
}

void gdmtemp() {
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);  // populate temp and humidity objects with fresh data
  //Calc the temps
  TempFAHT = (temp.temperature * 1.8) + 32;
  HAHT = humidity.relative_humidity;
  //Print to ssd1306
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(FIRST_COLUMN, FIRST_LINE);
  display.print(TempFAHT, 1);
  display.print((char)248);  // shows degree symbol
  display.print("F");
  display.setCursor(FIRST_COLUMN, SECOND_LINE);
  display.setCursor(SECOND_COLUMN, FIRST_LINE);
  display.print(" %rH");
  display.setCursor(SECOND_COLUMN, SECOND_LINE);
  display.println(HAHT, 1);

  //METER DISPLAY
  //TEMP
  for (int16_t i = 0; i <= TempFAHT; i += 4) {
    display.drawLine(i, 0, i, YELLOW_STRIP, SSD1306_WHITE);
  }
  //METER DISPLAY
  //rH
  for (int16_t i = 0; i <= HAHT; i += 4) {
    display.drawLine(i, display.height() - BUFFER, i, (display.height() / 2) + BUFFER, SSD1306_WHITE);
  }
  display.display();
}
