#pragma once
#include <stddef.h>

void quicksort(void* base, size_t num, size_t size, int (*compar)(const void*, const void*));