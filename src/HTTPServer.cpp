#include "HTTPServer.h"
#include "webfiles.h"
#include "UnlockLogic.h"

#include <AsyncTCP.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <Update.h>

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void handleWebFiles(AsyncWebServerRequest *request);
void handleWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
void onDoorStateChange(bool state);
void onLockStateChange(bool state);
void handleWebsocketMessage(char *data);
void handleOTAUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
void handleOTARequest(AsyncWebServerRequest *request);

void setupWebserver()
{
    ws.onEvent(handleWebSocketEvent);
    server.addHandler(&ws);

    server.on("/update", HTTP_POST, handleOTARequest, handleOTAUpload);

    registerDoorStatusCallback(onDoorStateChange);
    registerLockStatusCallback(onLockStateChange);

#ifndef USE_PROGMEM_WEB_FILES

    LittleFS.begin();

    server.serveStatic("/", LittleFS, "/web/").setDefaultFile("index.html");
    server.serveStatic("/login", LittleFS, "/web/index.html");
    server.serveStatic("/settings", LittleFS, "/web/index.html");
    server.serveStatic("/update", LittleFS, "/web/index.html"); 

#endif

    server.onNotFound(handleWebFiles);

    Serial.println("Webserver setup done");
}

void startWebserver()
{
    server.begin();
    Serial.println("Webserver started");
}

void stopWebserver()
{
    server.end();
    Serial.println("Webserver stopped");
}

/*----------------------Webserver Handling--------------------------*/

String getContentType(String filename)
{
    if (filename.endsWith(".html"))
        return "text/html";
    if (filename.endsWith(".css"))
        return "text/css";
    if (filename.endsWith(".js"))
        return "application/javascript";
    if (filename.endsWith(".png"))
        return "image/png";
    if (filename.endsWith(".jpg"))
        return "image/jpeg";
    if (filename.endsWith(".ico"))
        return "image/x-icon";
    if (filename.endsWith(".json"))
        return "application/json";
    if (filename.endsWith(".gz"))
        return "application/octet-stream";
    return "text/plain";
}

void handleWebFiles(AsyncWebServerRequest *request)
{

    String path = request->url();
    if (path == "/" || path == "/login" || path == "/settings" || path == "/update")
        path = "/index.html";
    Serial.printf("Page request: %s \n", path);

#ifdef USE_PROGMEM_WEB_FILES

    for (unsigned int i = 0; i < webFilesCount; i++)
    {
        if (path.equals(webFiles[i].filename))
        {
            String contentType = getContentType(path);
            AsyncWebServerResponse *response = request->beginResponse(200, contentType, webFiles[i].data, webFiles[i].length);
            response->addHeader("Content-Encoding", "gzip");
            request->send(response);
            return;
        }
    }

#endif

    request->send(404, "text/plain", "File Not Found");
}

/*----------------------WebSocket Handling--------------------------*/

void handleWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
    if (type == WS_EVT_CONNECT)
    {
        ws.textAll("new client connected");
        Serial.println("ws connect");
        client->setCloseClientOnQueueFull(false);
        client->ping();
    }
    else if (type == WS_EVT_DISCONNECT)
    {
        ws.textAll("client disconnected");
        Serial.println("ws disconnect");
    }
    else if (type == WS_EVT_ERROR)
    {
        Serial.println("ws error");
    }
    else if (type == WS_EVT_PONG)
    {
        Serial.println("ws pong");
    }
    else if (type == WS_EVT_DATA)
    {
        AwsFrameInfo *info = (AwsFrameInfo *)arg;
        // Serial.printf("index: %" PRIu64 ", len: %" PRIu64 ", final: %" PRIu8 ", opcode: %" PRIu8 "\n", info->index, info->len, info->final, info->opcode);
        String msg = "";
        if (info->final && info->index == 0 && info->len == len)
        {
            if (info->opcode == WS_TEXT)
            {
                
                handleWebsocketMessage((char *) data);
            }
        }
    }
}

void handleWebsocketMessage(char* m)
{
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, m);
    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
    }
    else
    {
        if (doc["action_type"] == "door")
        {
            if (doc["action"] == "unlock_door")
            {
                unlockDoor();
            }
            else if (doc["action"] == "lock_door")
            {
                lockDoor();
            }
            else if (doc["action"] == "refresh")
            {
                ws.textAll("{\"action\":\"door_state\", \"state\": \"" + String(isDoorOpened ? "opened" : "closed") + "\"}");
                ws.textAll("{\"action\":\"lock_state\", \"state\": \"" + String(isDoorLocked ? "locked" : "unlocked") + "\"}");
            }
            else
            {
                Serial.println("Unknown Door Action");
            }
        }
        else if (doc["action_type"] == "settings")
        {
        }
    }
}

/*---------------------------------State Change Notifiers-----------------------------------------*/

void onDoorStateChange(bool state)
{
    ws.textAll("{\"action\":\"door_state\", \"state\": \"" + String(state ? "opened" : "closed") + "\"}");
}

void onLockStateChange(bool state)
{
    ws.textAll("{\"action\":\"lock_state\", \"state\": \"" + String(state ? "locked" : "unlocked") + "\"}");
}

/*-----------------------------------------OTA Handler-------------------------------------------------*/

void handleOTARequest(AsyncWebServerRequest *request)
{
    AsyncWebServerResponse *response = request->beginResponse((Update.hasError()) ? 400 : 200, "text/plain", (Update.hasError()) ? "Update Error" : "OK");
    response->addHeader("Connection", "close");
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
    ESP.restart();
}

void handleOTAUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{

    if (!index)
    {
        String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
        Serial.println(logmessage);
        Serial.printf("Update: %s\n", filename.c_str());

        bool isFirmware = false;

        if (request->hasParam("mode"))
        {
            String mode = request->getParam("mode")->value();
            isFirmware = (mode == "firm");
        }

        if (request->hasParam("hash"))
        {
            String hash = request->getParam("hash")->value();
            if (!Update.setMD5(hash.c_str()))
            {
                return request->send(400, "text/plain", "MD5 parameter invalid");
            }
        }

        if (!Update.begin(UPDATE_SIZE_UNKNOWN, isFirmware ? U_FLASH : U_SPIFFS)) // start with max available size
        {
            Serial.println("Update begin failed");
            request->send(400, "text/plain", "Update begin failed");
            return;
        }
        else
        {
            Serial.printf("Update started: %s\n", filename.c_str());
            if (isFirmware)
            {
                Serial.println("Firmware update started");
            }
            else
            {
                Serial.println("SPIFFS update started");
            }
        }
    }

    if (len)
    {
        /* flashing firmware to ESP*/
        if (!Update.write(data, len) != index)
        {
            Update.printError(Serial);
        }
        Serial.printf("Update Progress: %u%%\n", (index + len) * 100 / Update.size());
    }

    if (final)
    {
        if (Update.end(true))
        { // true to set the size to the current progress
            Serial.printf("Update Success: %u\nRebooting...\n");
        }
        else
        {
            Serial.println("Update failed!");
            Update.printError(Serial);
        }
    }
}

