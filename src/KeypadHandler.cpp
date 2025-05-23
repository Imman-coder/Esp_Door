#include "KeypadHandler.h"
#include "LCDDisplay.h"
#include "UnlockLogic.h"
#include "ConfigStorage.h"
#include "BuzzerHandler.h"
#include "LockdownHandler.h"

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

String enteredName = "";
String selectedUser = "";

bool isSelectedUserFirst = true;
bool isSelectedUserLast = false;
int selectedUserIndex = -1;

void keypadEventHandler(KeypadEvent key);
bool changeUserPassword(String newPassword);
bool verifyPassword(String password);
String maskPassword(String password);
String reverseString(String str);
void makeNameWithKey(char key);
bool addUser(String username, String password, bool admin = false);
bool removeUser(String usernameToRemove);
void deleteLastChar();
void clearName();
void setUserAdmin(bool isAdmin);
void getPreviousUser();
void getNextUser();
bool isUserAdmin();
JsonDocument loadUsersFilterSelf();

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

void handleAcceptKeypress()
{
    bool success;
    switch (keypadMode)
    {
    case NFC_SCAN_TO_DELETE:
        break;
    case NFC_SCAN_TO_REGISTER:
        break;

    case PASSWORD_CHANGE:
        success = changeUserPassword(entered);
        keypadMode = NORMAL;
        entered = "";
        lcdPrint("");
        if (success)
            lcdPrintTemporary(MSG_SUCCESS_PASSWORD_CHANGE);
        else
            lcdPrintTemporary(MSG_FAIL_PASSWORD_CHANGE);
        break;

    case NFC_OPTIONS:
        lcdPrintImportant(MSG_SCAN_NEW_NFC);
        keypadMode = NFC_SCAN_TO_REGISTER;
        break;

    case FIRST_ADD_USERNAME:
        userId = enteredName;
        clearName();
        keypadMode = FIRST_ADD_PASSWORD;
        lcdPrintImportant(MSG_ENTER_NEW_PASSWORD);
        break;

    case FIRST_ADD_PASSWORD:
        success = addUser(userId, entered, true);
        Serial.printf("New User: %s, %s, %s\n", userId, entered, success ? "Success" : "Failed");
        setUserAdmin(true);
        keypadMode = NORMAL;
        entered = "";
        lcdPrint("");
        if (success)
            lcdPrintTemporary(MSG_SUCCESS_USER_CREATE);
        else
            lcdPrintTemporary(MSG_FAIL_USER_CREATE);
        break;

    case USER_OPTIONS:
        keypadMode = USER_NEW_USERNAME;
        lcdPrintImportant(MSG_ENTER_USERNAME);
        break;

    case USER_DELETE:
        keypadMode = NORMAL;
        success = removeUser(selectedUser);
        lcdPrint("");
        if (success)
            lcdPrintTemporary(MSG_SUCCESS_USER_DELETE);
        else
            lcdPrintTemporary(MSG_FAIL_USER_DELETE);
        break;

    case USER_NEW_USERNAME:
        userId = enteredName;
        clearName();
        keypadMode = USER_NEW_PASSWORD;
        lcdPrintImportant(MSG_ENTER_NEW_PASSWORD);
        break;

    case USER_NEW_PASSWORD:
        success = addUser(userId, entered);
        Serial.printf("New User: %s, %s, %s\n", userId, entered, success ? "Success" : "Failed");
        keypadMode = NORMAL;
        entered = "";
        lcdPrint("");
        if (success)
            lcdPrintTemporary(MSG_SUCCESS_USER_CREATE);
        else
            lcdPrintTemporary(MSG_FAIL_USER_CREATE);
        break;

    case USER_ADMIN_OPTION:
        setUserAdmin(true);
        keypadMode = NORMAL;
        lcdPrint("");
        lcdPrintHome();
        break;

    case NORMAL:
        // Check if password is correct
        if (verifyPassword(entered))
        {
            Serial.println("Password correct");
            unlockDoor();
            buzzSuccess();
        }
        else
        {
            Serial.println("Password incorrect");
            lcdPrintTemporary(MSG_INVALID_PASSWORD);
            failAttemptMade();
            buzzError();
        }
        entered = "";
        break;
    }
}

