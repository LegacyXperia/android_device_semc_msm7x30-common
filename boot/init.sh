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

# create directories
busybox mkdir -m 755 -p /cache
busybox mkdir -m 755 -p /dev/block
busybox mkdir -m 755 -p /dev/input
busybox mkdir -m 555 -p /proc
busybox mkdir -m 755 -p /sys

# create device nodes
busybox mknod -m 600 /dev/block/mmcblk0 b 179 0
busybox mknod -m 600 /dev/block/mtdblock2 b 31 2
# Per linux Documentation/devices.txt
for i in $(busybox seq 0 12); do
    busybox mknod -m 600 /dev/input/event${i} c 13 $(busybox expr 64 + ${i})
done
busybox mknod -m 666 /dev/null c 1 3

# mount filesystems
busybox mount -t proc proc /proc
busybox mount -t sysfs sysfs /sys
busybox mount -t yaffs2 /dev/block/mtdblock2 /cache

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
