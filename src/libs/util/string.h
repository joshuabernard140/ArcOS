#pragma once
#include <stddef.h>

const char* strchr(const char* str, char chr);
char* strcpy(char* dst, const char* src);
char* strncpy(char *dest, const char *src, size_t n);
unsigned strlen(const char* str);
int strcmp(const char *str1, const char *str2);
int strncmp(const char *str1, const char *str2, size_t n);
wchar_t* utf16_to_codepoint(wchar_t* string, int* codepoint);
char* codepoint_to_utf8(int codepoint, char* stringOutput);