void handleRejectKeypress()
{
    buzzShort();
    lastEntered = entered;
    int len;

    switch (keypadMode)
    {
    case PASSWORD_CHANGE:
        if (entered.length() == 0)
        {
            keypadMode = NORMAL;
            lcdPrint("");
            lcdPrintTemporary(MSG_CLEAR, MSG_CLEAR, 1);
        }
        else
        {
            entered = entered.substring(0, entered.length() - 1);
        }
        break;

    case NFC_OPTIONS:

        lcdPrintImportant(MSG_SCAN_OLD_NFC_TO_DELETE); // sus
        keypadMode = NFC_SCAN_TO_DELETE;
        break;

    case NFC_SCAN_TO_REGISTER:

        keypadMode = NORMAL;
        lcdPrintHome();
        break;

    case NFC_SCAN_TO_DELETE:
        keypadMode = NORMAL;
        lcdPrintHome();
        break;

    case FIRST_ADD_USERNAME:
        len = enteredName.length();
        deleteLastChar();
        lcdPrintImportant(MSG_ENTERING_USERNAME(enteredName));
        break;

    case FIRST_ADD_PASSWORD:
        if (entered.length() == 0)
        {
            keypadMode = FIRST_ADD_USERNAME;
            lcdPrintImportant(MSG_ENTERING_USERNAME(enteredName));
        }
        else
        {
            entered = entered.substring(0, entered.length() - 1);
            lcdPrintImportant(MSG_ENTERING_PASSWORD(entered));
        }
        break;

    case USER_OPTIONS:
        keypadMode = USER_DELETE;
        selectedUserIndex = -1;
        if (loadUsersFilterSelf().size() == 0)
        {
            keypadMode = NORMAL;
            lcdPrint("No Users To","Delete.");
            lcdPrintTemporary("No Users To","Delete.");
            return;
        }
        getNextUser();
        lcdPrintImportant(MSG_SELECT_USER_FIRST(userId));
        break;

    case USER_DELETE:
        keypadMode = NORMAL;
        lcdPrint("");
        lcdPrintHome();
        break;

    case USER_NEW_USERNAME:
        len = enteredName.length();
        if (len == 0)
        {
            entered = "";
            clearName();
            keypadMode = NORMAL;
            lcdPrintHome();
            return;
        }
        deleteLastChar();
        lcdPrintImportant(MSG_ENTERING_USERNAME(enteredName));
        break;

    case USER_NEW_PASSWORD:
        if (entered.length() == 0)
        {
            keypadMode = USER_NEW_USERNAME;
            lcdPrintImportant(MSG_ENTERING_USERNAME(enteredName));
            lcdPrint("");
        }
        else
        {
            entered = entered.substring(0, entered.length() - 1);
            lcdPrintImportant(MSG_ENTERING_PASSWORD(entered));
        }
        break;

    case USER_ADMIN_OPTION:
        keypadMode = NORMAL;
        lcdPrint("");
        lcdPrintHome();
        break;

    case NORMAL:
        if (!isDoorLocked && entered.length() > 0)
        {
            lockDoor();
        }
        else
        {
            lcdPrintHome();
        }
        entered = "";
        break;
    }
}

void handleNumKeypress(char key)
{
    if (keypadMode == USER_NEW_USERNAME || keypadMode == FIRST_ADD_USERNAME)
    {
        makeNameWithKey(key);
        lcdPrintImportant(MSG_ENTERING_USERNAME(enteredName));
        return;
    }

    entered += key;
    if (keypadMode == PASSWORD_CHANGE || keypadMode == USER_NEW_PASSWORD)
    {
        lcdPrintImportant(MSG_ENTERING_NEW_PASSWORD(entered));
    }
    else
    {
        lcdPrint("Enter Pin:", maskPassword(entered));
    }
    buzzShort();
    Serial.println(entered);
}

void handleRejectKeyHold()
{
    if (verifyPassword(reverseString(lastEntered)))
    {
        nvs_reset();
        Serial.println("Config Cleared");
    }
    ESP.restart();
    Serial.println("Restarting...");
}

void handleAKeyHold()
{
    if (isDoorOpened && !isDoorLocked)
    {
        lcdPrintImportant("Enter new", "Password");
        keypadMode = PASSWORD_CHANGE;
        entered = "";
    }
}

void handleBKeyHold()
{
    if (isDoorOpened && !isDoorLocked)
    {
        lcdPrintImportant("Card Options:", "(*)Remove (#)Add");
        keypadMode = NFC_OPTIONS;
    }
}

void handleCKeyHold()
{
    // probably check if user is admin
    if (true && isUserAdmin()
        // && isDoorOpened && !isDoorLocked
    )
    {
        keypadMode = USER_OPTIONS;
        lcdPrintImportant(MSG_SELECT_USER_ADD_DELETE);
    }
}

