TARGET_ASMFLAGS += -f elf
TARGET_CFLAGS += -ffreestanding -nostdlib
TARGET_LIBS += -lgcc
TARGET_LINKFLAGS += -T linker.ld -nostdlib

SOURCES_C=$(wildcard *.c)
SOURCES_ASM=$(wildcard *.asm)
OBJECTS_C=$(patsubst %.c, $(BUILD_DIR)/libs/core/c/%.obj, $(SOURCES_C))
OBJECTS_ASM=$(patsubst %.asm, $(BUILD_DIR)/libs/core/asm/%.obj, $(SOURCES_ASM))
BOOT_A=$(BUILD_DIR)/boot.a
UTIL_A=$(BUILD_DIR)/util.a

.PHONY: all core clean always

all: core

core: $(BUILD_DIR)/core.a

$(BUILD_DIR)/core.a: $(OBJECTS_ASM) $(OBJECTS_C) $(BOOT_A) $(UTIL_A)
	@mkdir -p $(@D)
	@$(AR) rcs $@ $^
	@echo "--> Created core.a"

$(BUILD_DIR)/libs/core/c/%.obj: %.c
	@mkdir -p $(@D)
	@$(TARGET_CC) $(TARGET_CFLAGS) -g -c -o $@ $<
	@echo "--> Compiled: " $<

$(BUILD_DIR)/libs/core/asm/%.obj: %.asm
	@mkdir -p $(@D)
	@$(TARGET_ASM) $(TARGET_ASMFLAGS) -o $@ $<
	@echo "--> Compiled: " $<

clean:
	@rm -f $(BUILD_DIR)/core.a