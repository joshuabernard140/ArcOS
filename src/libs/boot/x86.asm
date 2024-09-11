; make new call frame
    ;push ebp ; save old call frame
    ;mov ebp, esp ; initialize new call frame

; restore old call frame
    ;mov esp, ebp
    ;pop ebp
    ;ret    

%macro x86_EnterRealMode 0
    [bits 32]
    jmp dword 18h:.pmode16

.pmode16:
    [bits 16]
    mov eax, cr0
    and al, ~1
    mov cr0, eax

    jmp dword 00h:.rmode

.rmode:
    mov ax, 0
    mov ds, ax
    mov ss, ax

    sti
%endmacro

%macro x86_EnterProtectedMode 0
    cli

    mov eax, cr0
    or al, 1
    mov cr0, eax

    jmp dword 08h:.pmode

.pmode:
    [bits 32]

    mov ax, 0x10
    mov ds, ax
    mov ss, ax
%endmacro

; Convert linear address to segment:offset address
; Args:
;    1 - linear address
;    2 - (out) target segment (e.g. es)
;    3 - target 32-bit register to use (e.g. eax)
;    4 - target lower 16-bit half of #3 (e.g. ax)
%macro LinearToSegOffset 4
    mov %3, %1
    shr %3, 4
    mov %2, %4
    mov %3, %1
    and %3, 0xf
%endmacro

global x86_outb
x86_outb:
    [bits 32]
    mov dx, [esp + 4]
    mov al, [esp + 8]
    out dx, al
    ret

global x86_inb
x86_inb:
    [bits 32]
    mov dx, [esp + 4]
    xor eax, eax
    in al, dx
    ret


global x86_Disk_GetDriveParams
x86_Disk_GetDriveParams:
    [bits 32]
    push ebp
    mov ebp, esp

    x86_EnterRealMode

    [bits 16]
    push es
    push bx
    push esi
    push di

    mov dl, [bp + 8]
    mov ah, 08h
    mov di, 0 ; es:di - 0000:0000
    mov es, di
    stc
    int 13h

    mov eax, 1
    sbb eax, 0

    LinearToSegOffset [bp + 12], es, esi, si
    mov [es:si], bl

    ; Cylinders
    mov bl, ch ; Lower bits in ch
    mov bh, cl ; Upper bits in cl (6-7)
    shr bh, 6
    inc bx

    LinearToSegOffset [bp + 16], es, esi, si
    mov [es:si], bx

    ; Sectors
    xor ch, ch ; lower 5 bits in cl
    and cl, 3Fh
    
    LinearToSegOffset [bp + 20], es, esi, si
    mov [es:si], cx

    ; Heads
    mov cl, dh
    inc cx

    LinearToSegOffset [bp + 24], es, esi, si
    mov [es:si], cx

    pop di
    pop esi
    pop bx
    pop es

    push eax

    x86_EnterProtectedMode

    [bits 32]
    pop eax

    mov esp, ebp
    pop ebp
    ret

global x86_Disk_Reset
x86_Disk_Reset:
    [bits 32]
    push ebp
    mov ebp, esp

    x86_EnterRealMode

    mov ah, 0
    mov dl, [bp + 8]
    stc
    int 13h

    mov eax, 1
    sbb eax, 0 

    push eax

    x86_EnterProtectedMode

    pop eax

    mov esp, ebp
    pop ebp
    ret

global x86_Disk_Read
x86_Disk_Read:

    push ebp
    mov ebp, esp

    x86_EnterRealMode

    push ebx
    push es

    mov dl, [bp + 8] ; drive

    mov ch, [bp + 12] ; cylinder (lower 8 bits)
    mov cl, [bp + 13] ; cylinder to bits 6-7
    shl cl, 6
    
    mov al, [bp + 16] ; sector to bits 0-5
    and al, 3Fh
    or cl, al

    mov dh, [bp + 20] ; head

    mov al, [bp + 24] ; count

    LinearToSegOffset [bp + 28], es, ebx, bx

    mov ah, 02h
    stc
    int 13h

    mov eax, 1
    sbb eax, 0  

    pop es
    pop ebx

    push eax

    x86_EnterProtectedMode

    pop eax

    mov esp, ebp
    pop ebp
    ret

