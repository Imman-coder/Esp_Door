#pragma once

#include <Arduino.h>
#include <Keypad.h>

enum SetupMode
{
    NORMAL,

    PASSWORD_CHANGE, // When user changes it's password

    NFC_OPTIONS, // Options for user to select whether to register or unregister NFC tags 
    NFC_SCAN_TO_REGISTER, // When user wants to register a NFC Card
    NFC_SCAN_TO_DELETE, // When user user wants to unregister NFC Card

    FIRST_ADD_USERNAME, // When Setting up for first user's username
    FIRST_ADD_PASSWORD, // When Setting up for first user's password

    USER_OPTIONS, // Admin option to delete and register new user
    USER_DELETE, // Admin option to delte a user
    USER_NEW_USERNAME, // Admin option to enter new user's username
    USER_NEW_PASSWORD, // Admin option to enter new user's password
    USER_ADMIN_OPTION, // Admin option to allow other user to create and delete user.

};

extern SetupMode keypadMode;

String keypadModeToString();
void loopKeypad();
void clearPassword();
void setupKeypad();
void CheckFirstRun();