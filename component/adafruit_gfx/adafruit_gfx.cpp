#include "adafruit_gfx.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"
#include "esphome/core/application.h"

#define ORIENTATION 0
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 320
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

// For the PicoCalc, these are the default.
#define TFT_DC 14
#define TFT_CS 13
#define TFT_MOSI 11
#define TFT_CLK 10
#define TFT_RST 15
#define TFT_MISO 12

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

namespace esphome
{
    namespace picocalc
    {
        static const char *const TAG = "adafruit_gfx";
        uint8_t x = 0;
    
        void AdafruitGfx::setup()
        {
            ESP_LOGCONFIG(TAG, "Adafruit GFX Online!");
            tft.begin();
            tft.setRotation(ORIENTATION);
            tft.fillScreen(ILI9341_BLACK);
            tft.invertDisplay(true);

            // read diagnostics (optional but can help debug problems)
            uint8_t x = tft.readcommand8(ILI9341_RDMODE);
        }


        void AdafruitGfx::dump_config()
        {
            ESP_LOGCONFIG(TAG, "AdafruitGfx config:");
            ESP_LOGCONFIG(TAG, "Display Power Mode: %X", x);
            x = tft.readcommand8(ILI9341_RDMADCTL);
            ESP_LOGCONFIG(TAG, "MADCTL Mode: 0x %X", x);
            x = tft.readcommand8(ILI9341_RDPIXFMT);
            ESP_LOGCONFIG(TAG, "Pixel Format: %X", x);
            x = tft.readcommand8(ILI9341_RDIMGFMT);
            ESP_LOGCONFIG(TAG, "Image Format: %X", x);
            x = tft.readcommand8(ILI9341_RDSELFDIAG);
            ESP_LOGCONFIG(TAG, "Self Diagnostic: %X", x);
        }

        int cycle = 0;
        void AdafruitGfx::loop()
        {
            switch (cycle)
            {
                case 0: tft.fillScreen(ILI9341_BLACK); break;
                case 1: tft.fillScreen(ILI9341_RED); break;
                case 2: tft.fillScreen(ILI9341_GREEN); break;
                case 3: tft.fillScreen(ILI9341_BLUE); break;
                case 4: tft.fillScreen(ILI9341_BLACK); break;
                case 5: testText(); break;
                case 6: testLines(ILI9341_WHITE); break;
                case 7: testFastLines(ILI9341_RED, ILI9341_BLUE); break;
                case 8: testFilledRects(ILI9341_RED, ILI9341_BLUE); break;
                case 9: testFilledCircles(10, ILI9341_YELLOW); break;
                case 10: testCircles(10, ILI9341_YELLOW); break;
                case 11: testTriangles(); break;
                case 12: testFilledTriangles(); break;
                case 13: testRoundRects(); break;
                case 14: testFilledRoundRects(); break;
                default: cycle = 0; break;
            }
            
            cycle++;
            return;
        }

        void AdafruitGfx::delay(uint32_t ms)
        {
            App.feed_wdt();
            vTaskDelay(ms / portTICK_PERIOD_MS);
        }
    } // namespace picocalc
} // namespace esphome