#pragma once

#include <Preferences.h>
#include <ArduinoJson.h>

#define CONFIG_USERS "users"

#define CONFIG_PIN "pin"
#define CONFIG_ENABLE_NFC "enfc"
#define CONFIG_TAGS "tags"

#define CONFIG_AP_ENABLE "ape"
#define CONFIG_AP_SSID "aps"
#define CONFIG_AP_PASSWORD "app"
#define CONFIG_STA_ENABLE "stae"
#define CONFIG_STA_SSID "stas"
#define CONFIG_STA_PASSWORD "stap"

#define CONFIG_MDNS_ENABLE "mdnse"
#define CONFIG_MDNS_NAME "mdnsn"

#define CONFIG_OTA_ENABLE "otae"

#define CONFIG_LOCKDOWN_ENABLE "lde"
#define CONFIG_LOCKDOWN_DURATION "ldd"
#define CONFIG_LOCKDOWN_MAX_ATTEMPTS "ldma"

extern Preferences userPrefs;
extern Preferences globalConfig;

void setupConfigStorage();

JsonDocument loadUsers();
JsonDocument loadWiFiSettings();
JsonDocument loadLockdownSettings();

void unloadWiFiSettings(const JsonObject &wifi);
void unloadLockdownSettings(const JsonObject &lockdown);

bool getUserByUsername(String username, JsonDocument &user_ob);
void saveUsers(const JsonDocument &doc);

void nvs_reset();