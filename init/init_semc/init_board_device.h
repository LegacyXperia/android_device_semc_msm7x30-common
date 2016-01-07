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

#ifndef __INIT_BOARD_DEVICE_H__
#define __INIT_BOARD_DEVICE_H__

#include "init_board_common.h"
#include "init_prototypes.h"

// Constants: warmboot commands
#define WARMBOOT_CMDLINE "/cache/recovery/boot"
#define WARMBOOT_RECOVERY "recovery"

// Class: init_board_device
class init_board_device : public init_board_common
{
public:
    // Board: start init execution
    virtual void start_init()
    {
        // Mount /cache
        mkdir("/cache", 0755);
        mknod("/dev/block/mtdblock2", S_IFBLK | 0600, makedev(31, 2));
        mount("/dev/block/mtdblock2", "/cache", "yaffs2", 0, NULL);
    }

    // Board: introduction for keycheck
    virtual void introduce_keycheck()
    {
        // Short vibration
        vibrate(30);

        // Amber LED
        led_color(255, 0, 255);
    }

    // Board: introduction for Android
    virtual void introduce_android()
    {
        // Green LED
        led_color(0, 255, 0);
        msleep(250);
    }

    // Board: introduction for Recovery
    virtual void introduce_recovery()
    {
        // Delete the recovery warmboot file
        unlink(WARMBOOT_CMDLINE);

        // Short vibration
        vibrate(30);

        // Blue LED
        led_color(0, 0, 255);
        msleep(250);
    }

    // Board: finish init execution
    virtual void finish_init()
    {
        // Remove boot & recovery partitions from recovery fstab
        const char* argv_fstab_fix_boot[] = { EXEC_TOYBOX, "sed", "-i",
                "/boot/d", "/etc/recovery.fstab", nullptr };
        system_exec(argv_fstab_fix_boot);
        const char* argv_fstab_fix_recovery[] = { EXEC_TOYBOX, "sed", "-i",
                "/recovery/d", "/etc/recovery.fstab", nullptr };
        system_exec(argv_fstab_fix_recovery);

        // Power off LED and vibrator
        led_color(0, 0, 0);
        vibrate(0);

        // Unmount /cache
        umount("/cache");
        rmdir("/cache");
    }

    // Board: set led colors
    void led_color(uint8_t r, uint8_t g, uint8_t b)
    {
        write_int("/sys/class/leds/red/brightness", r);
        write_int("/sys/class/leds/green/brightness", g);
        write_int("/sys/class/leds/blue/brightness", b);
    }

    // Board: set hardware vibrator
    void vibrate(uint8_t strength)
    {
        write_int("/sys/class/timed_output/vibrator/enable", strength);
    }
};

#endif //__INIT_BOARD_DEVICE_H__
