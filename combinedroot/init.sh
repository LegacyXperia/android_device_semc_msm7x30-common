#!/sbin/busybox sh
set +x
_PATH="$PATH"
export PATH=/sbin

busybox cd /
busybox date >>boot.txt
exec >>boot.txt 2>&1
busybox rm /init

# include device specific vars
source /sbin/bootrec-device

# create directories & mount filesystems
busybox mount -o remount,rw rootfs /

busybox mkdir -p /sys /tmp /proc /data /dev /system/bin /cache
busybox mount -t sysfs sysfs /sys
busybox mount -t proc proc /proc
busybox mkdir /dev/input /dev/graphics /dev/block /dev/log

# create device nodes
busybox mknod -m 666 /dev/null c 1 3
busybox mknod -m 666 /dev/graphics/fb0 c 29 0
busybox mknod -m 666 /dev/tty0 c 4 0
busybox mknod -m 600 /dev/block/mmcblk0 b 179 0
busybox mknod -m 666 /dev/log/system c 10 19
busybox mknod -m 666 /dev/log/radio c 10 20
busybox mknod -m 666 /dev/log/events c 10 21
busybox mknod -m 666 /dev/log/main c 10 22
busybox mknod -m 666 /dev/ashmem c 10 37
busybox mknod -m 666 /dev/urandom c 1 9
for i in 0 1 2 3 4 5 6 7 8 9
do
	num=`busybox expr 64 + $i`
	busybox mknod -m 600 /dev/input/event${i} c 13 $num
done
MTDCACHE=`busybox cat /proc/mtd | busybox grep cache | busybox awk -F ':' {'print $1'} | busybox sed 's/mtd//'`
busybox mknod -m 600 /dev/block/mtdblock${MTDCACHE} b 31 $MTDCACHE

# leds & backlight configuration
busybox echo ${BOOTREC_LED_RED_CURRENT} > ${BOOTREC_LED_RED}/max_current
busybox echo ${BOOTREC_LED_GREEN_CURRENT} > ${BOOTREC_LED_GREEN}/max_current
busybox echo ${BOOTREC_LED_BLUE_CURRENT} > ${BOOTREC_LED_BLUE}/max_current
busybox echo ${BOOTREC_LED_BUTTONS_CURRENT} > ${BOOTREC_LED_BUTTONS}/max_current
busybox echo ${BOOTREC_LED_BUTTONS2_CURRENT} > ${BOOTREC_LED_BUTTONS2}/max_current
busybox echo ${BOOTREC_LED_LCD_CURRENT} > ${BOOTREC_LED_LCD}/max_current
busybox echo ${BOOTREC_LED_LCD_MODE} > ${BOOTREC_LED_LCD}/mode

keypad_input=''
for input in `busybox ls -d /sys/class/input/input*`
do
	type=`busybox cat ${input}/name`
	case "$type" in
    (*keypad*) keypad_input=`busybox echo $input | busybox sed 's/^.*input//'`;;
    (*)        ;;
    esac
done

# trigger amber LED & button-backlight
busybox echo 255 > ${BOOTREC_LED_RED}/brightness
busybox echo 0 > ${BOOTREC_LED_GREEN}/brightness
busybox echo 255 > ${BOOTREC_LED_BLUE}/brightness
busybox echo 255 > ${BOOTREC_LED_BUTTONS}/brightness
busybox echo 255 > ${BOOTREC_LED_BUTTONS2}/brightness

# keycheck
busybox cat /dev/input/event${keypad_input} > /dev/keycheck&
busybox sleep 3

# mount cache
busybox mount -t yaffs2 /dev/block/mtdblock${MTDCACHE} /cache

# android ramdisk
load_image=/sbin/ramdisk.cpio

# boot decision
if [ -s /dev/keycheck -o -e /cache/recovery/boot ]
then
	busybox echo 'RECOVERY BOOT' >>boot.txt
	busybox rm -fr /cache/recovery/boot
	# trigger blue led
	busybox echo 0 > ${BOOTREC_LED_RED}/brightness
	busybox echo 0 > ${BOOTREC_LED_GREEN}/brightness
	busybox echo 255 > ${BOOTREC_LED_BLUE}/brightness
	busybox echo 0 > ${BOOTREC_LED_BUTTONS}/brightness
	busybox echo 0 > ${BOOTREC_LED_BUTTONS2}/brightness
	# framebuffer fix
	busybox echo 0 > /sys/module/msm_fb/parameters/align_buffer
	# recovery ramdisk
	load_image=/sbin/ramdisk-recovery.cpio
else
	busybox echo 'ANDROID BOOT' >>boot.txt
	# poweroff LED & button-backlight
	busybox echo 0 > ${BOOTREC_LED_RED}/brightness
	busybox echo 0 > ${BOOTREC_LED_GREEN}/brightness
	busybox echo 0 > ${BOOTREC_LED_BLUE}/brightness
	busybox echo 0 > ${BOOTREC_LED_BUTTONS}/brightness
	busybox echo 0 > ${BOOTREC_LED_BUTTONS2}/brightness
	# framebuffer fix
	busybox echo 1 > /sys/module/msm_fb/parameters/align_buffer
fi

# kill the keycheck process
busybox pkill -f "busybox cat ${BOOTREC_EVENT}"

# unpack the ramdisk image
busybox cpio -i < ${load_image}

busybox umount /cache
busybox umount /proc
busybox umount /sys

busybox rm -fr /dev/*
busybox date >>boot.txt
export PATH="${_PATH}"
exec /init
