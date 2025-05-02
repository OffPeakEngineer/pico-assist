#ifndef CTX_SPLIT_H
#define CTX_SPLIT_H

#define CTX_STATIC static

#include <stdint.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/time.h>

#define CTX_IMPLEMENTATION 1
#include "ctx_config.h"

#include "ctx.h"
#include "ctx-list.h"
#include "config.h"
#include "ctx-extra.h"
#include "ctx-internal.h"
#include "constants.h"
#include "libc.c"
#include "drawlist.h"
#include "ctx-string.h"
// #include "vt-line.h"
// #include "ctx-clients.h"
#include "util.h"
#include "version.h"
#include "squoze.h"
#include "util.h"
#include "a85.h"
// #include "audio.h"
// #include "vt.h"
#include "ctx-font-ascii.h"
#include "css.h"
#include "miniz.h"
// #include "vt-audio.h"
// #include "vt-utf8.h"
#include "utf8.h"
//extern CtxFont *ctx_fonts;

#endif
