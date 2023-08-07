#pragma once

#include "wled.h"
#include <TFT_eSPI.h>
#include <SPI.h>
#include "WiFi.h"
#include <Wire.h>

// v2 Usermod to automatically save settings 
// to configurable preset after a change to any of
//
// * brightness
// * effect speed
// * effect intensity
// * mode (effect)
// * palette
//
// but it will wait for configurable number of seconds, a "settle" 
// period in case there are other changes (any change will 
// extend the "settle" window).
//
// It can be configured to load auto saved preset at startup,
// during the first `loop()`.
//
// AutoSaveUsermod is standalone, but if FourLineDisplayUsermod 
// is installed, it will notify the user of the saved changes.

// format: "~ MM-DD HH:MM:SS ~"
#define PRESET_NAME_BUFFER_SIZE 25

#ifndef TFT_DISPOFF
#define TFT_DISPOFF 0x28
#endif

#ifndef TFT_SLPIN
#define TFT_SLPIN   0x10
#endif

#define TFT_MOSI  19
#define TFT_SCLK  18
#define TFT_CS    5
#define TFT_DC    16
#define TFT_RST   23

#define TFT_BL    4  // Display backlight control pin
#define ADC_EN    14  // Used for enabling battery voltage measurements - not used in this program

TFT_eSPI tft = TFT_eSPI(135, 240); // Invoke custom library

class TDisplayUsermod : public Usermod {

  private:
    bool firstLoop = true;
    bool initDone = false;
    bool enabled = true;
    unsigned long nextLoopTime = 0;
    
    // configurable parameters
    #ifdef TTGO_LOOP_INTERVAL
      uint16_t loopInterval = TTGO_LOOP_INTERVAL;
    #else
      uint16_t loopInterval = 2000;
    #endif

    bool needRedraw = true;
    // Next variables hold the previous known values to determine if redraw is
    // required.
    String knownSsid = "";
    IPAddress knownIp;
    uint8_t knownBrightness = 0;
    uint8_t knownMode = 0;
    uint8_t knownPalette = 0;
    uint8_t tftcharwidth = 19;  // Number of chars that fit on screen with text size set to 2

    long lastUpdate = 0;
    long lastRedraw = 0;
    bool displayTurnedOff = false;

    // strings to reduce flash memory usage (used more than twice)
    static const char _name[];
    static const char _loopInterval[];
    
    void inline saveSettings() {
      char presetNameBuffer[PRESET_NAME_BUFFER_SIZE];
      updateLocalTime();
      sprintf_P(presetNameBuffer, 
        PSTR("~ %02d-%02d %02d:%02d:%02d ~"),
        month(localTime), day(localTime),
        hour(localTime), minute(localTime), second(localTime));
      cacheInvalidate++;  // force reload of presets
      // savePreset(autoSavePreset, presetNameBuffer);
    }
 
    void enable(bool enable) {
      enabled = enable;
    }

