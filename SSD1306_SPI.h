#ifndef MODULE_SSD1306_DISPLAY
#define MODULE_SSD1306_DISPLAY

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "configs.h"

void setup_ssd1306_module();
void drawSensorData(float temp, float hum, float pres);
void drawText(const char *text, size_t size = 1, size_t posX = 0, size_t posY = 0, bool clearDisplay = true);

#endif // MODULE_SSD1306_DISPLAY
