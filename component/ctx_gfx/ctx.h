/*
 * Copyright (c) 2012, 2015, 2019, 2020, 2021, 2022, 2023, 2024, 2025
 * Øyvind Kolps <pippin@gimp.org> with contributors.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * ctx is a 2D vector graphics protocol, and interactive application
 * development environment for microcontrollers, framebuffers and
 * terminals on unix systems.
 * 
 * To use ctx add ctx.h to the project path and do the following:
 *
 * #define CTX_IMPLEMENTATION
 * #include "ctx.h"
 *
 * Ctx contains a minimal default fallback font with only ascii, so
 * you probably want to also include a font, and perhaps enable
 * SDL2 optional backends, a more complete example:
 *
 * #include <cairo.h>
 * #include <SDL.h>
 * 
 * #define CTX_IMPLEMENTATION
 * #include "ctx.h"
 *
 * The behavior of ctx can be tweaked, and features can be configured, enabled
 * or disabled with other #defines, see further down in the start of this file
 * for details.
 */

#ifndef CTX_H
#define CTX_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>

/*** h2: context management */

typedef struct _Ctx            Ctx;

/**
 * ctx_new:
 * @width: with in device units
 * @height: height in device units
 * @backend: backend to use
 *
 *   valid values are:
 *     NULL/"auto", "drawlist", "sdl", "term", "ctx" the strings are
 *     the same as are valid for the CTX_BACKEND environment variable.
 *
 * Create a new drawing context, this context has no pixels but
 * accumulates commands and can be played back on other ctx
 * render contexts, this is a ctx context using the drawlist backend.
 */
Ctx *ctx_new (int width, int height, const char *backend);


/**
 * ctx_new_drawlist:
 *
 * Create a new drawing context that can record drawing commands,
 * this is also the basis for creating more complex contexts with
 * swapped out backend.
 */
Ctx * ctx_new_drawlist (int width, int height);

/** CtxEntry:
 *
 * A pointer to a command in binary ctx protocol.
 */
typedef struct _CtxEntry   CtxEntry;
/** CtxCommand:
 *
 * A pointer to a command in binary ctx protocol.
 */
typedef struct _CtxCommand CtxCommand;

/* The pixel formats supported as render targets, depending on
 * compile-time configuration not all formats are usable.
 */
enum _CtxPixelFormat
{
  CTX_FORMAT_NONE=0,
  CTX_FORMAT_GRAY8,  // 1  - these enum values are not coincidence
  CTX_FORMAT_GRAYA8, // 2  - but match bpp, for the common gray and
  CTX_FORMAT_RGB8,   // 3  - rgb cases up to 4bpp = RGBA8
  CTX_FORMAT_RGBA8,  // 4  -
  CTX_FORMAT_BGRA8,  // 5
  CTX_FORMAT_RGB565, // 6
  CTX_FORMAT_RGB565_BYTESWAPPED, // 7
  CTX_FORMAT_RGB332, // 8 // matching flags
  CTX_FORMAT_RGBAF,  // 9
  CTX_FORMAT_GRAYF,  // 10
  CTX_FORMAT_GRAYAF, // 11
  CTX_FORMAT_GRAY1,  // 12
  CTX_FORMAT_CMYK8,  // 13
  CTX_FORMAT_CMYKAF, // 14
  CTX_FORMAT_CMYKA8, // 15 
  CTX_FORMAT_GRAY2,  // 16 // matching flags
  CTX_FORMAT_YUV420, // 17
  CTX_FORMAT_BGR8,   // 
  CTX_FORMAT_RGBA8_SEPARATE_ALPHA, //
  CTX_FORMAT_GRAY4 =32, // to match flags
  CTX_FORMAT_BGRA8Z,    // 
};
typedef enum   _CtxPixelFormat CtxPixelFormat;

/**
 * ctx_new_for_framebuffer:
 *
 * Create a new drawing context for a framebuffer, rendering happens
 * immediately.
 */
Ctx *ctx_new_for_framebuffer (void *data,
                              int   width,
                              int   height,
                              int   stride,
                              CtxPixelFormat pixel_format);



/**
 * ctx_get_drawlist:
 * @ctx: a ctx context.
 * @count: return location for length of drawlist
 *
 * The returned pointer is only valid as long as no further drawing has been
 * done.
 *
 * Returns a read only pointer to the first entry of the contexts drawlist.
 */
const CtxEntry *ctx_get_drawlist (Ctx *ctx, int *count);


/**
 * ctx_new_for_drawlist:
 *
 * Create a new drawing context for a pre-existing raw drawlist.
 */
Ctx *ctx_new_for_drawlist   (int    width,
                             int    height,
                             void  *data,
                             size_t length);

/**
 * ctx_set_drawlist:
 *
 * Replaces the drawlist of a ctx context with a new one.  the length of the
 * data is expected to be length * 9;
 */
int  ctx_set_drawlist       (Ctx *ctx, void *data, int length);

/**
 * ctx_append_drawlist:
 *
 * Appends the commands in a binary drawlist, the length of the data is expected to
 * be length * 9;
 */
int  ctx_append_drawlist    (Ctx *ctx, void *data, int length);

/**
 * ctx_drawlist_clear:
 *
 * Clears the drawlist associated with the context.
 */
void  ctx_drawlist_clear (Ctx *ctx);

/**
 * ctx_drawlist_force_count:
 * @ctx: a ctx context
 * @count: new count to set, must be lower than the current count.
 *
 * Shortens the length of the internal drawlist, dropping the last
 * items.
 */
void ctx_drawlist_force_count (Ctx *ctx, int count);


/**
 * ctx_destroy:
 * @ctx: a ctx context
 */
void ctx_destroy (Ctx *ctx);



/*** h2: drawing api */


/*** h3: frame start/end */

/* most backends, apart from PDF expect to have the separate frames
 * to be shown bracketed by ctx_start_frame() and ctx_end_frame() calls.
 *
 * The combination of the calls are blocking if rendering is congested.
 */

/**
 * ctx_start_frame:
 *
 * Prepare for rendering a new frame, clears internal drawlist and initializes
 * the state.
 *
 * Returns time in seconds since previous start_frame.
 */
float ctx_start_frame    (Ctx *ctx);

/**
 * ctx_end_frame:
 *
 * We're done rendering a frame, this does nothing on a context created for a
 * framebuffer, where drawing commands are immediate.
 */
void ctx_end_frame      (Ctx *ctx);


/* create a new page
 */
void ctx_new_page         (Ctx *ctx);

/**
 * ctx_view_box:
 *
 * Specify the view box for the current page, should immediately follow
 * new_page if present, the PDF backend in particular makes use of this.
 */
void ctx_view_box         (Ctx *ctx,
                           float x0, float y0,
                           float w, float h);


/*** h3: path construction/manipulation */


/**
 * ctx_x:
 * @ctx: a context
 *
 * Returns the current path append x-coordinate.
 */
float ctx_x                    (Ctx *ctx);

/**
 * ctx_y:
 * @ctx: a context
 *
 * Returns the current path append y-coordinate.
 */
float ctx_y                    (Ctx *ctx);

/**
 * ctx_get_current_point:
 * @ctx: a context
 * @x: a pointer to store x coordinate in, or NULL
 * @y: a pointer to store y coordinate in, or NULL
 *
 * Returns the same value as ctx_x() and ctx_y()
 */
void  ctx_current_point        (Ctx *ctx, float *x, float *y);

/**
 * ctx_reset_path:
 * @ctx: a context
 *
 * Clears the current path if any, fill and stroke commands without a preceding preserve do an implicit reset_path.
 */
void ctx_reset_path     (Ctx *ctx);

#define ctx_begin_path(ctx) ctx_reset_path(ctx) // compatibility with old API

/**
 * ctx_move_to:
 * @ctx: a context
 * @x: target x coordinate
 * @y: target y coordinate
 *
 * Move the tip of the virtual pen to x,y, starting a new sub-path.
 */
void  ctx_move_to         (Ctx *ctx, float x, float y);

/**
 * ctx_line_to:
 * @ctx: a context
 * @x: target x coordinate
 * @y: target y coordinate
 *
 * Add a straight line segment to the current path. moving the
 * thip of the virtual pen to x,y.
 */
void  ctx_line_to         (Ctx *ctx, float x, float y);

/**
 * ctx_curve_to:
 * @ctx: a context
 * @cx0: control point x coordinate
 * @cy0: control point y coordinate
 * @cx1: control point x coordinate
 * @cy1: control point y coordinate
 * @x: target x coordinate
 * @y: target y coordinate
 *
 * Adds a quad curve segment to x, y to the current path.
 */
void  ctx_curve_to        (Ctx *ctx,
                           float cx0, float cy0,
                           float cx1, float cy1,
                           float x, float y);
/**
 * ctx_quad_to:
 * @ctx: a context
 * @cx: control point x coordinate
 * @cy: control point y coordinate
 * @x: target x coordinate
 * @y: target y coordinate
 *
 * Adds a quad curve segment to x, y to the current path.
 */
void  ctx_quad_to         (Ctx  *ctx,
                           float cx, float cy,
                           float x,  float y);
/**
 * ctx_arc:
 *
 * Add an arc segment for a circle centered at x,y with radius fromg angle1 to angle2 in radians,
 * XXX : look into specification of direction on other APIs
 */
void  ctx_arc             (Ctx  *ctx,
                           float x, float y,
                           float radius,
                           float angle1, float angle2,
                           int   direction);
/**
 * ctx_arc_to:
 * @ctx: a context
 */
void  ctx_arc_to          (Ctx *ctx,
                           float x1, float y1,
                           float x2, float y2,
                           float radius);
/**
 * ctx_rel_arc_to:
 * @ctx: a context
 */
void  ctx_rel_arc_to      (Ctx *ctx,
                           float x1, float y1,
                           float x2, float y2,
                           float radius);


/**
 * ctx_rectangle:
 * @ctx: a context
 * @x0: upper left x coordinate
 * @y0: upper left y coordiante
 * @width: width in user space coordinate
 * @height: height in user space coordinate
 *
 * Add rectangle to the current path.
 */
void  ctx_rectangle       (Ctx *ctx,
                           float x0, float y0,
                           float w, float h);
/**
 * ctx_round_rectangle:
 * @ctx: a context
 * @x0: upper left x coordinate
 * @y0: upper left y coordiante
 * @width: width in user space coordinate
 * @height: height in user space coordinate
 * @radius: rounding radius, if width or height are too small radius is clamped accordingly.
 *
 * Add a rectangle with rounded corners to the current path.
 */
void  ctx_round_rectangle (Ctx *ctx,
                           float x0, float y0,
                           float w, float h,
                           float radius);
/**
 * ctx_rel_line_to:
 * @ctx: a context
 * @x: target x coordinate
 * @y: target y coordinate
 *
 * Adds a straight segment to ctx_x()+x, ctx_y()+y to the current path.
 */
void  ctx_rel_line_to     (Ctx *ctx,
                           float x, float y);
/**
 * ctx_rel_move_to:
 * @ctx: a context
 * @x: target x coordinate
 * @y: target y coordinate
 *
 * Stop current sub path and move path append point to ctx_x()+x, ctx_y()+y
 */
void  ctx_rel_move_to     (Ctx *ctx,
                           float x, float y);
/**
 * ctx_rel_quad_to:
 * @ctx: a context
 * @cx0: control point x coordinate
 * @cy0: control point y coordinate
 * @cx: control point x coordinate
 * @cy: control point y coordinate
 * @x: target x coordinate
 * @y: target y coordinate
 *
 * Adds a cubic curve segment to ctx_x()+x, ctx_y()+y to the current path.
 */
void  ctx_rel_curve_to    (Ctx *ctx,
                           float cx0, float cy0,
                           float cx1, float cy1,
                           float x, float y);
/**
 * ctx_rel_quad_to:
 * @ctx: a context
 * @cx: control point x coordinate
 * @cy: control point y coordinate
 * @x: target x coordinate
 * @y: target y coordinate
 *
 * Adds a quad curve segment to ctx_x()+x, ctx_y()+y to the current path.
 */
void  ctx_rel_quad_to     (Ctx *ctx,
                           float cx, float cy,
                           float x, float y);
/**
 * ctx_close_path:
 * @ctx: a context
 *
 * Closes the currently open sub-path.
 */
void  ctx_close_path      (Ctx *ctx);

/**
 * ctx_in_fill:
 * @ctx: a ctx context
 * @x: x coordinate
 * @y: y coordinate
 *
 * Returns 1 if x, y are inside a fill of the current path with current fill-rule.
 */
int  ctx_in_fill    (Ctx *ctx, float x, float y);

/**
 * ctx_in_stroke:
 * @ctx: a ctx context
 * @x: x coordinate
 * @y: y coordinate
 *
 * Returns 1 if x, y are inside a stroke of the current path with current parameters.
 */
int  ctx_in_stroke  (Ctx *ctx, float x, float y);

typedef struct _CtxDrawlist CtxDrawlist;

/* to be freed with ctx_free
 */
CtxDrawlist * ctx_current_path (Ctx *ctx);

void
ctx_path_extents (Ctx *ctx, float *ex1, float *ey1, float *ex2, float *ey2);

/*** h3: context management  */

/* Attributes like transform, clipping state, fill and stroke sources, font sie,
 * stroking, texture interpolation and dashing are stored in stackable contexts.
 * 
 * This allows building up a hierarchy of transforms, as well as bringing the
 * drawing context back to a known state.
 */

/**
 * ctx_save:
 * @ctx: a context
 *
 * Stores the transform, clipping state, fill and stroke sources, font size,
 * stroking and dashing options.
 */
void ctx_save           (Ctx *ctx);

/**
 * ctx_restore:
 * @ctx: a context
 *
 * Restores the state previously saved with ctx_save, calls to
 * ctx_save/ctx_restore should be balanced.
 */
void ctx_restore        (Ctx *ctx);

/**
 * ctx_start_group:
 * @ctx: a context
 *
 * Start a compositing group.
 *
 */
void ctx_start_group    (Ctx *ctx);

/**
 * ctx_end_group:
 * @ctx: a context
 *
 * End a compositing group, the global alpha, compositing mode and blend mode
 * set before this call is used to apply the group.
 */