  public:
    void updateDisplay(){
      // if(!displayTurnedOff && millis() - lastRedraw > 5*60*1000) {
      //   digitalWrite(TFT_BL, LOW); // Turn backlight off. 
      //   displayTurnedOff = true;
      // } 

      // Check if values which are shown on display changed from the last time.
      if (((apActive) ? String(apSSID) : WiFi.SSID()) != knownSsid) {
        needRedraw = true;
      } else if (knownIp != (apActive ? IPAddress(4, 3, 2, 1) : WiFi.localIP())) {
        needRedraw = true;
      } else if (knownBrightness != bri) {
        needRedraw = true;
      } else if (knownMode != strip.getMainSegment().mode) {
        needRedraw = true;
      } else if (knownPalette != strip.getMainSegment().palette) {
        needRedraw = true;
      }

      if (!needRedraw) {
        return;
      }
      needRedraw = false;
      
      // if (displayTurnedOff)
      // {
      //  digitalWrite(TFT_BL, TFT_BACKLIGHT_ON); // Turn backlight on.
      //  displayTurnedOff = false;
      // }
      lastRedraw = millis();

      knownSsid = WiFi.SSID();
      knownIp = apActive ? IPAddress(4, 3, 2, 1) : WiFi.localIP();
      knownBrightness = bri;
      knownMode = strip.getMainSegment().mode;
      knownPalette = strip.getMainSegment().palette;

// #define TFT_NAVY        0x000F      /*   0,   0, 128 */
// #define TFT_DARKGREEN   0x03E0      /*   0, 128,   0 */
// #define TFT_DARKCYAN    0x03EF      /*   0, 128, 128 */
// #define TFT_MAROON      0x7800      /* 128,   0,   0 */
// #define TFT_PURPLE      0x780F      /* 128,   0, 128 */
// #define TFT_OLIVE       0x7BE0      /* 128, 128,   0 */
// #define TFT_LIGHTGREY   0xD69A      /* 211, 211, 211 */
// #define TFT_DARKGREY    0x7BEF      /* 128, 128, 128 */
// #define TFT_BLUE        0x001F      /*   0,   0, 255 */
// #define TFT_GREEN       0x07E0      /*   0, 255,   0 */
// #define TFT_CYAN        0x07FF      /*   0, 255, 255 */
// #define TFT_RED         0xF800      /* 255,   0,   0 */
// #define TFT_MAGENTA     0xF81F      /* 255,   0, 255 */
// #define TFT_YELLOW      0xFFE0      /* 255, 255,   0 */
// #define TFT_WHITE       0xFFFF      /* 255, 255, 255 */
// #define TFT_ORANGE      0xFDA0      /* 255, 180,   0 */
// #define TFT_GREENYELLOW 0xB7E0      /* 180, 255,   0 */
// #define TFT_PINK        0xFE19      /* 255, 192, 203 */ //Lighter pink, was 0xFC9F
// #define TFT_BROWN       0x9A60      /* 150,  75,   0 */
// #define TFT_GOLD        0xFEA0      /* 255, 215,   0 */
// #define TFT_SILVER      0xC618      /* 192, 192, 192 */
// #define TFT_SKYBLUE     0x867D      /* 135, 206, 235 */
// #define TFT_VIOLET      0x915C      /* 180,  46, 226 */

      tft.fillScreen(TFT_BLACK);
      tft.setTextSize(2);
      tft.setTextColor(TFT_YELLOW);

      // First row with Wifi name
      tft.setCursor(1, 1);
      tft.print(knownSsid.substring(0, tftcharwidth > 1 ? tftcharwidth - 1 : 0));
      // Print `~` char to indicate that SSID is longer, than our dicplay
      if (knownSsid.length() > tftcharwidth)
        tft.print("~");

      // Second row with AP IP and Password or IP
      tft.setTextSize(2);
      tft.setCursor(1, 24);
      tft.setTextColor(TFT_GREENYELLOW);
      // Print AP IP and password in AP mode or knownIP if AP not active.
      // if (apActive && bri == 0)
      //   tft.print(apPass);
      // else
      //   tft.print(knownIp);

      if (apActive) {
        tft.print("AP IP: ");
        tft.print(knownIp);
        tft.setCursor(1,46);
        tft.print("AP Pass:");
        tft.print(apPass);
      }
      else {
        tft.print("IP: ");
        tft.print(knownIp);
        tft.setCursor(1,46);
        tft.setTextColor(TFT_GREEN);
        //tft.print("Signal Strength: ");
        //tft.print(i.wifi.signal);
        tft.print("Bright: ");
        tft.print(((float(bri)/255)*100));
        tft.print("%");
      }

      // Third row with mode name
      tft.setCursor(1, 68);
      tft.setTextColor(TFT_SKYBLUE);
      char lineBuffer[tftcharwidth+1];
      extractModeName(knownMode, JSON_mode_names, lineBuffer, tftcharwidth);
      tft.print(lineBuffer);

      // Fourth row with palette name
      tft.setCursor(1, 90);
      tft.setTextColor(TFT_BLUE);
      extractModeName(knownPalette, JSON_palette_names, lineBuffer, tftcharwidth);
      tft.print(lineBuffer);

      // Fifth row with estimated mA usage
      tft.setCursor(1, 112);
      tft.setTextColor(TFT_VIOLET);
      // Print estimated milliamp usage (must specify the LED type in LED prefs for this to be a reasonable estimate).
      tft.print(strip.currentMilliamps);
      tft.print("mA est.");
    }

