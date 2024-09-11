TARGET_ASMFLAGS += -f elf
TARGET_CFLAGS += -ffreestanding -nostdlib
TARGET_LIBS += -lgcc
TARGET_LINKFLAGS += -T linker.ld -nostdlib

SOURCES_C=$(wildcard *.c)
SOURCES_ASM=$(wildcard *.asm)
OBJECTS_C=$(patsubst %.c, $(BUILD_DIR)/libs/util/c/%.obj, $(SOURCES_C))
OBJECTS_ASM=$(patsubst %.asm, $(BUILD_DIR)/libs/util/asm/%.obj, $(SOURCES_ASM))

.PHONY: all util clean always

all: util

util: $(BUILD_DIR)/util.a

$(BUILD_DIR)/util.a: $(OBJECTS_ASM) $(OBJECTS_C)
	@mkdir -p $(@D)
	@$(AR) rcs $@ $^
	@echo "--> Created util.a"

$(BUILD_DIR)/libs/util/c/%.obj: %.c
	@mkdir -p $(@D)
	@$(TARGET_CC) $(TARGET_CFLAGS) -g -c -o $@ $<
	@echo "--> Compiled: " $<

$(BUILD_DIR)/libs/util/asm/%.obj: %.asm
	@mkdir -p $(@D)
	@$(TARGET_ASM) $(TARGET_ASMFLAGS) -o $@ $<
	@echo "--> Compiled: " $<

clean:
	@rm -f $(BUILD_DIR)/util.a