void ctx_end_group      (Ctx *ctx);


/**
 * ctx_image_smoothing:
 * @ctx: a context
 * @enabled: 1 for enabled and 0 for disabled
 *
 * Set or unset bilinear / box filtering for textures, turning it off uses the
 * faster nearest neighbor for all cases.
 */
void ctx_image_smoothing  (Ctx *ctx, int enabled);
/**
 * ctx_get_image_smoothing:
 * @ctx: a context
 *
 * Returns the current setting for image_smoothing.
 */
int   ctx_get_image_smoothing  (Ctx *ctx);


/*** h3: drawing commands */


/**
 * ctx_fill:
 * @ctx: a context
 *
 * Fills the current path, and resets it (unless ctx_preserve has been called).
 */
void ctx_fill             (Ctx *ctx);

/**
 * ctx_paint:
 * @ctx: a context
 *
 * Fills the whole canvas with color, is affected by clipping.
 */
void ctx_paint            (Ctx *ctx);

/**
 * ctx_clip:
 * @ctx: a context
 *
 * Use the current path as a clipping mask, subsequent draw calls are limited
 * by the path. The only way to increase the visible area is to first call
 * ctx_save and then later ctx_restore to undo the clip.
 */
void ctx_clip           (Ctx *ctx);

/**
 * ctx_preserve:
 * @ctx: a context
 *
 * Make the following fill or stroke not reset the current path.
 */
void ctx_preserve         (Ctx *ctx);

/**
 * ctx_stroke:
 * @ctx: a context
 *
 * Stroke the current path with current line_width, dashing cap and join options.
 */
void ctx_stroke           (Ctx *ctx);


/*** h4: stroking options */
/**
 * ctx_miter_limit:
 * @ctx: a context.
 * @limit: new miter limit in user coordinates.
 *
 * Specify the miter limit used when stroking.
 */
void ctx_miter_limit      (Ctx *ctx, float limit);

/**
 * ctx_get_miter_limit:
 * @ctx: a context.
 *
 * Returns the current miter limit.
 */
float ctx_get_miter_limit (Ctx *ctx);


#define CTX_LINE_WIDTH_HAIRLINE -1000.0
#define CTX_LINE_WIDTH_ALIASED  -1.0
#define CTX_LINE_WIDTH_FAST     -1.0  /* aliased 1px wide line */

/**
 * ctx_line_width:
 * @ctx: a context.
 * @width: new stroking width in user space coordinates.
 *
 * Set the line width used when stroking.
 */
void ctx_line_width       (Ctx *ctx, float with);

/**
 * ctx_get_line_width:
 * @ctx: a context.
 *
 * Returns the current stroking line-width.
 */
float ctx_get_line_width  (Ctx *ctx);

/**
 * ctx_line_dash_offset:
 * @ctx: a context.
 * @offset: number of user-space units to wait before starting dash pattern.
 *
 * Specify phase offset for line dash pattern.
 */
void ctx_line_dash_offset (Ctx *ctx, float offset);

/**
 * ctx_get_line_dash_offset:
 * @ctx: a context.
 *
 * Returns the current setting for image_smoothing.
 */
float ctx_get_line_dash_offset (Ctx *ctx);

/**
 * ctx_line_dash:
 * @ctx: a context.
 * @dashes: pointer to an array of floats
 * @count: number of items in dash array.
 *
 * Specify the line dash pattern.
 */
void  ctx_line_dash       (Ctx *ctx, const float *dashes, int count);


/*** h3: transforms */

/**
 * ctx_identity:
 * @ctx: a context.
 *
 * Restore context to identity transform, NOTE: a bug makes this call currently
 * breaks mult-threaded rendering when used; since the rendering threads are
 * expecting an initial transform on top of the base identity.
 */
void ctx_identity       (Ctx *ctx);

/**
 * ctx_scale:
 * @ctx: a context.
 * @x: x scale factor
 * @y: y scale factor
 *
 * Scales the user to device transform.
 */
void  ctx_scale         (Ctx *ctx, float x, float y);

/**
 * ctx_translate:
 * @ctx: a context.
 * @x: x translation
 * @y: y translation
 *
 * Adds translation to the user to device transform.
 */
void  ctx_translate     (Ctx *ctx, float x, float y);

/**
 * ctx_rotate:
 * @ctx: a context.
 * @a: angle to rotate in radians.
 *
 * Add rotatation to the user to device space transform.
 */
void ctx_rotate         (Ctx *ctx, float a);

/**
 * ctx_apply_transform:
 * @ctx: a context.
 * @a..i: matrix components.
 *
 * Adds a 3x3 matrix on top of the existing user to device space transform.
 */
void ctx_apply_transform (Ctx *ctx, float a, float b, float c,
                                    float d, float e, float f,
                                    float g, float h, float i);

typedef struct _CtxMatrix     CtxMatrix;
struct _CtxMatrix { float m[3][3]; };

/**
 * @ctx: a context.
 * @matrix: a 3x3 matrix components.
 *
 * Adds a 3x3 matrix on top of the existing user to device space transform.
 */
void ctx_apply_matrix           (Ctx *ctx, CtxMatrix *matrix);
/**
 * ctx_set_transform:
 * @ctx: a context.
 * @a..i: matrix components.
 *
 * Set the user to device transform, * Redundant with identity+apply? XXX
 */
void ctx_set_transform    (Ctx *ctx, float a, float b, float c,
                                     float d, float e, float f,
                                     float g, float h, float i);


/*** h3: filling options */

typedef enum
{
  CTX_FILL_RULE_WINDING = 0,
  CTX_FILL_RULE_EVEN_ODD
} CtxFillRule;

/**
 * ctx_fill_rule:
 * @ctx: a ctx context.
 * @mode: new fill_rule to set, CTX_FULL_RULE_WINDING or CTX_FILL_RULE_EVEN_ODD.
 *
 * Sets the current fill rule.
 */
void        ctx_fill_rule      (Ctx *ctx, CtxFillRule        fill_rule);


/**
 * ctx_get_fill_rule:
 * @ctx: a context.
 *
 * Returns the current fill rule.
 */
CtxFillRule ctx_get_fill_rule  (Ctx *ctx);

typedef enum
{
#if 0
  CTX_COMPOSITE_SOURCE_OVER      = 0,
  CTX_COMPOSITE_COPY             = 32,
  CTX_COMPOSITE_SOURCE_IN        = 64,
  CTX_COMPOSITE_SOURCE_OUT       = 96,
  CTX_COMPOSITE_SOURCE_ATOP      = 128,
  CTX_COMPOSITE_CLEAR            = 160,

  CTX_COMPOSITE_DESTINATION_OVER = 192,
  CTX_COMPOSITE_DESTINATION      = 224,
  CTX_COMPOSITE_DESTINATION_IN   = 256,
  CTX_COMPOSITE_DESTINATION_OUT  = 288,
  CTX_COMPOSITE_DESTINATION_ATOP = 320,
  CTX_COMPOSITE_XOR              = 352,
  CTX_COMPOSITE_ALL              = (32+64+128+256)
#else
  CTX_COMPOSITE_SOURCE_OVER      = 0,
  CTX_COMPOSITE_COPY             ,
  CTX_COMPOSITE_SOURCE_IN        ,
  CTX_COMPOSITE_SOURCE_OUT       ,
  CTX_COMPOSITE_SOURCE_ATOP      ,
  CTX_COMPOSITE_CLEAR            ,

  CTX_COMPOSITE_DESTINATION_OVER ,
  CTX_COMPOSITE_DESTINATION      ,
  CTX_COMPOSITE_DESTINATION_IN   ,
  CTX_COMPOSITE_DESTINATION_OUT  ,
  CTX_COMPOSITE_DESTINATION_ATOP ,
  CTX_COMPOSITE_XOR              ,
#endif
} CtxCompositingMode;
#define CTX_COMPOSITE_LAST CTX_COMPOSITE_XOR

/**
 * ctx_compositing_mode:
 * @ctx: a ctx context.
 * @mode: new compositing mode to set.
 *
 * Sets the current compositing mode
 */
void               ctx_compositing_mode     (Ctx *ctx, CtxCompositingMode mode);
/**
 * ctx_get_compositing_mode:
 * @ctx: a ctx context.
 *
 * Return the current compositing mode
 */
CtxCompositingMode ctx_get_compositing_mode (Ctx *ctx);

typedef enum
{
  CTX_BLEND_NORMAL,
  CTX_BLEND_MULTIPLY,
  CTX_BLEND_SCREEN,
  CTX_BLEND_OVERLAY,
  CTX_BLEND_DARKEN,
  CTX_BLEND_LIGHTEN,
  CTX_BLEND_COLOR_DODGE,
  CTX_BLEND_COLOR_BURN,
  CTX_BLEND_HARD_LIGHT,
  CTX_BLEND_SOFT_LIGHT,
  CTX_BLEND_DIFFERENCE,
  CTX_BLEND_EXCLUSION,
  CTX_BLEND_HUE, 
  CTX_BLEND_SATURATION, 
  CTX_BLEND_COLOR, 
  CTX_BLEND_LUMINOSITY,  // 15
  CTX_BLEND_DIVIDE,
  CTX_BLEND_ADDITION,
  CTX_BLEND_SUBTRACT,    // 18
} CtxBlend;
#define CTX_BLEND_LAST CTX_BLEND_SUBTRACT

/**
 * ctx_blend_mode:
 * @ctx: a ctx context.
 * @mode: new blend mode to set.
 *
 * Sets the current blending mode
 */
void     ctx_blend_mode     (Ctx *ctx, CtxBlend mode);

/**
 * ctx_get_blend_mode:
 * @ctx: a ctx context.
 *
 * Returns the blending mode of the current graphics context.
 */
CtxBlend ctx_get_blend_mode (Ctx *ctx);

/*** h3: paint/stroke sources */

/**
 * ctx_set_pixel_u8:
 * @ctx: a ctx context
 * @x: x coordinate
 * @y: y coordinate
 * @r: red component
 * @g: green component
 * @b: blue component
 * @a: alpha component
 *
 * Set a single pixel to the nearest possible the specified r,g,b,a value. Fast
 * for individual few pixels, slow for doing textures.
 */
void
ctx_set_pixel_u8 (Ctx *ctx, uint16_t x, uint16_t y,
                  uint8_t r, uint8_t g, uint8_t b, uint8_t a);

/**
 * ctx_global_alpha:
 * @ctx: a ctx context
 * @global_alpha: a value in the range 0.0f-1.0f
 *
 * Set a global alpha value that the colors, textures and gradients are
 * modulated by.
 */
void  ctx_global_alpha (Ctx *ctx, float global_alpha);
/**
 * ctx_get_global_alpha:
 * @ctx: a ctx context
 *
 * Returns the current global_alpha value.
 */
float ctx_get_global_alpha     (Ctx *ctx);


/**
 * ctx_stroke_source:
 *
 * The next source definition applies to stroking rather than filling, when a stroke source is
 * not explicitly set the value of filling is inherited.
 */
void ctx_stroke_source (Ctx *ctx); // next source definition is for stroking

/**
 * ctx_rgba_stroke:
 * @ctx: a ctx context
 * @r: red component
 * @g: green component
 * @b: blue component
 * @a: alpha component
 *
 * Set the current stroking color to the color specified by parameters.
 */
void ctx_rgba_stroke   (Ctx *ctx, float r, float g, float b, float a);

/**
 * ctx_rgb_stroke:
 * @ctx: a ctx context
 * @r: red component
 * @g: green component
 * @b: blue component
 *
 * Set the current stroking color to the color specified by parameters.
 */
void ctx_rgb_stroke    (Ctx *ctx, float r, float g, float b);

/**
 * ctx_rgba_u8_stroke:
 * @ctx: a ctx context
 * @r: red component
 * @g: green component
 * @b: blue component
 * @a: alpha component
 *
 * Set the current stroking color to the color specified by parameters.
 */
void ctx_rgba8_stroke  (Ctx *ctx, uint8_t r, uint8_t g, uint8_t b, uint8_t a);

/**
 * ctx_gray_stroke:
 * @gray: value
 *
 * Set a grayscale value, valid value range 0.0-1.0f
 */
void ctx_gray_stroke   (Ctx *ctx, float gray);

/**
 * ctx_drgba_stroke:
 * @ctx: a ctx context
 * @r: red component
 * @g: green component
 * @b: blue component
 * @a: alpha component
 *
 * Set the current stroking color to the color specified by parameters directly
 * in device-space without any color transformation.
 */
void ctx_drgba_stroke  (Ctx *ctx, float r, float g, float b, float a);

/**
 * ctx_cmyka_stroke:
 * @ctx: a ctx context
 * @c: cyan component
 * @m: magenta component
 * @y: yellow component
 * @k: black component
 * @a: alpha component
 *
 * Set the current stroking color to the color specified by parameters.
 */
void ctx_cmyka_stroke  (Ctx *ctx, float c, float m, float y, float k, float a);
/**
 * ctx_cmyk_stroke:
 * @ctx: a ctx context
 * @c: cyan component
 * @m: magenta component
 * @y: yellow component
 * @k: black component
 *
 * Set the current stroking color to the color specified by parameters.
 */
void ctx_cmyk_stroke   (Ctx *ctx, float c, float m, float y, float k);
/**
 * ctx_dcmyka_stroke:
 * @ctx: a ctx context
 * @c: cyan component
 * @m: magenta component
 * @y: yellow component
 * @k: black component
 * @a: alpha component
 *
 * Set the current stroking color to the color specified by parameters directly
 * in device-space without any color transformation.
 */
void ctx_dcmyka_stroke (Ctx *ctx, float c, float m, float y, float k, float a);

/**
 * ctx_dcmyka_stroke:
 * @ctx: a ctx context
 * @c: cyan component
 * @m: magenta component
 * @y: yellow component
 * @k: black component
 *
 * Set the current stroking color to the color specified by parameters directly
 * in device-space without any color transformation.
 */
void ctx_dcmyk_stroke  (Ctx *ctx, float c, float m, float y, float k);

