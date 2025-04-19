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
#include "LockdownHandler.h"

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
  setupLockdown();
  setupBuzzer();
  setupUnlockLogic();
  Serial.println("Version 2.102");

  CheckFirstRun();
}

void loop()
{
  // if lockdown mode is enabled don't take any input.
  if (loopLockdownWithLockStatus())
  {
    return;
  }

  loopRFID();
  loopKeypad();
  loopBuzzer();
  loopUnlock();
  loopLCD();
}