#ifndef M5GFX_LVGL_FONT_COMPAT_H
#define M5GFX_LVGL_FONT_COMPAT_H

/*
 * M5GFX LVGL compatibility shim.
 *
 * Default behavior: use a tiny stub for lv_font_*.c generated files.
 * If you want to use the real LVGL headers, define M5GFX_USE_REAL_LVGL.
 */

#if defined(M5GFX_USE_REAL_LVGL)
    #if defined(__has_include)
        #if __has_include("lvgl/lvgl.h")
            #include "lvgl/lvgl.h"
            #define M5GFX_USING_REAL_LVGL 1
        #elif __has_include_next("lvgl.h")
            #include_next "lvgl.h"
            #define M5GFX_USING_REAL_LVGL 1
        #endif
    #endif
#endif

#ifndef M5GFX_USING_REAL_LVGL

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// config
#ifndef LV_COLOR_DEPTH
#define LV_COLOR_DEPTH 16
#endif

#ifndef LV_BIG_ENDIAN_SYSTEM
#define LV_BIG_ENDIAN_SYSTEM 0
#endif

#ifndef LVGL_VERSION_MAJOR
#define LVGL_VERSION_MAJOR 9
#endif
#ifndef LVGL_VERSION_MINOR
#define LVGL_VERSION_MINOR 3
#endif
#ifndef LVGL_VERSION_PATCH
#define LVGL_VERSION_PATCH 0
#endif

#ifndef LV_VERSION_CHECK
#define LV_VERSION_CHECK(x, y, z) \
    ((LVGL_VERSION_MAJOR > (x)) || \
    ((LVGL_VERSION_MAJOR == (x)) && (LVGL_VERSION_MINOR > (y))) || \
    ((LVGL_VERSION_MAJOR == (x)) && (LVGL_VERSION_MINOR == (y)) && (LVGL_VERSION_PATCH >= (z))))
#endif

#ifndef LV_ATTRIBUTE_LARGE_CONST
#define LV_ATTRIBUTE_LARGE_CONST
#endif

#ifndef LV_FONT_SUBPX_NONE
#define LV_FONT_SUBPX_NONE 0
#endif

#ifndef LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
#define LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY 0
#endif

#ifndef LV_FONT_FMT_TXT_CMAP_SPARSE_TINY
#define LV_FONT_FMT_TXT_CMAP_SPARSE_TINY 1
#endif

#ifndef LV_FONT_FMT_TXT_CMAP_FORMAT0_FULL
#define LV_FONT_FMT_TXT_CMAP_FORMAT0_FULL 2
#endif

#ifndef LV_FONT_FMT_TXT_CMAP_SPARSE_FULL
#define LV_FONT_FMT_TXT_CMAP_SPARSE_FULL 3
#endif

typedef struct {
    uint32_t adv_w;
    uint16_t box_w;
    uint16_t box_h;
    int16_t ofs_x;
    int16_t ofs_y;
    uint8_t bpp;
    uint8_t is_placeholder;
} lv_font_glyph_dsc_t;

typedef struct {
    uint32_t bitmap_index;
    uint16_t adv_w;
    uint8_t box_w;
    uint8_t box_h;
    int8_t ofs_x;
    int8_t ofs_y;
} lv_font_fmt_txt_glyph_dsc_t;

typedef struct {
    uint32_t range_start;
    uint16_t range_length;
    uint16_t glyph_id_start;
    const uint16_t *unicode_list;
    const void * glyph_id_ofs_list;
    uint16_t list_length;
    uint8_t type;
} lv_font_fmt_txt_cmap_t;

typedef struct {
    const int8_t *class_pair_values;
    const uint8_t *left_class_mapping;
    const uint8_t *right_class_mapping;
    uint8_t left_class_cnt;
    uint8_t right_class_cnt;
} lv_font_fmt_txt_kern_classes_t;

typedef struct {
    const uint8_t *glyph_bitmap;
    const lv_font_fmt_txt_glyph_dsc_t *glyph_dsc;
    const lv_font_fmt_txt_cmap_t *cmaps;
    const lv_font_fmt_txt_kern_classes_t *kern_dsc;
    uint16_t kern_scale;
    uint8_t cmap_num;
    uint8_t bpp;
    uint8_t kern_classes;
    uint8_t bitmap_format;
} lv_font_fmt_txt_dsc_t;