/**
 * ctx_rgba:
 * @ctx: a ctx context
 * @r: red component
 * @g: green component
 * @b: blue component
 * @a: alpha component
 *
 * Set the current fill and text color to the color specified by parameters.
 */

void ctx_rgba  (Ctx *ctx, float r, float g, float b, float a);

/**
 * ctx_rgba:
 * @ctx: a ctx context
 * @r: red component
 * @g: green component
 * @b: blue component
 *
 * Set the current fill and text color to the color specified by parameters.
 */
void ctx_rgb    (Ctx *ctx, float r, float g, float b);

/**
 * ctx_rgba_u8:
 * @ctx: a ctx context
 * @r: red component
 * @g: green component
 * @b: blue component
 * @a: alpha component
 *
 * Set the current fill and text color to the color specified by parameters.
 */
void ctx_rgba8  (Ctx *ctx, uint8_t r, uint8_t g, uint8_t b, uint8_t a);

/**
 * ctx_rgba_u8:
 * @ctx: a ctx context
 * @gray:  value
 *
 * Set the current fill and text color to the grayscale color specified by parameters.
 */
void ctx_gray   (Ctx *ctx, float gray);

/**
 * ctx_drgba:
 * @ctx: a ctx context
 * @r: red component
 * @g: green component
 * @b: blue component
 * @a: alpha component
 *
 * Set the current fill and text color to the color specified by parameters in
 * device space, without any color transforms.
 */
void ctx_drgba  (Ctx *ctx, float r, float g, float b, float a);

/**
 * ctx_cmyka:
 * @ctx: a ctx context
 * @c: cyan component
 * @m: magenta component
 * @y: yellow component
 * @k: black component
 * @a: alpha component
 *
 * Set the current fill and text color to the grayscale color specified by parameters.
 */
void ctx_cmyka  (Ctx *ctx, float c, float m, float y, float k, float a);

/**
 * ctx_cmyk:
 * @ctx: a ctx context
 * @c: cyan component
 * @m: magenta component
 * @y: yellow component
 * @k: black component
 *
 * Set the current fill and text color to the grayscale color specified by parameters.
 */
void ctx_cmyk   (Ctx *ctx, float c, float m, float y, float k);

/**
 * ctx_dcmyka:
 * @ctx: a ctx context
 * @c: cyan component
 * @m: magenta component
 * @y: yellow component
 * @k: black component
 * @a: alpha component
 *
 * Set the current fill and text color to the color specified by parameters in
 * device space, without any color transforms.
 */
void ctx_dcmyka (Ctx *ctx, float c, float m, float y, float k, float a);

/**
 * ctx_dcmyk:
 * @ctx: a ctx context
 * @c: cyan component
 * @m: magenta component
 * @y: yellow component
 * @k: black component
 *
 * Set the current fill and text color to the color specified by parameters in
 * device space, without any color transforms.
 */
void ctx_dcmyk  (Ctx *ctx, float c, float m, float y, float k);

/* there is also getters for colors, by first setting a color in one format and getting
 * it with another color conversions can be done
 */
void ctx_get_rgba   (Ctx *ctx, float *rgba);
void ctx_get_graya  (Ctx *ctx, float *ya);
void ctx_get_drgba  (Ctx *ctx, float *drgba);
void ctx_get_cmyka  (Ctx *ctx, float *cmyka);
void ctx_get_dcmyka (Ctx *ctx, float *dcmyka);



/**
 * ctx_linear_gradient:
 * Change the source to a linear gradient from x0,y0 to x1 y1, an empty gradient
 * is interpreted as grayscale from black to white, add stops with ctx_gradient_add_stop to specify a custom gradient.
 */
void ctx_linear_gradient (Ctx *ctx, float x0, float y0, float x1, float y1);

/**
 * ctx_radial_gradient:
 * Change the source to a radial gradient from a circle x0,y0 with radius r0 to an outher circle x1, y1 with radius r1. (NOTE: currently ctx is only using the second circles origin, both radiuses are in use.)
 */
void ctx_radial_gradient (Ctx *ctx, float x0, float y0, float r0,
                          float x1, float y1, float r1);

/**
 * ctx_conic_gradient:
 * Change the source to a conic/conic gradient cenetered at cx,cy with gradient starting at angle start_angle.
 */
void ctx_conic_gradient (Ctx *ctx, float cx, float cy, float start_angle, float cycles);


/**
 * ctx_gradient_add_stop_rgba:
 *
 * Add an RGBA gradient stop to the current gradient at position pos.
 *
 */
void ctx_gradient_add_stop_rgba (Ctx *ctx, float pos, float r, float g, float b, float a);
#define ctx_gradient_add_stop ctx_gradient_add_stop_rgba // compat

/**
 * ctx_gradient_add_stop_u8:
 *
 * Add an RGBA gradient stop to the current gradient at position pos.
 */
void ctx_gradient_add_stop_u8 (Ctx *ctx, float pos, uint8_t r, uint8_t g, uint8_t b, uint8_t a);

/* ctx_define_texture:
 */
void ctx_define_texture (Ctx        *ctx,
                         const char *eid,
                         int         width,
                         int         height,
                         int         stride,
                         int         format,
                         void       *data,
                         char       *ret_eid);

/** ctx_drop_eid:
 *
 * Drops the relevant texture eid freeing resources.
 */
void ctx_drop_eid (Ctx *ctx, const char *eid);

/* ctx_source_transform:
 */
void
ctx_source_transform (Ctx *ctx, float a, float b,  float c,
                      float d, float e, float f, 
                      float g, float h, float i); 

/* ctx_source_transform_matrix:
 */
void
ctx_source_transform_matrix (Ctx *ctx, CtxMatrix *matrix);


/*** h3: shadow */

/**
 * ctx_shadow_rgba:
 * sets the color of the shadow-blur, use a < 1.0 for softer blur
 */
void ctx_shadow_rgba      (Ctx *ctx, float r, float g, float b, float a);

/**
 * ctx_shadow_blur:
 * set the shadow_blur radius, which in HTML5 canvas is double the standard
 * deviation of an expected gaussian blur.
 */
void ctx_shadow_blur      (Ctx *ctx, float stddev_x_2);

/**
 * ctx_shadow_offset_x:
 * specify offset of generated shadow blur
 */
void ctx_shadow_offset_x  (Ctx *ctx, float x);

/**
 * ctx_shadow_offset_y:
 * specify offset of generated shadow blur
 */
void ctx_shadow_offset_y  (Ctx *ctx, float y);


/**
 * ctx_width:
 *
 * Returns the width of the ctx canvas in pixels.
 */
int   ctx_width                (Ctx *ctx);

/**
 * ctx_height:
 *
 * Returns the height of the ctx canvas in pixels.
 */
int   ctx_height               (Ctx *ctx);


/**
 * ctx_get_transform:
 *
 * Returns the currently set transform matrix coefficients in a..i.
 */
void  ctx_get_transform        (Ctx *ctx, float *a, float *b,
                                float *c, float *d,
                                float *e, float *f,
                                float *g, float *h,
                                float *i);

/**
 * ctx_clip_extents::
 *
 * Returns the upper-left, x0,y0  and lower-right x1,y1 coordinates for the currently set clip bounding box,
 * useful for getting culling bounds.
 */
void
ctx_clip_extents (Ctx *ctx, float *x0, float *y0,
                            float *x1, float *y1);

/**
 * ctx_get_image_data:
 *
 * Get a copy of pixel values - depending on backend might cause rendering
 * temporary rendering or providing the data directly from an immediate buffer.
 *
 */
void
ctx_get_image_data (Ctx *ctx, int sx, int sy, int sw, int sh,
                    CtxPixelFormat format, int dst_stride,
                    uint8_t *dst_data);

/**
 * ctx_put_image_data:
 *
 * draws a texture at the given coordinates.
 */
void
ctx_put_image_data (Ctx *ctx, int w, int h, int stride, int format,
                    uint8_t *data,
                    int ox, int oy,
                    int dirtyX, int dirtyY,
                    int dirtyWidth, int dirtyHeight);


/**
 * ctx_texture_load:
 *
 * loads an image file from disk into texture, returning pixel width, height
 * and eid, the eid is based on the path; not the contents - avoiding doing
 * sha1 checksum of contents. The width and height of the image is returned
 * along with the used eid, width height or eid can be NULL if we
 * do not care about their values.
 */
void ctx_texture_load (Ctx        *ctx,
                       const char *path,
                       int        *width,
                       int        *height,
                       char       *eid);

/**
 * ctx_texture:
 *
 * sets the paint source to be a texture by eid
 */
void ctx_texture              (Ctx *ctx, const char *eid, float x, float y);

/**
 * ctx_draw_texture:
 *
 * draw a texture at given coordinates with specified with/height.
 */
void ctx_draw_texture         (Ctx *ctx, const char *eid, float x, float y, float w, float h);

void ctx_draw_texture_clipped (Ctx *ctx, const char *eid, float x, float y, float w, float h, float sx, float sy, float swidth, float sheight);

/**
 * ctx_draw_image:
 *
 * Load an possibly cache an image from a png/svg/jpg sufficed file, at x, y with width and height
 */
void ctx_draw_image           (Ctx *ctx, const char *path, float x, float y, float w, float h);

void ctx_draw_image_clipped   (Ctx *ctx, const char *path, float x, float y, float w, float h, float sx, float sy, float swidth, float sheight);

/**
 * ctx_set_texture_source:
 *
 * used by the render threads of fb and sdl backends.
 */
void ctx_set_texture_source (Ctx *ctx, Ctx *texture_source);

/**
 * ctx_set_texture_cache:
 *
 * used when sharing cache state of eids between clients
 */
void ctx_set_texture_cache (Ctx *ctx, Ctx *texture_cache);


/**
 * ctx_pixel_format_bits_per_pixel:
 *
 * Returns bits per pixel for a pixel format.
 */
int ctx_pixel_format_bits_per_pixel (CtxPixelFormat format); // bits per pixel
/**
 * ctx_pixel_format_get_stride:
 *
 * Computes the stride of a scanline in bytes, rounding up to the neatest byte for 1/2/4bits
 */
int ctx_pixel_format_get_stride     (CtxPixelFormat format, int width);

/**
 * ctx_hasher_new:
 *
 * Create a new hashing context, for use with replays from another master context.
 */
Ctx *ctx_hasher_new          (int width, int height, int cols, int rows, CtxDrawlist *drawlist);

/**
 * ctx_hasher_get_hash:
 */
uint32_t ctx_hasher_get_hash (Ctx *ctx, int col, int row);



/*  In progress for implementing rubber spacing or space bands, in one pass layouting.
 */
void ctx_deferred_scale       (Ctx *ctx, const char *name, float x, float y);

/**
 * ctx_deferred_translate:
 */
void ctx_deferred_translate   (Ctx *ctx, const char *name, float x, float y);
/**
 * ctx_deferred_move_to:
 */
void ctx_deferred_move_to     (Ctx *ctx, const char *name, float x, float y);
/**
 * ctx_deferred_rel_line_to:
 */
void ctx_deferred_rel_line_to (Ctx *ctx, const char *name, float x, float y);
/**
 * ctx_deferred_rel_move_to:
 */
void ctx_deferred_rel_move_to (Ctx *ctx, const char *name, float x, float y);
/**
 * ctx_deferred_rectangle:
 */
void ctx_deferred_rectangle   (Ctx *ctx, const char *name, float x, float y,
                                                           float width, float height);

void ctx_deferred_round_rectangle   (Ctx *ctx, const char *name, float x, float y,
                                                           float width, float height,
                                                           float radius);


/**
 * ctx_resolve:
 *
 */
void ctx_resolve              (Ctx *ctx, const char *name,
                               void (*set_dim) (Ctx *ctx,
                                                void *userdata,
                                                const char *name,
                                                int         count,
                                                float *x,
                                                float *y,
                                                float *width,
                                                float *height),
                               void *userdata);


/* these are configuration flags for a ctx renderer, not all
 * flags are applicable for all rendereres, the cb backend
 * has the widest support currently.
 */
typedef enum CtxFlags {
  //CTX_FLAG_DEFAULTS    = 0,      // most of these flags apply to cb-backend
  CTX_FLAG_GRAY8         = 1 << 0, // use GRAY8, implies LOWFI
  CTX_FLAG_HASH_CACHE    = 1 << 1, // use a hashcache to determine which parts to redraw, implied by LOWFI
  CTX_FLAG_LOWFI         = 1 << 2, // lower res preview for performance during animations
  CTX_FLAG_SUBPIXEL      = 1 << 3, // re-render with subpixel precision
  CTX_FLAG_DAMAGE_CONTROL = 1 << 4,
  CTX_FLAG_SHOW_FPS      = 1 << 5, // possibly show fps in titlebar or shown in overlay
  CTX_FLAG_KEEP_DATA     = 1 << 6, // keep existing fb-data instead of doing an initial clear
  CTX_FLAG_RENDER_THREAD = 1 << 7,  // do rendering in separate thread
                                     
  CTX_FLAG_POINTER       = 1 << 8,  //  draw software cursor

  CTX_FLAG_HANDLE_ESCAPES = 1 << 9, // applies to parser config
  CTX_FLAG_FORWARD_EVENTS = 1 << 10, // applies to parser config

  CTX_FLAG_SYNC           = 1 << 11, // applies to ctx-backend
  CTX_FLAG_COMPRESS       = 1 << 12, // applies to ctx-backend
  CTX_FLAG_FULL_FB        = 1 << 13, // only valid with a fb pointer passed in,
                                     // swap/render the whole frame when drawlist
                                     // is full, this is slower than hash cache
                                     // unless geometry is simpler, can not be
                                     // combined with CTX_FLAG_HASH_CACHE
} CtxFlags;

