#include "RFIDHandler.h"
#include "ConfigStorage.h"
#include "UnlockLogic.h"
#include "LCDDisplay.h"
#include "KeypadHandler.h"
#include "BuzzerHandler.h"

#include <MFRC522v2.h>
#include <MFRC522DriverSPI.h>
#include <MFRC522DriverPinSimple.h>
#include <MFRC522Debug.h>
#include <ArduinoJson.h>

// RFID definitions
#define RFID_MOSI 23
#define RFID_MISO 19
#define RFID_SCK 18
#define RFID_SDA 5
#define RFID_RST 21

#define RFID_RESET_TIME 120000

#define MRESET_PIN 35 // Pin for manual reset of RFID

bool lastResetState = false; // Variable to store the last state of the reset pin
long lastResetTime = 0;

void removeTagFromUser(String tag);
bool addTagToUser(String tag);
bool isTagRegistered(String tag);

MFRC522DriverPinSimple ss_pin(RFID_SDA);
SPIClass &spiClass = SPI;
const SPISettings spiSettings = SPISettings(SPI_CLOCK_DIV4, MSBFIRST, SPI_MODE0);
MFRC522DriverSPI driver{ss_pin, spiClass, spiSettings};
MFRC522 rfid(driver);

void setupRFID()
{
    rfid.PCD_Init();
    pinMode(MRESET_PIN, INPUT_PULLDOWN);
    Serial.println("NFC Reader Initialized");
}

String byteArrayToHexString(const byte *data, size_t length)
{
    String hexString = "";
    char buf[3]; // Buffer for two hex digits plus the null terminator

    for (size_t i = 0; i < length; i++)
    {
        // Convert each byte to a two-character hexadecimal string.
        sprintf(buf, "%02X", data[i]);
        hexString += buf;
    }

    return hexString;
}

void loopRFID()
{

    if (millis() - lastResetTime > RFID_RESET_TIME)
    {
        lastResetTime = millis();
        rfid.PCD_Init();
    }

    // Check if a new card is present and read its UID
    if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial())
    {
        return;
    }

    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();

    String uid = byteArrayToHexString(rfid.uid.uidByte, rfid.uid.size);

    Serial.print("UID: ");
    Serial.println(uid);

    if (keypadMode == NFC_SCAN_TO_DELETE || keypadMode == NFC_SCAN_TO_REGISTER)
    {
        lcdPrint("");
        if (keypadMode == NFC_SCAN_TO_DELETE)
        {
            if (isTagRegistered(uid))
            {
                // unregister card logic
                removeTagFromUser(uid);
                lcdPrintTemporary("Card UnRegistered");
                Serial.printf("Card Unregistered: %s\n", uid);
            }
            else
            {
                // Invalid card(card not registered ever)
                Serial.printf("new not found: %s\n", uid);
                lcdPrintTemporary("Card not ", "Registered");
            }
        }
        else
        {
            if (!isTagRegistered(uid))
            {
                // register card logic
                addTagToUser(uid);
                Serial.printf("Registered new card: %s\n", uid);
                lcdPrintTemporary("New Card ", "Registered");
            }
            else
            {
                // Card already registered.
                Serial.printf("Card already registered: %s\n", uid);
                lcdPrintTemporary("Already Registered");
            }
        }

        keypadMode = NORMAL;
        Serial.printf("keypadMode: %s", keypadModeToString());
        return;
    }

    // valid logic
    if (isTagRegistered(uid))
    {
        Serial.println("Valid Tag Detected");
        clearPassword();
        unlockDoor();
        buzzSuccess();
    }
    else
    {
        Serial.println("Invalid Tag Detected");
        lcdPrintTemporary("Invalid Tag", "Access Denied");
        buzzError();
    }
}

bool isTagRegistered(String tag)
{
    JsonDocument doc = loadUsers();
    JsonArray users = doc.as<JsonArray>();

    for (JsonObject user : users)
    {
        JsonArray tags = user["tags"];
        for (String t : tags)
        {
            if (t == tag)
            {
                userId = user["username"].as<String>();
                return true;
            }
        }
    }
    return false;
}

bool addTagToUser(String tag)
{
    JsonDocument doc = loadUsers();
    JsonArray users = doc.as<JsonArray>();

    // check if tag is not registered
    for (JsonObject user : users)
    {
        JsonArray tags = user["tags"];
        for(auto t: tags){
            if(t.as<String>() == tag){
                return false;
            }
        }
        break;
    }

    for (JsonObject user : users)
    {
        if (user["username"] == userId)
        {
            if (!user.containsKey("tags") || !user["tags"].is<JsonArray>())
            {
                user.createNestedArray("tags");
            }
            JsonArray tags = user["tags"];
            tags.add(tag);
            break;
        }
    }

    saveUsers(doc);
    return true;
}

void removeTagFromUser(String tag)
{
    JsonDocument doc = loadUsers();
    JsonArray users = doc.as<JsonArray>();

    for (JsonObject user : users)
    {
        if (user["username"] == userId)
        {
            JsonArray tags = user["tags"];
            for (size_t i = 0; i < tags.size(); i++)
            {
                if (tags[i] == tag)
                {
                    tags.remove(i);
                    break;
                }
            }
            break;
        }
    }

    saveUsers(doc);
}