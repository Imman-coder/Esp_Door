#pragma once

#include <Arduino.h>

void setupBuzzer();
void buzzSuccess();
void buzzError();
void buzzShort();
void buzzLong();

void playNote(int note, int duration);

void loopBuzzer();