typedef struct CtxCbConfig {
   CtxPixelFormat format;
   int            buffer_size;
   void          *buffer;    // scratch buffer should be in sram if possible
   int            flags;

   int            chunk_size; // number of entries in drawlist before flush,
                              // full flush on end-frame

   void          *fb;        // if provided is a backing-fb for rendering
                             // buffer comes on top as a scratch area;
   void          *user_data; // provided to the callback functions
                             //
   void (*set_pixels)     (Ctx *ctx, void *user_data, 
                           int x, int y, int w, int h, void *buf);
   void  *set_pixels_user_data;

   // runs after all subregion updates in renderer thread
   // if CTX_FLAG_RENDER_THREAD then this is run in renderer thread.
   int (*update_fb)       (Ctx *ctx, void *user_data, int x, int y, int w, int h);
   void  *update_fb_user_data;

   // run as an idle call in render thread, between chunks
   int (*intra)       (Ctx *ctx, void *user_data);
   void  *intra_user_data;

   int  (*renderer_init)  (Ctx *ctx, void *user_data); // return non 0 on failure to init
   void  *renderer_init_user_data;
   void (*renderer_idle)  (Ctx *ctx, void *user_data);
   void  *renderer_idle_user_data;

   void (*renderer_stop)  (Ctx *ctx, void *user_data);
   void  *renderer_stop_user_data;

   void (*consume_events) (Ctx *ctx, void *user_data); // runs in the main (not renderer thread)
   void  *consume_events_user_data;

   void (*set_fullscreen)  (Ctx *ctx, void *user_data, int fullscreen);
   void *set_fullscreen_user_data;

   int  (*get_fullscreen)  (Ctx *ctx, void *user_data);
   void *get_fullscreen_user_data;

   void (*windowtitle)     (Ctx *ctx, void *user_data, const char *utf8);
   void *windowtitle_user_data;

   void (*set_clipboard)  (Ctx *ctx, void *user_data, const char *text);
   void *set_clipboard_user_data;

   char *(*get_clipboard) (Ctx *ctx, void *user_data);
   void *get_clipboard_user_data;

   void *padding[10];
} CtxCbConfig;

/**
 * ctx_new_cb:
 */
Ctx *ctx_new_cb (int width, int height, CtxCbConfig *config);

/**
 * ctx_new_cb_old:
 */
Ctx *ctx_new_cb_old (int width, int height, CtxPixelFormat format,
                 void (*set_pixels) (Ctx *ctx, void *user_data, 
                                     int x, int y, int w, int h, void *buf),
                 void *set_pixels_user_data,
                 int (*update_fb) (Ctx *ctx, void *user_data),
                 void *update_fb_user_data,
                 int   memory_budget,
                 void *scratch_fb,
                 int   flags);

/**
 * ctx_cb_set_flags:
 */
void ctx_cb_set_flags (Ctx *ctx, int flags);
/**
 * ctx_cb_get_flags:
 */
int  ctx_cb_get_flags (Ctx *ctx);

/**
 * ctx_cb_set_memory_budget:
 */
void ctx_cb_set_memory_budget (Ctx *ctx, int memory_budget);

/***h3: serialization/formatting API */

typedef enum CtxFormatterFlag{
  CTX_FORMATTER_FLAG_NONE = 0,
  CTX_FORMATTER_FLAG_LONGFORM = (1<<0),
  CTX_FORMATTER_FLAG_FLUSH = (1<<1),
} CtxFormatterFlag;

/**
 * ctx_render_string:
 * @ctx: a ctx context containing a drawing
 * @longform: if 1 use human readable encoding, 0 for compact
 * @retlen: optional location to store length of returned string.
 *
 * returns an allocated string containing serialization of the drawing in ctx,
 * free with ctx_free.
 */
char *ctx_render_string (Ctx *ctx, CtxFormatterFlag flags, int *retlen);

/**
 * ctx_render_stream:
 * @ctx: a ctx context containing a drawing
 * @stream: stream to serialize to
 * @longform: 0 for compact, 1 for human readable
 *
 * Render a ctx context to a stream.
 */
void ctx_render_stream  (Ctx *ctx, FILE *stream, CtxFormatterFlag flags);

/**
 * ctx_render_fd:
 * @ctx: a ctx context containing a drawing
 * @fd: an open file descriptor to write to
 * @longform: 0 for compact, 1 for human readable
 *
 * Render a ctx context to an open file.
 */
void ctx_render_fd      (Ctx *ctx, int fd, CtxFormatterFlag flag);

/**
 * ctx_render_ctx:
 * @ctx: a source context containing a drawing
 * @d_ctx: destination context.
 *
 * Render one context onto another.
 */
void ctx_render_ctx     (Ctx *ctx, Ctx *d_ctx);

/**
 * ctx_render_ctx_textures:
 * @ctx: a source context containing a drawing
 * @d_ctx: destination context.
 *
 * Render one context onto another, without doing any drawing - but using all used textures.
 */
void ctx_render_ctx_textures (Ctx *ctx, Ctx *d_ctx);

typedef enum
{
  CTX_JOIN_BEVEL = 0,
  CTX_JOIN_ROUND = 1,
  CTX_JOIN_MITER = 2
} CtxLineJoin;
void ctx_line_join  (Ctx *ctx, CtxLineJoin        join);
CtxLineJoin          ctx_get_line_join        (Ctx *ctx);

typedef enum
{
  CTX_CAP_NONE   = 0,
  CTX_CAP_ROUND  = 1,
  CTX_CAP_SQUARE = 2
} CtxLineCap;
void       ctx_line_cap     (Ctx *ctx, CtxLineCap         cap);
CtxLineCap ctx_get_line_cap (Ctx *ctx);

typedef enum
{
  CTX_EXTEND_NONE    = 0,
  CTX_EXTEND_REPEAT  = 1,
  CTX_EXTEND_REFLECT = 2,
  CTX_EXTEND_PAD     = 3
} CtxExtend;
#define CTX_EXTEND_LAST CTX_EXTEND_PAD
void      ctx_extend     (Ctx *ctx, CtxExtend extend);
CtxExtend ctx_get_extend (Ctx *ctx);

void ctx_gradient_add_stop_string (Ctx *ctx, float pos, const char *color);


/*** h3: text */

/**
 * ctx_font_size:
 */
void  ctx_font_size       (Ctx *ctx, float x);

/**
 * ctx_get_font_size:
 *
 * Returns the current font_size.
 */
float ctx_get_font_size        (Ctx *ctx);

/**
 * ctx_font_family:
 */
void  ctx_font_family     (Ctx *ctx, const char *font_family);

/**
 * ctx_font:
 */
void  ctx_font            (Ctx *ctx, const char *font);

/**
 * ctx_get_font:
 *
 * Returns the currently set font.
 */
const char *ctx_get_font       (Ctx *ctx);

int
ctx_font_extents (Ctx   *ctx,
                  float *ascent,
                  float *descent,
                  float *line_gap);


/**
 * ctx_wrap_left:
 *
 * Specify left edge of text column for word-wrapping, default value is 0.0f for
 * none.
 */
void ctx_wrap_left   (Ctx *ctx, float x);
float ctx_get_wrap_left        (Ctx *ctx);

/**
 * ctx_wrap_right:
 *
 * Specify right edge of text column for word-wrapping, default value is 0.0f for
 * none.
 */
void ctx_wrap_right  (Ctx *ctx, float x);
float ctx_get_wrap_right       (Ctx *ctx);

/**
 * ctx_line_height:
 *
 * Specify right edge of text column for word-wrapping, default value is 0.0f for
 * none.
 */
void ctx_line_height (Ctx *ctx, float y);
float ctx_get_line_height      (Ctx *ctx);



typedef enum
{
  CTX_TEXT_ALIGN_START = 0,  // in mrg these didnt exist
  CTX_TEXT_ALIGN_END,        // but left/right did
  CTX_TEXT_ALIGN_JUSTIFY, // not handled in ctx
  CTX_TEXT_ALIGN_CENTER,
  CTX_TEXT_ALIGN_LEFT,
  CTX_TEXT_ALIGN_RIGHT
} CtxTextAlign;
void         ctx_text_align     (Ctx *ctx, CtxTextAlign align);
CtxTextAlign ctx_get_text_align (Ctx *ctx);

typedef enum
{
  CTX_TEXT_BASELINE_ALPHABETIC = 0,
  CTX_TEXT_BASELINE_TOP,
  CTX_TEXT_BASELINE_HANGING,
  CTX_TEXT_BASELINE_MIDDLE,
  CTX_TEXT_BASELINE_IDEOGRAPHIC,
  CTX_TEXT_BASELINE_BOTTOM
} CtxTextBaseline;
void            ctx_text_baseline     (Ctx *ctx, CtxTextBaseline    baseline);
CtxTextBaseline ctx_get_text_baseline (Ctx *ctx);


typedef enum
{
  CTX_TEXT_DIRECTION_INHERIT = 0,
  CTX_TEXT_DIRECTION_LTR,
  CTX_TEXT_DIRECTION_RTL
} CtxTextDirection;
void               ctx_text_direction       (Ctx *ctx, CtxTextDirection   direction);
CtxTextDirection   ctx_get_text_direction   (Ctx *ctx);

/**
 * ctx_text:
 *
 * Draw the UTF8 string at current position, wrapping if ctx_wrap_left and ctx_wrap_right have been set.
 */
void  ctx_text          (Ctx        *ctx,
                         const char *utf8);

/**
 * ctx_text_width:
 *
 * returns the total horizontal advance if string had been rendered
 */
float ctx_text_width    (Ctx        *ctx,
                         const char *utf8);


/**
 * ctx_get_font_name:
 *
 * Get the name a font is registered under, substrings are often sufficient for
 * specifying, iterating up from 0 until NULL is retrieved is the current way
 * to enumerate registered fonts.
 */
const char *ctx_get_font_name (Ctx *ctx, int no);

int   ctx_load_font_file     (Ctx *ctx, const char *name, const char *path);
int   ctx_load_font          (Ctx *ctx, const char *name, const char *data, unsigned int length);

float ctx_glyph_width   (Ctx *ctx, int glyph_id);

/*
 * low level glyph drawing calls, unless you are integrating harfbuzz
 * you probably want to use ctx_text instead.
 */
typedef struct _CtxGlyph CtxGlyph;
struct
_CtxGlyph
{
  uint32_t index; // glyph index in font
  float    x;
  float    y;
};

/**
 * ctx_glyph_allocate:
 */
CtxGlyph *ctx_glyph_allocate     (int n_glyphs);

/**
 * ctx_glyph_free:
 */
void      ctx_glyph_free         (Ctx *ctx, CtxGlyph *glyphs);

/**
 * ctx_glyph_id:
 */
int       ctx_glyph_id           (Ctx *ctx, uint32_t glyphid, int stroke);

int       ctx_glyph_unichar      (Ctx *ctx, uint32_t unichar, int stroke);

void      ctx_glyphs             (Ctx      *ctx,
                                  CtxGlyph *glyphs,
                                  int       n_glyphs);

int       ctx_glyph_lookup       (Ctx *ctx, uint32_t unichar);

/**
 */
void  ctx_glyphs_stroke  (Ctx        *ctx,
                          CtxGlyph   *glyphs,
                          int         n_glyphs);


/**
 * ctx_dirty_rect:
 *
 * Query the dirtied bounding box of drawing commands thus far.
 */
void  ctx_dirty_rect (Ctx *ctx, int *x, int *y, int *width, int *height);


#ifdef CTX_X86_64
int ctx_x86_64_level (void);
#endif


enum _CtxModifierState
{
  CTX_MODIFIER_STATE_SHIFT   = (1<<0),
  CTX_MODIFIER_STATE_CONTROL = (1<<1),
  CTX_MODIFIER_STATE_ALT     = (1<<2),
  CTX_MODIFIER_STATE_BUTTON1 = (1<<3),
  CTX_MODIFIER_STATE_BUTTON2 = (1<<4),
  CTX_MODIFIER_STATE_BUTTON3 = (1<<5),
  CTX_MODIFIER_STATE_DRAG    = (1<<6), // pointer button is down (0 or any)
};
typedef enum _CtxModifierState CtxModifierState;

enum _CtxScrollDirection
{
  CTX_SCROLL_DIRECTION_UP,
  CTX_SCROLL_DIRECTION_DOWN,
  CTX_SCROLL_DIRECTION_LEFT,
  CTX_SCROLL_DIRECTION_RIGHT
};
typedef enum _CtxScrollDirection CtxScrollDirection;

typedef struct _CtxEvent CtxEvent;

/**
 * ctx_handle_events:
 *
 * Calls the backends consume events, to consume events until none pending, this also
 * deals with client events.
 */
void ctx_handle_events (Ctx *ctx);

/**
 * ctx_need_redraw:
 *
 * Returns non 0 if the ctx context needs a redraw, as queued by ctx_queue_draw since the last
 * end_frame.
 *
 */
int ctx_need_redraw (Ctx *ctx);

/**
 * ctx_queue_draw:
 *
 * Mark the current ctx output as dirty and in need of recomputation.
 */
void ctx_queue_draw (Ctx *ctx);

/**
 * ctx_exit:
 * @ctx: a context
 *
 * Set a flag on the context indicating that execution is finished.
 */
void ctx_exit (Ctx *ctx);
/**
 * ctx_has_exited:
 * @ctx: a context
 *
 * returns 1 if ctx_exit() has been called
 */
int  ctx_has_exited (Ctx *ctx);

/**
 * ctx_reset_has_exited:
 * @ctx: a context
 *
 * Reset the has_exited flag of context.
 */
void ctx_reset_has_exited (Ctx *ctx);

/**
 * ctx_ticks:
 *
 * Returns number of microseconds since startup.
 */
unsigned long ctx_ticks (void);

/**
 * ctx_ms:
 * @ctx: a context
 *
 * Returns number of milliseconds since startup.
 */
uint32_t ctx_ms         (Ctx *ctx);

/**
 * ctx_strhash:
 * @string: a string
 *
 * Returns an integer that is a hash/compaction of string
 */
uint32_t    ctx_strhash    (const char *string);

/**
 * ctx_str_decode:
 * @number: an integer, previously encoded with ctx_strhash
 *
 * Returns a constant string decoding the number, if it is reversible -
 * this is mostly possible with strings <5 or 6 chars.
 */
const char *ctx_str_decode (uint32_t number);

/**
 * ctx_set_float:
 * @ctx: a context
 * @hash: a key - as created with ctx_strhash
 * @value: a value
 *
 * Set a key/value mapping maintained in the graphics context to value, the values
 * are maintained by ctx_save.
 */
void        ctx_set_float  (Ctx *ctx, uint32_t hash, float value);

