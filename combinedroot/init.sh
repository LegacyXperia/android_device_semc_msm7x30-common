#!/sbin/busybox sh
# Copyright (C) 2011-2013 The CyanogenMod Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set +x
_PATH="$PATH"
export PATH=/sbin

busybox cd /
busybox date >>boot.txt
exec >>boot.txt 2>&1
busybox rm /init

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
BOOTREC_LED_RED="/sys/class/leds/red/brightness"
BOOTREC_LED_GREEN="/sys/class/leds/green/brightness"
BOOTREC_LED_BLUE="/sys/class/leds/blue/brightness"
BOOTREC_LED_BUTTONS="/sys/class/leds/button-backlight/brightness"
BOOTREC_LED_BUTTONS_RGB1="/sys/class/leds/button-backlight-rgb1/brightness"
BOOTREC_LED_BUTTONS_RGB2="/sys/class/leds/button-backlight-rgb2/brightness"

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
busybox echo 255 > ${BOOTREC_LED_RED}
busybox echo 0 > ${BOOTREC_LED_GREEN}
busybox echo 255 > ${BOOTREC_LED_BLUE}
busybox echo 255 > ${BOOTREC_LED_BUTTONS}
busybox echo 255 > ${BOOTREC_LED_BUTTONS_RGB1}
busybox echo 255 > ${BOOTREC_LED_BUTTONS_RGB2}

# keycheck
busybox cat /dev/input/event${keypad_input} > /dev/keycheck&
busybox echo $! > /dev/keycheck.pid
busybox sleep 3
echo 30 > /sys/class/timed_output/vibrator/enable
busybox kill -9 $(cat /dev/keycheck.pid)

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
	busybox echo 0 > ${BOOTREC_LED_RED}
	busybox echo 0 > ${BOOTREC_LED_GREEN}
	busybox echo 255 > ${BOOTREC_LED_BLUE}
	busybox echo 0 > ${BOOTREC_LED_BUTTONS}
	busybox echo 0 > ${BOOTREC_LED_BUTTONS_RGB1}
	busybox echo 0 > ${BOOTREC_LED_BUTTONS_RGB2}
	# framebuffer fix
	busybox echo 0 > /sys/module/msm_fb/parameters/align_buffer
	# recovery ramdisk
	load_image=/sbin/ramdisk-recovery.cpio
	busybox sleep 2
else
	busybox echo 'ANDROID BOOT' >>boot.txt
	# poweroff LED & button-backlight
	busybox echo 0 > ${BOOTREC_LED_RED}
	busybox echo 0 > ${BOOTREC_LED_GREEN}
	busybox echo 0 > ${BOOTREC_LED_BLUE}
	busybox echo 0 > ${BOOTREC_LED_BUTTONS}
	busybox echo 0 > ${BOOTREC_LED_BUTTONS_RGB1}
	busybox echo 0 > ${BOOTREC_LED_BUTTONS_RGB2}
	# framebuffer fix
	busybox echo 1 > /sys/module/msm_fb/parameters/align_buffer
fi

# unpack the ramdisk image
busybox cpio -i < ${load_image}

busybox umount /cache
busybox umount /proc
busybox umount /sys

busybox rm -fr /dev/*
busybox date >>boot.txt
export PATH="${_PATH}"
exec /init
