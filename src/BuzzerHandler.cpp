#include "BuzzerHandler.h"








const int NOTE_FREQUENCIES[] = {
    31,   // NOTE_B0
    33,   // NOTE_C1
    35,   // NOTE_CS1
    37,   // NOTE_D1
    39,   // NOTE_DS1
    41,   // NOTE_E1
    44,   // NOTE_F1
    46,   // NOTE_FS1
    49,   // NOTE_G1
    52,   // NOTE_GS1
    55,   // NOTE_A1
    58,   // NOTE_AS1
    62,   // NOTE_B1
    65,   // NOTE_C2
    69,   // NOTE_CS2
    73,   // NOTE_D2
    78,   // NOTE_DS2
    82,   // NOTE_E2
    87,   // NOTE_F2
    93,   // NOTE_FS2
    98,   // NOTE_G2
    104,  // NOTE_GS2
    110,  // NOTE_A2
    117,  // NOTE_AS2
    123,  // NOTE_B2
    131,  // NOTE_C3
    139,  // NOTE_CS3
    147,  // NOTE_D3
    156,  // NOTE_DS3
    165,  // NOTE_E3
    175,  // NOTE_F3
    185,  // NOTE_FS3
    196,  // NOTE_G3
    208,  // NOTE_GS3
    220,  // NOTE_A3
    233,  // NOTE_AS3
    247,  // NOTE_B3
    262,  // NOTE_C4
    277,  // NOTE_CS4
    294,  // NOTE_D4
    311,  // NOTE_DS4
    330,  // NOTE_E4
    349,  // NOTE_F4
    370,  // NOTE_FS4
    392,  // NOTE_G4
    415,  // NOTE_GS4
    440,  // NOTE_A4
    466,  // NOTE_AS4
    494,  // NOTE_B4
    523,  // NOTE_C5
    554,  // NOTE_CS5
    587,  // NOTE_D5
    622,  // NOTE_DS5
    659,  // NOTE_E5
    698,  // NOTE_F5
    740,  // NOTE_FS5
    784,  // NOTE_G5
    831,  // NOTE_GS5
    880,  // NOTE_A5
    932,  // NOTE_AS5
    988,  // NOTE_B5
    1047, // NOTE_C6
    1109, // NOTE_CS6
    1175, // NOTE_D6
    1245, // NOTE_DS6
    1319, // NOTE_E6
    1397, // NOTE_F6
    1480, // NOTE_FS6
    1568, // NOTE_G6
    1661, // NOTE_GS6
    1760, // NOTE_A6
    1865, // NOTE_AS6
    1976, // NOTE_B6
    2093, // NOTE_C7
    2217, // NOTE_CS7
    2349, // NOTE_D7
    2489, // NOTE_DS7
    2637, // NOTE_E7
    2794, // NOTE_F7
    2960, // NOTE_FS7
    3136, // NOTE_G7
    3322, // NOTE_GS7
    3520, // NOTE_A7
    3729, // NOTE_AS7
    3951, // NOTE_B7
    4186, // NOTE_C8
    4435, // NOTE_CS8
    4699, // NOTE_D8
    4978  // NOTE_DS8
};



// Buzzer definition
#define BUZZER_PIN 15
// #define ACTIVE_LOW_CHECK_PIN 34

bool buzzerActive = false;
unsigned long buzzerStartTime = 0;
unsigned long buzzerDuration = 0;

bool isActiveLow = false;

void playNote(int note, int duration)
{
    tone(BUZZER_PIN, NOTE_FREQUENCIES[note], duration);
}

void BUZZER_WRITE(int pin, int state)
{
    // !isActiveLow ?
     digitalWrite(pin, state);
    //  : digitalWrite(pin, !state);
}

void setupBuzzer()
{
    // Check if buzzer is active low
    // pinMode(ACTIVE_LOW_CHECK_PIN, INPUT_PULLDOWN);
    // isActiveLow = digitalRead(ACTIVE_LOW_CHECK_PIN);
    
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
    // tone(BUZZER_PIN, 440, 150);
}

void buzzError()
{
    // 31, 600ms
    // buzzStart(400);
    noTone(BUZZER_PIN);
    tone(BUZZER_PIN, 175, 500);
}

void buzzShort()
{
    buzzStart(100);
    // tone(BUZZER_PIN, 440, 100);
}

void buzzLong()
{
    // tone(BUZZER_PIN, 440, 800);
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