    void setup() {
      Serial.println("T-Display Start");
      tft.init();
      tft.setRotation(3);  //Rotation here is set up for the text to be readable with the port on the left. Use 1 to flip.
      tft.fillScreen(TFT_BLACK);
      tft.setTextSize(2);
      tft.setTextColor(TFT_WHITE);
      tft.setCursor(1, 10);
      tft.setTextDatum(MC_DATUM);
      tft.setTextSize(3);
      tft.print("Loading...");

      if (TFT_BL > 0) { // TFT_BL has been set in the TFT_eSPI library in the User Setup file TTGO_T_Display.h
          pinMode(TFT_BL, OUTPUT); // Set backlight pin to output mode
          digitalWrite(TFT_BL, HIGH); // Turn backlight on. 
      }

      initDone = true;
      knownBrightness = bri;
      //knownMode = strip.getMainSegment().mode;
      //knownPalette = strip.getMainSegment().palette;
    }

    // gets called every time WiFi is (re-)connected. Initialize own network
    // interfaces here
    void connected() {}

    /*
     * Da loop.
     */
    void loop() {
      // if (!autoSaveAfterSec || !enabled || strip.isUpdating() || currentPreset>0) return;  // setting 0 as autosave seconds disables autosave
      if(nextLoopTime > millis()) return;
      
      nextLoopTime  = loopInterval + millis();
      updateDisplay();
      // uint8_t currentMode = strip.getMainSegment().mode;
      // uint8_t currentPalette = strip.getMainSegment().palette;

      // unsigned long wouldAutoSaveAfter = now + autoSaveAfterSec*1000;
      // if (knownBrightness != bri) {
      //   knownBrightness = bri;
      //   autoSaveAfter = wouldAutoSaveAfter;
      // }

      // if (autoSaveAfter && now > autoSaveAfter) {
      //   autoSaveAfter = 0;
      //   // Time to auto save. You may have some flickry?
      //   saveSettings();
      // }
    }

    /*
     * addToJsonInfo() can be used to add custom entries to the /json/info part of the JSON API.
     * Creating an "u" object allows you to add custom key/value pairs to the Info section of the WLED web UI.
     * Below it is shown how this could be used for e.g. a light sensor
     */
    void addToJsonInfo(JsonObject& root) {
      JsonObject user = root["u"];
      if (user.isNull()) {
        user = root.createNestedObject("u");
      }

      JsonArray infoArr = user.createNestedArray(FPSTR(_name));  // name

      String uiDomString = F("<button class=\"btn btn-xs\" onclick=\"requestJson({");
      uiDomString += FPSTR(_name);
      uiDomString += F(":{");
      uiDomString += FPSTR(_loopInterval);
      uiDomString += enabled ? F(":false}});\">") : F(":true}});\">");
      uiDomString += F("<i class=\"icons ");
      uiDomString += enabled ? "on" : "off";
      uiDomString += F("\">&#xe08f;</i></button>");
      infoArr.add(uiDomString);
    }