struct _lv_font_t;
typedef bool (*lv_font_get_glyph_dsc_cb_t)(const struct _lv_font_t *, lv_font_glyph_dsc_t *, uint32_t, uint32_t);
typedef const uint8_t *(*lv_font_get_bitmap_cb_t)(const struct _lv_font_t *, uint32_t);

#if LV_BIG_ENDIAN_SYSTEM
typedef struct {
    uint32_t reserved_2: 16;    /**< Reserved to be used later*/
    uint32_t stride: 16;        /**< Number of bytes in a row*/
    uint32_t h: 16;
    uint32_t w: 16;
    uint32_t flags: 16;         /**< Image flags, see `lv_image_flags_t`*/
    uint32_t cf : 8;            /**< Color format: See `lv_color_format_t`*/
    uint32_t magic: 8;          /**< Magic number. Must be LV_IMAGE_HEADER_MAGIC*/
} lv_image_header_t;
#else
typedef struct {
    uint32_t magic: 8;          /**< Magic number. Must be LV_IMAGE_HEADER_MAGIC*/
    uint32_t cf : 8;            /**< Color format: See `lv_color_format_t`*/
    uint32_t flags: 16;         /**< Image flags, see `lv_image_flags_t`*/

    uint32_t w: 16;
    uint32_t h: 16;
    uint32_t stride: 16;        /**< Number of bytes in a row*/
    uint32_t reserved_2: 16;    /**< Reserved to be used later*/
} lv_image_header_t;
#endif

