#include "ctx-split.h"

int ctx_utf8_len (const unsigned char first_byte)
{
  return _ctx_utf8_len (first_byte);
}

int ctx_utf8_strlen (const char *s)
{
  return _ctx_utf8_strlen (s);
}

const char *ctx_utf8_skip (const char *s, int utf8_length)
{
  return _ctx_utf8_skip (s, utf8_length);
}

int
ctx_unichar_to_utf8 (uint32_t  ch,
                     uint8_t  *dest)
{
  return _ctx_unichar_to_utf8 (ch, dest);
}
uint32_t
ctx_utf8_to_unichar (const char *input)
{
  return _ctx_utf8_to_unichar (input);
}
