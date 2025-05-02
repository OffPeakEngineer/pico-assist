#define OVERCLOCK     1

#include <stdint.h>

#define TFT_DC 14
#define TFT_CS 13
#define TFT_MOSI 11
#define TFT_CLK 10
#define TFT_RST 15
#define TFT_MISO 12
#define ORIENTATION 0
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 320

#if PICO_RP2350
uint8_t scratch[SCREEN_WIDTH * SCREEN_HEIGHT * 2]; 
#else
uint8_t scratch[64 * 1024];
#endif

#define CTX_PICO                           1
#define CTX_PTY                            0
#define CTX_1BIT_CLIP                      1
#define CTX_RASTERIZER_AA                  3
#define CTX_RASTERIZER_MAX_CIRCLE_SEGMENTS 36
#define CTX_MIN_EDGE_LIST_SIZE             800
#define CTX_MAX_EDGE_LIST_SIZE             800
#define CTX_MIN_JOURNAL_SIZE               6000
#define CTX_MAX_JOURNAL_SIZE               6000

#define CTX_LIMIT_FORMATS             1
#define CTX_DITHER                    1
#define CTX_32BIT_SEGMENTS            0
#define CTX_ENABLE_RGB565             1
#define CTX_ENABLE_RGB565_BYTESWAPPED 1
#define CTX_BITPACK_PACKER            0
#define CTX_COMPOSITING_GROUPS        0
#define CTX_RENDERSTREAM_STATIC       0
#define CTX_GRADIENT_CACHE            1
#define CTX_ENABLE_CLIP               1
#define CTX_BLOATY_FAST_PATHS         0

#define CTX_VT              1
#define CTX_PARSER          1
#define CTX_PARSER_MAXLEN   (3 *1024)
#define CTX_RASTERIZER      1
#define CTX_EVENTS          1
#define CTX_RAW_KB_EVENTS   0
#define CTX_STRINGPOOL_SIZE 512
#define CTX_FORMATTER       0
#define CTX_TERMINAL_EVENTS 1
#define CTX_FONTS_FROM_FILE 0

#define CTX_STATIC_FONT(font) \
  ctx_load_font_ctx(ctx_font_##font##_name, \
                    ctx_font_##font,       \
                    sizeof (ctx_font_##font))
#include <stdint.h>
#include "Roboto-Regular.h"
#include "Cousine-Regular.h"

#define CTX_FONT_0   CTX_STATIC_FONT(Roboto_Regular)
#define CTX_FONT_8   CTX_STATIC_FONT(Cousine_Regular)

#include <pico/stdlib.h>
#include <pico/time.h>

#define CTX_IMPLEMENTATION 1
#include "ctx.h"

#include "hardware/gpio.h"
#include "hardware/clocks.h"
#include "hardware/vreg.h"
#include "Adafruit_ILI9341.h"

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

static void fb_set_pixels(Ctx *ctx, void *user_data, int x, int y, int w, int h, void *buf) {
  uint16_t *pixels = (uint16_t *)buf;
  tft.startWrite();
  tft.setAddrWindow(x, y, w, h);
  tft.writePixels(pixels, w * h, true);
  tft.endWrite();
}

Ctx *ctx_pico_ili9341_init(int fb_width, int fb_height) {
  tft.begin();
  tft.setRotation(ORIENTATION);
  tft.fillScreen(ILI9341_BLACK);
  tft.invertDisplay(true);

  CtxCbConfig config = {
    .set_pixels = fb_set_pixels,
    .format = CTX_FORMAT_RGB565,
    .buffer = scratch,
    .buffer_size = sizeof(scratch),
    .flags = CTX_FLAG_HASH_CACHE | CTX_FLAG_SHOW_FPS | CTX_FLAG_RENDER_THREAD
  };

  return ctx_new_cb(fb_width, fb_height, &config);
}

Ctx *pico_ctx = NULL;

Ctx *ctx_host(void) {
  if (pico_ctx)
    return pico_ctx;

  stdio_init_all();

#if OVERCLOCK
  vreg_set_voltage(VREG_VOLTAGE_1_30);
  set_sys_clock_khz(360000, false);
#endif

  setup_default_uart();

  pico_ctx = ctx_pico_ili9341_init(SCREEN_WIDTH, SCREEN_HEIGHT);
  return pico_ctx;
}
