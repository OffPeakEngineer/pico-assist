#ifndef _gfxtest_
#define _gfxtest_

#include <FreeRTOS.h>
#include <task.h>

#include "esphome/core/application.h"

// #include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

unsigned long testFillScreen();
unsigned long testText();
unsigned long testLines(uint16_t color);
unsigned long testFastLines(uint16_t color1, uint16_t color2);
unsigned long testRects(uint16_t color);
unsigned long testFilledRects(uint16_t color1, uint16_t color2);
unsigned long testFilledCircles(uint8_t radius, uint16_t color);
unsigned long testCircles(uint8_t radius, uint16_t color);
unsigned long testTriangles();
unsigned long testFilledTriangles();
unsigned long testRoundRects();
unsigned long testFilledRoundRects();

  #endif // _gfxtest_