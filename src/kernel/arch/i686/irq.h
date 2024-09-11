#pragma once
#include <stdbool.h>
#include "isr.h"

typedef void (*IRQHandler)(Registers* regs);

void i686_IRQ_Handler(Registers* regs);
void i686_IRQ_Initialize();
void i686_IRQ_keyboardHandler(Registers* regs);
char i686_IRQ_ReadChar();