global x86_Video_GetVbeInfo
x86_Video_GetVbeInfo:

    push ebp
    mov ebp, esp

    x86_EnterRealMode

    push edi
    push es
    push ebp

    mov ax, 0x4f00
    LinearToSegOffset [bp + 8], es, edi, di
    int 10h

    cmp al, 4fh
    jne .error
    
    mov al, ah
    and eax, 0xFF
    jmp .cont

.error:
    mov eax, -1

.cont:
    pop ebp
    pop es
    pop ebx

    push eax

    x86_EnterProtectedMode

    pop eax

    mov esp, ebp
    pop ebp
    ret

global x86_Video_GetModeInfo
x86_Video_GetModeInfo:

    push ebp
    mov ebp, esp

    x86_EnterRealMode

    push edi
    push es
    push ebp
    push ecx

    mov ax, 0x4f01
    mov cx, [bp + 8]
    LinearToSegOffset [bp + 12], es, edi, di
    int 10h

    cmp al, 4fh
    jne .error
    
    mov al, ah
    and eax, 0xFF
    jmp .cont

.error:
    mov eax, -1

.cont:
    pop ecx
    pop ebp
    pop es
    pop ebx

    push eax

    x86_EnterProtectedMode

    pop eax

    mov esp, ebp
    pop ebp
    ret

global x86_Video_SetMode
x86_Video_SetMode:

    push ebp
    mov ebp, esp

    x86_EnterRealMode

    push edi
    push es
    push ebp
    push ebx

    mov ax, 0
    mov es, ax
    mov edi, 0
    mov ax, 0x4f02
    mov bx, [bp + 8]
    int 10h

    cmp al, 4fh
    jne .error
    
    mov al, ah
    and eax, 0xFF
    jmp .cont

.error:
    mov eax, -1

.cont:
    pop ebx
    pop ebp
    pop es
    pop ebx

    push eax

    x86_EnterProtectedMode

    pop eax

    mov esp, ebp
    pop ebp
    ret

; int ASMCALL x86_E820GetNextBlock(E820MemoryBlock* block, uint32_t* continuationId);
E820Signature equ 0x534D4150

global x86_E820GetNextBlock
x86_E820GetNextBlock:

    push ebp
    mov ebp, esp

    x86_EnterRealMode

    push ebx
    push ecx
    push edx
    push esi
    push edi
    push ds
    push es

    LinearToSegOffset [bp + 8], es, edi, di ; es:di pointer to structure
    
    LinearToSegOffset [bp + 12], ds, esi, si ; ebx - pointer to continuationId
    mov ebx, ds:[si]

    mov eax, 0xE820 ; Function
    mov edx, E820Signature ; Signature
    mov ecx, 24 ; Size of structure

    int 0x15

    cmp eax, E820Signature
    jne .Error

    .IfSuccedeed:
        mov eax, ecx ; Return size
        mov ds:[si], ebx ; Fill continuation parameter
        jmp .EndIf

    .Error:
        mov eax, -1

    .EndIf:

    pop es
    pop ds
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx

    push eax

    x86_EnterProtectedMode

    pop eax

    mov esp, ebp
    pop ebp
    ret

global x86_WaitForEnter
x86_WaitForEnter:

    push ebp
    mov ebp, esp

    x86_EnterRealMode

    push ebx
    push es

.wait_for_enter:
    xor ah, ah
    int 0x16 ; Wait for key press
    cmp al, 0x0D ; Compare to Enter key (ASCII 0x0D)
    jne .wait_for_enter

    mov al, 1 ; If enter key pressed, return true

    pop es
    pop ebx

    x86_EnterProtectedMode

    mov esp, ebp
    pop ebp
    ret