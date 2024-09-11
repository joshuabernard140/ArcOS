[bits 32]

; void __attribute__((cdecl)) i686_IDT_Load(IDTDescriptor* idtDescriptor);
global i686_IDT_Load
i686_IDT_Load:

    ; make new call frame
    push ebp
    mov ebp, esp
    
    ; load idt
    mov eax, [ebp + 8]
    lidt [eax]

    ; restore old call frame
    mov esp, ebp
    pop ebp
    ret