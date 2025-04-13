#pragma once

#include <Arduino.h>

//
#define MSG_CLEAR "                "
#define MSG_HOME "Enter Pin or", "Scan NFC"

#define MSG_SUCCESS_PASSWORD_CHANGE "Password Changed", MSG_CLEAR
#define MSG_FAIL_PASSWORD_CHANGE "Password Change ", "FAILED          "

#define MSG_SUCCESS_USER_DELETE "User Deleted", MSG_CLEAR
#define MSG_FAIL_USER_DELETE "User Delete", "FAILED          "

#define MSG_SCAN_NEW_NFC "Scan new Card.", MSG_CLEAR
#define MSG_SCAN_OLD_NFC_TO_DELETE "Scan Card to", "Unregister.     "
#define MSG_SET_ADMIN_OPTION "Make this Admin?", "(*) NO   (#) YES"

#define MSG_DOOR_UNLOCKED(name) "Hey, " + name + "       ", "Door is Unlocked"
#define MSG_INVALID_PASSWORD "Invalid Password", "Try Again!      "
#define MSG_INVALID_TAG "Invalid Card,   ", "Try Again!      "

#define MSG_ENTERING_PASSWORD(password) "Enter Password", password + ""

#define MSG_ENTER_USERNAME "Enter new Name: ", MSG_CLEAR
#define MSG_ENTERING_USERNAME(name) name, "(*) Del (#) Save"

#define MSG_ENTER_NEW_PASSWORD "Enter new       ", "Password        "
#define MSG_ENTERING_NEW_PASSWORD(password) password + MSG_CLEAR, "(*) Del (#) Save"

#define MSG_SUCCESS_USER_CREATE "User Created!", MSG_CLEAR
#define MSG_FAIL_USER_CREATE "User Create", "Failed!         "

#define MSG_SELECT_USER_ADD_DELETE "User Options", "(*) Del  (#) Add"

#define MSG_SELECT_USER(username) username + MSG_CLEAR, " <- A      B -> "
#define MSG_SELECT_USER_FIRST(username) username + MSG_CLEAR, "           B -> "
#define MSG_SELECT_USER_LAST(username) username + MSG_CLEAR, " <- A           "

void setupLCD();
void lcdPrint(const String &line1, const String &line2 = "");
void lcdPrintImportant(const String &line1, const String &line2 = "");
void lcdClear();
void lcdPrintHome();
void lcdPrintTemporary(const String &line1, const String &line2 = "", unsigned long timeout = 2500);

void setBacklight(bool state);
void toggleBacklight();

void loopLCD();