typedef enum {
    LV_COLOR_FORMAT_UNKNOWN           = 0,

    LV_COLOR_FORMAT_RAW               = 0x01,
    LV_COLOR_FORMAT_RAW_ALPHA         = 0x02,

    /*<=1 byte (+alpha) formats*/
    LV_COLOR_FORMAT_L8                = 0x06,
    LV_COLOR_FORMAT_I1                = 0x07,
    LV_COLOR_FORMAT_I2                = 0x08,
    LV_COLOR_FORMAT_I4                = 0x09,
    LV_COLOR_FORMAT_I8                = 0x0A,
    LV_COLOR_FORMAT_A8                = 0x0E,

    /*2 byte (+alpha) formats*/
    LV_COLOR_FORMAT_RGB565            = 0x12,
    LV_COLOR_FORMAT_ARGB8565          = 0x13,   /**< Not supported by sw renderer yet. */
    LV_COLOR_FORMAT_RGB565A8          = 0x14,   /**< Color array followed by Alpha array*/
    LV_COLOR_FORMAT_AL88              = 0x15,   /**< L8 with alpha >*/

    /*3 byte (+alpha) formats*/
    LV_COLOR_FORMAT_RGB888            = 0x0F,
    LV_COLOR_FORMAT_ARGB8888          = 0x10,
    LV_COLOR_FORMAT_XRGB8888          = 0x11,

    /*Formats not supported by software renderer but kept here so GPU can use it*/
    LV_COLOR_FORMAT_A1                = 0x0B,
    LV_COLOR_FORMAT_A2                = 0x0C,
    LV_COLOR_FORMAT_A4                = 0x0D,
    LV_COLOR_FORMAT_ARGB1555          = 0x16,
    LV_COLOR_FORMAT_ARGB4444          = 0x17,
    LV_COLOR_FORMAT_ARGB2222          = 0X18,

    /* reference to https://wiki.videolan.org/YUV/ */
    /*YUV planar formats*/
    LV_COLOR_FORMAT_YUV_START         = 0x20,
    LV_COLOR_FORMAT_I420              = LV_COLOR_FORMAT_YUV_START,  /*YUV420 planar(3 plane)*/
    LV_COLOR_FORMAT_I422              = 0x21,  /*YUV422 planar(3 plane)*/
    LV_COLOR_FORMAT_I444              = 0x22,  /*YUV444 planar(3 plane)*/
    LV_COLOR_FORMAT_I400              = 0x23,  /*YUV400 no chroma channel*/
    LV_COLOR_FORMAT_NV21              = 0x24,  /*YUV420 planar(2 plane), UV plane in 'V, U, V, U'*/
    LV_COLOR_FORMAT_NV12              = 0x25,  /*YUV420 planar(2 plane), UV plane in 'U, V, U, V'*/

    /*YUV packed formats*/
    LV_COLOR_FORMAT_YUY2              = 0x26,  /*YUV422 packed like 'Y U Y V'*/
    LV_COLOR_FORMAT_UYVY              = 0x27,  /*YUV422 packed like 'U Y V Y'*/

    LV_COLOR_FORMAT_YUV_END           = LV_COLOR_FORMAT_UYVY,

    LV_COLOR_FORMAT_PROPRIETARY_START = 0x30,

    LV_COLOR_FORMAT_NEMA_TSC_START    = LV_COLOR_FORMAT_PROPRIETARY_START,
    LV_COLOR_FORMAT_NEMA_TSC4         = LV_COLOR_FORMAT_NEMA_TSC_START,
    LV_COLOR_FORMAT_NEMA_TSC6         = 0x31,
    LV_COLOR_FORMAT_NEMA_TSC6A        = 0x32,
    LV_COLOR_FORMAT_NEMA_TSC6AP       = 0x33,
    LV_COLOR_FORMAT_NEMA_TSC12        = 0x34,
    LV_COLOR_FORMAT_NEMA_TSC12A       = 0x35,
    LV_COLOR_FORMAT_NEMA_TSC_END      = LV_COLOR_FORMAT_NEMA_TSC12A,

    /*Color formats in which LVGL can render*/
#if LV_COLOR_DEPTH == 1
    LV_COLOR_FORMAT_NATIVE            = LV_COLOR_FORMAT_I1,
    LV_COLOR_FORMAT_NATIVE_WITH_ALPHA = LV_COLOR_FORMAT_I1,
#elif LV_COLOR_DEPTH == 8
    LV_COLOR_FORMAT_NATIVE            = LV_COLOR_FORMAT_L8,
    LV_COLOR_FORMAT_NATIVE_WITH_ALPHA = LV_COLOR_FORMAT_AL88,
#elif LV_COLOR_DEPTH == 16
    LV_COLOR_FORMAT_NATIVE            = LV_COLOR_FORMAT_RGB565,
    LV_COLOR_FORMAT_NATIVE_WITH_ALPHA = LV_COLOR_FORMAT_RGB565A8,
#elif LV_COLOR_DEPTH == 24
    LV_COLOR_FORMAT_NATIVE            = LV_COLOR_FORMAT_RGB888,
    LV_COLOR_FORMAT_NATIVE_WITH_ALPHA = LV_COLOR_FORMAT_ARGB8888,
#elif LV_COLOR_DEPTH == 32
    LV_COLOR_FORMAT_NATIVE            = LV_COLOR_FORMAT_XRGB8888,
    LV_COLOR_FORMAT_NATIVE_WITH_ALPHA = LV_COLOR_FORMAT_ARGB8888,
#else
#error "LV_COLOR_DEPTH should be 1, 8, 16, 24 or 32"
#endif

} lv_color_format_t;

/** Represents an area of the screen.*/
typedef struct {
    int32_t x1;
    int32_t y1;
    int32_t x2;
    int32_t y2;
} lv_area_t;

/* Forward declarations for draw buffer types used in callback typedefs */
struct _lv_draw_buf_t;
typedef struct _lv_draw_buf_t lv_draw_buf_t;

typedef void * (*lv_draw_buf_malloc_cb)(size_t size, lv_color_format_t color_format);

typedef void (*lv_draw_buf_free_cb)(void * draw_buf);

typedef void * (*lv_draw_buf_align_cb)(void * buf, lv_color_format_t color_format);

typedef void (*lv_draw_buf_cache_operation_cb)(const lv_draw_buf_t * draw_buf, const lv_area_t * area);

typedef uint32_t (*lv_draw_buf_width_to_stride_cb)(uint32_t w, lv_color_format_t color_format);


struct _lv_draw_buf_handlers_t {
    lv_draw_buf_malloc_cb buf_malloc_cb;
    lv_draw_buf_free_cb buf_free_cb;
    lv_draw_buf_align_cb align_pointer_cb;
    lv_draw_buf_cache_operation_cb invalidate_cache_cb;
    lv_draw_buf_cache_operation_cb flush_cache_cb;
    lv_draw_buf_width_to_stride_cb width_to_stride_cb;
};

