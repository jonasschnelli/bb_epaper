#!/usr/bin/env python3
"""
LVGL Font Converter for BB_EPaper Library
Converts LVGL font .c files to bb_epaper compatible format

Usage: python convert_lvgl_font.py input_font.c output_font.c
"""

import sys
import re
import os
from pathlib import Path

class LVGLFontConverter:
    def __init__(self):
        # Struct type mappings
        self.type_mappings = {
            'lv_font_t': 'bb_lv_font_t',
            'lv_font_fmt_txt_dsc_t': 'bb_lv_font_fmt_txt_dsc_t',
            'lv_font_fmt_txt_glyph_dsc_t': 'bb_lv_font_fmt_txt_glyph_dsc_t',
            'lv_font_fmt_txt_cmap_t': 'bb_lv_font_fmt_txt_cmap_t',
            'lv_font_fmt_txt_kern_pair_t': 'bb_lv_font_fmt_txt_kern_pair_t',
            'lv_font_fmt_txt_cmap_type_t': 'bb_lv_font_fmt_txt_cmap_type_t',
            'LV_FONT_FMT_TXT_CMAP_FORMAT0_FULL': 'BB_LV_FONT_FMT_TXT_CMAP_FORMAT0_FULL',
            'LV_FONT_FMT_TXT_CMAP_SPARSE_FULL': 'BB_LV_FONT_FMT_TXT_CMAP_SPARSE_FULL',
            'LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY': 'BB_LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY',
            'LV_FONT_FMT_TXT_CMAP_SPARSE_TINY': 'BB_LV_FONT_FMT_TXT_CMAP_SPARSE_TINY'
        }
        
        # Fields that should have PROGMEM attribute for ESP32
        self.progmem_arrays = [
            'glyph_bitmap',
            'glyph_dsc',
            'cmaps',
            'kern_pair_glyph_ids',
            'kern_pair_values',
            'unicode_list',
            'glyph_id_ofs_list'
        ]
    
    def convert_file(self, input_path, output_path):
        """Convert LVGL font file to bb_epaper format"""
        try:
            with open(input_path, 'r') as f:
                content = f.read()
            
            # Extract font name from filename
            font_name = Path(input_path).stem
            
            print(f"Converting {input_path} -> {output_path}")
            print(f"Font name: {font_name}")
            
            # Apply conversions
            content = self._remove_lvgl_includes(content)
            content = self._add_bb_epaper_include(content)
            content = self._convert_struct_types(content)
            content = self._remove_lvgl_version_checks(content)
            content = self._add_progmem_attributes(content)
            content = self._simplify_font_structure(content, font_name)
            content = self._add_header_comment(content, font_name)
            
            # Write output
            with open(output_path, 'w') as f:
                f.write(content)
            
            print(f"✓ Successfully converted {font_name}")
            return True
            
        except Exception as e:
            print(f"✗ Error converting {input_path}: {e}")
            return False
    
    def _remove_lvgl_includes(self, content):
        """Remove LVGL includes and replace with bb_epaper include"""
        # Remove LVGL includes
        content = re.sub(r'#ifdef LV_LVGL_H_INCLUDE_SIMPLE.*?#endif', '', content, flags=re.DOTALL)
        content = re.sub(r'#include "lvgl\.h"', '', content)
        content = re.sub(r'#include "lvgl/lvgl\.h"', '', content)
        return content
    
    def _add_bb_epaper_include(self, content):
        """Add bb_epaper include at the top"""
        # Find the position after the initial comments but before any code
        lines = content.split('\n')
        insert_pos = 0
        
        # Skip initial comments
        for i, line in enumerate(lines):
            stripped = line.strip()
            if stripped and not stripped.startswith('/*') and not stripped.startswith('*') and not stripped.startswith('//'):
                insert_pos = i
                break
        
        # Add the include
        lines.insert(insert_pos, '#include "lvgl_compat.h"')
        lines.insert(insert_pos + 1, '')
        return '\n'.join(lines)
    
    def _convert_struct_types(self, content):
        """Convert LVGL struct types to bb_epaper types"""
        for old_type, new_type in self.type_mappings.items():
            content = re.sub(r'\b' + re.escape(old_type) + r'\b', new_type, content)
        return content
    
    def _remove_lvgl_version_checks(self, content):
        """Remove LVGL version conditionals and include guards"""
        # Remove version checks like #if LVGL_VERSION_MAJOR >= 8
        content = re.sub(r'#if LVGL_VERSION_MAJOR.*?\n', '', content)
        content = re.sub(r'#if LV_VERSION_CHECK.*?\n', '', content)
        content = re.sub(r'#if !\(LVGL_VERSION_MAJOR.*?\n', '', content)
        content = re.sub(r'#else\n', '', content)
        
        # Remove include guards - we don't need them in .c files
        content = re.sub(r'#ifndef \w+\n#define \w+ 1\n#endif\n\n#if \w+\n', '', content)
        content = re.sub(r'#endif /\*#if \w+\*/\s*$', '', content.rstrip()) + '\n'
        
        # Remove cache declaration and usage
        content = re.sub(r'static\s+lv_font_fmt_txt_glyph_cache_t\s+cache;\s*\n', '', content)
        content = re.sub(r'\.cache = &cache\n', '', content)
        
        # Remove version-specific fields
        content = re.sub(r'\.get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,.*?\n', '', content)
        content = re.sub(r'\.get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,.*?\n', '', content)
        content = re.sub(r'\.subpx = LV_FONT_SUBPX_NONE,\n', '', content)
        content = re.sub(r'\.fallback = NULL,\n', '', content)
        
        # Clean up duplicate font_dsc declarations and stray #endif
        content = re.sub(r'static const bb_lv_font_fmt_txt_dsc_t font_dsc = \{\nstatic bb_lv_font_fmt_txt_dsc_t font_dsc = \{', 'static const bb_lv_font_fmt_txt_dsc_t font_dsc = {', content)
        content = re.sub(r'static lv_font_fmt_txt_dsc_t font_dsc = \{\n#endif\n    \.', 'static const bb_lv_font_fmt_txt_dsc_t font_dsc = {\n    .', content)
        
        # Remove stray #endif lines
        lines = content.split('\n')
        clean_lines = []
        for line in lines:
            if line.strip() in ['#endif', '#endif\n']:
                continue
            clean_lines.append(line)
        content = '\n'.join(clean_lines)
        
        return content
    
    def _add_progmem_attributes(self, content):
        """Add PROGMEM attributes for ESP32 flash storage"""
        # Remove LV_ATTRIBUTE_LARGE_CONST and add PROGMEM
        content = re.sub(r'static\s+LV_ATTRIBUTE_LARGE_CONST\s+const\s+', 'static const ', content)
        
        for array_name in self.progmem_arrays:
            # Add PROGMEM to static const arrays
            pattern = r'(static\s+const\s+[^=]+\s+' + array_name + r'(?:\[\])?)\s*='
            replacement = r'\1 PROGMEM ='
            content = re.sub(pattern, replacement, content)
        
        return content
    
    def _simplify_font_structure(self, content, font_name):
        """Simplify font structure initialization"""
        # Remove const keyword from font descriptor if present
        content = re.sub(r'const\s+bb_lv_font_t\s+' + font_name, f'const bb_lv_font_t {font_name}', content)
        
        # Simplify font structure - remove function pointers and version-specific fields
        font_pattern = r'(const bb_lv_font_t\s+' + font_name + r'\s*=\s*\{)(.*?)(\};)'
        
        def simplify_font_struct(match):
            struct_content = match.group(2)
            
            # Keep only essential fields
            essential_fields = []
            for line in struct_content.split('\n'):
                line = line.strip()
                if any(field in line for field in ['.line_height', '.base_line', '.underline_position', '.underline_thickness', '.dsc', '.user_data']):
                    essential_fields.append('    ' + line)
            
            return match.group(1) + '\n' + '\n'.join(essential_fields) + '\n' + match.group(3)
        
        content = re.sub(font_pattern, simplify_font_struct, content, flags=re.DOTALL)
        
        return content
    
    def _add_header_comment(self, content, font_name):
        """Add header comment with conversion info"""
        header = f'''/*
 * Font: {font_name}
 * Converted from LVGL format to bb_epaper format
 * Generated by convert_lvgl_font.py
 */

'''
        return header + content

def main():
    if len(sys.argv) != 3:
        print("Usage: python convert_lvgl_font.py input_font.c output_font.c")
        sys.exit(1)
    
    input_file = sys.argv[1]
    output_file = sys.argv[2]
    
    if not os.path.exists(input_file):
        print(f"Error: Input file '{input_file}' not found")
        sys.exit(1)
    
    converter = LVGLFontConverter()
    success = converter.convert_file(input_file, output_file)
    
    if success:
        print(f"\n✓ Conversion complete!")
        print(f"Output: {output_file}")
        print(f"\nNext steps:")
        print(f"1. Review the converted file")
        print(f"2. Add font declaration to your registry header file:")
        print(f"   extern const bb_lv_font_t {Path(input_file).stem};")
        print(f"3. Include the .c file in your build system")
    else:
        sys.exit(1)

if __name__ == "__main__":
    main()