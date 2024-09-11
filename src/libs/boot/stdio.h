#pragma once
#include <stdint.h>

void putchr(int x, int y, char c);
void putcolor(int x, int y, uint8_t color);
char getchr(int x, int y);
uint8_t getcolor(int x, int y);
void setcursor(int x, int y);
void clrscr();
void scrollback(int lines);
void putc(char c);
void puts(const char* str);
void printf_unsigned(unsigned long long number, int radix);
void printf_signed(long long number, int radix);
void printf(const char* fmt, ...);
void print_buffer(const char* msg, const void* buffer, uint32_t count);
