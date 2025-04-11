#include "KeypadHandler.h"
#include "LCDDisplay.h"
#include "UnlockLogic.h"
#include "ConfigStorage.h"
#include "BuzzerHandler.h"

#include "ArduinoJson.h"

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

const unsigned long multiTapDelay = 1000;
unsigned long lastKeyPressTime = 0;
char currentKey = '\0';
int pressCount = 0;
String enteredName = "";

void keypadEventHandler(KeypadEvent key);
bool changeUserPassword(String newPassword);
bool verifyPassword(String password);
String maskPassword(String password);
String reverseString(String str);
void makeNameWithKey(char key);
bool addUser(String username, String password);

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
            if (keypadMode == PASSWORD_CHANGE)
            {
                changeUserPassword(entered);
                keypadMode = NORMAL;
                entered = "";
                lcdPrintTemporary("Password Changed", "");
                return;
            }

            if (keypadMode == ADD_PASSWORD)
            {
                addUser(userId, entered);
                keypadMode = NORMAL;
                entered = "";
                lcdPrintTemporary("New User", "Registered");
                return;
            }

            if (keypadMode == NFC_OPTIONS)
            {
                lcdPrint("Scan new Card");
                keypadMode = NFC_SCAN_TO_REGISTER;
                return;
            }

            if (keypadMode == NFC_SCAN_TO_DELETE || keypadMode == NFC_SCAN_TO_REGISTER)
            {
                keypadMode = NORMAL;
                lcdPrintTemporary("", "", 1);
                return;
            }

            if (keypadMode == ADD_USERNAME)
            {
                userId = enteredName;
                keypadMode = PASSWORD_CHANGE;
                lcdPrint("Enter new", "Password");
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

            if (keypadMode == PASSWORD_CHANGE)
            {
                keypadMode = NORMAL;
                entered = "";
                lcdPrintTemporary("Password Reset", "Cancelled");
                return;
            }

            if (keypadMode == NFC_OPTIONS)
            {
                lcdPrint("Scan Card to", "Unregister");
                keypadMode = NFC_SCAN_TO_DELETE;
                return;
            }

            if (keypadMode == NFC_SCAN_TO_DELETE || keypadMode == NFC_SCAN_TO_REGISTER)
            {
                keypadMode = NORMAL;
                lcdPrintTemporary("", "", 1);
                return;
            }

            if (keypadMode == ADD_USERNAME)
            {
                enteredName = enteredName.substring(0, enteredName.length() - 1);
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
            toggleBacklight();
            return;
        }
        if (key == 'D')
        {
            return;
        }

        if (keypadMode == ADD_USERNAME)
        {
            makeNameWithKey(key);
            lcdPrint("New User Name:", enteredName);
            return;
        }

        entered += key;
        if (keypadMode == PASSWORD_CHANGE)
        {
            lcdPrint((entered));
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
        else if (key == 'A' && isDoorOpened && !isDoorLocked)
        {
            lcdPrint("Enter new", "Password");
            keypadMode = PASSWORD_CHANGE;
            entered = "";
        }
        else if (key == 'B' && isDoorOpened && !isDoorLocked)
        {
            lcdPrint("Card Options:", "(*)Remove (#)Add");
            keypadMode = NFC_OPTIONS;
        }
        else if (key == 'C' && isDoorOpened && !isDoorLocked)
        {
            if (verifyPassword(reverseString(entered)))
            {
                Serial.printf("Tags: %s", userPrefs.getString(CONFIG_USERS));
            }

            lcdPrint("New User Name:");
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
    JsonDocument usersDoc = loadUsers();
    JsonArray users = usersDoc.as<JsonArray>();
    if (users.size() == 0 && password == "1234")
    {
        userId = "Admin";
        return true;
    }
    for (JsonObject user : users)
    {
        if (user["password"] == password)
        {
            userId = user["username"].as<String>();
            return true;
        }
    }

    return false;
}

/*------------------------------------User Manipulation methods--------------------------------*/

bool changeUserPassword(String newPassword)
{
    JsonDocument doc = loadUsers();
    JsonArray users = doc.as<JsonArray>();

    for (JsonObject user : users)
    {
        if (user["username"] == userId)
        {
            user["password"] = newPassword;
            saveUsers(doc);
            return true;
        }
    }

    return false; // user not found
}

bool addUser(String username, String password)
{
    JsonDocument doc = loadUsers();
    JsonArray users = doc.as<JsonArray>();

    // Check if the user already exists
    for (JsonObject user : users)
    {
        if (user["username"] == username)
        {
            return false;
        }
    }

    JsonObject newUser = users.add<JsonObject>();
    newUser["username"] = username;
    newUser["password"] = password;
    newUser["tags"] = JsonArray();

    saveUsers(doc);
    return true;
}

/*----------------------------------T9 Name Entry----------------------------------------*/

// Function to map a key to its corresponding letters
String getKeyMapping(char key)
{
    switch (key)
    {
    case '2':
        return "ABC2";
    case '3':
        return "DEF3";
    case '4':
        return "GHI4";
    case '5':
        return "JKL5";
    case '6':
        return "MNO6";
    case '7':
        return "PQRS7";
    case '8':
        return "TUV8";
    case '9':
        return "WXYZ9";
    case '0':
        return " 0";
    default:
        return "";
    }
}

void appendCurrentLetter()
{
    String mapping = getKeyMapping(currentKey);
    if (mapping.length() > 0)
    {
        int index = (pressCount - 1) % mapping.length();
        char letter = mapping.charAt(index);
        enteredName += letter;
    }
    else
    {
        enteredName += currentKey;
    }
    Serial.print("Current text: ");
    Serial.println(enteredName);
}

void makeNameWithKey(char key)
{
    if (key != NO_KEY)
    {
        if (key == currentKey)
        {
            pressCount++;
        }
        else
        {
            if (currentKey != '\0')
            {
                appendCurrentLetter();
            }
            currentKey = key;
            pressCount = 1;
        }
        lastKeyPressTime = millis();
    }

    if (currentKey != '\0' && (millis() - lastKeyPressTime > multiTapDelay))
    {
        appendCurrentLetter();
        currentKey = '\0';
        pressCount = 0;
    }
}

/*---------------------UTILITY FUNCTIONS----------------------*/

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
    case PASSWORD_CHANGE:
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