void handleDKeyHold()
{
    toggleBacklight();
    // if (true
    //     // && isDoorOpened && !isDoorLocked
    // )
    // {
    //     keypadMode = USER_DELETE;
    //     String opts = "";

    //     lcdPrintImportant("Select a User:  ", "(A)Next   (#)Del");
    // }
}

void handleAKeypress()
{
    if (keypadMode == USER_DELETE && !isSelectedUserFirst)
    {
        getPreviousUser();
        if (isSelectedUserFirst)
        {
            lcdPrintImportant(MSG_SELECT_USER_FIRST(selectedUser));
        }
        else
        {
            lcdPrintImportant(MSG_SELECT_USER(selectedUser));
        }
    }
}

void handleBKeypress()
{
    if (keypadMode == USER_DELETE && !isSelectedUserLast)
    {
        getNextUser();
        if (isSelectedUserLast)
        {
            lcdPrintImportant(MSG_SELECT_USER_FIRST(selectedUser));
        }
        else
        {
            lcdPrintImportant(MSG_SELECT_USER(selectedUser));
        }
    }
}

void keypadEventHandler(KeypadEvent key)
{
    switch (keypad.getState())
    {
    case PRESSED:

        switch (key)
        {
        case '#':
            handleAcceptKeypress();
            break;
        case '*':
            handleRejectKeypress();
            break;
        case 'A':
            handleAKeypress();
            break;
        case 'B':
            handleBKeypress();
            break;
        case 'C':
            break;
        case 'D':
            toggleBacklight();
            break;

        default:
            handleNumKeypress(key);
            break;
        }
        break;

    case RELEASED:
        break;

    case HOLD:

        switch (key)
        {
        case '*':
            handleRejectKeyHold();
            break;
        case '#':
            nvs_reset();
            ESP.restart();
            break;
        case 'A':
            handleAKeyHold();
            break;
        case 'B':
            handleBKeyHold();
            break;
        case 'C':
            handleCKeyHold();
            break;
        case 'D':
            handleDKeyHold();
            break;

        default:
            break;
        }
        break;
    }
}

void CheckFirstRun()
{
    JsonDocument userDoc = loadUsers();
    JsonArray users = userDoc.as<JsonArray>();
    if (users.size() != 0)
        return;
    keypadMode = FIRST_ADD_USERNAME;
    lcdPrintImportant("New User Name:");
}

void clearPassword()
{
    entered = "";
}

bool getUserByPassword(String password, JsonDocument &user_ob)
{
    JsonDocument usersDoc = loadUsers();
    JsonArray users = usersDoc.as<JsonArray>();
    for (JsonObject user : users)
    {
        if (user["password"] == password)
        {
            user_ob.set(user);
            serializeJsonPretty(user, Serial);
            Serial.println("");
            serializeJsonPretty(user_ob, Serial);
            Serial.println("------");
            return true;
        }
    }
    return false;
}

