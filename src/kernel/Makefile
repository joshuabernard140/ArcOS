TARGET_ASMFLAGS += -f elf
TARGET_CFLAGS += -ffreestanding -nostdlib -I.
TARGET_LIBS += -lgcc
TARGET_LINKFLAGS += -T linker.ld -nostdlib

HEADERS_C = $(wildcard *.h) \
			$(wildcard */*.h) \
			$(wildcard */*/*.h) \
			$(wildcard */*/*/*.h)

SOURCES_C = $(wildcard *.c) \
			$(wildcard */*.c) \
			$(wildcard */*/*.c) \
			$(wildcard */*/*/*.c) \
			arch/i686/isrs.c

OBJECTS_C = $(patsubst %.c, $(BUILD_DIR)/kernel/c/%.obj, $(SOURCES_C))

HEADERS_ASM = $(wildcard *.inc) \
			  $(wildcard */*.inc) \
			  $(wildcard */*/*.inc) \
			  $(wildcard */*/*/*.inc) \
			  arch/i686/isrs.inc

SOURCES_ASM = $(wildcard *.asm) \
			  $(wildcard */*.asm) \
			  $(wildcard */*/*.asm) \
			  $(wildcard */*/*/*.asm)

OBJECTS_ASM = $(patsubst %.asm, $(BUILD_DIR)/kernel/asm/%.obj, $(SOURCES_ASM))

CORE_A=$(BUILD_DIR)/core.a
BOOT_A=$(BUILD_DIR)/boot.a
UTIL_A=$(BUILD_DIR)/util.a

.PHONY: all kernel clean always

all: kernel

kernel: $(BUILD_DIR)/kernel.elf

$(BUILD_DIR)/kernel.elf: $(OBJECTS_ASM) $(OBJECTS_C) $(CORE_A) $(BOOT_A) $(UTIL_A)
	@$(TARGET_LD) $(TARGET_LINKFLAGS) -Wl,-Map=$(BUILD_DIR)/kernel.map -o $@ $^ $(TARGET_LIBS)
	@echo "--> Created:  kernel.elf"

$(BUILD_DIR)/kernel/c/%.obj: %.c $(HEADERS_C)
	@mkdir -p $(@D)
	@$(TARGET_CC) $(TARGET_CFLAGS) -g -c -o $@ $<
	@echo "--> Compiled: " $<

$(BUILD_DIR)/kernel/asm/%.obj: %.asm $(HEADERS_ASM)
	@mkdir -p $(@D)
	@$(TARGET_ASM) $(TARGET_ASMFLAGS) -o $@ $<
	@echo "--> Compiled: " $<

clean:
	rm -f $(BUILD_DIR)/kernel.elf