#define SSD1306_NO_SPLASH

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
int deviceCount = 0;  //device count to 0 for a fresh start
int returncode = 0;
float tempF;
float TempFAHT;
float HAHT;  //Farenheit
float tmin = 50;
float tmax = 100;
float hmin = 50;
float hmax = 100;
int precision = 12;         // Precision of the sensor
int resolutionSet = 0;      //Resolution Result before and after setting
int resolutionTarget = 12;  //Resultion that you want to set it to
int resolution = 0;         //current resolution
int SuccessDelay = 600;
int FailureDelay = 22;
int StartupDelay = 0;
float Tgraph[SCREEN_WIDTH];  //graphing array

boolean fuzzy = false;
int grommet = 0;


// Thingspeak Channel and API key
unsigned long myChannelNumber = 2407634;         // change this to your channel number
const char* myWriteAPIKey = "7RULZHQA528ANR8Z";  // change this to your channels write API key

// WiFi parameters to be configured
const char* ssid = "HomeSprinkler";   // Write here your router's username
const char* password = "8021880218";  // Write here your router's passward


void setup() {
  Serial.begin(115200);

  delay(StartupDelay);
  //WiFi Connect
 WiFi.begin(ssid, password);
  // while wifi not connected yet, print '.'
  // then after it connected, get out of the loop
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  //print a new line, then print WiFi connected and the IP address
  Serial.println("");
  Serial.println("WiFi connected");
  // Print the IP address
  Serial.println(WiFi.localIP());

  //Start the Display
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }

  display.cp437(true);

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(10);

  // Clear the buffer
  //display.clearDisplay();
  display.clearDisplay();
  display.display();
  display.setTextColor(SSD1306_WHITE);

  //AHT Sensor setup
  if (!aht.begin()) {
    Serial.println("Could not find AHT? Check wiring");
    while (1) delay(10);
  }
  Serial.println("AHT10 or AHT20 found");
  aht.getTemperatureSensor();
  delay(2000);
  ThingSpeak.begin(client);
}

void loop() {
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);  // populate temp and humidity objects with fresh data
  delay(100);
  
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
  //display.print("RENT");
  display.setCursor(SECOND_COLUMN, FIRST_LINE);
  display.print(" %rH");
  display.setCursor(SECOND_COLUMN, SECOND_LINE);
  display.println(HAHT, 1);


  /*
  Tgraph[SCREEN_WIDTH] = TempFAHT;  //STore the latest temp
  for (int i = 0; i <= SCREEN_WIDTH; i += 1) {
    Tgraph[i] = Tgraph[i + 1];  //move all of the the temperatures up one spot
  }
*/
  for (int16_t i = 0; i <= TempFAHT; i += 4) {
    display.drawLine(i, 0, i, YELLOW_STRIP, SSD1306_WHITE);
  }
  
  for (int16_t i = 0; i <= HAHT; i += 4) {
    display.drawLine(i, display.height()-BUFFER, i, (display.height() / 2) + BUFFER, SSD1306_WHITE);
  }
  display.display();
  delay(500);

  //  display.drawLine(XLENGTH - (XLENGTH - i), YELLOW_STRIP, XLENGTH - (XLENGTH - i), YELLOW_STRIP - (YELLOW_STRIP / 2) + (Tgraph[i - 1] - Tgraph[i]) * 3, SSD1306_WHITE);
  /*Set THingspeak Fields
  ThingSpeak.setField(1, TempFAHT);
  ThingSpeak.setField(2, HAHT);
  returncode = 0;
  returncode = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  Serial.print("the return code is ");
  Serial.println(returncode);
  delay(1 * 1000);
  if (returncode == 210) {
    delay(FailureDelay * 1000);
    Serial.print("Failure Updating, delaying for ");
    Serial.print(FailureDelay);
    Serial.println(" seconds and trying again.");
    returncode = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
    Serial.print("The retried Return code is ");
    Serial.print(returncode);
    if (returncode != 200) {
      Serial.print("If it failed this time it has failed on this loop");
    }
  }
  if (returncode == 200) {
    Serial.print("Successful. Delaying for ");
    Serial.print(SuccessDelay);
    Serial.println(" seconds");
    delay(SuccessDelay * 1000);
  }
  */
}