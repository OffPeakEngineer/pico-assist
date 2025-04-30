#include "ctx_gfx.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"
#include "esphome/core/application.h"

#define ORIENTATION 0
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 320
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
    static const char *const TAG = "ctx_gfx";

    // Ctx *ctx;

    void CtxGraphics::setup()
    {
        ESP_LOGCONFIG(TAG, "CTX GFX Online!");
        // CtxCbConfig config = {
        //     .set_pixels = fb_set_pixels,
        //     .format = CTX_FORMAT_RGB565,
        //     .buffer = scratch,
        //     .buffer_size = sizeof(scratch),
        //     .flags = 0 | CTX_FLAG_HASH_CACHE | CTX_FLAG_SHOW_FPS | CTX_FLAG_RENDER_THREAD
        // };

        // ctx = ctx_new_cb(SCREEN_WIDTH, SCREEN_HEIGHT, &config);  
    }

    void CtxGraphics::dump_config()
    {
        ESP_LOGCONFIG(TAG, "CtxGraphics config:");
    }

    void CtxGraphics::loop()
    {
       // ctx_fill_color(ctx, 0xffff);  // white
        // ctx_clear(ctx);
        // ctx_font(ctx, 0);
        // ctx_move_to(ctx, 10, 20);
        // ctx_text(ctx, "Hello ILI9341!");
        // ctx_frame(ctx);
    }

    // void CtxGraphics::fb_set_pixels(Ctx *ctx, void *user_data, int x, int y, int w, int h, void *buf)
    // {
    //     uint16_t *pixels = (uint16_t *)buf;  // Already RGB565
    //     tft.startWrite();
    //     for (int row = 0; row < h; row++) {
    //         tft.setAddrWindow(x, y + row, w, 1);
    //         tft.writePixels(pixels + (row * w), w, true);
    //     }
    //     tft.endWrite();
    // }
}
}