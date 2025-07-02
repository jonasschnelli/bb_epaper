//
// bb_ep_font.inl
// Font handling functions with kerning support for BB_EPaper library
//

//
// Helper function to convert glyph ID back to Unicode character for sparse fonts
//
uint32_t bbep_glyph_id_to_unicode(const bb_lv_font_t *font, uint16_t glyph_id)
{
    const bb_lv_font_fmt_txt_dsc_t *fdsc = (bb_lv_font_fmt_txt_dsc_t*)font->dsc;
    if (!fdsc || !fdsc->cmaps || fdsc->cmap_num == 0) {
        return 0; // Invalid
    }
    
    // Search through all cmaps to find the glyph_id
    for (uint16_t cmap_idx = 0; cmap_idx < fdsc->cmap_num; cmap_idx++) {
        const bb_lv_font_fmt_txt_cmap_t *cmap = &fdsc->cmaps[cmap_idx];
        
        if (glyph_id < cmap->glyph_id_start || 
            glyph_id >= cmap->glyph_id_start + cmap->list_length) {
            continue; // Not in this cmap range
        }
        
        uint16_t local_glyph_idx = glyph_id - cmap->glyph_id_start;
        
        if (cmap->type == BB_LV_FONT_FMT_TXT_CMAP_SPARSE_TINY && cmap->unicode_list) {
            // Sparse mapping: the local_glyph_idx is the index into unicode_list
            if (local_glyph_idx < cmap->list_length) {
                uint16_t unicode_offset = cmap->unicode_list[local_glyph_idx];
                return cmap->range_start + unicode_offset;
            }
        } else if (cmap->type == BB_LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY) {
            // Dense mapping: direct offset from range_start
            return cmap->range_start + local_glyph_idx;
        }
        // Handle other cmap types if needed
    }
    
    return 0; // Not found
}

//
// Debug function to show available kerning pairs
//
void bbep_debug_kerning_pairs(const bb_lv_font_t *font)
{
    const bb_lv_font_fmt_txt_dsc_t *fdsc = (bb_lv_font_fmt_txt_dsc_t*)font->dsc;
    if (!fdsc || !fdsc->kern_dsc) {
        printf("No kerning data available\n");
        return;
    }
    
    const bb_lv_font_fmt_txt_kern_pair_t *kern_pairs = (bb_lv_font_fmt_txt_kern_pair_t*)fdsc->kern_dsc;
    const uint8_t *glyph_ids = (const uint8_t*)kern_pairs->glyph_ids;
    const int8_t *values = (const int8_t*)kern_pairs->values;
    uint32_t pair_cnt = kern_pairs->pair_cnt;
    
    printf("Kerning pairs available (%ld total, scale=%d):\n", (long)pair_cnt, fdsc->kern_scale);
    
    // Show first 10 pairs as examples
    for (uint32_t i = 0; i < pair_cnt && i < 10; i++) {
        uint8_t left = glyph_ids[i * 2];
        uint8_t right = glyph_ids[i * 2 + 1];
        int8_t raw_value = values[i];
        int scaled_value = (raw_value * fdsc->kern_scale) / 16;
        
        // Convert glyph IDs back to Unicode characters using proper cmap lookup
        uint32_t left_unicode = bbep_glyph_id_to_unicode(font, left);
        uint32_t right_unicode = bbep_glyph_id_to_unicode(font, right);
        
        char left_char = (left_unicode > 0 && left_unicode < 127) ? (char)left_unicode : '?';
        char right_char = (right_unicode > 0 && right_unicode < 127) ? (char)right_unicode : '?';
        
        printf("  %ld: '%c%c' (glyph %d->%d, unicode %ld->%ld) raw=%d scaled=%d\n", 
               (long)i, left_char, right_char, left, right, 
               (long)left_unicode, (long)right_unicode, raw_value, scaled_value);
    }
    
    if (pair_cnt > 10) {
        printf("  ... and %ld more pairs\n", (long)(pair_cnt - 10));
    }
}