/**
 * ctx_get_float:
 * @ctx: a context
 * @hash: a key - as created with ctx_strhash
 *
 * Get a key/value mapping maintained in the graphics context to value as previously
 * set by ctx_set_float, if not set returns 0.0f.
 */
float       ctx_get_float  (Ctx *ctx, uint32_t hash);

/**
 * ctx_is_set:
 * @ctx: a context
 * @hash: a key - as created with ctx_strhash
 *
 * Check if a value has been set at all (when 0.0f needs to be explicitly detected).
 */
int         ctx_is_set     (Ctx *ctx, uint32_t hash);

/**
 * ctx_set_clipboard:
 * @ctx: a context
 * @text: new clipboard text.
 *
 * Set clipboard text, this is the copy action in copy+paste.
 */
void ctx_set_clipboard (Ctx *ctx, const char *text);

/**
 * ctx_get_clipboard:
 * @ctx: a context
 *
 * Get clipboard contents or NULL if a result the result should be freed with ctx_free.
 */
char *ctx_get_clipboard (Ctx *ctx);

/**
 * ctx_windowtitle:
 *
 * Set window title.
 */
void ctx_windowtitle (Ctx *ctx, const char *text);

void _ctx_events_init     (Ctx *ctx);
typedef struct _CtxIntRectangle CtxIntRectangle;
struct _CtxIntRectangle {
  int x;
  int y;
  int width;
  int height;
};
typedef struct _CtxFloatRectangle CtxFloatRectangle;
struct _CtxFloatRectangle {
  float x;
  float y;
  float width;
  float height;
};


typedef void (*CtxCb) (CtxEvent *event,
                       void     *data,
                       void     *data2);

enum _CtxEventType {
  CTX_PRESS        = 1 << 0,
  CTX_MOTION       = 1 << 1,
  CTX_RELEASE      = 1 << 2,
  CTX_ENTER        = 1 << 3,
  CTX_LEAVE        = 1 << 4,
  CTX_TAP          = 1 << 5,
  CTX_TAP_AND_HOLD = 1 << 6,

  /* NYI: SWIPE, ZOOM ROT_ZOOM, */

  CTX_DRAG_PRESS   = 1 << 7,
  CTX_DRAG_MOTION  = 1 << 8,
  CTX_DRAG_RELEASE = 1 << 9,
  CTX_KEY_PRESS    = 1 << 10,
  CTX_KEY_DOWN     = 1 << 11,
  CTX_KEY_UP       = 1 << 12,
  CTX_SCROLL       = 1 << 13,
  CTX_MESSAGE      = 1 << 14,
  CTX_DROP         = 1 << 15,

  CTX_SET_CURSOR   = 1 << 16, // used internally

  /* client should store state - preparing
                                 * for restart
                                 */
  CTX_POINTER  = (CTX_PRESS | CTX_MOTION | CTX_RELEASE | CTX_DROP),
  CTX_TAPS     = (CTX_TAP | CTX_TAP_AND_HOLD),
  CTX_CROSSING = (CTX_ENTER | CTX_LEAVE),
  CTX_DRAG     = (CTX_DRAG_PRESS | CTX_DRAG_MOTION | CTX_DRAG_RELEASE),
  CTX_KEY      = (CTX_KEY_DOWN | CTX_KEY_UP | CTX_KEY_PRESS),
  CTX_MISC     = (CTX_MESSAGE),
  CTX_ANY      = (CTX_POINTER | CTX_DRAG | CTX_CROSSING | CTX_KEY | CTX_MISC | CTX_TAPS),
};
typedef enum _CtxEventType CtxEventType;

#define CTX_CLICK   CTX_PRESS   // SHOULD HAVE MORE LOGIC
typedef struct _CtxClient CtxClient;

struct _CtxEvent {
  CtxEventType  type;
  uint32_t time;
  Ctx     *ctx;
  int stop_propagate; /* when set - propagation is stopped */

  CtxModifierState state;

  int     device_no; /* 0 = left mouse button / virtual focus */
                     /* 1 = middle mouse button */
                     /* 2 = right mouse button */
                     /* 4 = first multi-touch .. (NYI) */

  float   device_x; /* untransformed (device) coordinates  */
  float   device_y;

  /* coordinates; and deltas for motion/drag events in user-coordinates: */
  float   x;
  float   y;
  float   start_x; /* start-coordinates (press) event for drag, */
  float   start_y; /*    untransformed coordinates */
  float   prev_x;  /* previous events coordinates */
  float   prev_y;
  float   delta_x; /* x - prev_x, redundant - but often useful */
  float   delta_y; /* y - prev_y, redundant - ..  */


  unsigned int unicode; /* only valid for key-events, re-use as keycode? */
  const char *string;   /* as key can be "up" "down" "space" "backspace" "a" "b" "ø" etc .. */
                        /* this is also where the message is delivered for
                         * MESSAGE events
                         *
                         * and the data for drop events are delivered
                         *
                         */
                         /* XXX lifetime of this string should be longer
                         * than the events, preferably interned. XXX
                         * maybe add a flag for this?
                         */
  int owns_string; /* if 1 call free.. */
  CtxScrollDirection scroll_direction;

  // would be nice to add the bounding box of the hit-area causing
  // the event, making for instance scissored enter/leave repaint easier.
};

// layer-event "layer"  motion x y device_no 


/**
 * ctx_add_timeout_full:
 * @ctx: a context
 * @ms: milliseconds to elapse before calling 
 * @idle_cb: callback function to call
 * @idle_data: data for callback
 * @destroy_notify: function to call to destroy something when timeout is over
 * @destroy_data: data passed to destroy_notify
 *
 * add an idle callback, which is a function taking a ctx context and a the
 * provided idle_data, returning 1 if the callback should continue being
 * registered and called again after a new delay and 0 if it should be removed.
 *
 * Returns an integer handle that can be used with ctx_remove_idle.
 */
int   ctx_add_timeout_full   (Ctx *ctx, int ms, int (*idle_cb)(Ctx *ctx, void *idle_data), void *idle_data,
                              void (*destroy_notify)(void *destroy_data), void *destroy_data);
/**
 * ctx_add_timeout:
 * @ctx: a context
 * @ms: milliseconds to elapse before calling 
 * @idle_cb: callback function to call
 * @idle_data: data for callback
 *
 * add an idle callback, which is a function taking a ctx context and a the
 * provided idle_data, returning 1 if the callback should continue being
 * registered and called again after a new delay and 0 if it should be removed.
 *
 * Returns an integer handle that can be used with ctx_remove_idle.
 */
int   ctx_add_timeout        (Ctx *ctx, int ms, int (*idle_cb)(Ctx *ctx, void *idle_data), void *idle_data);

/**
 * ctx_add_idle_full:
 * @ctx: a context
 * @idle_cb: callback function to call
 * @idle_data: data for callback
 * @destroy_notify: function to call to destroy something when timeout is over
 * @destroy_data: data passed to destroy_notify
 *
 * add an idle callback, which is a function taking a ctx context and a the provided idle_data, returning
 * 1 if the callback should continue being registered and 0 if it should be removed.
 *
 * Returns an integer handle that can be used with ctx_remove_idle.
 */
int   ctx_add_idle_full      (Ctx *ctx, int (*idle_cb)(Ctx *ctx, void *idle_data), void *idle_data,
                              void (*destroy_notify)(void *destroy_data), void *destroy_data);
/**
 * ctx_add_idle:
 * @ctx: a context
 * @idle_cb: callback function to call
 * @idle_data: data for callback
 *
 * add an idle callback, which is a function taking a ctx context and a the provided idle_data, returning
 * 1 if the callback should continue being registered and 0 if it should be removed.
 *
 * Returns an integer handle that can be used with ctx_remove_idle.
 */
int   ctx_add_idle           (Ctx *ctx, int (*idle_cb)(Ctx *ctx, void *idle_data), void *idle_data);

/**
 * ctx_remove_idle:
 * @ctx: a context
 * @handle: a handle referencing a timeout or idle.
 *
 * Remove a previously registered idle callback / timeout.
 */
void  ctx_remove_idle        (Ctx *ctx, int handle);


typedef void (*CtxDestroyNotify) (void *data);

/**
 * ctx_add_key_binding_full:
 * @ctx: a context
 * @key: a string describing the keybinding like "a" "space" "shift-f3"
 * @action: a string action to take, is passed to cb as data2
 * @label: title to use in ui
 * @cb: callback function to call
 * @cb_data: data for callback
 * @destroy_notify: function to call to destroy something when timeout is over
 * @destroy_data: data passed to destroy_notify
 *
 * Register a key binding, with a finalizer.
 */
void ctx_add_key_binding_full (Ctx *ctx,
                               const char *key,
                               const char *action,
                               const char *label,
                               CtxCb       cb,
                               void       *cb_data,
                               CtxDestroyNotify destroy_notify,
                               void       *destroy_data);

/**
 * ctx_add_key_binding:
 * @ctx: a context
 * @key: a string describing the keybinding like "a" "space" "shift-f3"
 * @action: a string action to take, is passed to cb as data2
 * @label: title to use in ui
 * @cb: callback function to call
 * @cb_data: data for callback
 *
 * Register a key binding, with a finalizer.
 */
void ctx_add_key_binding (Ctx *ctx,
                          const char *key,
                          const char *action,
                          const char *label,
                          CtxCb cb,
                          void  *cb_data);





void ctx_add_hit_region (Ctx *ctx, const char *id);

/**
 * ctx_listen_full:
 * @ctx: a context
 * @x: bounding box x coordinate
 * @y: bounding box y coordinate
 * @width: bounding box width
 * @height: bounding box height
 * @types:
 * @cb: callback function to call
 * @cb_data1: data for callback
 * @cb_data2: second data for callback
 * @finalize: function to call to destroy something when timeout is over
 * @finalize_data: data passed to destroy_notify
 *
 * Register a pointer event with a finalizer.
 */
void ctx_listen_full (Ctx     *ctx,
                      float    x,
                      float    y,
                      float    width,
                      float    height,
                      CtxEventType  types,
                      CtxCb    cb,
                      void    *data1,
                      void    *data2,
                      void   (*finalize)(void *listen_data, void *listen_data2,
                                         void *finalize_data),
                      void    *finalize_data);
void  ctx_event_stop_propagate (CtxEvent *event);


/**
 * ctx_listen:
 * @ctx: a context
 * @types:
 * @cb: callback function to call
 * @cb_data1: data for callback
 * @cb_data2: second data for callback
 * @finalize: function to call to destroy something when timeout is over
 * @finalize_data: data passed to destroy_notify
 *
 * Register a pointer event handler, using the extent of the current path
 * as bounding box.
 */
void  ctx_listen               (Ctx          *ctx,
                                CtxEventType  types,
                                CtxCb         cb,
                                void*         data1,
                                void*         data2);
/**
 * ctx_listen_with_finalize:
 * @ctx: a context
 * @types:
 * @cb: callback function to call
 * @cb_data1: data for callback
 * @cb_data2: second data for callback
 * @finalize: function to call to destroy something when timeout is over
 * @finalize_data: data passed to destroy_notify
 *
 * Register a pointer event with a finalizer, using the extent of the current path
 * as bounding box.
 */
void  ctx_listen_with_finalize (Ctx          *ctx,
                                CtxEventType  types,
                                CtxCb         cb,
                                void*         data1,
                                void*         data2,
                      void   (*finalize)(void *listen_data, void *listen_data2,
                                         void *finalize_data),
                      void    *finalize_data);

CtxEvent *ctx_get_event (Ctx *ctx);
void      ctx_get_event_fds (Ctx *ctx, int *fd, int *count);


int   ctx_pointer_is_down (Ctx *ctx, int no);
int   ctx_touch_count (Ctx *ctx); // returns number of touch points / pointers down
float ctx_pointer_x (Ctx *ctx);
float ctx_pointer_y (Ctx *ctx);
void  ctx_freeze (Ctx *ctx);
void  ctx_thaw   (Ctx *ctx);
int   ctx_events_frozen (Ctx *ctx);
void  ctx_events_clear_items (Ctx *ctx);
/* The following functions drive the event delivery, registered callbacks
 * are called in response to these being called.
 */

int ctx_key_down  (Ctx *ctx, unsigned int keyval,
                   const char *string, uint32_t time);
int ctx_key_up    (Ctx *ctx, unsigned int keyval,
                   const char *string, uint32_t time);
int ctx_key_press (Ctx *ctx, unsigned int keyval,
                   const char *string, uint32_t time);
int ctx_scrolled  (Ctx *ctx, float x, float y, CtxScrollDirection scroll_direction, uint32_t time);
void ctx_incoming_message (Ctx *ctx, const char *message, long time);
int ctx_pointer_motion    (Ctx *ctx, float x, float y, int device_no, uint32_t time);
int ctx_pointer_release   (Ctx *ctx, float x, float y, int device_no, uint32_t time);
int ctx_pointer_press     (Ctx *ctx, float x, float y, int device_no, uint32_t time);
int ctx_pointer_drop      (Ctx *ctx, float x, float y, int device_no, uint32_t time,
                           char *string);





typedef enum CtxBackendType {
  CTX_BACKEND_NONE,
  CTX_BACKEND_CTX,
  CTX_BACKEND_RASTERIZER,
  CTX_BACKEND_HASHER,
  CTX_BACKEND_TERM,
  CTX_BACKEND_DRAWLIST,
  CTX_BACKEND_PDF,
  CTX_BACKEND_CB,
} CtxBackendType;

CtxBackendType ctx_backend_type (Ctx *ctx);

typedef struct _CtxBuffer CtxBuffer;
CtxBuffer *ctx_buffer_new_for_data (void *data, int width, int height,
                                    int stride,
                                    CtxPixelFormat pixel_format,
                                    void (*freefunc) (void *pixels, void *user_data),
                                    void *user_data);

int   ctx_client_resize        (Ctx *ctx, int id, int width, int height);
void  ctx_client_set_font_size (Ctx *ctx, int id, float font_size);
float ctx_client_get_font_size (Ctx *ctx, int id);
void  ctx_client_maximize      (Ctx *ctx, int id);
void ctx_client_focus          (Ctx *ctx, int id);

