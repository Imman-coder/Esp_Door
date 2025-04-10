#include "WiFiHandler.h"
#include <WiFi.h>
#include <WiFiAP.h>
#include <ESPmDNS.h>
#include "ConfigStorage.h"

void updateMDNS();
void updateWiFi();

bool shouldReconnect = false;

unsigned long lastReconnectAttempt = 0;
const unsigned long reconnectInterval = 10000;

void setupWiFi()
{

    updateWiFi();
    updateMDNS();

    Serial.println("Web server started");
    Serial.println("IP address: " + WiFi.localIP().toString());
}

void updateMDNS()
{
    if (globalConfig.getBool(CONFIG_MDNS_ENABLE))
    {
        if (!MDNS.begin(globalConfig.getString(CONFIG_MDNS_NAME, "espdoor")))
        {
            Serial.println("Error setting up MDNS responder!");
            return;
        }
        Serial.println("MDNS responder started.");
        Serial.print(globalConfig.getString(CONFIG_MDNS_NAME, "espdoor"));
        Serial.println(".local");
    }
    else
    {
        MDNS.end();
        Serial.println("MDNS responder stopped");
    }
}

void updateWiFi()
{
    bool ap_enable = globalConfig.getBool(CONFIG_AP_ENABLE);
    String ap_ssid = globalConfig.getString(CONFIG_AP_SSID);
    String ap_password = globalConfig.getString(CONFIG_AP_PASSWORD);
    bool sta_enable = globalConfig.getBool(CONFIG_STA_ENABLE);
    String sta_ssid = globalConfig.getString(CONFIG_STA_SSID);
    String sta_password = globalConfig.getString(CONFIG_STA_PASSWORD);

    if (ap_enable && sta_enable)
    {
        shouldReconnect = true;
        Serial.println("\n\nAP+STA mode enabled");
        WiFi.mode(WIFI_MODE_APSTA);
        Serial.println("Starting AP mode...");
        Serial.println("SSID: " + ap_ssid);
        Serial.println("Password: " + ap_password);
        WiFi.softAP(ap_ssid, ap_password);

        Serial.println("------------------");
        Serial.println("Starting STA mode...");
        Serial.println("SSID: " + sta_ssid);
        Serial.println("Password: " + sta_password);
        WiFi.begin(sta_ssid, sta_password);
    }
    else if (ap_enable)
    {
        Serial.println("\n\nStarting AP mode...");
        Serial.println("SSID: " + ap_ssid);
        Serial.println("Password: " + ap_password);
        WiFi.mode(WIFI_MODE_AP);
        WiFi.softAP(ap_ssid, ap_password);
    }
    else if (sta_enable)
    {
        shouldReconnect = true;
        Serial.println("\n\nStarting STA mode...");
        Serial.println("SSID: " + sta_ssid);
        Serial.println("Password: " + sta_password);
        WiFi.mode(WIFI_MODE_STA);
        WiFi.begin(sta_ssid, sta_password);

        Serial.print("Connecting to WiFi ..");
        while (WiFi.status() != WL_CONNECTED)
        {
            Serial.print('.');
            delay(1000);
        }
        Serial.println(WiFi.localIP());
    }
    else
    {
        WiFi.mode(WIFI_MODE_NULL);
    }
}

void loopWifi()
{
    if (shouldReconnect && millis() - lastReconnectAttempt > lastReconnectAttempt)
    {

        switch (WiFi.status())
        {
        case WL_CONNECT_FAILED:
        case WL_CONNECTION_LOST:
        case WL_DISCONNECTED:
            WiFi.reconnect();
            break;
        default:
            break;
        }
    }
}