//
// Get kerning adjustment between two glyph IDs (returns value in 12.4 fixed point format)
//
int32_t bbep_get_kerning(const bb_lv_font_t *font, uint8_t left_glyph_id, uint8_t right_glyph_id)
{
    const bb_lv_font_fmt_txt_dsc_t *fdsc = (bb_lv_font_fmt_txt_dsc_t*)font->dsc;
    if (!fdsc || !fdsc->kern_dsc) {
        return 0; // No kerning data available
    }
    
    const bb_lv_font_fmt_txt_kern_pair_t *kern_pairs = (bb_lv_font_fmt_txt_kern_pair_t*)fdsc->kern_dsc;
    uint32_t pair_cnt = kern_pairs->pair_cnt;
    
    const uint8_t *glyph_ids = (const uint8_t*)kern_pairs->glyph_ids;
    const int8_t *values = (const int8_t*)kern_pairs->values;
    
    if (!glyph_ids || !values) {
        return 0; // Invalid pointers
    }
    
    // Search for the kerning pair
    for (uint32_t i = 0; i < pair_cnt; i++) {
        if (glyph_ids[i * 2] == left_glyph_id && glyph_ids[i * 2 + 1] == right_glyph_id) {
            // Found the pair, apply LVGL scaling: (value * kern_scale) >> 4
            int32_t kern_value = values[i];
            int32_t result = ((int32_t)((int32_t)kern_value * fdsc->kern_scale) >> 4);
            printf("  FOUND kerning pair at index %ld: raw=%ld, scaled_fp=%ld (%.1f pixels)\n", 
                   (long)i, (long)kern_value, (long)result, (float)result / 16.0f);
            return result;
        }
    }
    
    return 0; // No kerning for this pair
}