typedef struct _VT VT;
void vt_feed_keystring    (VT *vt, CtxEvent *event, const char *str);
void vt_paste             (VT *vt, const char *str);
char *vt_get_selection    (VT *vt);
long vt_rev               (VT *vt);
void vt_set_line_spacing (VT *vt, float line_spacing);
void vt_set_baseline     (VT *vt, float baseline);
int  vt_has_blink         (VT *vt);
int ctx_vt_had_alt_screen (VT *vt);
int  vt_get_cursor_x      (VT *vt);
int  vt_get_cursor_y      (VT *vt);
void vt_draw (VT *vt, Ctx *ctx, double x0, double y0, int has_focus);

void vt_set_palette(int color_no, uint8_t red, uint8_t green, uint8_t blue);

typedef struct _CtxList CtxList;
CtxList *ctx_clients (Ctx *ctx);

void ctx_set_fullscreen (Ctx *ctx, int val);
int ctx_get_fullscreen (Ctx *ctx);


typedef enum CtxClientFlags {
  CTX_CLIENT_UI_RESIZABLE = 1<<0,
  CTX_CLIENT_CAN_LAUNCH   = 1<<1,
  CTX_CLIENT_MAXIMIZED    = 1<<2,
  CTX_CLIENT_ICONIFIED    = 1<<3,
  CTX_CLIENT_SHADED       = 1<<4,
  CTX_CLIENT_TITLEBAR     = 1<<5,
  CTX_CLIENT_LAYER2       = 1<<6,  // used for having a second set
                                   // to draw - useful for splitting
                                   // scrolled and HUD items
                                   // with HUD being LAYER2
                                  
  CTX_CLIENT_KEEP_ALIVE   = 1<<7,  // do not automatically
  CTX_CLIENT_FINISHED     = 1<<8,  // do not automatically
                                   // remove after process quits
  CTX_CLIENT_PRELOAD      = 1<<9,
  CTX_CLIENT_LIVE         = 1<<10
} CtxClientFlags;
typedef void (*CtxClientFinalize)(CtxClient *client, void *user_data);

int ctx_client_id (CtxClient *client);
int ctx_client_flags (CtxClient *client);
VT *ctx_client_vt (CtxClient *client);
void ctx_client_add_event (CtxClient *client, CtxEvent *event);
const char *ctx_client_title (CtxClient *client);
CtxClient *ctx_client_find (Ctx *ctx, const char *label);

void *ctx_client_userdata (CtxClient *client);

void ctx_client_quit (CtxClient *client);
CtxClient *vt_get_client (VT *vt);
CtxClient *ctx_client_new (Ctx *ctx,
                           const char *commandline,
                           int x, int y, int width, int height,
                           float font_size,
                           CtxClientFlags flags,
                           void *user_data,
                           CtxClientFinalize client_finalize);

CtxClient *ctx_client_new_argv (Ctx *ctx, char **argv, int x, int y, int width, int height, float font_size, CtxClientFlags flags, void *user_data,
                CtxClientFinalize client_finalize);
int ctx_clients_need_redraw (Ctx *ctx);

CtxClient *ctx_client_new_thread (Ctx *ctx, void (*start_routine)(Ctx *ctx, void *user_data),
                                  int x, int y, int width, int height, float font_size, CtxClientFlags flags, void *user_data, CtxClientFinalize finalize);

extern float ctx_shape_cache_rate;
extern int _ctx_max_threads;

CtxEvent *ctx_event_copy (CtxEvent *event);

void  ctx_client_move           (Ctx *ctx, int id, int x, int y);
void  ctx_client_shade_toggle   (Ctx *ctx, int id);
float ctx_client_min_y_pos      (Ctx *ctx);
float ctx_client_max_y_pos      (Ctx *ctx);
void  ctx_client_paste          (Ctx *ctx, int id, const char *str);
char  *ctx_client_get_selection (Ctx *ctx, int id);

void  ctx_client_rev_inc      (CtxClient *client);
long  ctx_client_rev          (CtxClient *client);

int   ctx_clients_active      (Ctx *ctx);

CtxClient *ctx_client_by_id (Ctx *ctx, int id);

int ctx_clients_draw            (Ctx *ctx, int layer2);

void ctx_client_feed_keystring  (CtxClient *client, CtxEvent *event, const char *str);
// need not be public?
void ctx_client_register_events (CtxClient *client, Ctx *ctx, double x0, double y0);

void ctx_client_remove          (Ctx *ctx, CtxClient *client);

int  ctx_client_height           (Ctx *ctx, int id);
int  ctx_client_width            (Ctx *ctx, int id);
int  ctx_client_x                (Ctx *ctx, int id);
int  ctx_client_y                (Ctx *ctx, int id);
void ctx_client_raise_top        (Ctx *ctx, int id);
void ctx_client_raise_almost_top (Ctx *ctx, int id);
void ctx_client_lower_bottom     (Ctx *ctx, int id);
void ctx_client_lower_almost_bottom (Ctx *ctx, int id);
void ctx_client_iconify          (Ctx *ctx, int id);
int  ctx_client_is_iconified     (Ctx *ctx, int id);
void ctx_client_uniconify        (Ctx *ctx, int id);
void ctx_client_maximize         (Ctx *ctx, int id);
int  ctx_client_is_maximized     (Ctx *ctx, int id);
void ctx_client_unmaximize       (Ctx *ctx, int id);
void ctx_client_maximized_toggle (Ctx *ctx, int id);
void ctx_client_shade            (Ctx *ctx, int id);
int  ctx_client_is_shaded        (Ctx *ctx, int id);
void ctx_client_unshade          (Ctx *ctx, int id);
void ctx_client_toggle_maximized (Ctx *ctx, int id);
void ctx_client_shade_toggle     (Ctx *ctx, int id);
void ctx_client_move             (Ctx *ctx, int id, int x, int y);
int  ctx_client_resize           (Ctx *ctx, int id, int width, int height);
void ctx_client_set_opacity      (Ctx *ctx, int id, float opacity);
float ctx_client_get_opacity     (Ctx *ctx, int id);
void ctx_client_set_title        (Ctx *ctx, int id, const char *title);
const char *ctx_client_get_title (Ctx *ctx, int id);




typedef enum
{
  CTX_GRAY           = 1,
  CTX_RGB            = 3,
  CTX_DRGB           = 4,
  CTX_CMYK           = 5,
  CTX_DCMYK          = 6,
  CTX_LAB            = 7,
  CTX_LCH            = 8,
  CTX_GRAYA          = 101,
  CTX_RGBA           = 103,
  CTX_DRGBA          = 104,
  CTX_CMYKA          = 105,
  CTX_DCMYKA         = 106,
  CTX_LABA           = 107,
  CTX_LCHA           = 108,
  CTX_GRAYA_A        = 201,
  CTX_RGBA_A         = 203,
  CTX_RGBA_A_DEVICE  = 204,
  CTX_CMYKA_A        = 205,
  CTX_DCMYKA_A       = 206,
  // RGB  device and  RGB  ?
} CtxColorModel;


enum _CtxColorSpace
{
  CTX_COLOR_SPACE_DEVICE_RGB,
  CTX_COLOR_SPACE_DEVICE_CMYK,
  CTX_COLOR_SPACE_USER_RGB,
  CTX_COLOR_SPACE_USER_CMYK,
  CTX_COLOR_SPACE_TEXTURE
};
typedef enum _CtxColorSpace CtxColorSpace;

#define CTX_COLOR_SPACE_LAST CTX_COLOR_SPACE_TEXTURE

/* sets the color space for a slot, the space is either a string of
 * "sRGB" "rec2020" .. etc or an icc profile.
 *
 * The slots device_rgb and device_cmyk is mostly to be handled outside drawing 
 * code, and user_rgb and user_cmyk is to be used. With no user_cmyk set
 * user_cmyk == device_cmyk.
 *
 * The set profiles follows the graphics state.
 */
void ctx_colorspace (Ctx                 *ctx,
                     CtxColorSpace        space_slot,
                     const unsigned char *data,
                     int                  data_length);


enum _CtxCursor
{
  CTX_CURSOR_UNSET,
  CTX_CURSOR_NONE,
  CTX_CURSOR_ARROW,
  CTX_CURSOR_IBEAM,
  CTX_CURSOR_WAIT,
  CTX_CURSOR_HAND,
  CTX_CURSOR_CROSSHAIR,
  CTX_CURSOR_RESIZE_ALL,
  CTX_CURSOR_RESIZE_N,
  CTX_CURSOR_RESIZE_S,
  CTX_CURSOR_RESIZE_E,
  CTX_CURSOR_RESIZE_NE,
  CTX_CURSOR_RESIZE_SE,
  CTX_CURSOR_RESIZE_W,
  CTX_CURSOR_RESIZE_NW,
  CTX_CURSOR_RESIZE_SW,
  CTX_CURSOR_MOVE
};
typedef enum _CtxCursor CtxCursor;

/* to be used immediately after a ctx_listen or ctx_listen_full causing the
 * cursor to change when hovering the listen area.
 */
void ctx_listen_set_cursor (Ctx      *ctx,
                            CtxCursor cursor);


/* lower level cursor setting that is independent of ctx event handling
 */
void      ctx_set_cursor           (Ctx *ctx, CtxCursor cursor);
CtxCursor ctx_get_cursor           (Ctx *ctx);


/* draw the ctx logo */
void ctx_logo (Ctx *ctx, float x, float y, float dim);

/*** h2: parsing */

/**
 * ctx_parse:
 *
 * Parse ctx-syntax interpreting the commands in ctx.
 *
 */
void ctx_parse            (Ctx *ctx, const char *string);

/**
 * ctx_parse_animation:
 * elapsed_time the 
 *
 * Parses a string containg ctx protocol data, including an overlayed
 * scene and key-framing synax.
 *
 * pass in the scene_no you expect to render in the pointer for scene_no
 * actual rendered scene is returned here.
 *
 * elapsed time for scene to render, if we are beyond the specified scene
 * adjust elapsed_time to reflect elapsed time in actually rendered scene.
 */
void
ctx_parse_animation (Ctx *ctx, const char *string,
                     float *elapsed_time, 
                     int   *scene_no);


/* configuration of a parser, with callbacks for customization
 * of behavior.
 */
typedef struct CtxParserConfig {
  int      width;       // <- maybe should be float?
  int      height;
  float    cell_width;
  float    cell_height;
  int      cursor_x;
  int      cursor_y;
  int      flags;
  void    *user_data;

  int   (*set_prop)       (Ctx *ctx, void *user_data, uint32_t key, const char *data, int len);
  void   *set_prop_user_data;

  int   (*get_prop)       (Ctx *ctx, void *user_data, const char *key, char **data, int *len);
  void   *get_prop_user_data;

  void  (*start_frame)    (Ctx *ctx, void *user_data);
  void   *start_frame_user_data;

  void  (*end_frame)      (Ctx *ctx, void *user_data);
  void   *end_frame_user_data;

  void  (*response) (Ctx *ctx, void *user_data, char *response, int len);
  void   *response_user_data;

} CtxParserConfig;

typedef struct _CtxParser CtxParser;
  CtxParser *ctx_parser_new (
  Ctx       *ctx,
  CtxParserConfig *config);

void ctx_parser_destroy (CtxParser *parser);

int ctx_parser_neutral (CtxParser *parser);

void
ctx_parser_set_size (CtxParser *parser,
                     int        width,
                     int        height,
                     float      cell_width,
                     float      cell_height);

void ctx_parser_feed_bytes (CtxParser *parser, const char *data, int count);


typedef struct _CtxBackend CtxBackend;
struct _CtxBackend
{
  Ctx                      *ctx;

  void  (*process)         (Ctx *ctx, const CtxCommand *entry);

  /* for interactive/event-handling backends */
  void  (*start_frame)     (Ctx *ctx);
  void  (*end_frame)       (Ctx *ctx);
  void  (*consume_events)  (Ctx *ctx);
  void  (*get_event_fds)   (Ctx *ctx, int *fd, int *count);

  void  (*set_windowtitle) (Ctx *ctx, const char *text);

  char *(*get_clipboard)   (Ctx *ctx);
  void  (*set_clipboard)   (Ctx *ctx, const char *text);
  void  (*destroy)         (void *backend); /* the free pointers are abused as the differentiatior
                                               between different backends   */
  void  (*reset_caches)    (Ctx *ctx);
  CtxFlags                  flags;
  CtxBackendType            type;
  void                     *user_data; // not used by ctx core
};
void ctx_set_backend  (Ctx *ctx, void *backend);
void *ctx_get_backend (Ctx *ctx);

typedef struct _CtxIterator CtxIterator;

CtxCommand *ctx_iterator_next (CtxIterator *iterator);
void
ctx_iterator_init (CtxIterator  *iterator,
                   CtxDrawlist  *drawlist,  // replace with Ctx*  ? XXX XXX ? for one less type in public API
                   int           start_pos, 
                   int           flags);    // need exposing for font bits
int ctx_iterator_pos (CtxIterator *iterator);


// utility calls not tied directly to Ctx
int
ctx_get_contents (const char     *path,
                   unsigned char **contents,
                   long           *length);
int
ctx_get_contents2 (const char     *path,
                   unsigned char **contents,
                   long           *length,
                   long            max_len);

typedef struct _CtxSHA1 CtxSHA1;

void
ctx_bin2base64 (const void *bin,
                size_t      bin_length,
                char       *ascii);
int
ctx_base642bin (const char    *ascii,
                int           *length,
                unsigned char *bin);


void ctx_matrix_apply_transform (const CtxMatrix *m, float *x, float *y);
void ctx_matrix_apply_transform_distance (const CtxMatrix *m, float *x, float *y);
void ctx_matrix_invert          (CtxMatrix *m);
void ctx_matrix_identity        (CtxMatrix *matrix);
void ctx_matrix_scale           (CtxMatrix *matrix, float x, float y);
void ctx_matrix_rotate          (CtxMatrix *matrix, float angle);
void ctx_matrix_translate       (CtxMatrix *matrix, float x, float y);
void ctx_matrix_multiply        (CtxMatrix       *result,
                                 const CtxMatrix *t,
                                 const CtxMatrix *s);

