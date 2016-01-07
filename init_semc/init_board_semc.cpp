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

#include "init_board.h"
#include "init_common.h"

// Board: set led brightness
void board_led_brightness(unsigned char, unsigned char, unsigned char)
{
}

// Board: set led colors
void board_led_color(unsigned char r, unsigned char g, unsigned char b)
{
    write_int("/sys/class/leds/red/brightness", r);
    write_int("/sys/class/leds/green/brightness", g);
    write_int("/sys/class/leds/blue/brightness", b);
}

// Board: set hardware vibrator
void board_vibrate(unsigned int strength)
{
    write_int("/sys/class/timed_output/vibrator/enable", strength);
}

// Board: introduction for keycheck
void board_introduce_keycheck()
{
    // Short vibration
    board_vibrate(30);

    // Amber LED
    board_led_color(255, 0, 255);
}

// Board: introduction for Android
void board_introduce_android()
{
    // Power off LED
    board_led_color(0, 0, 0);
}

// Board: introduction for Recovery
void board_introduce_recovery()
{
    // Short vibration
    board_vibrate(30);

    // Power off LED
    board_led_color(0, 0, 0);
}
