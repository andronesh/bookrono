## ATENTION! TFT_eSPI manual config

To make TFT-eSPI lib work with custom 76x284 ST7789 display you need to modify `ST7789_Rotation.h` file (GREAT thanks to **LODAM** for helpful [blog post](https://forum.arduino.cc/t/esp32-and-st7789p3-2-25-76x284-tft-from-estardyn/1425937/2)). You can find that file in loaded lib files (`.pio/libdeps/esp32_s3_zero/TFT_eSPI/TFT_Drivers`), replace content of that file with:

```cpp
// This is the command sequence that rotates the ST7789 driver coordinate frame
writecommand(TFT_MADCTL);
rotation = m % 4;

switch (rotation) {

  case 0: // Portrait
#ifdef CGRAM_OFFSET
    if (_init_width == 76 && _init_height == 284) {
      colstart = 82;
      rowstart = 18;
    }
    else if (_init_width == 135) {
      colstart = 52;
      rowstart = 40;
    }

    else if (_init_height == 280) {
      colstart = 0;
      rowstart = 20;
    }
    else if (_init_width == 172) {
      colstart = 34;
      rowstart = 0;
    }
    else if (_init_width == 170) {
      colstart = 35;
      rowstart = 0;
    }
    else {
      colstart = 0;
      rowstart = 0;
    }
#endif
    writedata(TFT_MAD_COLOR_ORDER);
    _width  = _init_width;
    _height = _init_height;
    break;

  case 1: // Landscape (Portrait + 90)
#ifdef CGRAM_OFFSET
    if (_init_width == 76 && _init_height == 284) {
      colstart = 18;
      rowstart = 82;
    }
    else if (_init_width == 135) {
      colstart = 40;
      rowstart = 53;
    }
    else if (_init_height == 280) {
      colstart = 20;
      rowstart = 0;
    }
    else if (_init_width == 172) {
      colstart = 0;
      rowstart = 34;
    }
    else if (_init_width == 170) {
      colstart = 0;
      rowstart = 35;
    }
    else {
      colstart = 0;
      rowstart = 0;
    }
#endif
    writedata(TFT_MAD_MX | TFT_MAD_MV | TFT_MAD_COLOR_ORDER);
    _width  = _init_height;
    _height = _init_width;
    break;

  case 2: // Inverted portrait (180°)
#ifdef CGRAM_OFFSET
    if (_init_width == 76 && _init_height == 284) {
      colstart = 82;
      rowstart = 18;
    }
    else if (_init_width == 135) {
      colstart = 53;
      rowstart = 40;
    }
    else if (_init_height == 280) {
      colstart = 0;
      rowstart = 20;
    }
    else if (_init_width == 172) {
      colstart = 34;
      rowstart = 0;
    }
    else if (_init_width == 170) {
      colstart = 35;
      rowstart = 0;
    }
    else {
      colstart = 0;
      rowstart = 0;
    }

#endif
    writedata(TFT_MAD_MX | TFT_MAD_MY | TFT_MAD_COLOR_ORDER);
    _width  = _init_width;
    _height = _init_height;
    break;

  case 3: // Inverted landscape (270°)
#ifdef CGRAM_OFFSET
    if (_init_width == 76 && _init_height == 284) {
      colstart = 18;
      rowstart = 82;
    }
    else if (_init_width == 135) {
      colstart = 40;
      rowstart = 52;
    }
    else if (_init_height == 280) {
      colstart = 20;
      rowstart = 0;
    }
    else if (_init_width == 172) {
      colstart = 0;
      rowstart = 34;
    }
    else if (_init_width == 170) {
      colstart = 0;
      rowstart = 35;
    }
    else {
      colstart = 0;
      rowstart = 0;
    }
#endif
    writedata(TFT_MAD_MV | TFT_MAD_MY | TFT_MAD_COLOR_ORDER);
    _width  = _init_height;
    _height = _init_width;
    break;
}
```
