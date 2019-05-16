/*
  RUN WITH BOARD = ESP32_DEV_MODULE IN ARDUINO IDE
*/
  
/* LOAD LIBRARIES */
  #include <SPI.h>
  #include <SD.h>
  #include <Adafruit_GFX.h>         // Core graphics library
  #include <Adafruit_ST7735.h>      // Hardware-specific library
  #include <Adafruit_ImageReader.h> // Image-reading functions
  #include <ESP32Servo.h> //servos
  #include <Adafruit_NeoPixel.h> // neopixels!
  #include <ArduinoJson.h> // work with JSON for config

/* DEFINE PINS */

  #define SD_CS    5 // SD card select pin
  #define TFT_CS   4 // TFT select pin
  #define TFT_DC   2 // TFT display/command pin
  #define TFT_RST  -1 // Or set to -1 and connect to Arduino RESET pin
  #define LServo  33 
  #define RServo  27
  #define BOOST   13 //this turns power on to motors 
  #define PIXEL 12 //neopixel pin
  #define TFT_MOSI 23  // Data out
  #define TFT_SCLK 18   // Clock out

/* CREATE LIBRARY OBJECT INSTANCES */

  Adafruit_ST7735      tft    = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
  Adafruit_ImageReader reader;     // Class w/image-reading functions
  Adafruit_Image       img;        // An image loaded into RAM
  
  // number of pixels, pin, pixel type
  Adafruit_NeoPixel strip = Adafruit_NeoPixel(2, PIXEL, NEO_GRB + NEO_KHZ800);
  Servo leftServo; 
  Servo rightServo;

/* CREATE VARIABLES */
  int32_t width  = 128,height = 160; // BMP image dimensions 
  int pos = 0; //for servos
  const char *strConfigFile = "/config.js";
  size_t intOnceCount=0;
  size_t intLoopCount=0;
  DynamicJsonDocument doc(4092);
  
void setup() {
/* INIT COMPONENTS */
  //this is for the arduino serial monitor, Tools->serial monitor
  Serial.begin(9600);
  //Serial.print(F("init screen"));

  //screen init 

  pinMode(TFT_CS, OUTPUT);
  digitalWrite(TFT_CS, LOW);
  tft.initR(INITR_BLACKTAB); // Initialize screen
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextWrap(true);
  tft.print("SCREEN OK");

  //SPI setup
  pinMode(14, OUTPUT);
  sigmaDeltaSetup(0, 312500);
    //attach pin 14 to channel 0
    sigmaDeltaAttachPin(14,0);
    //initialize channel 0 to off
    sigmaDeltaWrite(0, 511);

  //neopixel init
  strip.begin();
  strip.show();
  strip.clear();
  tft.print("NEO Pixels initialized");

  //servos init
  leftServo.setPeriodHertz(50);
  leftServo.attach(LServo, 1000, 2000);
  rightServo.setPeriodHertz(50);
  rightServo.attach(RServo, 1000, 2000);
  
  //init SD
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  if (!SD.begin(SD_CS)) {
    tft.setCursor(0, 40);
    tft.print("Error: SD Card");
    Serial.print(F("no sd"));
    return;
  }else{
    tft.setCursor(0, 40);
    tft.print("SD Card on");
  }


  // Open file for reading
  File file = SD.open(strConfigFile);
  
  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, file);
  if (error){
    Serial.println(F("Failed to read file, using default configuration"));
    Serial.println(error.c_str());
  }
  
  //const char* strLogFile = doc["logfile"];
  JsonArray arrOnce = doc["once"].as<JsonArray>();
  //intOnceCount = arrOnce.size();
  
  for(JsonVariant v : arrOnce) {
    //JsonVariant varComponent = v["component"];
    fnActions( v );
    //i=i+1;
  }
}

void loop() {
  /* LOOP THROUGH THE DEFINED ACTIONS  */
  JsonArray arrLoop = doc["loop"].as<JsonArray>();
  byte i=0;
  for(JsonVariant v : arrLoop) {
    //JsonVariant varComponent = v["component"];
    fnActions( v );
    i=i+1;
    //Serial.println(i);
  }
}

/* ----====||############* FUNCTIONS *###########||====----  */




void fnActions( JsonVariant objAction ){
  const char* strComponent = objAction["component"].as<char*>();
  const char* strAction = objAction["action"].as<char*>();
  const uint16_t intId = objAction["id"].as<uint16_t>();
  //DISPLAY
  if( strcmp(strComponent,"display") == 0 ){
    if( strcmp(strAction,"image") == 0 ){
      ImageReturnCode      stat;
      const char* strVal = objAction["value"].as<const char*>();
      stat = reader.loadBMP(const_cast<char*>(strVal), img);
      img.draw(tft, 0, 0);
    }
    if( strcmp(strAction,"rotate") == 0 ){
      if( strcmp(strAction,"landscape") == 0 ){ tft.setRotation(-45); }
      if( strcmp(strAction,"landscape2") == 0 ){ tft.setRotation(45); }
      if( strcmp(strAction,"portrait") == 0 ){ tft.setRotation(0); }
      if( strcmp(strAction,"portrait2") == 0 ){ tft.setRotation(90); }
    }
  }
  // LED
  if( strcmp(strComponent,"led") == 0 ){
    if( strcmp(strAction,"color") == 0 ){
      JsonArray arrColor=objAction["color"];
      const uint16_t intR = arrColor[0].as<uint16_t>();
      const uint16_t intG = arrColor[1].as<uint16_t>();
      const uint16_t intB = arrColor[2].as<uint16_t>();
      strip.setPixelColor(intId-1,intR,intG,intB); 
      strip.show(); 
    }
    if( strcmp(strAction,"off") == 0 ){ strip.clear(); }
    if( strcmp(strAction,"brightness") == 0){ 
      const uint16_t intVal = objAction["value"].as<uint16_t>();
      strip.setBrightness(intVal); strip.show(); 
    }
  }
  // SERVO
  if( strcmp(strComponent,"servo") == 0 ){
    const uint16_t intVal = objAction["value"].as<uint16_t>();
    if( strcmp(strAction,"move") == 0 ){
      pinMode (BOOST, OUTPUT);
      digitalWrite (BOOST, HIGH);
    }
    if( intId == 1 ){ leftServo.write( intVal ); }
    if( intId == 2 ){ rightServo.write( intVal ); }
  }
  // BOARD
  if( strcmp(strComponent,"board") == 0 ){
    if( strcmp(strAction,"delay") == 0 ){
       const uint16_t intVal = objAction["value"].as<uint16_t>();
       delay( intVal );
    }
  }
  
}