/* we already have the start of the file available which disambiguates some
 * of our important supported formats, give preference to magic, then extension
 * then text plain vs binary.
 */
const char *ctx_guess_media_type (const char *path, const char *content, int len);

/* get media-type, with preference towards using extension of path and
 * not reading the data at all.
 */
const char *ctx_path_get_media_type (const char *path);

typedef enum {
  CTX_MEDIA_TYPE_NONE=0,
  CTX_MEDIA_TYPE_TEXT,
  CTX_MEDIA_TYPE_HTML,
  CTX_MEDIA_TYPE_IMAGE,
  CTX_MEDIA_TYPE_VIDEO,
  CTX_MEDIA_TYPE_AUDIO,
  CTX_MEDIA_TYPE_INODE,
  CTX_MEDIA_TYPE_APPLICATION,
} CtxMediaTypeClass;

int ctx_media_type_is_text (const char *media_type);
CtxMediaTypeClass ctx_media_type_class (const char *media_type);


float ctx_term_get_cell_width (Ctx *ctx);
float ctx_term_get_cell_height (Ctx *ctx);

Ctx * ctx_new_pdf (const char *path, float width, float height);
void ctx_render_pdf (Ctx *ctx, const char *path);




//#if CTX_GSTATE_PROTECT
/* sets the current gstate stack (number of unpaired ctx_save calls) as a
 * limit that can not be restored beyond. For now can not be used recursively.
 */
void ctx_gstate_protect   (Ctx *ctx);

/* removes the limit set by ctx_gstate_protect, if insufficient ctx_restore
 * calls have been made, 
 */
void ctx_gstate_unprotect (Ctx *ctx);
//#endif

/* set the logical clock used for the texture eviction policy */
void ctx_set_textureclock (Ctx *ctx, int frame);
int  ctx_textureclock (Ctx *ctx);

/* reinitialize an existing rasterizer with new dimensions/pixel_format
 *
 */
void
ctx_rasterizer_reinit (Ctx  *ctx,
                       void *fb,
                       int   x0,
                       int   y0,
                       int   width,
                       int   height,
                       int   stride,
                       CtxPixelFormat pixel_format);

// causes text commands to directly operate instead of expanding
// in backend
void ctx_set_frontend_text (Ctx *ctx, int frontend_text);

/* this is an interface used when CTX_PTY=0 and CTX_VT=1 , it permits
 * interacting with a single terminal on for instance micro controllers
 */
int ctx_vt_available (Ctx *ctx);
void ctx_vt_write    (Ctx *ctx, uint8_t byte);
int ctx_vt_has_data  (Ctx *ctx);
int ctx_vt_read      (Ctx *ctx);

int ctx_vt_cursor_x (CtxClient *client);
int ctx_vt_cursor_y (CtxClient *client);

/* only valid for rasterizer backend, not kept in graphics state
 */
enum _CtxAntialias
{
  CTX_ANTIALIAS_DEFAULT, //
  CTX_ANTIALIAS_NONE, // non-antialiased
  CTX_ANTIALIAS_FAST, // vertical aa 3 for complex scanlines
  CTX_ANTIALIAS_GOOD, // vertical aa 5 for complex scanlines
  CTX_ANTIALIAS_FULL, // vertical aa 15 for complex scanlines
};
typedef enum _CtxAntialias CtxAntialias;
void         ctx_set_antialias (Ctx *ctx, CtxAntialias antialias);
CtxAntialias ctx_get_antialias (Ctx *ctx);

float ctx_get_stroke_pos (Ctx *ctx);
void ctx_stroke_pos (Ctx *ctx, float x);

// used by fontgen
void _ctx_set_transformation (Ctx *ctx, int transformation);


void ctx_write_png (const char *dst_path, int w, int h, int num_chans, void *data);

//////////////////////////////////////////////////////////////////
#pragma pack(push,1)
struct
  _CtxEntry
{
  uint8_t code;
  union
  {
    float    f[2];
    uint8_t  u8[8];
    int8_t   s8[8];
    uint16_t u16[4];
    int16_t  s16[4];
    uint32_t u32[2];
    int32_t  s32[2];
    uint64_t u64[1]; // unused
  } data; // 9bytes long, we're favoring compactness and correctness
  // over performance. By sacrificing float precision, zeroing
  // first 8bit of f[0] would permit 8bytes long and better
  // aglinment and cacheline behavior.
};
#pragma pack(pop)

typedef enum
{
  CTX_CONT             = '\0', // - contains args from preceding entry
  CTX_NOP              = ' ', //
  CTX_DATA             = '(', // size size-in-entries - u32
  CTX_DATA_REV         = ')', // reverse traversal data marker
  CTX_SET_RGBA_U8      = '*', // r g b a - u8
                   //     ,    UNUSED/RESERVED
  CTX_SET_PIXEL        = '-', // 8bit "fast-path" r g b a x y - u8 for rgba, and u16 for x,y
  // set pixel might want a shorter ascii form with hex-color? or keep it an embedded
  // only option?
                   //     ^    used for unit
                   //     &    UNUSED
                   //     +    UNUSED
                   //     !    UNUSED
                   //     "    start/end string
                   //     #    comment in parser
                   //     $    UNUSED
                   //     %    percent of viewport width or height
                   //     '    start/end string
                   //     .    decimal seperator
                   //     /    UNUSED
                   //     ;    UNUSED
                   //     <    UNUSED
                   //     =    UNUSED/RESERVED
                   //     >    UNUSED
                   //     ?    UNUSED
                   //     \    UNUSED
                   //     ^    PARSER - vh unit
                       // ~    UNUSED/textenc
 
  /* optimizations that reduce the number of entries used,
   * not visible outside the drawlist compression, thus
   * using entries that cannot be used directly as commands
   * since they would be interpreted as numbers - if values>127
   * then the embedded font data is harder to escape.
   */
  CTX_REL_LINE_TO_X4            = '0', // x1 y1 x2 y2 x3 y3 x4 y4   -- s8
  CTX_REL_LINE_TO_REL_CURVE_TO  = '1', // x1 y1 cx1 cy1 cx2 cy2 x y -- s8
  CTX_REL_CURVE_TO_REL_LINE_TO  = '2', // cx1 cy1 cx2 cy2 x y x1 y1 -- s8
  CTX_REL_CURVE_TO_REL_MOVE_TO  = '3', // cx1 cy1 cx2 cy2 x y x1 y1 -- s8
  CTX_REL_LINE_TO_X2            = '4', // x1 y1 x2 y2 -- s16
  CTX_MOVE_TO_REL_LINE_TO       = '5', // x1 y1 x2 y2 -- s16
  CTX_REL_LINE_TO_REL_MOVE_TO   = '6', // x1 y1 x2 y2 -- s16
  CTX_FILL_MOVE_TO              = '7', // x y
  CTX_REL_QUAD_TO_REL_QUAD_TO   = '8', // cx1 x1 cy1 y1 cx1 x2 cy1 y1 -- s8
  CTX_REL_QUAD_TO_S16           = '9', // cx1 cy1 x y                 - s16
  CTX_END_FRAME        = 'X',

  CTX_DEFINE_FONT      = 15,

  CTX_DEFINE_GLYPH     = '@', // unichar width - u32
  CTX_ARC_TO           = 'A', // x1 y1 x2 y2 radius
  CTX_ARC              = 'B', // x y radius angle1 angle2 direction
  CTX_CURVE_TO         = 'C', // cx1 cy1 cx2 cy2 x y
  CTX_PAINT            = 'D', // 
                       // 'E' // scientific notation
  CTX_FILL             = 'F', //
  CTX_RESTORE          = 'G', //
  CTX_HOR_LINE_TO      = 'H', // x
  CTX_DEFINE_TEXTURE   = 'I', // "eid" width height format "data"
  CTX_ROTATE           = 'J', // radians
  CTX_COLOR            = 'K', // model, c1 c2 c3 ca - variable arg count
  CTX_LINE_TO          = 'L', // x y
  CTX_MOVE_TO          = 'M', // x y
  CTX_RESET_PATH       = 'N', //
  CTX_SCALE            = 'O', // xscale yscale
  CTX_NEW_PAGE         = 'P', // - NYI optional page-size
  CTX_QUAD_TO          = 'Q', // cx cy x y
  CTX_VIEW_BOX         = 'R', // x y width height
  CTX_SMOOTH_TO        = 'S', // cx cy x y
  CTX_SMOOTHQ_TO       = 'T', // x y
  CTX_CONIC_GRADIENT   = 'U', // cx cy start_angle cycles
  CTX_VER_LINE_TO      = 'V', // y
  CTX_APPLY_TRANSFORM  = 'W', // a b c d e f g h i j - for set_transform combine with identity
  CTX_TRANSLATE        = 'Y', // x y  

  CTX_CLOSE_PATH2      = 'Z', //
                              
  CTX_START_FRAME      = ':', // 
  CTX_KERNING_PAIR     = '[', // glA glB kerning, glA and glB in u16 kerning in s32
  CTX_COLOR_SPACE      = ']', // IccSlot  data  data_len,
                         //    data can be a string with a name,
                         //    icc data or perhaps our own serialization
                         //    of profile data
  CTX_STROKE_SOURCE    = '_', // next source definition applies to strokes
  CTX_SOURCE_TRANSFORM = '`',
  CTX_REL_ARC_TO       = 'a', // x1 y1 x2 y2 radius
  CTX_CLIP             = 'b',
  CTX_REL_CURVE_TO     = 'c', // cx1 cy1 cx2 cy2 x y
  CTX_LINE_DASH        = 'd', // dashlen0 [dashlen1 ...]
                     //  'e'  -- scientific notation for SVG numbers
  CTX_LINEAR_GRADIENT  = 'f', // x1 y1 x2 y2
  CTX_SAVE             = 'g',
  CTX_REL_HOR_LINE_TO  = 'h', // x
  CTX_TEXTURE          = 'i',
  CTX_PRESERVE         = 'j', // XXX - fix!
  CTX_SET_KEY          = 'k', // - used together with another char to identify
                              //   a key to set
  CTX_REL_LINE_TO      = 'l', // x y
  CTX_REL_MOVE_TO      = 'm', // x y
  CTX_FONT             = 'n', // as used by text parser XXX: move to keyvals?
  CTX_RADIAL_GRADIENT  = 'o', // x1 y1 radius1 x2 y2 radius2
  CTX_GRADIENT_STOP    = 'p', // argument count depends on current color model
  CTX_REL_QUAD_TO      = 'q', // cx cy x y
  CTX_RECTANGLE        = 'r', // x y width height
  CTX_REL_SMOOTH_TO    = 's', // cx cy x y
  CTX_REL_SMOOTHQ_TO   = 't', // x y
  CTX_STROKE           = 'u', // string - utf8 string
  CTX_REL_VER_LINE_TO  = 'v', // y
  CTX_GLYPH            = 'w', // unichar fontsize
  CTX_TEXT             = 'x', // string | kern - utf8 data to shape or horizontal kerning amount
  CTX_IDENTITY         = 'y', // XXX remove? - or reset to baseline.. which is what identity expects
  CTX_CLOSE_PATH       = 'z', //
  CTX_START_GROUP      = '{',
  CTX_END_GROUP        = '}',
  CTX_ROUND_RECTANGLE  = '|', // x y width height radius

  /* though expressed as two chars in serialization we have
   * dedicated byte commands for the setters to keep the dispatch
   * simpler. There is no need for these to be human readable thus we go >128
   * they also should not be emitted when outputting, even compact mode ctx.
   *
   * rasterizer:    &^+
   * font:          @[
   *
   * unused:        !&<=>?: =/\`,
   * reserved:      '"&   #. %^@
   */

  CTX_FILL_RULE        = 128, // kr rule - u8, default = CTX_FILLE_RULE_EVEN_ODD
  CTX_BLEND_MODE       = 129, // kB mode - u32 , default=0

  CTX_MITER_LIMIT      = 130, // km limit - float, default = 0.0

  CTX_LINE_JOIN        = 131, // kj join - u8 , default=0
  CTX_LINE_CAP         = 132, // kc cap - u8, default = 0
  CTX_LINE_WIDTH       = 133, // kw width, default = 2.0
  CTX_GLOBAL_ALPHA     = 134, // ka alpha - default=1.0
  CTX_COMPOSITING_MODE = 135, // kc mode - u32 , default=0

  CTX_FONT_SIZE        = 136, // kf size - float, default=?
  CTX_TEXT_ALIGN       = 137, // kt align - u8, default = CTX_TEXT_ALIGN_START
  CTX_TEXT_BASELINE    = 138, // kb baseline - u8, default = CTX_TEXT_ALIGN_ALPHABETIC
  CTX_TEXT_DIRECTION   = 139, // kd

  CTX_SHADOW_BLUR      = 140, // ks
  CTX_SHADOW_COLOR     = 141, // kC
  CTX_SHADOW_OFFSET_X  = 142, // kx
  CTX_SHADOW_OFFSET_Y  = 143, // ky
  CTX_IMAGE_SMOOTHING  = 144, // kS
  CTX_LINE_DASH_OFFSET = 145, // kD lineDashOffset


  CTX_EXTEND           = 146, // ke u32 extend mode, default=0
  CTX_WRAP_LEFT        = 147, // kL
  CTX_WRAP_RIGHT       = 148, // kR
  CTX_LINE_HEIGHT      = 149, // kH
                              
  CTX_STROKE_POS       = 150, // kp
  CTX_FEATHER          = 151, // kp
                             
#define CTX_LAST_COMMAND  CTX_FEATHER

  CTX_STROKE_RECT      = 200, // strokeRect - only exist in long form
  CTX_FILL_RECT        = 201, // fillRect   - only exist in long form


  CTX_FROM_PREV        = 26, // references previous frame
  CTX_FROM_THIS        = 16, //
} CtxCode;


