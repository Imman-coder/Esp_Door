#pragma once

#include <Arduino.h>


void setupLCD();
void lcdPrint(const String& line1, const String& line2 = "");
void lcdClear();
void lcdWelcome();
void lcdPrintTemporary(const String& line1, const String& line2 = "", unsigned long timeout = 2500);

void setBacklight(bool state);
void toggleBacklight();

void loopLCD();