//
// Write a string of text using the new testfont format
//
int bbepWriteStringNew(BBEPDISP *pBBEP, const bb_lv_font_t *font, int x, int y, char *szMsg, int iColor, int iBG, bool use_kerning)
{
    int i;
    unsigned char c;
    const bb_lv_font_fmt_txt_glyph_dsc_t *glyph_dsc;
    const bb_lv_font_fmt_txt_cmap_t *cmap;
    const uint8_t *glyph_bitmap;
    
    if (pBBEP == NULL) {
        return BBEP_ERROR_BAD_PARAMETER;
    }
    
    if (iColor != BBEP_TRANSPARENT) {
        iColor = pBBEP->pColorLookup[iColor & 0xf];
    }
    if (iBG == -1) iBG = BBEP_TRANSPARENT;
    if (iBG != BBEP_TRANSPARENT) {
        iBG = pBBEP->pColorLookup[iBG & 0xf];
    }
    
    if (x == -1 || y == -1) {
        x = pBBEP->iCursorX; 
        y = pBBEP->iCursorY;
    } else {
        pBBEP->iCursorX = x; 
        pBBEP->iCursorY = y;
    }
    
    if (pBBEP->iCursorX >= pBBEP->width || pBBEP->iCursorY >= pBBEP->height) {
        pBBEP->last_error = BBEP_ERROR_BAD_PARAMETER;
        return BBEP_ERROR_BAD_PARAMETER;
    }
    
    // Use the font structure
    glyph_dsc = font->dsc ? ((bb_lv_font_fmt_txt_dsc_t*)font->dsc)->glyph_dsc : NULL;
    cmap = font->dsc ? ((bb_lv_font_fmt_txt_dsc_t*)font->dsc)->cmaps : NULL;
    glyph_bitmap = font->dsc ? ((bb_lv_font_fmt_txt_dsc_t*)font->dsc)->glyph_bitmap : NULL;
    
    if (!glyph_dsc || !cmap || !glyph_bitmap) {
        pBBEP->last_error = BBEP_ERROR_BAD_DATA;
        return BBEP_ERROR_BAD_DATA;
    }
    
    // Show available kerning pairs (first time only)
    static bool debug_shown = false;
    if (!debug_shown) {
        bbep_debug_kerning_pairs(font);
        debug_shown = true;
    }
    
    i = 0;
    
    while (szMsg[i] != 0 && x < pBBEP->width) {
        c = (unsigned char)szMsg[i];
        
        // Find glyph index for character
        uint16_t glyph_id = 0;
        if (c >= cmap->range_start && c < (cmap->range_start + cmap->range_length)) {
            if (cmap->type == BB_LV_FONT_FMT_TXT_CMAP_SPARSE_TINY && cmap->unicode_list) {
                // Sparse mapping: search in unicode_list
                uint16_t char_offset = c - cmap->range_start;
                bool found = false;
                for (uint16_t j = 0; j < cmap->list_length; j++) {
                    if (cmap->unicode_list[j] == char_offset) {
                        glyph_id = cmap->glyph_id_start + j;
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    // Character not in sparse list, skip
                    i++;
                    continue;
                }
            } else {
                // Dense mapping
                glyph_id = cmap->glyph_id_start + (c - cmap->range_start);
            }
        } else {
            // Character not found, skip
            i++;
            continue;
        }
        
        // Get glyph descriptor first
        const bb_lv_font_fmt_txt_glyph_dsc_t *glyph = &glyph_dsc[glyph_id];
        
        // Debug y-positioning for various characters to understand font structure
        if (c == 'A' || c == 'V' || c == 'N' || c == 'o' || c == 'w' || c == 'y') {
            int top_to_baseline = font->line_height - font->base_line;
            int glyph_y = y + top_to_baseline - glyph->box_h - glyph->ofs_y;
            printf("  DEBUG '%c': box_h=%d, ofs_y=%d, top_to_baseline=%d, glyph_y=%d\n", 
                   c, glyph->box_h, glyph->ofs_y, top_to_baseline, glyph_y);
        }
        
        // Get bitmap data
        const uint8_t *bitmap_data = &glyph_bitmap[glyph->bitmap_index];
        
        
        // Draw the glyph bitmap
        for (int py = 0; py < glyph->box_h; py++) {
            for (int px = 0; px < glyph->box_w; px++) {
                int bit_index = py * glyph->box_w + px;
                int byte_index = bit_index / 8;
                int bit_offset = 7 - (bit_index % 8);
                
                if (bitmap_data[byte_index] & (1 << bit_offset)) {
                    int draw_x = x + glyph->ofs_x + px;
                    // Calculate baseline-aligned position using LVGL formula
                    // pos->y + (font->line_height - font->base_line) - g.box_h - g.ofs_y
                    int top_to_baseline = font->line_height - font->base_line;
                    int glyph_y = y + top_to_baseline - glyph->box_h - glyph->ofs_y;
                    int draw_y = glyph_y + py;
                    
                    if (draw_x >= 0 && draw_x < pBBEP->width && draw_y >= 0 && draw_y < pBBEP->height) {
                        if (pBBEP->pfnSetPixelFast) {
                            pBBEP->pfnSetPixelFast(pBBEP, draw_x, draw_y, iColor);
                        }
                    }
                }
            }
        }
        
        // Calculate advance width using LVGL's fixed-point method
        uint32_t adv_w = glyph->adv_w; // Keep in 12.4 fixed point format
        
        // Look ahead for kerning with next character (LVGL style)
        int32_t kerning_fp = 0; // Kerning in 12.4 fixed point
        if (use_kerning && szMsg[i+1] != 0) {
            unsigned char next_c = (unsigned char)szMsg[i+1];
            if (next_c >= cmap->range_start && next_c < (cmap->range_start + cmap->range_length)) {
                uint16_t next_glyph_id = 0;
                
                // Use proper mapping for sparse fonts
                if (cmap->type == BB_LV_FONT_FMT_TXT_CMAP_SPARSE_TINY && cmap->unicode_list) {
                    // Sparse mapping: search in unicode_list
                    uint16_t char_offset = next_c - cmap->range_start;
                    bool found = false;
                    for (uint16_t j = 0; j < cmap->list_length; j++) {
                        if (cmap->unicode_list[j] == char_offset) {
                            next_glyph_id = cmap->glyph_id_start + j;
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        // Character not in sparse list, no kerning
                        next_glyph_id = 0;
                    }
                } else {
                    // Dense mapping
                    next_glyph_id = cmap->glyph_id_start + (next_c - cmap->range_start);
                }
                
                // Get kerning value only if we found a valid glyph
                if (next_glyph_id > 0) {
                    kerning_fp = bbep_get_kerning(font, glyph_id, next_glyph_id);
                    
                    if (kerning_fp != 0) {
                        printf("  Kerning '%c%c' (glyph %d->%d): %.1f pixels (fp: %ld)\n", 
                               c, next_c, glyph_id, next_glyph_id, (float)kerning_fp / 16.0f, (long)kerning_fp);
                    }
                }
            }
        }
        
        // Apply kerning to advance width in fixed point (LVGL method)
        adv_w += kerning_fp;
        
        // Convert to integer with rounding (LVGL method)
        // This matches: adv_w = (adv_w + (1 << 3)) >> 4;
        uint32_t advance = (adv_w + (1 << 3)) >> 4;
        
        printf("  '%c' drawn at x=%d, base_adv_fp: %ld, kerning_fp: %ld, final_adv_fp: %ld, final_advance: %ld, cursor moves to x=%ld\n", 
               c, x, (long)glyph->adv_w, (long)kerning_fp, (long)adv_w, (long)advance, (long)(x + advance));
        x += advance;
        i++;
    }
    
    pBBEP->iCursorX = x;
    return BBEP_SUCCESS;
} /* bbepWriteStringNew() */