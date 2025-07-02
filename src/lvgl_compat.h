#include <stdio.h>

#ifndef PROGMEM
#define PROGMEM
#endif


#ifndef __LVGL_COMPAT__
#define __LVGL_COMPAT__

/** A simple mapping of kern values from pairs*/
typedef struct {
    /*To get a kern value of two code points:
       1. Get the `glyph_id_left` and `glyph_id_right` from `lv_font_fmt_txt_cmap_t
       2. for(i = 0; i < pair_cnt * 2; i += 2)
             if(glyph_ids[i] == glyph_id_left &&
                glyph_ids[i+1] == glyph_id_right)
                 return values[i / 2];
     */
    const void * glyph_ids;
    const int8_t * values;
    uint32_t pair_cnt   : 30;
    uint32_t glyph_ids_size : 2;    /**< 0: `glyph_ids` is stored as `uint8_t`; 1: as `uint16_t` */
} bb_lv_font_fmt_txt_kern_pair_t;

typedef struct {
    uint32_t bitmap_index : 20;     /**< Start index of the bitmap. A font can be max 1 MB.*/
    uint32_t adv_w : 12;            /**< Draw the next glyph after this width. 8.4 format (real_value * 16 is stored).*/
    uint8_t box_w;                  /**< Width of the glyph's bounding box*/
    uint8_t box_h;                  /**< Height of the glyph's bounding box*/
    int8_t ofs_x;                   /**< x offset of the bounding box*/
    int8_t ofs_y;                   /**< y offset of the bounding box. Measured from the top of the line*/
} bb_lv_font_fmt_txt_glyph_dsc_t;

/** Format of font character map.*/
typedef enum {
    BB_LV_FONT_FMT_TXT_CMAP_FORMAT0_FULL,
    BB_LV_FONT_FMT_TXT_CMAP_SPARSE_FULL,
    BB_LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY,
    BB_LV_FONT_FMT_TXT_CMAP_SPARSE_TINY,
} bb_lv_font_fmt_txt_cmap_type_t;

/**
 * Map codepoints to a `glyph_dsc`s
 * Several formats are supported to optimize memory usage
 * See https://github.com/lvgl/lv_font_conv/blob/master/doc/font_spec.md
 */
typedef struct {
    /** First Unicode character for this range*/
    uint32_t range_start;

    /** Number of Unicode characters related to this range.
     * Last Unicode character = range_start + range_length - 1*/
    uint16_t range_length;

    /** First glyph ID (array index of `glyph_dsc`) for this range*/
    uint16_t glyph_id_start;

    /*
    According the specification there are 4 formats:
        https://github.com/lvgl/lv_font_conv/blob/master/doc/font_spec.md

    For simplicity introduce "relative code point":
        rcp = codepoint - range_start

    and a search function:
        search a "value" in an "array" and returns the index of "value".

    Format 0 tiny
        unicode_list == NULL && glyph_id_ofs_list == NULL
        glyph_id = glyph_id_start + rcp

    Format 0 full
        unicode_list == NULL && glyph_id_ofs_list != NULL
        glyph_id = glyph_id_start + glyph_id_ofs_list[rcp]

    Sparse tiny
        unicode_list != NULL && glyph_id_ofs_list == NULL
        glyph_id = glyph_id_start + search(unicode_list, rcp)

    Sparse full
        unicode_list != NULL && glyph_id_ofs_list != NULL
        glyph_id = glyph_id_start + glyph_id_ofs_list[search(unicode_list, rcp)]
    */

    const uint16_t * unicode_list;

    /** if(type == LV_FONT_FMT_TXT_CMAP_FORMAT0_...) it's `uint8_t *`
     * if(type == LV_FONT_FMT_TXT_CMAP_SPARSE_...)  it's `uint16_t *`
     */
    const void * glyph_id_ofs_list;

    /** Length of `unicode_list` and/or `glyph_id_ofs_list`*/
    uint16_t list_length;

    /** Type of this character map*/
    bb_lv_font_fmt_txt_cmap_type_t type;
} bb_lv_font_fmt_txt_cmap_t;

/** Describe store for additional data for fonts */
typedef struct {
    /** The bitmaps of all glyphs */
    const uint8_t * glyph_bitmap;

    /** Describe the glyphs */
    const bb_lv_font_fmt_txt_glyph_dsc_t * glyph_dsc;

    /** Map the glyphs to Unicode characters.
     *Array of `lv_font_cmap_fmt_txt_t` variables */
    const bb_lv_font_fmt_txt_cmap_t * cmaps;

    /**
     * Store kerning values.
     * Can be `lv_font_fmt_txt_kern_pair_t *  or `lv_font_kern_classes_fmt_txt_t *`
     * depending on `kern_classes`
     */
    const void * kern_dsc;

    /** Scale kern values in 12.4 format */
    uint16_t kern_scale;

    /** Number of cmap tables */
    uint16_t cmap_num       : 9;

    /** Bit per pixel: 1, 2, 3, 4, 8 */
    uint16_t bpp            : 4;

    /** Type of `kern_dsc` */
    uint16_t kern_classes   : 1;

    /**
     * storage format of the bitmap
     * from `lv_font_fmt_txt_bitmap_format_t`
     */
    uint16_t bitmap_format  : 2;

    /**
     * Bytes to which each line is padded.
     * 0: means no align and padding
     * 1: e.g. with bpp=4 lines are aligned to 1 byte, so there can be a 4 bits of padding
     * 4, 8, 16, 32, 64: each line is padded to the given byte boundaries
     */
    uint8_t stride;
} bb_lv_font_fmt_txt_dsc_t;


/** Describe the properties of a font*/
typedef  struct bb_lv_font_t {
    /*Pointer to the font in a font pack (must have the same line height)*/
    int32_t line_height;         /**< The real line height where any text fits*/
    int32_t base_line;           /**< Base line measured from the bottom of the line_height*/
    uint8_t subpx   : 2;            /**< An element of `lv_font_subpx_t`*/
    uint8_t kerning : 1;            /**< An element of `lv_font_kerning_t`*/
    uint8_t static_bitmap : 1;      /**< The font will be used as static bitmap */

    int8_t underline_position;      /**< Distance between the top of the underline and base line (< 0 means below the base line)*/
    int8_t underline_thickness;     /**< Thickness of the underline*/

    const void * dsc;               /**< Store implementation specific or run_time data or caching here*/
    void * user_data;               /**< Custom user data for font.*/
} bb_lv_font_t;

#endif //__LVGL_COMPAT__