#pragma once

#include <Arduino.h>

void failAttemptMade();

bool loopLockdownWithLockStatus();

void setupLockdown();
void resetFailedAttempts();