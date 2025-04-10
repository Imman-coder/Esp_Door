#include "LCDDisplay.h"
#include <LiquidCrystal_I2C.h>


enum LCDState
{
    LCD_IDLE,
    LCD_TEMPORARY
};

LCDState lcdState = LCD_IDLE;
unsigned long errorDisplayStartTime = 0;
unsigned long errorDisplayDuration = 3000;

static bool backlightState = true;

// LED Display definitions
#define LCD_ADDR 0x27
#define LCD_SDA 4
#define LCD_SCL 22

#define LCD_ROW 2
#define LCD_COL 16

LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COL, LCD_ROW);

void setupLCD()
{
    lcd.init(LCD_SDA, LCD_SCL);
    lcd.backlight();
    lcdPrint("Enter Pin or", "Scan NFC");
    Serial.println("LCD initialized");
}

void lcdPrint(const String &line1, const String &line2)
{
    lcdState = LCD_IDLE;
    lcd.clear();
    delay(5); // Allow time for the LCD to clear before printing new text
    lcd.setCursor(0, 0);
    lcd.print(line1.c_str());
    if (!line2.isEmpty())
    {
        lcd.setCursor(0, 1);
        lcd.print(line2.c_str());
    }
}

void lcdClear()
{
    lcd.clear();
}

void lcdWelcome()
{
    lcd.clear();
    
    lcd.setCursor(0, 0);
    lcd.print("Welcome!");
}

void setBacklight(bool state)
{
    if (state)
    {
        lcd.backlight();
    }
    else
    {
        lcd.noBacklight();
    }
}

void toggleBacklight()
{
    backlightState = !backlightState;
    setBacklight(backlightState);
}

void lcdPrintTemporary(const String &line1, const String &line2, unsigned long timeout)
{
    lcdPrint(line1, line2);
    errorDisplayStartTime = millis();
    errorDisplayDuration = timeout;
    lcdState = LCD_TEMPORARY;
}

void loopLCD()
{
    if (lcdState == LCD_TEMPORARY)
    {
        if (millis() - errorDisplayStartTime > errorDisplayDuration)
        {
            // lcdPromptAuthentication();
            lcdPrint("Enter Pin or", "Scan NFC");
            lcdState = LCD_IDLE;
        }
    }
}