#include "vbe.h"
#include "x86.h"
#include "memory.h"
#include "stdio.h"
#include "font.c"
#include "../util/string.h"
#include "../util/memdefs.h"

VbeInfoBlock* info = (VbeInfoBlock*)MEMORY_VESA_INFO;
VbeModeInfo* modeInfo = (VbeModeInfo*)MEMORY_MODE_INFO;

const int desiredWidth = 1024;
const int desiredHeight = 768;
const int desiredBpp = 32;
uint16_t pickedMode = 0xffff;

bool VBE_GetControllerInfo(VbeInfoBlock* info) {
    if (x86_Video_GetVbeInfo(info) == 0) {
        info->VideoModePtr = SEGOFF2LIN(info->VideoModePtr); //Convert from seg:off to a linear address
        return true;
    }
    return false;
}
bool VBE_GetModeInfo(uint16_t mode, VbeModeInfo* info) {
    if (x86_Video_GetModeInfo(mode, info) == 0) {
        return true;
    }
    return false;
}

bool VBE_SetMode(uint16_t mode) {
    return x86_Video_SetMode(mode) == 0;
}

bool VBE_Initialize() {
    if (VBE_GetControllerInfo(info)) {
        uint16_t* mode = (uint16_t*)(info->VideoModePtr);
        for (int i = 0; mode[i] != 0xFFFF; i++) {
            if (!VBE_GetModeInfo(mode[i], modeInfo)) {
                printf("Can't get mode info %x\n", mode[i]);
                continue;
            }
            bool hasFB = (modeInfo->attributes & 0x90) == 0x90;

            if (hasFB && modeInfo->width == desiredWidth && modeInfo->height == desiredHeight && modeInfo->bpp == desiredBpp) {
                pickedMode = mode[i];
                break;
            }
        }

        if (pickedMode != 0xFFFF && VBE_SetMode(pickedMode)) {
            Point topLeft = { 0, 0 };
            Point bottomRight = { desiredWidth, desiredHeight };
            VBE_DrawRectangle(topLeft, bottomRight, 0x0000FF);

            const char *text = "Press Enter to Continue to Kernel";
            VBE_DrawString(text, (desiredWidth - (strlen(text) * FONT_WIDTH)) / 2, (desiredHeight - FONT_HEIGHT) / 2, 0xFFFFFFFF);
        }
    } else {
        printf("No VBE extensions\n");
        return false;
    }

    return true;
}

void VBE_PutPixel(int x, int y, uint32_t color) {
    uint32_t* fb = (uint32_t*)(modeInfo->framebuffer);
    fb[y * modeInfo->pitch / 4 + x] = color;
}

void VBE_DrawChar(char c, int x, int y, uint32_t color) {
    if (c < 0x20 || c > 0x7F) return;

    const unsigned char *glyph = font[c - 0x20];
    for (int row = 0; row < FONT_HEIGHT; ++row) {
        unsigned char bits = glyph[row];
        for (int col = 0; col < FONT_WIDTH; ++col) {
            if (bits & (1 << (7 - col))) {
                VBE_PutPixel(x + col, y + row, color);
            }
        }
    }
}

void VBE_DrawString(const char *str, int x, int y, uint32_t color) {
    while (*str) {
        VBE_DrawChar(*str++, x, y, color);
        x += FONT_WIDTH;
    }
}

void VBE_DrawOS() {
    uint32_t* fb = (uint32_t*)(modeInfo->framebuffer);
    int w = modeInfo->width;
    int h = modeInfo->height;

    int taskbarHeight = 50;
    int taskbarPadding = 240;
    int taskbarSpaceBelow = 20;

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            uint8_t r = 255;
            uint8_t g = (y * 255) / h;
            uint8_t b = 255 - ((x * 255) / w);
            VBE_PutPixel(x, y, COLOR(r, g, b));
        }
    }

    Point topLeft = { taskbarPadding, h - taskbarHeight - taskbarSpaceBelow };
    Point bottomRight = { w - taskbarPadding, h - taskbarSpaceBelow };
    VBE_DrawRectangle(topLeft, bottomRight, COLOR(0, 0, 0));
}

void VBE_DrawRectangle(Point topLeft, Point bottomRight, uint32_t color) {
    for (uint16_t y = topLeft.Y; y < bottomRight.Y; y++) {
        for (uint16_t x = topLeft.X; x < bottomRight.X; x++) {
            VBE_PutPixel(x, y, color);
        }
    }
}
