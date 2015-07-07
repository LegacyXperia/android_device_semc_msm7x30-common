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
for i in $(busybox seq 0 12); do
    busybox mknod -m 600 /dev/input/event${i} c 13 $(busybox expr 64 + ${i})
done
MTDCACHE=`busybox cat /proc/mtd | busybox grep cache | busybox awk -F ':' {'print $1'} | busybox sed 's/mtd//'`
busybox mknod -m 600 /dev/block/mtdblock${MTDCACHE} b 31 $MTDCACHE

# mount cache
busybox mount -t yaffs2 /dev/block/mtdblock${MTDCACHE} /cache

# leds configuration
BOOTREC_LED_RED="/sys/class/leds/red/brightness"
BOOTREC_LED_GREEN="/sys/class/leds/green/brightness"
BOOTREC_LED_BLUE="/sys/class/leds/blue/brightness"

# trigger amber LED
busybox echo 30 > /sys/class/timed_output/vibrator/enable
busybox echo 255 > ${BOOTREC_LED_RED}
busybox echo 0 > ${BOOTREC_LED_GREEN}
busybox echo 255 > ${BOOTREC_LED_BLUE}

# keycheck
busybox timeout -t 3 keycheck

# boot decision
if [ $? -eq 42 ] || busybox grep -q recovery /cache/recovery/boot
then
    busybox echo 'RECOVERY BOOT' >>boot.txt
    busybox rm -fr /cache/recovery/boot
    # unpack the recovery ramdisk
    busybox cpio -i < /sbin/ramdisk-recovery.cpio
    # remove boot & recovery partitions from recovery fstab
    busybox sed -i '/boot/d' /etc/recovery.fstab
    busybox sed -i '/recovery/d' /etc/recovery.fstab
else
    busybox echo 'ANDROID BOOT' >>boot.txt
    # unpack the android ramdisk
    busybox cpio -i < /sbin/ramdisk.cpio
fi

# poweroff LED
busybox echo 30 > /sys/class/timed_output/vibrator/enable
busybox echo 0 > ${BOOTREC_LED_RED}
busybox echo 0 > ${BOOTREC_LED_GREEN}
busybox echo 0 > ${BOOTREC_LED_BLUE}

busybox umount /cache
busybox umount /proc
busybox umount /sys

busybox rm -fr /dev/*
export PATH="${_PATH}"
exec /init