bool verifyPassword(String password)
{
    JsonDocument user;
    bool found = getUserByPassword(password, user);
    Serial.println("\nverify:");
    serializeJsonPretty(user, Serial);
    Serial.println("");
    if (found)
    {
        userId = user["username"].as<String>();
        return true;
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
String generateUID() {
    String uid = "user-";
    const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    for (int i = 0; i < 8; i++) {
        uid += charset[random(0, sizeof(charset) - 1)];
    }
    return uid;
}


bool addUser(String username, String password, bool admin)
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
    newUser["uid"] = generateUID();
    newUser["admin"] = admin;
    JsonArray a;
    newUser["tags"] = a;
    // newUser["tags"] = JsonArray();

    saveUsers(doc);
    return true;
}

bool removeUser(String usernameToRemove)
{
    JsonDocument doc = loadUsers();
    JsonArray users = doc.as<JsonArray>();

    bool found = false;

    for (size_t i = 0; i < users.size(); i++)
    {
        JsonObject user = users[i];
        if (user["username"] == usernameToRemove)
        {
            users.remove(i);
            found = true;
            break;
        }
    }

    if (found)
    {
        saveUsers(doc);
        Serial.printf("User '%s' removed successfully\n", usernameToRemove.c_str());
        return true;
    }
    else
    {
        Serial.printf("User '%s' not found\n", usernameToRemove.c_str());
        return false;
    }
}

void setUserAdmin(bool isAdmin)

{
    JsonDocument doc = loadUsers();
    JsonArray users = doc.as<JsonArray>();

    for (JsonObject user : users)
    {
        if (user["username"] == userId)
        {
            user["admin"] = isAdmin;
        }
    }

    saveUsers(doc);
}

JsonDocument loadUsersFilterSelf()
{
    JsonDocument doc = loadUsers();
    JsonDocument doc2;

    JsonArray users = doc.as<JsonArray>();
    JsonArray users2 = doc2.to<JsonArray>();

    for (JsonObject user : users)
    {
        if (user["username"] != userId)
        {
            users2.add(user);
        }
    }

    serializeJsonPretty(doc2, Serial);

    return doc2;
}

void getNextUser()
{
    JsonDocument doc = loadUsersFilterSelf();
    JsonArray users = doc.as<JsonArray>();
    selectedUserIndex++;
    selectedUser = users[selectedUserIndex]["username"].as<String>();
    isSelectedUserFirst = selectedUserIndex <= 0;
    isSelectedUserLast = selectedUserIndex >= users.size() - 1;
}

void getPreviousUser()
{
    JsonDocument doc = loadUsersFilterSelf();
    JsonArray users = doc.as<JsonArray>();
    selectedUserIndex--;
    selectedUser = users[selectedUserIndex]["username"].as<String>();
    isSelectedUserFirst = selectedUserIndex <= 1;
    isSelectedUserLast = selectedUserIndex >= users.size();
}

bool isUserAdmin()
{
    JsonDocument doc = loadUsers();
    JsonArray users = doc.as<JsonArray>();

    for (JsonObject user : users)
    {
        if (user["username"] == userId)
        {
            if (user["admin"].is<bool>() && user["admin"])
            {
                return true;
            }
        }
    }
    return false;
}

/*----------------------------------T9 Name Entry----------------------------------------*/

String inputString = "";
char lastKey = '\0';
int cycleIndex = 0;
unsigned long lastPressTime = 0;
const unsigned long timeoutDelay = 1000; // 1 second

void printLiveString(char currentChar = '\0')
{
    enteredName = inputString;
    if (currentChar != '\0')
    {
        enteredName += currentChar;
    }
}

struct KeyMap
{
    char key;
    const char *letters;
};

// Define the mapping (adjust as needed)
KeyMap keyMappings[] = {
    {'2', "ABC2"},
    {'3', "DEF3"},
    {'4', "GHI4"},
    {'5', "JKL5"},
    {'6', "MNO6"},
    {'7', "PQRS7"},
    {'8', "TUV8"},
    {'9', "WXYZ9"},
    {'0', " 0"}};

const int numMappings = sizeof(keyMappings) / sizeof(KeyMap);

void clearName()
{
    deleteLastChar();
    inputString = "";
}

void deleteLastChar()
{
    if (lastKey != '\0')
    {
        lastKey = '\0';
    }
    else
    {
        if (inputString.length() > 0)
        {
            inputString.remove(inputString.length() - 1);
        }
    }

    printLiveString();
}

void makeNameWithKey(char key)
{
    unsigned long currentTime = millis();

    if (key != NO_KEY)
    {
        if (key == lastKey && (currentTime - lastPressTime < timeoutDelay))
        {
            // cycle through letters
            for (int i = 0; i < numMappings; i++)
            {
                if (keyMappings[i].key == key)
                {
                    cycleIndex = (cycleIndex + 1) % strlen(keyMappings[i].letters);
                    printLiveString(keyMappings[i].letters[cycleIndex]);
                    break;
                }
            }
            lastPressTime = currentTime;
        }
        else
        {
            // confirm the previous character (if any)
            if (lastKey != '\0')
            {
                for (int i = 0; i < numMappings; i++)
                {
                    if (keyMappings[i].key == lastKey)
                    {
                        inputString += keyMappings[i].letters[cycleIndex];
                        break;
                    }
                }
            }

            // prepare for new key
            lastKey = key;
            lastPressTime = currentTime;
            cycleIndex = 0;

            // check if current key has mapping
            bool hasMapping = false;
            for (int i = 0; i < numMappings; i++)
            {
                if (keyMappings[i].key == key)
                {
                    printLiveString(keyMappings[i].letters[cycleIndex]);
                    hasMapping = true;
                    break;
                }
            }

            // if not mapped (like 1, 0, *, #), accept immediately
            if (!hasMapping)
            {
                inputString += key;
                lastKey = '\0'; // no cycling
                printLiveString();
            }
        }
    }

    // confirm character if timeout exceeded
    if (lastKey != '\0' && (currentTime - lastPressTime > timeoutDelay))
    {
        for (int i = 0; i < numMappings; i++)
        {
            if (keyMappings[i].key == lastKey)
            {
                inputString += keyMappings[i].letters[cycleIndex];
                break;
            }
        }
        lastKey = '\0';
        printLiveString(); // final update after timeout
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