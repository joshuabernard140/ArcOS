set -e

export FILESYSTEM=${2:-fat16}
TARGET=$1
SIZE=$2

STAGE1_STAGE2_LOCATION_OFFSET=393
DISK_SECTOR_COUNT=$(((${SIZE} + 511 ) / 512))
DISK_PART1_BEGIN=2048
DISK_PART1_END=$((${DISK_SECTOR_COUNT} - 1))

#Generate image file
echo "Generating disk image ${TARGET} (${DISK_SECTOR_COUNT} sectors)"
dd if=/dev/zero of=$TARGET bs=512 count=${DISK_SECTOR_COUNT} >/dev/null

#Create partition table
echo "Creating partition"
parted -s $TARGET mklabel msdos
parted -s $TARGET mkpart primary ${DISK_PART1_BEGIN}s ${DISK_PART1_END}s
parted -s $TARGET set 1 boot on

STAGE2_SIZE=$(stat -c%s ${BUILD_DIR}/stage2.bin)
echo ${STAGE2_SIZE}
STAGE2_SECTORS=$(((${STAGE2_SIZE} + 511) / 512))
echo ${STAGE2_SECTORS}

echo ${DISK_PART1_BEGIN} - 1
if [${STAGE2_SECTORS} -gt $((${DISK_PART1_BEGIN} - 1))]; then
    echo "Stage2 too big"
    exit 2
fi

dd if=${BUILD_DIR}/stage2.bin of=$TARGET conv=notrunc bs=512 seek=1

#Create loopback device
DEVICE=$(losetup -fP --show ${TARGET})
echo "Created loopback device ${DEVICE}"
TARGET_PARTITION="${DEVICE}p1"

#Create file system
echo "Formatting ${TARGET_PARTITION}"
mkfs.fat -n "OS" $TARGET_PARTITION >/dev/null

#Install bootloader
echo "Installing bootloader on ${TARGET_PARTITION}"
dd if=${BUILD_DIR}/stage1.bin of=$TARGET_PARTITION conv=notrunc bs=1 count=3 2>&1 >/dev/null
dd if=${BUILD_DIR}/stage1.bin of=$TARGET_PARTITION conv=notrunc bs=1 seek=62 skip=62 2>&1 >/dev/null

#Write LBA address of stage2 to bootloader
echo "01 00 00 00" | xxd -r -p | dd of=$TARGET_PARTITION conv=notrunc bs=1 seek=$STAGE1_STAGE2_LOCATION_OFFSET
printf "%x" ${STAGE2_SECTORS} | xxd -r -p | dd of=$TARGET_PARTITION conv=notrunc bs=1 seek=$(($STAGE1_STAGE2_LOCATION_OFFSET + 4))

#Copy files
echo "Copying files to ${TARGET_PARTITION} (mounted on /tmp/OS)"
mkdir -p /tmp/OS
mount ${TARGET_PARTITION} /tmp/OS
cp ${BUILD_DIR}/kernel.elf /tmp/OS
mkdir /tmp/OS/root
cp ${SOURCE_DIR}/image/root/test.txt /tmp/OS/root
umount /tmp/OS

#Destroy loopback device
losetup -d ${DEVICE}