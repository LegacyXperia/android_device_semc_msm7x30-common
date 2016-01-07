/*
 * Copyright (C) 2016 The CyanogenMod Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>

#include <sys/mount.h>
#include <sys/stat.h>
#include <unistd.h>

#include "init_header.h"

// Function: color to led - Device dependent
static void led_color(unsigned char r, unsigned char g, unsigned char b)
{
    write_int("/sys/class/leds/red/brightness", r);
    write_int("/sys/class/leds/green/brightness", g);
    write_int("/sys/class/leds/blue/brightness", b);
}

// Function: hardware vibrator - Device dependent
static void vibrate(unsigned int strength)
{
    write_int("/sys/class/timed_output/vibrator/enable", strength);
}

// Main: executable - Device dependent
int main(int __attribute__((unused)) argc, char** __attribute__((unused)) argv)
{
    // Execution variables
    bool recoveryBoot;
    bool volKeyPressed;
    int i;

    // Generate boot log
    unlink(BOOT_TXT);
    write_string(BOOT_TXT, "SEMC INIT");
    write_date(BOOT_TXT, true);

    // Delete /init symlink
    unlink("/init");

    // Create directories
    mkdir("/cache", 0755);
    mkdir("/dev/block", 0755);
    mkdir("/dev/input", 0755);
    mkdir("/proc", 0555);
    mkdir("/sys", 0755);

    // Create device nodes
    mknod("/dev/block/mtdblock2", S_IFBLK | 0600, makedev(31, 2));
    for (i = 0; i <= 12; i++)
        mknod("/dev/input/event" + i, S_IFCHR | 0600, makedev(13, 64 + i));
    mknod("/dev/null", S_IFCHR | 0666, makedev(1, 3));

    // Mount filesystems
    mount("proc", "/proc", "proc", 0, NULL);
    mount("sysfs", "/sys", "sysfs", 0, NULL);
    mount("/dev/block/mtdblock2", "/cache", "yaffs2", 0, NULL);

    // Short vibration
    vibrate(30);

    // Amber LED
    led_color(255, 0, 255);

    // Check keys for recovery
    const char* argv_key_check[] = { EXEC_BOX, "timeout",
            "3", "/sbin/keycheck", nullptr };
    volKeyPressed = (system_exec(argv_key_check) == 42);

    // Recovery boot detection
    const char* argv_boot_recovery[] = { EXEC_BOX, "grep",
            "-q", "recovery", "/cache/recovery/boot", nullptr };
    recoveryBoot = (system_exec(argv_boot_recovery) == 0);

    // Keys boot decision
    if (recoveryBoot || volKeyPressed) {
        // Recovery boot
        write_string(BOOT_TXT, "RECOVERY BOOT", true);
        vibrate(30);

        // Recovery ramdisk
        const char* argv_ramdiskcpio[] = { EXEC_BOX, "cpio", "-i", "-F",
                "/sbin/ramdisk-recovery.cpio", nullptr };
        system_exec(argv_ramdiskcpio);

        // Remove boot & recovery partitions from recovery fstab
        const char* argv_fstab_fix_boot[] = { EXEC_BOX, "sed", "-i",
                "/boot/d", "/etc/recovery.fstab", nullptr };
        system_exec(argv_fstab_fix_boot);
        const char* argv_fstab_fix_recovery[] = { EXEC_BOX, "sed", "-i",
                "/recovery/d", "/etc/recovery.fstab", nullptr };
        system_exec(argv_fstab_fix_recovery);
    } else {
        // Android boot
        write_string(BOOT_TXT, "ANDROID BOOT", true);

        // Android ramdisk
        const char* argv_ramdiskcpio[] = { EXEC_BOX, "cpio", "-i", "-F",
                "/sbin/ramdisk.cpio", nullptr };
        system_exec(argv_ramdiskcpio);

        // Remove boot & recovery partitions from android fstab
        const char* argv_fstab_fix_boot[] = { EXEC_BOX, "sed", "-i",
                "/boot/d", "/etc/fstab.semc", nullptr };
        system_exec(argv_fstab_fix_boot);
        const char* argv_fstab_fix_recovery[] = { EXEC_BOX, "sed", "-i",
                "/recovery/d", "/etc/fstab.semc", nullptr };
        system_exec(argv_fstab_fix_recovery);
    }

    // Poweroff LED
    led_color(0, 0, 0);

    // Unmount filesystems
    umount("/cache");
    umount("/proc");
    umount("/sys");

    // End init_semc
    const char* argv_rm_dev[] = { EXEC_BOX, "rm", "-fr", "/dev/*", nullptr };
    system_exec(argv_rm_dev);
    write_date(BOOT_TXT, true);

    // Launch ramdisk /init
    const char* argv_init[] = { "/init", nullptr };
    system_exec(argv_init);

    return 0;
}
