#include "LCDDisplay.h"
#include <LiquidCrystal_I2C.h>

enum LCDState
{
    LCD_IDLE,
    LCD_TEMPORARY,
    LCD_IMPORTANT
};

LCDState lcdState = LCD_IDLE;
unsigned long errorDisplayStartTime = 0;
unsigned long errorDisplayDuration = 3000;

static bool backlightState = true;

String lastDisplayMessage = "";

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
    lcdPrintHome();
    Serial.println("LCD initialized");
}

void lcdPrint(const String &line1, const String &line2)
{
    // if previous message is same as now then ignore.
    if (lastDisplayMessage == line1 + line2)
        return;

    lastDisplayMessage = line1 + line2;

    lcdState = LCD_IDLE;
    lcdClear();

    delay(5); // Allow time for the LCD to clear before printing new text
    lcd.setCursor(0, 0);
    lcd.print(line1.c_str());
    if (!line2.isEmpty())
    {
        lcd.setCursor(0, 1);
        lcd.print(line2.c_str());
    }
}

void lcdPrintImportant(const String &line1, const String &line2)
{
    // if previous message is same as now then ignore.
    if (lastDisplayMessage == line1 + line2)
        return;

    lastDisplayMessage = line1 + line2;
    lcdState = LCD_IMPORTANT;
    lcdClear();
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
    // lcd.setCursor(0,0);
    // lcd.print("                  ");
    // lcd.setCursor(1,0);
    // lcd.print("                  ");
}

void lcdPrintHome()
{
    lcdPrint(MSG_HOME);
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
    // if previous message is same as now then ignore.
    if (lastDisplayMessage == line1 + line2)
        return;

    if (lcdState == LCD_IMPORTANT)
        return;

    lastDisplayMessage = line1 + line2;

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
            lcdPrint(MSG_HOME);
            lcdState = LCD_IDLE;
        }
    }
}