TARGET_ASMFLAGS += -f elf
TARGET_CFLAGS += -ffreestanding -nostdlib
TARGET_LIBS += -lgcc
TARGET_LINKFLAGS += -T linker.ld -nostdlib

SOURCES_ASM=$(wildcard *.asm)
OBJECTS_ASM=$(patsubst %.asm, $(BUILD_DIR)/bootloader/stage1/asm/%.obj, $(SOURCES_ASM))

.PHONY: all stage1 clean always

all: stage1

stage1: $(BUILD_DIR)/stage1.bin

$(BUILD_DIR)/stage1.bin: $(OBJECTS_ASM)
	@$(TARGET_LD) $(TARGET_LINKFLAGS) -Wl,-Map=$(BUILD_DIR)/stage1.map -o $@ $^ $(TARGET_LIBS)
	@echo "--> Created  stage1.bin"	

$(BUILD_DIR)/bootloader/stage1/asm/%.obj: %.asm
	@mkdir -p $(@D)
	@$(TARGET_ASM) $(TARGET_ASMFLAGS) -o $@ $<
	@echo "--> Compiled: " $<	

clean:
	@rm -f $(BUILD_DIR)/stage1.bin