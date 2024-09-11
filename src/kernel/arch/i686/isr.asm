[bits 32]
extern i686_ISR_Handler

; CPU pushes ss, esp, eflags, cs, eip

%macro ISR_NOERRORCODE 1
global i686_ISR%1:
i686_ISR%1:
    push 0
    push %1
    jmp isr_common
%endmacro

%macro ISR_ERRORCODE 1
global i686_ISR%1:
i686_ISR%1:
    push %1
    jmp isr_common
%endmacro

%include "arch/i686/isrs.inc"

isr_common:
    pusha ; Order: eax, ecx, edx, ebx, esp, ebp, esi, edi

    xor eax, eax
    mov ax, ds
    push eax

    mov ax, 0x10 ; Kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    push esp ; Pass pointer
    call i686_ISR_Handler
    add esp, 4

    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa
    add esp, 8 ; Remove error code and interrupt number
    iret