typedef struct _lv_draw_buf_handlers_t lv_draw_buf_handlers_t;

struct _lv_draw_buf_t {
    lv_image_header_t header;
    uint32_t data_size;       /**< Total buf size in bytes */
    uint8_t * data;
    void * unaligned_data;    /**< Unaligned address of `data`, used internally by lvgl */
    const lv_draw_buf_handlers_t * handlers; /**< draw buffer alloc/free ops. */
};


typedef struct _lv_font_t {
    /** Get a glyph's descriptor from a font*/
    bool (*get_glyph_dsc)(const struct _lv_font_t *, lv_font_glyph_dsc_t *, uint32_t letter, uint32_t letter_next);

    /** Get a glyph's bitmap from a font*/
    const uint8_t * (*get_glyph_bitmap)(const struct _lv_font_t *, uint32_t);

    /** Release a glyph*/
    void (*release_glyph)(const struct _lv_font_t *, lv_font_glyph_dsc_t *);

    /*Pointer to the font in a font pack (must have the same line height)*/
    int32_t line_height;         /**< The real line height where any text fits*/
    int32_t base_line;           /**< Base line measured from the bottom of the line_height*/
    uint8_t subpx   : 2;            /**< An element of `lv_font_subpx_t`*/
    uint8_t kerning : 1;            /**< An element of `lv_font_kerning_t`*/

    int8_t underline_position;      /**< Distance between the top of the underline and base line (< 0 means below the base line)*/
    int8_t underline_thickness;     /**< Thickness of the underline*/

    const void * dsc;               /**< Store implementation specific or run_time data or caching here*/
    const struct _lv_font_t * fallback;   /**< Fallback font for missing glyph. Resolved recursively */
    void * user_data;               /**< Custom user data for font.*/
} lv_font_t;

/*
 * Stub implementations for non-LVGL builds.
 * These keep linking successful when generated font C files are compiled.
 */
static inline bool lv_font_get_glyph_dsc_fmt_txt(const lv_font_t *font,
                                                 lv_font_glyph_dsc_t *dsc_out,
                                                 uint32_t letter,
                                                 uint32_t letter_next)
{
    (void)font;
    (void)dsc_out;
    (void)letter;
    (void)letter_next;
    if (font == NULL || dsc_out == NULL || font->dsc == NULL) {
        return false;
    }

    const lv_font_fmt_txt_dsc_t *fdsc = (const lv_font_fmt_txt_dsc_t *)font->dsc;
    if (fdsc->cmaps == NULL || fdsc->glyph_dsc == NULL) {
        return false;
    }

    uint32_t glyph_id = 0;
    for (uint32_t i = 0; i < fdsc->cmap_num; ++i) {
        const lv_font_fmt_txt_cmap_t *cmap = &fdsc->cmaps[i];
        if (letter < cmap->range_start) {
            continue;
        }
        uint32_t rel = letter - cmap->range_start;
        if (rel >= cmap->range_length) {
            continue;
        }

        switch (cmap->type) {
        case LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY:
            glyph_id = cmap->glyph_id_start + rel;
            break;

        case LV_FONT_FMT_TXT_CMAP_FORMAT0_FULL:
            if (cmap->glyph_id_ofs_list != NULL) {
                const uint8_t *gid_list = (const uint8_t *)cmap->glyph_id_ofs_list;
                uint16_t gid_ofs = gid_list[rel];
                if (gid_ofs != 0) {
                    glyph_id = cmap->glyph_id_start + gid_ofs;
                }
            }
            break;

        case LV_FONT_FMT_TXT_CMAP_SPARSE_TINY:
            if (cmap->unicode_list != NULL) {
                for (uint32_t j = 0; j < cmap->list_length; ++j) {
                    if (cmap->unicode_list[j] == rel) {
                        glyph_id = cmap->glyph_id_start + j;
                        break;
                    }
                }
            }
            break;

        case LV_FONT_FMT_TXT_CMAP_SPARSE_FULL:
            if (cmap->unicode_list != NULL && cmap->glyph_id_ofs_list != NULL) {
                const uint16_t *gid_list = (const uint16_t *)cmap->glyph_id_ofs_list;
                for (uint32_t j = 0; j < cmap->list_length; ++j) {
                    if (cmap->unicode_list[j] == rel) {
                        uint16_t gid_ofs = gid_list[j];
                        if (gid_ofs != 0) {
                            glyph_id = cmap->glyph_id_start + gid_ofs;
                        }
                        break;
                    }
                }
            }
            break;

        default:
            break;
        }

        if (glyph_id != 0) {
            break;
        }
    }

    if (glyph_id == 0) {
        return false;
    }

    const lv_font_fmt_txt_glyph_dsc_t *gd = &fdsc->glyph_dsc[glyph_id];
    dsc_out->adv_w = gd->adv_w;
    dsc_out->box_w = gd->box_w;
    dsc_out->box_h = gd->box_h;
    dsc_out->ofs_x = gd->ofs_x;
    dsc_out->ofs_y = gd->ofs_y;
    dsc_out->bpp = fdsc->bpp;
    dsc_out->is_placeholder = 0;
    return true;
}

