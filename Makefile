include build_scripts/config.mk

.PHONY: all floppy_image disk_image toolchain libs util boot core bootloader stage1 stage2 kernel clean always

all: floppy_image disk_image

include build_scripts/toolchain.mk

#Toolchain
toolchain: $(SOURCE_DIR)/toolchain

$(SOURCE_DIR)/toolchain:
	@if [ ! -d $@ ]; then \
        $(MAKE) -f ./build_scripts/toolchain.mk; \
		touch $(SOURCE_DIR)/toolchain/.built; \
		echo "--> Created: " $@; \
	else \
		echo "--> $@ already exists"; \
	fi

#Floppy image
floppy_image: $(BUILD_DIR)/main_floppy.img

$(BUILD_DIR)/main_floppy.img: bootloader kernel
	@if [ ! -f $@ ]; then \
		./build_scripts/make_floppy_image.sh $@; \
		echo "--> Created: " $@; \
	else \
		echo "--> $@ already exists"; \
	fi

#Disk image
disk_image: $(BUILD_DIR)/main_disk.raw

$(BUILD_DIR)/main_disk.raw: bootloader kernel
	@if [ ! -f $@ ]; then \
        ./build_scripts/make_disk_image.sh $@ $(MAKE_DISK_SIZE); \
        echo "--> Created: " $@; \
    else \
        echo "--> $@ already exists"; \
    fi

#Libs
libs: util boot core

util: $(BUILD_DIR)/util.a

$(BUILD_DIR)/util.a: always
	@$(MAKE) -C src/libs/util BUILD_DIR=$(abspath $(BUILD_DIR))	

boot: $(BUILD_DIR)/boot.a

$(BUILD_DIR)/boot.a: always
	@$(MAKE) -C src/libs/boot BUILD_DIR=$(abspath $(BUILD_DIR))

core: $(BUILD_DIR)/core.a

$(BUILD_DIR)/core.a: always
	@$(MAKE) -C src/libs/core BUILD_DIR=$(abspath $(BUILD_DIR))		

#Bootloader
bootloader: libs stage1 stage2

stage1: $(BUILD_DIR)/stage1.bin

$(BUILD_DIR)/stage1.bin: always
	@$(MAKE) -C src/bootloader/stage1 BUILD_DIR=$(abspath $(BUILD_DIR))

stage2: $(BUILD_DIR)/stage2.bin

$(BUILD_DIR)/stage2.bin: always
	@$(MAKE) -C src/bootloader/stage2 BUILD_DIR=$(abspath $(BUILD_DIR))

#Kernel
kernel: $(BUILD_DIR)/kernel.elf

$(BUILD_DIR)/kernel.elf: always libs
	@$(MAKE) -C src/kernel BUILD_DIR=$(abspath $(BUILD_DIR))

#Always
always: clean toolchain
	@mkdir -p $(BUILD_DIR)

#Clean
clean:
	@$(MAKE) -C src/libs/util BUILD_DIR=$(abspath $(BUILD_DIR)) clean
	@$(MAKE) -C src/libs/boot BUILD_DIR=$(abspath $(BUILD_DIR)) clean
	@$(MAKE) -C src/libs/core BUILD_DIR=$(abspath $(BUILD_DIR)) clean
	@$(MAKE) -C src/bootloader/stage1 BUILD_DIR=$(abspath $(BUILD_DIR)) clean
	@$(MAKE) -C src/bootloader/stage2 BUILD_DIR=$(abspath $(BUILD_DIR)) clean
	@$(MAKE) -C src/kernel BUILD_DIR=$(abspath $(BUILD_DIR)) clean
	@rm -rf $(BUILD_DIR)/*