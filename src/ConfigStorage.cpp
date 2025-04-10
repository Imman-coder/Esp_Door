#include "ConfigStorage.h"
#include "WiFiHandler.h"
#include <nvs_flash.h>

Preferences userConfig;
Preferences globalConfig;

void setupConfigStorage()
{
    userConfig.begin("user");
    globalConfig.begin("global");

    if (!userConfig.isKey(CONFIG_PIN))
    {
        userConfig.putString(CONFIG_PIN, DEFAULT_PASSWORD);
        userConfig.putBool(CONFIG_ENABLE_NFC, DEFAULT_ENABLE_NFC);
        userConfig.putString(CONFIG_TAGS, "[\"21F21902\"]");
    }
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

void nvs_reset()
{
    nvs_flash_erase();
    nvs_flash_init();
};