    /*
     * addToJsonState() can be used to add custom entries to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
    //void addToJsonState(JsonObject& root) {
    //}

    /*
     * readFromJsonState() can be used to receive data clients send to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
    void readFromJsonState(JsonObject& root) {
      if (!initDone) return;  // prevent crash on boot applyPreset()
      bool en = enabled;
      JsonObject um = root[FPSTR(_name)];
      if (!um.isNull()) {
        if (um[FPSTR(_loopInterval)].is<bool>()) {
          en = um[FPSTR(_loopInterval)].as<bool>();
        } else {
          String str = um[FPSTR(_loopInterval)]; // checkbox -> off or on
          en = (bool)(str!="off"); // off is guaranteed to be present
        }
        if (en != enabled) enable(en);
      }
    }

    /*
     * addToConfig() can be used to add custom persistent settings to the cfg.json file in the "um" (usermod) object.
     * It will be called by WLED when settings are actually saved (for example, LED settings are saved)
     * If you want to force saving the current state, use serializeConfig() in your loop().
     * 
     * CAUTION: serializeConfig() will initiate a filesystem write operation.
     * It might cause the LEDs to stutter and will cause flash wear if called too often.
     * Use it sparingly and always in the loop, never in network callbacks!
     * 
     * addToConfig() will also not yet add your setting to one of the settings pages automatically.
     * To make that work you still have to add the setting to the HTML, xml.cpp and set.cpp manually.
     * 
     * I highly recommend checking out the basics of ArduinoJson serialization and deserialization in order to use custom settings!
     */
    void addToConfig(JsonObject& root) {
      // we add JSON object: {"Autosave": {"autoSaveAfterSec": 10, "autoSavePreset": 99}}
      JsonObject top = root.createNestedObject(FPSTR(_name)); // usermodname
      top[FPSTR(_loopInterval)]     = enabled;
      // top[FPSTR(_autoSaveAfterSec)]    = autoSaveAfterSec;  // usermodparam
      DEBUG_PRINTLN(F("Autosave config saved."));
    }

    /*
     * readFromConfig() can be used to read back the custom settings you added with addToConfig().
     * This is called by WLED when settings are loaded (currently this only happens once immediately after boot)
     * 
     * readFromConfig() is called BEFORE setup(). This means you can use your persistent values in setup() (e.g. pin assignments, buffer sizes),
     * but also that if you want to write persistent values to a dynamic buffer, you'd need to allocate it here instead of in setup.
     * If you don't know what that is, don't fret. It most likely doesn't affect your use case :)
     * 
     * The function should return true if configuration was successfully loaded or false if there was no configuration.
     */
    bool readFromConfig(JsonObject& root) {
      // we look for JSON object: {"Autosave": {"enabled": true, "autoSaveAfterSec": 10, "autoSavePreset": 250, ...}}
      JsonObject top = root[FPSTR(_name)];
      if (top.isNull()) {
        DEBUG_PRINT(FPSTR(_name));
        DEBUG_PRINTLN(F(": No config found. (Using defaults.)"));
        return false;
      }

      enabled             = top[FPSTR(_loopInterval)] | enabled;
      // autoSaveAfterSec    = top[FPSTR(_autoSaveAfterSec)] | autoSaveAfterSec;
      // autoSaveAfterSec    = (uint16_t) min(3600,max(10,(int)autoSaveAfterSec)); // bounds checking
      // autoSaveAfterSec      = top[FPSTR(_autoSavePreset)] | autoSavePreset;
      // autoSaveAfterSec      = (uint8_t) min(250,max(100,(int)autoSavePreset)); // bounds checking
      DEBUG_PRINT(FPSTR(_name));
      DEBUG_PRINTLN(F(" config (re)loaded."));

      // use "return !top["newestParameter"].isNull();" when updating Usermod with new features
      return true;
  }

    /*
     * getId() allows you to optionally give your V2 usermod an unique ID (please define it in const.h!).
     * This could be used in the future for the system to determine whether your usermod is installed.
     */
    uint16_t getId() {
      return USERMOD_ID_TDISPLAY;
    }
};

// strings to reduce flash memory usage (used more than twice)
const char TDisplayUsermod::_name[]         PROGMEM = "T-Display";
const char TDisplayUsermod::_loopInterval[] PROGMEM = "interval";