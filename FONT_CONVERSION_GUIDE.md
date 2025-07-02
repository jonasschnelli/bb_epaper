# LVGL Font Conversion Guide

## Conversion Results

The Python script `convert_lvgl_font.py` successfully converts LVGL font files to bb_epaper format.

### Key Differences from Manual Conversion

| Aspect | Manual (square140.h) | Automatic (square140_converted_v2.c) |
|--------|---------------------|-------------------------------------|
| **File Type** | Header (.h) | Source (.c) - **Better for avoiding conflicts** |
| **PROGMEM** | Missing | ✅ Added to all arrays for flash storage |
| **Include Guard** | `#pragma once` | `#ifndef SQUARE140` + conditional compilation |
| **Structure** | Simplified | Complete with proper LVGL compatibility |

### Advantages of Automatic Conversion

1. **PROGMEM Attributes**: Automatically adds `PROGMEM` to all font data arrays for ESP32 flash storage
2. **No Symbol Conflicts**: Uses `.c` files instead of `.h` files
3. **Consistent Conversion**: All struct types properly renamed (`lv_*` → `bb_lv_*`)
4. **Flash Efficiency**: Better memory management for ESP32

## Registry System Recommendation

### Create a Font Registry Header

Create `/components/bb_epaper/src/font_registry.h`:

```c
#ifndef __FONT_REGISTRY__
#define __FONT_REGISTRY__

#include "lvgl_compat.h"

// Font declarations
extern const bb_lv_font_t square140;
// Add more fonts here as you convert them:
// extern const bb_lv_font_t another_font;

#endif // __FONT_REGISTRY__
```

### Usage in Your Code

```c
#include "font_registry.h"

// Use fonts directly
bbep.drawStringNew(&square140, "AVy:AV:..", 100, 200, true);
```

### Build System Integration

Add to your CMakeLists.txt:
```cmake
# Add font source files
target_sources(your_component PRIVATE
    fonts/square140.c
    # fonts/another_font.c
)
```

## Converting New Fonts

```bash
# Convert any LVGL font
python convert_lvgl_font.py path/to/original_font.c fonts/converted_font.c

# Then add to registry header:
# extern const bb_lv_font_t converted_font;
```

## Benefits

- ✅ **No Symbol Conflicts**: Multiple fonts can coexist
- ✅ **Flash Efficiency**: Only linked fonts consume flash memory
- ✅ **ESP32 Optimized**: PROGMEM attributes for proper memory usage
- ✅ **Scalable**: Easy to add/remove fonts
- ✅ **Automated**: No manual conversion errors