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

String *splitStringByComma(const String &input, int &count);
void registerNewTag(String);
void unregisterTag(String);
bool isValidTag2(String);

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

    if(millis() - lastResetTime > RFID_RESET_TIME){
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
        if (keypadMode == NFC_SCAN_TO_DELETE)
        {
            if (isValidTag2(uid))
            {
                // unregister card logic
                unregisterTag(uid);
                lcdPrintTemporary("Card UnRegistered");
                Serial.printf("Card Unregistered: %s\n",uid);
            }
            else
            {
                // Invalid card(card not registered ever)
                Serial.printf("new not found: %s\n",uid);
                lcdPrintTemporary("Card not ","Registered");
            }
        }
        else
        {
            if (!isValidTag2(uid))
            {
                // register card logic
                registerNewTag(uid);
                Serial.printf("Registered new card: %s\n",uid);
                lcdPrintTemporary("New Card ","Registered");
            }
            else
            {
                // Card already registered.
                Serial.printf("Card already registered: %s\n",uid);
                lcdPrintTemporary("Already Registered");
            }
        }
        
        keypadMode = NORMAL;
        Serial.printf("keypadMode: %s", keypadModeToString());
        return;
    }

    // valid logic
    if (isValidTag2(uid))
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

// bool isValidTag(String tag)
// {
//     String saved_tags = userConfig.getString(CONFIG_TAGS);
//     int count = 0;
//     String *tags = splitStringByComma(saved_tags, count);
//     for (int i = 0; i < count; i++)
//     {
//         if (tag == tags[i])
//         {
//             return true;
//         }
//     }
//     return false;
// }

JsonDocument getSavedTagsDoc()
{
    JsonDocument doc;
    String tags = userConfig.getString(CONFIG_TAGS);
    DeserializationError error = deserializeJson(doc, tags);
    if (error)
    {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        return JsonDocument();
    }
    return doc;
}

void unregisterTag(String tag)
{
    JsonDocument doc = getSavedTagsDoc();
    JsonArray saved_tags = doc.as<JsonArray>();
    for (size_t i = 0; i < saved_tags.size(); i++)
    {
        if (saved_tags[i] == tag)
        {
            saved_tags.remove(i);
            break;
        }
    }
    String s;
    serializeJson(saved_tags, s);
    userConfig.putString(CONFIG_TAGS, s);
}

void registerNewTag(String tag)
{
    JsonDocument doc = getSavedTagsDoc();
    JsonArray saved_tags =  doc.as<JsonArray>();
    saved_tags.add(tag);
    String s;
    serializeJson(saved_tags, s);
    userConfig.putString(CONFIG_TAGS, s);
}

bool isValidTag2(String tag)
{
    JsonDocument doc = getSavedTagsDoc();
    JsonArray saved_tags = doc.as<JsonArray>();

    for (String s : saved_tags)
    {
        if (s == tag)
        {
            return true;
        }
    }

    return false;
}

String *splitStringByComma(const String &input, int &count)
{
    // Count the commas to determine the number of tokens.
    count = 0;
    for (unsigned int i = 0; i < input.length(); i++)
    {
        if (input.charAt(i) == ',')
        {
            count++;
        }
    }
    // There is one more token than the number of commas.
    count = count + 1;

    // Dynamically allocate an array of String objects.
    String *tokens = new String[count];

    int index = 0;
    int start = 0;
    int pos = input.indexOf(',', start);

    // Extract tokens separated by commas.
    while (pos != -1)
    {
        tokens[index++] = input.substring(start, pos);
        start = pos + 1;
        pos = input.indexOf(',', start);
    }
    // Add the last token.
    tokens[index] = input.substring(start);

    return tokens;
}