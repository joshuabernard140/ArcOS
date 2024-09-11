#pragma once
#include "stdint.h"
#include "../../libs/boot/bootparams.h"

#define SEG(segoff) (segoff>>16)
#define OFF(segoff) (segoff&0xFFFF)
#define SEGOFF2LIN(segoff) ((SEG(segoff) << 4) + OFF(segoff))
#define MAX_REGIONS 256

void* memcpy(void* dst, const void* src, uint16_t num);
void* memset(void* ptr, int value, uint16_t num);
int memcmp(const void* ptr1, const void* ptr2, uint16_t num);
void* segoffset_to_linear(void* addr);
void mem_Detect(MemoryInfo* memoryInfo);