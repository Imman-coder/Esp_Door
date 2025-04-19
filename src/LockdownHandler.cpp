#include "LockdownHandler.h"
#include "LCDDisplay.h"
#include "ConfigStorage.h"

int MAX_FAILED_ATTEMPTS = 5;
unsigned long LOCKDOWN_DURATION = 300000;
bool enableLockdown = true;

int failedAttempts = 0;
bool isLockdown = false;
unsigned long lockdownStartTime = 0;

unsigned long lastLockdownTimerDisplayed = 0;
bool shownFirstMsg = false;

void setupLockdown()
{
    LOCKDOWN_DURATION = globalConfig.getUInt(CONFIG_LOCKDOWN_DURATION, 5) * 60 * 1000;
    enableLockdown = globalConfig.getBool(CONFIG_LOCKDOWN_ENABLE, true);
    MAX_FAILED_ATTEMPTS = globalConfig.getUInt(CONFIG_LOCKDOWN_MAX_ATTEMPTS, 5);
}

void resetFailedAttempts()
{
    failedAttempts = 0;
}

void failAttemptMade()
{
    failedAttempts++;
    if (failedAttempts >= MAX_FAILED_ATTEMPTS)
    {
        isLockdown = true;
        lockdownStartTime = millis();
        lcdPrintImportant("Too many attempts", "System locked");
    }
}

void displayLockdownCountdown()
{
    if (millis() - lastLockdownTimerDisplayed < 1000)
        return;
    lastLockdownTimerDisplayed = millis();
    unsigned long remainingTime = LOCKDOWN_DURATION - (millis() - lockdownStartTime);
    lcdPrintImportant("Locked", String(remainingTime / 1000) + "s remaining");
}

bool loopLockdownWithLockStatus()
{
    if (isLockdown && enableLockdown)
    {
        if (!shownFirstMsg)
        {
            lcdPrint("System Locked", "Try later");
            shownFirstMsg = true;
        }
        // Check if the lockdown duration has elapsed
        if (millis() - lockdownStartTime >= LOCKDOWN_DURATION)
        {
            isLockdown = false;
            failedAttempts = 0;
            lcdPrint("Lockdown ended", "Try again");
            return false;
        }
        else if (millis() - lockdownStartTime >= 1000)
        {
            displayLockdownCountdown();
        }
        return true;
    }
    if (shownFirstMsg)
        shownFirstMsg = false;
    return false;
}