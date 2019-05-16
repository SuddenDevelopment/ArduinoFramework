# ArduinoFramework
my evolving framework to work with Arduino code. configure by JSON on an SD card, log, act, react, connect etc. The one uncommon thing this assumes is an SD card, could be an API call to get config too though, that would be pretty cool.

# Goal
Create a more pattern of code that allows any reconfiguring of actions and their order to be defined easily by a web page. I want tpeople to be able to customize my idevices without knowing Arduino, or being intimidated by something overly complex. 

# Sections
1. Libraries: all the things you have to install to make it work
2. Pins: mapping of pins to functions
2. Instances: For example, how many servos do you have? Each neds an instance of Servo
3. Variables: Logic variables
4. Setup: what happens once. includes whatever is required to initialize components
5. Loop: what happens repeatedly
6. Actions: map all of the possible things your device can do to the JSON values on the SD card

## Example config.js on SD card
```javascript

{
    "once": [
         {"component":"display","id":0,"action":"type","value":"testing"}
        ,{"component":"led","id":1,"action":"color","color":[255,0,0]}
        ,{"component":"display","id":0,"action":"image","value":"/face.bmp"}
        ,{"component":"board","id":0,"action":"delay","value":5000}
        ,{"component":"display","id":0,"action":"type","value":"done"}
    ],"loop": [
         {"component":"led","id":1,"action":"color","color":[255,0,0]}
        ,{"component":"led","id":2,"action":"color","color":[0,0,255]}
        ,{"component":"board","id":0,"action":"delay","value":500}
    ],"payload": 'payload.txt'
     ,"logfile": 'log.txt'
}


```

## Example code section in actions
```c

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

```

You can see where the component, action and value matches up for the display. This is done once and then as long as the hardware stays the same you can reconfigure it in JSON.

## Gotchas
1. Data types in Arduino are unforgiving. if you give it a value format it doesnt expect you'll need to play around with it.