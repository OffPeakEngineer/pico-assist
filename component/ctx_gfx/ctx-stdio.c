#include "ctx.h"

/* this program is a mini-wrapper for ctx applications using the ctx protocol
 * the same source code also runs on the rp2040 and rp2350 MCUs and possibly
 * other micro-controllers unmodified; then providing an external - possibly
 * interactive screen over the whatever stdin/stdout is configured as, on
 * the rp ones this is the serial device.
 *
 * When build locally a ctx application can be launched within the wrapper with:\
 *
 *     : | { app_to_launch | ctx-stdio; } > /dev/fd0
 *
 * For a microcontroller build attached to a device the same overriding
 * of standard input and output is achieved by the simpler:
 *
 *     app_to_launch < /dev/ttyACM0 > /dev/ttyACM0
 *
 */

void stdio_start_frame (Ctx *ctx, void *data)
{
  ctx_start_frame (ctx);
}

void stdio_end_frame (Ctx *ctx, void *data)
{
  ctx_end_frame (ctx);
}

void stdio_response (Ctx *ctx, void *data, char *response, int len)
{
  fwrite (response, 1, len, stdout);
  fflush (stdout);
}

#if CTX_BIN_BUNDLE
int ctx_stdio_main (int argc, char **argv)
#else
int main (int argc, char **argv)
#endif
{
#if !defined(PICO_BUILD)
  if (argv[1] && (!strcmp (argv[1], "--help") || !strcmp(argv[1], "-h")))
  {
    printf ("ctx stdio\n");
    printf ("  a minimal ctx terminal, communicating ctx protocol via stdin and stdout");
    printf ("  for testing on a system with bash:\n   : | { app_to_launch | ctx stdio; } > /dev/fd0\n");
    return 0;
  }
#endif

  Ctx *ctx = ctx_new(-1, -1, NULL);
  ctx_start_frame (ctx);
  ctx_rgba (ctx, 0,0,0,0);
  ctx_paint (ctx);
  ctx_logo (ctx, ctx_width(ctx)/2, ctx_height(ctx)/2, (ctx_width(ctx)+ctx_height(ctx))/4);
  ctx_end_frame (ctx);
  CtxParserConfig config = {
    .width       = ctx_width (ctx),
    .height      = ctx_height (ctx), 
    .cell_width  = ctx_width (ctx)/30,
    .cell_height = ctx_width(ctx)/30*1.5,
    .start_frame = stdio_start_frame,
    .end_frame   = stdio_end_frame,
    .response    = stdio_response,
    .user_data   = &config,
    .flags       = CTX_FLAG_FORWARD_EVENTS | CTX_FLAG_HANDLE_ESCAPES
  };
  CtxParser *parser = ctx_parser_new (ctx, &config);

  int rd = 0;
  do
  {
    rd = getchar();
    if (rd >=0)
    { 
      char buf[2] = {rd, 0};
      ctx_parser_feed_bytes (parser, buf, 1);
    }
  }
  while (rd != EOF);

  ctx_parser_destroy (parser);
  ctx_destroy(ctx);
  return 0;
}
