#include <Arduino.h>

#include <ESPAsyncWebServer.h> // don't know but causing compilation error

#include "ConfigStorage.h"
#include "BuzzerHandler.h"
#include "LCDDisplay.h"
#include "KeypadHandler.h"
#include "RFIDHandler.h"
#include "WiFiHandler.h"
#include "HTTPServer.h"
#include "UnlockLogic.h"

void setup()
{
  Serial.begin(115200);
  setupConfigStorage();
  setupLCD();
  setupRFID();
  setupKeypad();
  setupWiFi();
  setupWebserver();
  startWebserver();
  setupBuzzer();
  setupUnlockLogic();
  Serial.println("Version 2.62");
}

void loop()
{
  loopRFID();
  loopKeypad();
  loopBuzzer();
  loopUnlock();
  loopLCD();
}