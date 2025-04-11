#include "UnlockLogic.h"
#include "LCDDisplay.h"
#include "RFIDHandler.h"

#define SOLENOID_PIN 16
#define DOOR_STATUS_PIN 17

#define DOOR_RELOCK_DELAY 5000

long lastUnlockTime = 0;

StateChangeCallback doorStatusCallback[5];
StateChangeCallback lockStatusCallback[5];

bool isDoorOpened = true;
bool isDoorLocked = true;

bool lastTickReset = true;

String userId = "";

void setUserId(String s){
    userId = s;
}

String getUserId(){
    return userId;
}

void setupUnlockLogic()
{
    pinMode(SOLENOID_PIN, OUTPUT);
    pinMode(DOOR_STATUS_PIN, INPUT_PULLUP); // Use internal pull-up resistor (High when door is open)
    digitalWrite(SOLENOID_PIN, LOW);        // Ensure solenoid is off at startup
}

void registerDoorStatusCallback(StateChangeCallback callback)
{
    for (int i = 0; i < 5; i++)
    {
        if (doorStatusCallback[i] == nullptr)
        {
            doorStatusCallback[i] = callback;
            break;
        }
    }
}
void registerLockStatusCallback(StateChangeCallback callback)
{
    for (int i = 0; i < 5; i++)
    {
        if (lockStatusCallback[i] == nullptr)
        {
            lockStatusCallback[i] = callback;
            break;
        }
    }
}

void lockDoor()
{
    isDoorLocked = true;
    Serial.println("Lock is locked");
    lcdPrintTemporary("Door Locked", "Door is closed");

    for (int i = 0; i < 5; i++)
    {
        if (lockStatusCallback[i] != nullptr)
        {
            lockStatusCallback[i](isDoorLocked);
        }
    }
}

void unlockDoor()
{
    lastUnlockTime = millis();
    isDoorLocked = false;
    Serial.println("Lock is unlocked");
    lcdPrint("Welcome, ",userId);
    lastTickReset = false;
    for (int i = 0; i < 5; i++)
    {
        if (lockStatusCallback[i] != nullptr)
        {
            lockStatusCallback[i](isDoorLocked);
        }
    }
}

void loopUnlock()
{
    if (!isDoorLocked && (millis() - lastUnlockTime) > DOOR_RELOCK_DELAY)
    {
        lockDoor();
    }

    // synchronize the solenoid with the door status
    if (isDoorLocked)
    {
        digitalWrite(SOLENOID_PIN, LOW);

        // rfid.PCD_SoftPowerDown();
        // rfid.PCD_SoftPowerUp();
        if( !lastTickReset ){
            rfid.PCD_Init();
            lastTickReset = true;
        }
    }
    else
    {
        digitalWrite(SOLENOID_PIN, HIGH);
    }

    // Check if the door state changed
    bool doorOpenedNow = digitalRead(DOOR_STATUS_PIN);
    if (doorOpenedNow != isDoorOpened)
    {
        isDoorOpened = doorOpenedNow;
        if (isDoorOpened) // Door is open
        {
            Serial.println("Door is open");
        }
        else
        {
            Serial.println("Door is closed");
        }

        // Notify to all listeners.
        for (int i = 0; i < 5; i++)
        {
            if (doorStatusCallback[i] != nullptr)
            {
                doorStatusCallback[i](isDoorOpened);
            }
        }
    }
}