#pragma pack(push,1)
struct
  _CtxCommand
{
  union
  {
    uint8_t  code;
    CtxEntry entry;
    struct
    {
      uint8_t code;
      float scalex;
      float scaley;
    } scale;
    struct
    {
      uint8_t code;
      uint32_t stringlen;
      uint32_t blocklen;
      uint8_t cont;
      uint8_t data[8]; /* ... and continues */
    } data;
    struct
    {
      uint8_t code;
      uint32_t stringlen;
      uint32_t blocklen;
    } data_rev;
    struct
    {
      uint8_t code;
      uint32_t next_active_mask; // the tilehasher active flags for next
                                 // drawing command
      float pad2;
      uint8_t code_data;
      uint32_t stringlen;
      uint32_t blocklen;
      uint8_t code_cont;
      uint8_t utf8[8]; /* .. and continues */
    } text;
    struct
    {
      uint8_t  code;
      uint32_t key_hash;
      float    pad;
      uint8_t  code_data;
      uint32_t stringlen;
      uint32_t blocklen;
      uint8_t  code_cont;
      uint8_t  utf8[8]; /* .. and continues */
    } set;
    struct
    {
      uint8_t  code;
      uint32_t pad0;
      float    pad1;
      uint8_t  code_data;
      uint32_t stringlen;
      uint32_t blocklen;
      uint8_t  code_cont;
      uint8_t  utf8[8]; /* .. and continues */
    } get;
    struct {
      uint8_t  code;
      uint32_t count; /* better than byte_len in code, but needs to then be set   */
      float    pad1;
      uint8_t  code_data;
      uint32_t byte_len;
      uint32_t blocklen;
      uint8_t  code_cont;
      float    data[2]; /* .. and - possibly continues */
    } line_dash;
    struct {
      uint8_t  code;
      uint32_t space_slot;
      float    pad1;
      uint8_t  code_data;
      uint32_t data_len;
      uint32_t blocklen;
      uint8_t  code_cont;
      uint8_t  data[8]; /* .. and continues */
    } colorspace;
    struct
    {
      uint8_t  code;
      float    x;
      float    y;
      uint8_t  code_data;
      uint32_t stringlen;
      uint32_t blocklen;
      uint8_t  code_cont;
      char     eid[8]; /* .. and continues */
    } texture;
    struct
    {
      uint8_t  code;
      uint32_t width;
      uint32_t height;
      uint8_t  code_cont0;
      uint16_t format;
      uint16_t pad0;
      uint32_t pad1;
      uint8_t  code_data;
      uint32_t stringlen;
      uint32_t blocklen;
      uint8_t  code_cont1;
      char     eid[8]; /* .. and continues */
      // followed by - in variable offset code_Data, data_len, datablock_len, cont, pixeldata
    } define_texture;
    struct
    {
      uint8_t  code;
      float    pad;
      float    pad2;
      uint8_t  code_data;
      uint32_t stringlen;
      uint32_t blocklen;
      uint8_t  code_cont;
      uint8_t  utf8[8]; /* .. and continues */
    } text_stroke;
    struct
    {
      uint8_t  code;
      float    pad;
      float    pad2;
      uint8_t  code_data;
      uint32_t stringlen;
      uint32_t blocklen;
      uint8_t  code_cont;
      uint8_t  utf8[8]; /* .. and continues */
    } set_font;


    struct  // NYI - should be able to do old-style numerals, regular ligs, discretionary ligs
    {       //       the shaper shall look after current glyph, for matching utf8 and flags
            //       matching of 0 length utf8 maps to stylistic replacement.
      uint8_t  code;
      uint32_t glyph;
      uint32_t replacement;
      uint8_t  code_data;  /// < store which open-type flags activate it here
      uint32_t stringlen;
      uint32_t blocklen;
      uint8_t  code_cont;
      uint8_t  utf8[8]; /* .. and continues */
    } ligature;


    struct
    {
      uint8_t code;
      float model;
      float r;
      uint8_t pad1;
      float g;
      float b;
      uint8_t pad2;
      float a;
    } rgba;
    struct
    {
      uint8_t code;
      float model;
      float c;
      uint8_t pad1;
      float m;
      float y;
      uint8_t pad2;
      float k;
      float a;
    } cmyka;
    struct
    {
      uint8_t code;
      float model;
      float g;
      uint8_t pad1;
      float a;
    } graya;

    struct
    {
      uint8_t code;
      float model;
      float c0;
      uint8_t pad1;
      float c1;
      float c2;
      uint8_t pad2;
      float c3;
      float c4;
      uint8_t pad3;
      float c5;
      float c6;
      uint8_t pad4;
      float c7;
      float c8;
      uint8_t pad5;
      float c9;
      float c10;
    } set_color;
    struct
    {
      uint8_t code;
      float x;
      float y;
    } rel_move_to;
    struct
    {
      uint8_t code;
      float x;
      float y;
    } rel_line_to;
    struct
    {
      uint8_t code;
      float x;
      float y;
    } line_to;
    struct
    {
      uint8_t code;
      float cx1;
      float cy1;
      uint8_t pad0;
      float cx2;
      float cy2;
      uint8_t pad1;
      float x;
      float y;
    } rel_curve_to;
    struct
    {
      uint8_t code;
      float x;
      float y;
    } move_to;
    struct
    {
      uint8_t code;
      float cx1;
      float cy1;
      uint8_t pad0;
      float cx2;
      float cy2;
      uint8_t pad1;
      float x;
      float y;
    } curve_to;
    struct
    {
      uint8_t code;
      float x1;
      float y1;
      uint8_t pad0;
      float r1;
      float x2;
      uint8_t pad1;
      float y2;
      float r2;
    } radial_gradient;
    struct
    {
      uint8_t code;
      float x1;
      float y1;
      uint8_t pad0;
      float x2;
      float y2;
    } linear_gradient;
    struct
    {
      uint8_t code;
      float x;
      float y;
      uint8_t pad0;
      float start_angle;
      float cycles;
    } conic_gradient;
    struct
    {
      uint8_t code;
      float x;
      float y;
      uint8_t pad0;
      float width;
      float height;
      uint8_t pad1;
      float radius;
    } rectangle;
    struct {
      uint8_t code;
      float x;
      float y;
      uint8_t pad0;
      float width;
      float height;
    } view_box;

    struct
    {
      uint8_t code;
      uint16_t glyph_before;
      uint16_t glyph_after;
       int32_t amount;
    } kern;


    struct
    {
      uint8_t code;
      uint32_t glyph;
      uint32_t advance; // * 256
    } define_glyph;

    struct
    {
      uint8_t code;
      uint8_t rgba[4];
      uint16_t x;
      uint16_t y;
    } set_pixel;
    struct
    {
      uint8_t code;
      float cx;
      float cy;
      uint8_t pad0;
      float x;
      float y;
    } quad_to;
    struct
    {
      uint8_t code;
      float cx;
      float cy;
      uint8_t pad0;
      float x;
      float y;
    } rel_quad_to;
    struct
    {
      uint8_t code;
      float x;
      float y;
      uint8_t pad0;
      float radius;
      float angle1;
      uint8_t pad1;
      float angle2;
      float direction;
    }
    arc;
    struct
    {
      uint8_t code;
      float x1;
      float y1;
      uint8_t pad0;
      float x2;
      float y2;
      uint8_t pad1;
      float radius;
    }
    arc_to;
    /* some format specific generic accesors:  */
    struct
    {
      uint8_t code;
      float   x0;
      float   y0;
      uint8_t pad0;
      float   x1;
      float   y1;
      uint8_t pad1;
      float   x2;
      float   y2;
      uint8_t pad2;
      float   x3;
      float   y3;
      uint8_t pad3;
      float   x4;
      float   y4;
    } c;
    struct
    {
      uint8_t code;
      float   a0;
      float   a1;
      uint8_t pad0;
      float   a2;
      float   a3;
      uint8_t pad1;
      float   a4;
      float   a5;
      uint8_t pad2;
      float   a6;
      float   a7;
      uint8_t pad3;
      float   a8;
      float   a9;
    } f;
    struct
    {
      uint8_t  code;
      uint32_t a0;
      uint32_t a1;
      uint8_t  pad0;
      uint32_t a2;
      uint32_t a3;
      uint8_t  pad1;
      uint32_t a4;
      uint32_t a5;
      uint8_t  pad2;
      uint32_t a6;
      uint32_t a7;
      uint8_t  pad3;
      uint32_t a8;
      uint32_t a9;
    } u32;
    struct
    {
      uint8_t  code;
      uint64_t a0;
      uint8_t  pad0;
      uint64_t a1;
      uint8_t  pad1;
      uint64_t a2;
      uint8_t  pad2;
      uint64_t a3;
      uint8_t  pad3;
      uint64_t a4;
    } u64;
    struct
    {
      uint8_t code;
      int32_t a0;
      int32_t a1;
      uint8_t pad0;
      int32_t a2;
      int32_t a3;
      uint8_t pad1;
      int32_t a4;
      int32_t a5;
      uint8_t pad2;
      int32_t a6;
      int32_t a7;
      uint8_t pad3;
      int32_t a8;
      int32_t a9;
    } s32;
    struct
    {
      uint8_t code;
      int16_t a0;
      int16_t a1;
      int16_t a2;
      int16_t a3;
      uint8_t pad0;
      int16_t a4;
      int16_t a5;
      int16_t a6;
      int16_t a7;
      uint8_t pad1;
      int16_t a8;
      int16_t a9;
      int16_t a10;
      int16_t a11;
      uint8_t pad2;
      int16_t a12;
      int16_t a13;
      int16_t a14;
      int16_t a15;
      uint8_t pad3;
      int16_t a16;
      int16_t a17;
      int16_t a18;
      int16_t a19;
    } s16;
    struct
    {
      uint8_t code;
      uint16_t a0;
      uint16_t a1;
      uint16_t a2;
      uint16_t a3;
      uint8_t pad0;
      uint16_t a4;
      uint16_t a5;
      uint16_t a6;
      uint16_t a7;
      uint8_t pad1;
      uint16_t a8;
      uint16_t a9;
      uint16_t a10;
      uint16_t a11;
      uint8_t pad2;
      uint16_t a12;
      uint16_t a13;
      uint16_t a14;
      uint16_t a15;
      uint8_t pad3;
      uint16_t a16;
      uint16_t a17;
      uint16_t a18;
      uint16_t a19;
    } u16;
    struct
    {
      uint8_t code;
      uint8_t a0;
      uint8_t a1;
      uint8_t a2;
      uint8_t a3;
      uint8_t a4;
      uint8_t a5;
      uint8_t a6;
      uint8_t a7;
      uint8_t pad0;
      uint8_t a8;
      uint8_t a9;
      uint8_t a10;
      uint8_t a11;
      uint8_t a12;
      uint8_t a13;
      uint8_t a14;
      uint8_t a15;
      uint8_t pad1;
      uint8_t a16;
      uint8_t a17;
      uint8_t a18;
      uint8_t a19;
      uint8_t a20;
      uint8_t a21;
      uint8_t a22;
      uint8_t a23;
    } u8;
    struct
    {
      uint8_t code;
      int8_t a0;
      int8_t a1;
      int8_t a2;
      int8_t a3;
      int8_t a4;
      int8_t a5;
      int8_t a6;
      int8_t a7;
      uint8_t pad0;
      int8_t a8;
      int8_t a9;
      int8_t a10;
      int8_t a11;
      int8_t a12;
      int8_t a13;
      int8_t a14;
      int8_t a15;
      uint8_t pad1;
      int8_t a16;
      int8_t a17;
      int8_t a18;
      int8_t a19;
      int8_t a20;
      int8_t a21;
      int8_t a22;
      int8_t a23;
    } s8;
  };
  CtxEntry next_entry; // also pads size of CtxCommand slightly.
};
#pragma pack(pop)
#define ctx_arg_string()  ((char*)&entry[2].data.u8[0])

/* access macros for nth argument of a given type when packed into
 * an CtxEntry pointer in current code context
 */
#define ctx_arg_float(no) entry[(no)>>1].data.f[(no)&1]
#define ctx_arg_u64(no)   entry[(no)].data.u64[0]
#define ctx_arg_u32(no)   entry[(no)>>1].data.u32[(no)&1]
#define ctx_arg_s32(no)   entry[(no)>>1].data.s32[(no)&1]
#define ctx_arg_u16(no)   entry[(no)>>2].data.u16[(no)&3]
#define ctx_arg_s16(no)   entry[(no)>>2].data.s16[(no)&3]
#define ctx_arg_u8(no)    entry[(no)>>3].data.u8[(no)&7]
#define ctx_arg_s8(no)    entry[(no)>>3].data.s8[(no)&7]
#define ctx_arg_string()  ((char*)&entry[2].data.u8[0])
////////////////////////////////////////////////////////////////////

/*
 * removes any backend specific (SDL) clipboard integration and
 * use internal fallback if internal_clipboard is set to 1, currently
 * setting internal clipboard is irreversible.
 */
void ctx_internal_clipboard (Ctx *ctx, int internal_clipboard);

void ctx_wait_for_renderer (Ctx *ctx);


typedef enum CtxSubPixel
{
  CTX_SUBPIXEL_NONE = 0,
  CTX_SUBPIXEL_HRGB = 1,
  CTX_SUBPIXEL_HBGR = 2,
  CTX_SUBPIXEL_VRGB = 3,
  CTX_SUBPIXEL_VBGR = 4
} CtxSubPixel;

/* UTF8 utility functions:
 */
int ctx_utf8_strlen (const char *s);
int ctx_utf8_len (const unsigned char first_byte);
uint32_t ctx_utf8_to_unichar (const char *input);
int      ctx_unichar_to_utf8 (uint32_t  ch, uint8_t  *dest);
const char *ctx_utf8_skip (const char *s, int utf8_length);

int ctx_has_focus (Ctx *ctx);

int       ctx_get_major_version (void);
int       ctx_get_minor_version (void);
int       ctx_get_micro_version (void);

/* currently unused */
void      ctx_set_render_threads   (Ctx *ctx, int n_threads);
int       ctx_get_render_threads   (Ctx *ctx);

#if CTX_ASSERT==1
#define ctx_assert(a)  if(!(a)){fprintf(stderr,"%s:%i assertion failed\n", __FUNCTION__, __LINE__);  }
#else
#define ctx_assert(a)
#endif

#ifdef __cplusplus
}
#endif
#endif

#include "ctx-split.h"
