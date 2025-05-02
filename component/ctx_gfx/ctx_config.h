#ifndef CTX_CONFIG_H
#define CTX_CONFIG_H

//#include "local.conf"

#define CTX_COMPRESS             1
#define CTX_DECOMPRESSOR         1
#define CTX_GLYPH_CACHE          1
#define CTX_FONT_SHAPE_CACHE     0
#define CTX_SHAPE_GLYPHS        32
#ifndef CTX_VT
#define CTX_VT                   1
#endif
#define CTX_FONTGEN              1
#define CTX_BITPACK_PACKER       1
#define CTX_RASTERIZER_AA        3
#define CTX_THREADS              1
#define CTX_MATH                 1
#define CTX_32BIT_SEGMENTS       1
#define CTX_EXTRAS               0
#define CTX_GSTATE_PROTECT       1
//#define CTX_MAX_EDGES          255
#define CTX_MAX_TEXTURES         1024  // default: 32
#define CTX_RASTERIZER_BEZIER_FIXED_POINT 0
				       //
#define CTX_RASTERIZER_ALLOW_DIRECT 1
#define CTX_MIN_JOURNAL_SIZE     1024 // default: 512 ~4kb
#define CTX_MAX_JOURNAL_SIZE     1024*1024*8 // default: 1024*1024*8 // 72mb
#define CTX_MIN_EDGE_LIST_SIZE   1024*16     // default: 1024*4
					     // when max is not set it is set to min
//#define CTX_PARSER_MAXLEN      1024*1024*64 // default: 1024*1024*16

#define CTX_HASH_COLS            8    // default: 6
#define CTX_HASH_ROWS            4    // default: 5
/* for misc storage with compressed/
   variable size for each save|restore*/
#define CTX_MAX_KEYDB            128  // default: 64
#define CTX_GET_CONTENTS         1
#define CTX_MAGIC                1
#define CTX_PROTOCOL_U8_COLOR    1
#define CTX_NET                  1

#ifdef CTX_WANT_FONT

#define CTX_STATIC_FONT(font) \
  ctx_load_font_ctx(ctx_font_##font##_name, \
                    ctx_font_##font,       \
                    sizeof (ctx_font_##font))

//#define CTX_FONT0 CTX_STATIC_FONT("sans-ctx", ascii)
#if CTX_STATIC_FONTS
#include "Cousine-Regular.h"
//#include "Arimo-Regular.h"
#include "Roboto-Regular.h"
#if 0  // build more static fonts and uncomment these..
#include "Carlito-Regular.h"
#include "Carlito-Bold.h"
#include "Carlito-Italic.h"
#include "Carlito-BoldItalic.h"
#include "Cousine-Bold.h"
#include "Cousine-Italic.h"
#include "Cousine-BoldItalic.h"
#include "Arimo-Bold.h"
#include "Arimo-Italic.h"
#include "Arimo-BoldItalic.h"
#include "Tinos-Regular.h"
#include "Tinos-Bold.h"
#include "Tinos-Italic.h"
#include "Tinos-BoldItalic.h"
#endif

//#define CTX_FONT_0   CTX_STATIC_FONT(Arimo_Regular)
#define CTX_FONT_0   CTX_STATIC_FONT(Roboto_Regular)
#if 0
#define CTX_FONT_1   CTX_STATIC_FONT(Arimo_Bold)
#define CTX_FONT_2   CTX_STATIC_FONT(Arimo_Italic)
#define CTX_FONT_3   CTX_STATIC_FONT(Arimo_BoldItalic)
#define CTX_FONT_4   CTX_STATIC_FONT(Tinos_Regular)
#define CTX_FONT_5   CTX_STATIC_FONT(Tinos_Bold)
#define CTX_FONT_6   CTX_STATIC_FONT(Tinos_Italic)
#define CTX_FONT_7   CTX_STATIC_FONT(Tinos_BoldItalic)
#endif

#define CTX_FONT_8   CTX_STATIC_FONT(Cousine_Regular)
#if 0
#define CTX_FONT_9   CTX_STATIC_FONT(Cousine_Italic)
#define CTX_FONT_10  CTX_STATIC_FONT(Cousine_Bold)
#define CTX_FONT_11  CTX_STATIC_FONT(Cousine_BoldItalic)
#define CTX_FONT_12  CTX_STATIC_FONT(Carlito_Regular)
#define CTX_FONT_13  CTX_STATIC_FONT(Carlito_Bold)
#define CTX_FONT_14  CTX_STATIC_FONT(Carlito_Italic)
#define CTX_FONT_15  CTX_STATIC_FONT(Carlito_BoldItalic)
#endif
#endif

#endif

#endif
