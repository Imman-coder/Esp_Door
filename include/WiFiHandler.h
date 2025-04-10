#pragma once

#include <Arduino.h>

// Default values
#define DEFAULT_PASSWORD "1234"
#define DEFAULT_ENABLE_NFC false
#define DEFAULT_DISPLAY_PASSWORD false

#define DEFAULT_STA_ENABLE false
#define DEFAULT_STA_SSID ""
#define DEFAULT_STA_PASSWORD ""

#define DEFAULT_AP_ENABLE true
#define DEFAULT_AP_SSID "ESP Door"
#define DEFAULT_AP_PASSWORD ""


void setupWiFi();

void loopWiFi();