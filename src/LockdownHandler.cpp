#include "LockdownHandler.h"
#include "LCDDisplay.h"

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
    unsigned long remainingTime = LOCKDOWN_DURATION - (millis() - lockdownStartTime);
    lcdPrintImportant("Locked", String(remainingTime / 1000) + "s remaining");
}

bool loopLockdownWithLockStatus()
{
    if (isLockdown)
    {
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
            lcdPrint("System Locked", "Try later");
        }
        return true;
    }
    return false;
}