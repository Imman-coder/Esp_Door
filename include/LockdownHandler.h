#include <Arduino.h>

int MAX_FAILED_ATTEMPTS = 5;
unsigned long LOCKDOWN_DURATION = 300000 ;

int failedAttempts = 0;
bool isLockdown = false;
unsigned long lockdownStartTime = 0;

void failAttemptMade();

bool loopLockdownWithLockStatus();