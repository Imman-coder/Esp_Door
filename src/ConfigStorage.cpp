#include "ConfigStorage.h"
#include "WiFiHandler.h"
#include <nvs_flash.h>

Preferences userPrefs;
Preferences globalConfig;

void setupConfigStorage()
{
    userPrefs.begin("user");
    globalConfig.begin("global");

    if (!userPrefs.isKey(CONFIG_USERS))
    {
        userPrefs.putString(CONFIG_USERS, "[]");
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

        globalConfig.putBool(CONFIG_MDNS_ENABLE, true);
        globalConfig.putString(CONFIG_MDNS_NAME, "espdoor");

        globalConfig.putBool(CONFIG_LOCKDOWN_ENABLE, true);
        globalConfig.putUInt(CONFIG_LOCKDOWN_DURATION, 5);
        globalConfig.putUInt(CONFIG_LOCKDOWN_MAX_ATTEMPTS, 3);
    }

    Serial.println("Config storage initialized");

    JsonDocument d = loadUsers();

    Serial.printf("Users: \n");
    serializeJsonPretty(d, Serial);
    Serial.println("");
}

JsonDocument loadWiFiSettings()
{
    JsonDocument doc;
    doc["sta_enable"] = globalConfig.getBool(CONFIG_STA_ENABLE);
    doc["sta_ssid"] = globalConfig.getString(CONFIG_STA_SSID);
    doc["sta_password"] = globalConfig.getString(CONFIG_STA_PASSWORD);
    doc["ap_enable"] = globalConfig.getBool(CONFIG_AP_ENABLE);
    doc["ap_ssid"] = globalConfig.getString(CONFIG_AP_SSID);
    doc["ap_password"] = globalConfig.getString(CONFIG_AP_PASSWORD);
    doc["mdns_enable"] = globalConfig.getBool(CONFIG_MDNS_ENABLE);
    doc["mdns_name"] = globalConfig.getString(CONFIG_MDNS_NAME);
    return doc;
}

void unloadWiFiSettings(const JsonObject &wifi)
{
    if (wifi["sta_enable"].is<bool>())
        globalConfig.putBool(CONFIG_STA_ENABLE, wifi["sta_enable"]);

    if (wifi["sta_ssid"].is<String>())
        globalConfig.putString(CONFIG_STA_SSID, wifi["sta_ssid"].as<const char *>());

    if (wifi["sta_password"].is<String>())
        globalConfig.putString(CONFIG_STA_PASSWORD, wifi["sta_password"].as<const char *>());

    if (wifi["ap_enable"].is<bool>())
        globalConfig.putBool(CONFIG_AP_ENABLE, wifi["ap_enable"]);

    if (wifi["ap_ssid"].is<String>())
        globalConfig.putString(CONFIG_AP_SSID, wifi["ap_ssid"].as<const char *>());

    if (wifi["ap_password"].is<String>())
        globalConfig.putString(CONFIG_AP_PASSWORD, wifi["ap_password"].as<const char *>());

    if (wifi["mdns_enable"].is<bool>())
        globalConfig.putBool(CONFIG_MDNS_ENABLE, wifi["mdns_enable"]);

    if (wifi["mdns_name"].is<String>())
        globalConfig.putString(CONFIG_MDNS_NAME, wifi["mdns_name"].as<const char *>());
}

JsonDocument loadLockdownSettings()
{
    JsonDocument doc;
    doc["enable"] = globalConfig.getBool(CONFIG_LOCKDOWN_ENABLE);
    doc["max_attempts"] = globalConfig.getUInt(CONFIG_LOCKDOWN_MAX_ATTEMPTS);
    doc["duration"] = globalConfig.getUInt(CONFIG_LOCKDOWN_DURATION);
    return doc;
}

void unloadLockdownSettings(const JsonObject &lockdown)
{
    if (lockdown["enable"].is<bool>())
        globalConfig.putBool(CONFIG_LOCKDOWN_ENABLE, lockdown["enable"]);

    if (lockdown["max_attempts"].is<int>())
        globalConfig.putUInt(CONFIG_LOCKDOWN_MAX_ATTEMPTS, lockdown["max_attempts"]);

    if (lockdown["duration"].is<int>())
        globalConfig.putUInt(CONFIG_LOCKDOWN_DURATION, lockdown["duration"]);
}

JsonDocument loadUsers()
{
    String json = userPrefs.getString(CONFIG_USERS);
    JsonDocument doc;
    deserializeJson(doc, json);
    return doc;
}

bool getUserByUsername(String username, JsonDocument &user_ob)
{
    JsonDocument usersDoc = loadUsers();
    JsonArray users = usersDoc.as<JsonArray>();
    for (JsonObject user : users)
    {
        if (user["username"] == username)
        {
            user_ob.set(user);
            return true;
        }
    }
    return false;
}

void saveUsers(const JsonDocument &doc)
{
    String out;
    serializeJson(doc, out);
    userPrefs.putString(CONFIG_USERS, out);
}

void nvs_reset()
{
    nvs_flash_erase();
    nvs_flash_init();
};