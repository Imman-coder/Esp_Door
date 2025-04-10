#pragma once

#include <Arduino.h>

using StateChangeCallback = std::function<void (bool)>;

extern bool isDoorOpened;
extern bool isDoorLocked;

void registerDoorStatusCallback(StateChangeCallback callback);
void registerLockStatusCallback(StateChangeCallback callback);

void setupUnlockLogic();
void lockDoor();
void unlockDoor(); 
void loopUnlock();