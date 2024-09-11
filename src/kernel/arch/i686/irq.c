#include <stddef.h>
#include "irq.h"
#include "pic.h"
#include "io.h"
#include "i8259.h"
#include "../../../libs/core/stdio.h"
#include "../../../libs/core/arrays.h"

#define PIC_REMAP_OFFSET 0x20
#define BUFFER_SIZE 1024

//Keymap for converting scan codes to ASCII (temporary)
static const char keymap[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', /* 9 */
    '9', '0', '-', '=', '\b', /* Backspace */
    '\t', /* Tab */
    'q', 'w', 'e', 'r', /* 19 */
    't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', /* Enter key */
    0, /* 29   - Control */
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', /* 39 */
    '\'', '`',  0, /* Left shift */
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', /* 49 */
    'm', ',', '.', '/',  0, /* Right shift */
    '*',
    0, /* Alt */
    ' ', /* Space bar */
    0, /* Caps lock */
    0, /* 59 - F1 key ... > */
    0, /* F2 key */
    0, /* F3 key */
    0, /* F4 key */
    0, /* F5 key */
    0, /* F6 key */
    0, /* F7 key */
    0, /* F8 key */
    0, /* F9 key */
    0, /* F10 key */
    0, /* 69 - Num lock*/
    0, /* Scroll Lock */
    0, /* Home key */
    0, /* Up Arrow */
    0, /* Page Up */
    '-',
    0, /* Left Arrow */
    0,
    0, /* Right Arrow */
    '+',
    0, /* 79 - End key*/
    0, /* Down Arrow */
    0, /* Page Down */
    0, /* Insert Key */
    0, /* Delete Key */
    0, 0, 0,
    0, /* F11 Key */
    0, /* F12 Key */
    0, /* All other keys are undefined */
};

IRQHandler g_IRQHandlers[16];
static const PICDriver* g_Driver = NULL;

static char keyBuffer[BUFFER_SIZE];
static volatile int bufferHead = 0;
static volatile int bufferTail = 0;

void i686_IRQ_Handler(Registers* regs) {
    int irq = regs->interrupt - PIC_REMAP_OFFSET;
    
    if (g_IRQHandlers[irq] != NULL) {
        g_IRQHandlers[irq](regs);
    } else {
        printf("Unhandled IRQ %d\n", irq);
    }

    g_Driver->SendEndOfInterrupt(irq);
}

void i686_IRQ_Initialize() {
    const PICDriver* drivers[] = {
        i8259_GetDriver(),
    };

    for (int i = 0; i < SIZE(drivers); i++) {
        if (drivers[i]->Probe()) {
            g_Driver = drivers[i];
        }
    }

    if (g_Driver == NULL) {
        printf("Warning: No PIC found!\n");
        return;
    }

    printf("Found %s PIC.\n", g_Driver->Name);
    g_Driver->Initialize(PIC_REMAP_OFFSET, PIC_REMAP_OFFSET + 8, false);

    //Register ISR handlers for each of the 16 irq lines
    for (int i = 0; i < 16; i++) {
        i686_ISR_RegisterHandler(PIC_REMAP_OFFSET + i, i686_IRQ_Handler);
    }

    //Enable interrupts
    i686_EnableInterrupts();

    //Keyboard
    g_IRQHandlers[1] = i686_IRQ_keyboardHandler;
    g_Driver->Unmask(1);
}

void i686_IRQ_keyboardHandler(Registers* regs) {
    uint8_t ioCode = i686_inb(DATA_PORT);
    if (ioCode < 128 && !(ioCode & 0x80)) {
        char c = keymap[ioCode];
        printf("%c", c);

        //Store the character in the buffer
        int nextHead = (bufferHead + 1) % BUFFER_SIZE;
        if (nextHead != bufferTail) {
            keyBuffer[bufferHead] = c;
            bufferHead = nextHead;
        }
    }

    g_Driver->SendEndOfInterrupt(regs->interrupt - PIC_REMAP_OFFSET);
}

char i686_IRQ_ReadChar() {
    //Wait until a character is available
    while (bufferHead == bufferTail) {}

    //Retrieve the character from the buffer
    char c = keyBuffer[bufferTail];
    bufferTail = (bufferTail + 1) % BUFFER_SIZE;
    return c;
}