static inline const uint8_t *lv_font_get_bitmap_fmt_txt(const lv_font_t *font,
                                                        uint32_t letter)
{
    if (font == NULL || font->dsc == NULL) {
        return (const uint8_t *)0;
    }

    const lv_font_fmt_txt_dsc_t *fdsc = (const lv_font_fmt_txt_dsc_t *)font->dsc;
    if (fdsc->cmaps == NULL || fdsc->glyph_dsc == NULL || fdsc->glyph_bitmap == NULL) {
        return (const uint8_t *)0;
    }

    uint32_t glyph_id = 0;
    for (uint32_t i = 0; i < fdsc->cmap_num; ++i) {
        const lv_font_fmt_txt_cmap_t *cmap = &fdsc->cmaps[i];
        if (letter < cmap->range_start) {
            continue;
        }
        uint32_t rel = letter - cmap->range_start;
        if (rel >= cmap->range_length) {
            continue;
        }

        switch (cmap->type) {
        case LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY:
            glyph_id = cmap->glyph_id_start + rel;
            break;

        case LV_FONT_FMT_TXT_CMAP_FORMAT0_FULL:
            if (cmap->glyph_id_ofs_list != NULL) {
                const uint8_t *gid_list = (const uint8_t *)cmap->glyph_id_ofs_list;
                uint16_t gid_ofs = gid_list[rel];
                if (gid_ofs != 0) {
                    glyph_id = cmap->glyph_id_start + gid_ofs;
                }
            }
            break;

        case LV_FONT_FMT_TXT_CMAP_SPARSE_TINY:
            if (cmap->unicode_list != NULL) {
                for (uint32_t j = 0; j < cmap->list_length; ++j) {
                    if (cmap->unicode_list[j] == rel) {
                        glyph_id = cmap->glyph_id_start + j;
                        break;
                    }
                }
            }
            break;

        case LV_FONT_FMT_TXT_CMAP_SPARSE_FULL:
            if (cmap->unicode_list != NULL && cmap->glyph_id_ofs_list != NULL) {
                const uint16_t *gid_list = (const uint16_t *)cmap->glyph_id_ofs_list;
                for (uint32_t j = 0; j < cmap->list_length; ++j) {
                    if (cmap->unicode_list[j] == rel) {
                        uint16_t gid_ofs = gid_list[j];
                        if (gid_ofs != 0) {
                            glyph_id = cmap->glyph_id_start + gid_ofs;
                        }
                        break;
                    }
                }
            }
            break;

        default:
            break;
        }

        if (glyph_id != 0) {
            break;
        }
    }

    if (glyph_id == 0) {
        return (const uint8_t *)0;
    }

    const lv_font_fmt_txt_glyph_dsc_t *gd = &fdsc->glyph_dsc[glyph_id];
    if (gd->box_w == 0 || gd->box_h == 0) {
        return (const uint8_t *)0;
    }
    return &fdsc->glyph_bitmap[gd->bitmap_index];
}

#ifdef __cplusplus
}
#endif

#endif /* M5GFX_USING_REAL_LVGL */

#endif /* M5GFX_LVGL_COMPAT_H */
