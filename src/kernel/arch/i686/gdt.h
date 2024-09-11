#pragma once

#define i686_GDT_CODE_SEGMENT 0x08
#define i686_GDT_DATA_SEGMENT 0x10
#define GDT_LIMIT_LOW(limit) (limit & 0xFFFF)
#define GDT_BASE_LOW(base) (base & 0xFFFF)
#define GDT_BASE_MIDDLE(base) ((base >> 16) & 0xFF)
#define GDT_FLAGS_LIMIT_HI(limit, flags) (((limit >> 16) & 0xF) | (flags & 0xF0))
#define GDT_BASE_HIGH(base) ((base >> 24) & 0xFF)
#define GDT_ENTRY(base, limit, access, flags) {GDT_LIMIT_LOW(limit), GDT_BASE_LOW(base), GDT_BASE_MIDDLE(base), access, GDT_FLAGS_LIMIT_HI(limit, flags), GDT_BASE_HIGH(base)}

void i686_GDT_Initialize();