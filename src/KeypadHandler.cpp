#include "KeypadHandler.h"
#include "LCDDisplay.h"
#include "UnlockLogic.h"
#include "ConfigStorage.h"
#include "BuzzerHandler.h"

// Keypad definations
#define ROWS 4
#define COLS 4

byte rowPins[ROWS] = {13, 12, 14, 27};
byte colPins[COLS] = {26, 25, 33, 32};
char keys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};

String lastEntered = "";
String entered = "";
SetupMode keypadMode = NORMAL;

void keypadEventHandler(KeypadEvent key);
bool verifyPassword(String password);
String maskPassword(String password);
String reverseString(String str);

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

void setupKeypad()
{
    keypad.setHoldTime(1000);
    keypad.setDebounceTime(50);
    keypad.addEventListener(keypadEventHandler);

    Serial.println("Keypad initialized");
}

void loopKeypad()
{
    keypad.getKey();
}

void keypadEventHandler(KeypadEvent key)
{
    switch (keypad.getState())
    {
    case PRESSED:

        if (key == '#')
        {
            if (keypadMode == PASSWORD_RESET)
            {
                userConfig.putString(CONFIG_PIN, entered);
                keypadMode = NORMAL;
                Serial.printf("keypadMode: %s\n", keypadModeToString());
                entered = "";
                lcdPrintTemporary("Password Changed", "");
                return;
            }

            if (keypadMode == NFC_OPTIONS)
            {
                lcdPrint("Scan new Card");
                keypadMode = NFC_SCAN_TO_REGISTER;
                Serial.printf("keypadMode: %s\n", keypadModeToString());
                return;
            }

            if (keypadMode == NFC_SCAN_TO_DELETE || keypadMode == NFC_SCAN_TO_REGISTER)
            {
                keypadMode = NORMAL;
                Serial.printf("keypadMode: %s\n", keypadModeToString());
                lcdPrintTemporary("", "", 1);
                return;
            }

            // verify enterd password
            if (verifyPassword(entered))
            {
                Serial.println("Password correct");
                unlockDoor();
                buzzSuccess();
            }
            else
            {
                Serial.println("Password incorrect");
                lcdPrintTemporary("Wrong Password", "Try again");
                buzzError();
            }
            entered = "";
            return;
        }
        if (key == '*')
        {
            buzzShort();
            lastEntered = entered;

            if (keypadMode == PASSWORD_RESET)
            {
                keypadMode = NORMAL;
                Serial.printf("keypadMode: %s\n", keypadModeToString());
                entered = "";
                lcdPrintTemporary("Password Reset", "Cancelled");
                return;
            }

            if (keypadMode == NFC_OPTIONS)
            {
                lcdPrint("Scan Card to", "Unregister");
                keypadMode = NFC_SCAN_TO_DELETE;
                Serial.printf("del - keypadMode: %s\n", keypadModeToString());
                return;
            }

            if (keypadMode == NFC_SCAN_TO_DELETE || keypadMode == NFC_SCAN_TO_REGISTER)
            {
                keypadMode = NORMAL;
                Serial.printf("keypadMode: %s\n", keypadModeToString());
                lcdPrintTemporary("", "", 1);
                return;
            }

            if (!isDoorLocked && entered.length() > 0)
            {
                lockDoor();
            }
            else
            {
                lcdPrintTemporary("", "", 1);
            }
            entered = "";
            return;
        }
        if (key == 'A')
        {
            return;
        }
        if (key == 'B')
        {
            return;
        }
        if (key == 'C')
        {
            // setBacklight(false);
            toggleBacklight();
            return;
        }
        if (key == 'D')
        {
            // setBacklight(true);
            return;
        }

        entered += key;
        if (keypadMode == PASSWORD_RESET)
        {
            lcdPrint(maskPassword(entered));
        }
        else
        {
            lcdPrint("Enter Pin:", maskPassword(entered));
        }
        buzzShort();
        Serial.println(entered);

        break;

    case RELEASED:
        break;

    case HOLD:
        if (key == '*')
        {
            if (verifyPassword(reverseString(lastEntered)))
            {
                nvs_reset();
                Serial.println("Config Cleared");
            }
            ESP.restart();
            Serial.println("Restarting...");
        }
        else if (key == 'A' && isDoorOpened)
        {
            lcdPrint("Enter new", "Password");
            keypadMode = PASSWORD_RESET;
            Serial.printf("keypadMode: %s\n", keypadModeToString());
            entered = "";
        }
        else if (key == 'B' && isDoorOpened)
        {
            lcdPrint("Card Options:", "(*)Remove (#)Add");
            keypadMode = NFC_OPTIONS;
            Serial.printf("keypadMode: %s\n", keypadModeToString());
        }
        else if (key == 'C')
        {
            if (verifyPassword(reverseString(entered)))
            {
                Serial.printf("Tags: %s",userConfig.getString(CONFIG_TAGS));
            }
        }
        break;
    }
}

void clearPassword()
{
    entered = "";
}

bool verifyPassword(String password)
{
    String storedPassword = userConfig.getString(CONFIG_PIN, "");

    // Compare the entered password with the stored password
    if (password == storedPassword)
    {
        return true;
    }
    else
    {
        return false;
    }
}

String maskPassword(String password)
{
    String masked = "";
    for (int i = 0; i < password.length(); i++)
    {
        masked += "*";
    }
    return masked;
}

String keypadModeToString()
{
    switch (keypadMode)
    {
    case NORMAL:
        return "NORMAL";
    case PASSWORD_RESET:
        return "PASSWORD_RESET";
    case NFC_OPTIONS:
        return "NFC_OPTIONS";
    case NFC_SCAN_TO_DELETE:
        return "NFC_SCAN_TO_DELETE";
    case NFC_SCAN_TO_REGISTER:
        return "NFC_SCAN_TO_REGISTER";
    default:
        return "UNKNOWN";
    }
}

String reverseString(String str)
{
    int len = str.length();
    for (int i = 0; i < len / 2; i++)
    {
        char temp = str[i];
        str[i] = str[len - 1 - i];
        str[len - 1 - i] = temp;
    }
    return str;
}