# ArcOS
ArcOS is a custom operating system developed from scratch, built for the x86 i686 architecture. This is a work in progress.

## Features
### Build Scripts:
- Toolchain
- Config
- Floppy Image
- Disk Image

### Bootloader:
- FAT Support 
- Boot Descriptor Block (BDB) 
- Extended Boot Record (EBR)
- Executable and Linkable Format (ELF)
- Master Boot Record (MBR)
- Disk Functions (Initialization, LBA to CHS Conversion, Reading Sectors)
- Kernel Entry (Protected Mode, Global Descriptor Table (GDT), A20 and Keyboard)
- x86 Functions (Real mode, Protected mode, VESA BIOS Extensions (VBE), E820 Memory Map)
- Memory Functions (memdetect, memdefs, memory)
- Standard Functions (ctype, minmax, stdio, stdlib, string)

### Kernel:
- Instruction Set Architecture (i686 ISA)
- Programmable Interrupt Controller (i8259 PIC)
- Hardware Abstraction Layer (HAL)
- Global Descriptor Table (GDT)
- Interrupt Descriptor Table (IDT)
- Interrupt Request (IRQ)
- Interrupt Service Routine (ISR)
- input/Output
- Command Line Interface

## Requirements
- [Linux (only tested in Arch Linux)](https://archlinux.org/)
- [QEMU](https://www.qemu.org/)
- [nasm](https://nasm.us/)
- [mtools](https://www.gnu.org/software/mtools/)
- [dosfstools](https://github.com/dosfstools/dosfstools)
- [Visual Studio Code (Recommended)](https://code.visualstudio.com/)
- [gdb (for debugging)](https://sourceware.org/gdb/)

## Instructions
Any issues with instructions can likely be solved by using sudo: "sudo" and/or chmod: "chmod -R u+rwx,go+rx,go-w OS"
1. Ensure required packages are installed (in Arch Linux, run: "sudo pacman -Syu qemu nasm mtools dosfstools")
2. Use Makefile in OS directory ("sudo make") (building toolchain may take a few minutes)
3. Run run_disk.sh ("sudo ./run_disk.sh") (run_floppy.sh works as well but currently has issues with filesystem in kernel)

## Sources
- [Nanobyte](https://www.youtube.com/@nanobyte-dev)
- [Daedalus Community](https://www.youtube.com/@DaedalusCommunity)
- [Intel Manuals](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html#combined)
- [GCC Cross Compiler (i686-elf)](https://wiki.osdev.org/GCC_Cross-Compiler)
- [Libgcc](https://gcc.gnu.org/onlinedocs/gccint/Libgcc.html)
- [x86 Registers](https://commons.wikimedia.org/wiki/File:Table_of_x86_Registers_svg.svg)
- [x86 Instruction Set Reference](https://c9x.me/x86/)
- [x86 Calling Conventions](https://en.wikipedia.org/wiki/X86_calling_conventions)
- [Cylinder Head Sector (CHS)](https://en.wikipedia.org/wiki/Cylinder-head-sector)
- [CHS to LBA](https://en.wikipedia.org/wiki/Logical_block_addressing#CHS_conversion)
- [FAT (12, 16, 32)](https://wiki.osdev.org/FAT)
- [FAT Design](https://en.wikipedia.org/wiki/Design_of_the_FAT_file_system)
- [Ralph Brown's Interupt List](https://www.ctyme.com/rbrown.htm)
- [OSDev Interrupt List](https://wiki.osdev.org/Interrupts)
- [Non Maskable Interrupt](https://wiki.osdev.org/Non_Maskable_Interrupt)
- [INT 13h](https://www.stanislavs.org/helppc/int_13.html)
- [INT 10h, AX=0Eh](http://www.ctyme.com/intr/rb-0106.htm)
- [Printing](https://wiki.osdev.org/Printing_To_Screen)
- [Protected Mode](https://wiki.osdev.org/Protected_Mode)
- [Segmentation](https://wiki.osdev.org/Segmentation)
- [Master Boot Record (MBR)](https://wiki.osdev.org/MBR_(x86))
- [Global Descriptor Table (GDT)](https://wiki.osdev.org/GDT)
- [GDT Tutorial](https://wiki.osdev.org/GDT_Tutorial)
- [Interrupt Descriptor Table (IDT)](https://wiki.osdev.org/IDT)
- [Exception](https://wiki.osdev.org/Exceptions)
- [Programmable Interrupt Controller (8259 PIC)](https://wiki.osdev.org/8259_PIC)
- [8259 PIC Data Sheet](https://pdos.csail.mit.edu/6.828/2005/readings/hardware/8259A.pdf)
- [Floppy Disk Controller (FDC)](https://wiki.osdev.org/Floppy_Disk_Controller)