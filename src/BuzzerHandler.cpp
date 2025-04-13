#include "BuzzerHandler.h"

// Buzzer definition
#define BUZZER_PIN 15

bool buzzerActive = false;
unsigned long buzzerStartTime = 0;
unsigned long buzzerDuration = 0;

bool isActiveLow = false;

void BUZZER_WRITE(int pin, int state)
{
     digitalWrite(pin, state);
}

void setupBuzzer()
{
    
    pinMode(BUZZER_PIN, OUTPUT);
    BUZZER_WRITE(BUZZER_PIN, LOW);
    Serial.println("Buzzer initialized");
}

void buzzStart(unsigned long duration)
{
    pinMode(BUZZER_PIN, OUTPUT);
    BUZZER_WRITE(BUZZER_PIN, HIGH);
    buzzerActive = true;
    buzzerStartTime = millis();
    buzzerDuration = duration;
}

void buzzSuccess()
{

    buzzStart(200);
}

void buzzError()
{
    noTone(BUZZER_PIN);
    tone(BUZZER_PIN, 175, 500);
}

void buzzShort()
{
    buzzStart(100);
}

void buzzLong()
{
    buzzStart(800);
}

void loopBuzzer()
{
    if (buzzerActive && millis() - buzzerStartTime >= buzzerDuration)
    {
        BUZZER_WRITE(BUZZER_PIN, LOW);
        buzzerActive = false;
    }
}