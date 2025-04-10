#pragma once

#include <Preferences.h>

#define CONFIG_PIN "pin"
#define CONFIG_ENABLE_NFC "enable_nfc"
#define CONFIG_TAGS "tags"
#define CONFIG_AP_ENABLE "ap_enable"
#define CONFIG_AP_SSID "ap_ssid"
#define CONFIG_AP_PASSWORD "ap_password"
#define CONFIG_STA_ENABLE "sta_enable"
#define CONFIG_STA_SSID "sta_ssid"
#define CONFIG_STA_PASSWORD "sta_password"
#define CONFIG_MDNS_ENABLE "mdns_enable"
#define CONFIG_MDNS_NAME "mdns_name"
#define CONFIG_OTA_ENABLE "ota_enable"
#define CONFIG_LOCKDOWN_ENABLE "lockdown_enable"
#define CONFIG_MAX_ATTEMPTS "max_attempts"
#define CONFIG_LOCKDOWN_DURATION "lockdown_duration"


extern Preferences userConfig;
extern Preferences globalConfig;


void setupConfigStorage();

void nvs_reset();