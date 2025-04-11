#include "ConfigStorage.h"
#include "WiFiHandler.h"
#include <nvs_flash.h>

Preferences userPrefs;
Preferences globalConfig;

void setupConfigStorage()
{
    userPrefs.begin("user");
    globalConfig.begin("global");

    if (!globalConfig.isKey(CONFIG_STA_ENABLE))
    {
        globalConfig.putBool(CONFIG_STA_ENABLE, DEFAULT_STA_ENABLE);
        globalConfig.putString(CONFIG_STA_SSID, DEFAULT_STA_SSID);
        globalConfig.putString(CONFIG_STA_PASSWORD, DEFAULT_STA_PASSWORD);
        globalConfig.putBool(CONFIG_AP_ENABLE, DEFAULT_AP_ENABLE);
        globalConfig.putString(CONFIG_AP_SSID, DEFAULT_AP_SSID);
        globalConfig.putString(CONFIG_AP_PASSWORD, DEFAULT_AP_PASSWORD);
        globalConfig.putBool(CONFIG_OTA_ENABLE, true);
        globalConfig.putBool(CONFIG_LOCKDOWN_ENABLE, false);
    }

    Serial.println("Config storage initialized");
}

JsonDocument loadUsers() {
    String json = userPrefs.getString("users", "[]");
    JsonDocument doc;
    deserializeJson(doc, json);
    return doc;
}

void saveUsers(const JsonDocument& doc) {
    String out;
    serializeJson(doc, out);
    userPrefs.putString("users", out);
}

void nvs_reset()
{
    nvs_flash_erase();
    nvs_flash_init();
};