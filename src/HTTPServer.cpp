#include "HTTPServer.h"
#include "webfiles.h"
#include "UnlockLogic.h"
#include "KeypadHandler.h"
#include "ConfigStorage.h"

#include <map>
#include <AsyncTCP.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <Update.h>
#include <ArduinoOTA.h>

std::map<String, AsyncWebSocketClient *> userClients;
std::map<uint32_t, String> clientIdToUsername;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void handleWebFiles(AsyncWebServerRequest *request);
void handleWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
void onDoorStateChange(bool state);
void onLockStateChange(bool state);
void handleWebsocketMessage(AsyncWebSocketClient *client, char *data);
void handleOTAUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
void handleOTARequest(AsyncWebServerRequest *request);
void initOTA();
void handleLogin(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);

void setupWebserver()
{
    ws.onEvent(handleWebSocketEvent);
    server.addHandler(&ws);

    server.on("/update", HTTP_POST, handleOTARequest, handleOTAUpload);

    server.on("/login", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, handleLogin);

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

void handleLogin(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
    JsonDocument jsonDoc;
    DeserializationError error = deserializeJson(jsonDoc, data);

    if (error)
    {
        Serial.println("Failed to parse JSON");
        request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        return;
    }

    JsonObject body = jsonDoc.as<JsonObject>();

    const char *username = body["username"];
    const char *password = body["password"];

    Serial.printf("User: %s, Password: %s\n", username, password);
    JsonDocument user;

    if (getUserByPassword(username, user) && user["username"].as<String>() == username)
    {
        request->send(200, "application/json", "{\"status\":\"ok\"}");
    }
    else
    {
        request->send(400, "application/json", "{\"status\":\"unauthorized\"}");
    }
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

                handleWebsocketMessage(client, (char *)data);
            }
        }
    }
}

String makeStatusUpdateResponse()
{
    JsonDocument r;
    r["type"] = "status_update";
    r["door_status"] = String(isDoorOpened ? "opened" : "closed");
    r["lock_status"] = String(isDoorLocked ? "locked" : "unlocked");
    r["current_user"] = userId;
    String s;
    serializeJson(r, s);
    return s;
}

String makeSettingsResponse(JsonDocument currentUser)
{
    JsonDocument r;
    r["type"] = "settings_update";

    JsonDocument d;

    if (currentUser["admin"].as<bool>())
    {
        // Load and filter users
        JsonDocument usersDoc = loadUsers();
        JsonArray users = usersDoc.as<JsonArray>();
        JsonArray filteredUsers = d["users"].to<JsonArray>();

        for (JsonObject user : users)
        {
            JsonDocument u;
            u["username"] = user["username"];
            u["admin"] = user["admin"];
            u["uid"] = user["uid"];
            u["num_tags"] = user["tags"].as<JsonArray>().size();
            filteredUsers.add(u);
        }

        // Add other settings
        d["wifi"] = loadWiFiSettings();
        d["lockdown"] = loadLockdownSettings();
    }
    else
    {
        // For non-admins, return just their own info under "users"
        JsonArray userArray = d["users"].to<JsonArray>();
        JsonDocument u;
        u["username"] = currentUser["username"];
        u["admin"] = currentUser["admin"];
        u["num_tags"] = currentUser["tags"].as<JsonArray>().size();

        userArray.add(u);
    }

    r["data"] = d;

    String s;
    serializeJson(r, s);
    return s;
}

void handleWebsocketMessage(AsyncWebSocketClient *client, char *m)
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

        if (doc["type"] == "auth")
        {
            JsonDocument user;
            getUserByPassword(doc["password"], user);

            String username = user["username"] | "";
            if (username != doc["username"])
            {
                ws.text(client->id(), "{\"type\":\"unauthorized\"}");
                client->close();
                return;
            }
            else
            {
                userClients[user["username"].as<String>()] = client;
                clientIdToUsername[client->id()] = user["username"].as<String>();
            }

            return;
        }

        // check authentication status
        String username = clientIdToUsername[client->id()];
        if (username.length() == 0)
        {
            ws.text(client->id(), "{\"type\":\"unauthorized\"}");
            return;
        }
        else
        {
            String s = m;
            Serial.printf("User %s : %s\n", username, s.c_str());
        }

        if (doc["type"] == "get_status")
        {
            ws.text(client->id(), makeStatusUpdateResponse());
        }
        else if (doc["type"] == "get_settings")
        {
            JsonDocument user;
            if (getUserByUsername(username, user))
            {
                ws.text(client->id(), makeSettingsResponse(user));
            }
            else
            {
                Serial.println("\nUser not Found!");
            }
        }
        else if (doc["type"] == "command")
        {
            if (doc["action_type:"] == "unlock")
            {
                userId = username;
                unlockDoor();
            }
            else if (doc["action_type:"] == "lock")
            {
                lockDoor();
            }
        }
        else if (doc["type"] = "save_settings")
        {
            unloadWiFiSettings(doc["data"]["wifi"]);
            unloadLockdownSettings(doc["data"]["lockdown"]);

            JsonArray users = doc["data"]["users"].as<JsonArray>();
            JsonDocument orgUsersDoc = loadUsers();
            JsonArray orgUsers = orgUsersDoc.as<JsonArray>();

            Serial.println("users:");
            serializeJsonPretty(users, Serial);
            Serial.println("\norgUser:");
            serializeJsonPretty(orgUsers, Serial);
            Serial.println();

            for (size_t i = 0; i < users.size(); i++)
            {
                JsonObject newUser = users[i];
                if (newUser["new_user"].as<bool>())
                {
                    Serial.printf("Creating new user with %s, %s, %d.\n", newUser["username"], newUser["password"], newUser["admin"]);
                    // addUser(newUser["username"], newUser["pin"], newUser["admin"]);

                    JsonObject nu = orgUsers.add<JsonObject>();
                    nu["username"] = newUser["username"];
                    nu["password"] = newUser["pin"];
                    nu["uid"] = generateUID();
                    nu["admin"] = newUser["admin"];
                    JsonArray a;
                    nu["tags"] = a;
                }
                else if (newUser["delete"].as<bool>())
                {
                    Serial.printf("Deleting user %s.\n", newUser["username"]);
                    // removeUser(newUser["username"]);
                    for (size_t i = 0; i < orgUsers.size(); i++)
                    {
                        JsonObject user = orgUsers[i];
                        if (user["username"] == newUser["username"])
                        {
                            orgUsers.remove(i);
                            break;
                        }
                    }
                }
                else
                {
                    for (JsonObject oldUser : orgUsers)
                    {
                        if (strcmp(oldUser["uid"], newUser["uid"]) == 0)
                        {
                            Serial.printf("Merging if necessary!.\n");
                            oldUser["username"] = newUser["username"].as<String>();
                            oldUser["admin"] = newUser["admin"].as<bool>();

                            if (!newUser["pin"].isNull())
                                oldUser["password"] = newUser["pin"].as<String>();

                            break;
                        }
                    }
                }
            }
            saveUsers(orgUsersDoc);
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

        bool isFirmware = true;

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
        size_t written = Update.write(data, len);
        if (written != len)
        {
            Serial.printf("Write failed. Wrote %u of %u\n", written, len);
            Update.printError(Serial);
        }
        if (Update.size() > 0)
        {
            Serial.printf("Update Progress: %u%%\n", (index + len) * 100 / Update.size());
        }
        else
        {
            Serial.println("Update Progress: Unknown (size not set)");
        }
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
            request->send(500, "text/plain", "Update failed");
            return;
        }
    }
}
