#pragma once

#include <Arduino.h>
#include <Keypad.h>


enum SetupMode  {
    NORMAL,
    PASSWORD_RESET,
    NFC_OPTIONS,
    NFC_SCAN_TO_REGISTER,
    NFC_SCAN_TO_DELETE
};

extern SetupMode keypadMode;


String keypadModeToString();

void loopKeypad();

void clearPassword();

void setupKeypad();
