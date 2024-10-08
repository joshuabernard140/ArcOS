global i686_outb
i686_outb:
    [bits 32]
    mov dx, [esp + 4]
    mov al, [esp + 8]
    out dx, al
    ret

global i686_outw
i686_outw:
    [bits 32]
    mov dx, [esp + 4]
    mov ax, [esp + 8]
    out dx, ax
    ret

global i686_inb
i686_inb:
    [bits 32]
    mov dx, [esp + 4]
    xor eax, eax
    in al, dx
    ret

global i686_inw
i686_inw:
    [bits 32]
    mov dx, [esp + 4]
    in ax, dx
    ret   

global i686_Panic
i686_Panic:
    cli
    hlt

global i686_EnableInterrupts
i686_EnableInterrupts:
    sti
    ret

global i686_DisableInterrupts
i686_DisableInterrupts:
    cli
    ret