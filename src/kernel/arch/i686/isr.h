#pragma once
#include <stdint.h>

typedef struct { //Reversed order they are pushed
    uint32_t ds; //Data segment
    uint32_t edi, esi, ebp, useless, ebx, edx, ecx, eax; //pusha
    uint32_t interrupt, error; //Interrupt pushed, error pushed automatically
    uint32_t eip, cs, eflags, esp, ss; //Pushed automatically by CPU
} __attribute__((packed)) Registers;

typedef void (*ISRHandler)(Registers* regs);

void i686_ISR_Initialize();
void i686_ISR_RegisterHandler(int interrupt, ISRHandler handler);