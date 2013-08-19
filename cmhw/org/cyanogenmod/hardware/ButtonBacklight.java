/*
 * Copyright (C) 2013 The CyanogenMod Project
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

package org.cyanogenmod.hardware;

import org.cyanogenmod.hardware.util.FileUtils;

import java.io.File;

public class ButtonBacklight {
    private static final String[] BUTTON_BRIGHTNESS_PATHS = new String[] {
            "/sys/class/leds/button-backlight-rgb1/max_brightness",
            "/sys/class/leds/button-backlight-rgb2/max_brightness",
            "/sys/class/leds/button-backlight-rgb1/brightness",
            "/sys/class/leds/button-backlight-rgb2/brightness"
    };

    /**
     * @return Whether the device has button backlight or not
     */
    public static boolean isSupported() {
        File f = new File(BUTTON_BRIGHTNESS_PATHS[0]);
        return f.exists();
    }

    public static int getMaxBrightness() {
        return 255;
    }

    public static int getMinBrightness() {
        return 0;
    }

    public static int getCurBrightness() {
        return Integer.parseInt(FileUtils.readOneLine(BUTTON_BRIGHTNESS_PATHS[0]));
    }

    public static int getDefaultBrightness() {
        return 255;
    }

    public static boolean setBrightness(int brightness) {
        for (int i = 0; i < 4; ++i) {
            FileUtils.writeLine(BUTTON_BRIGHTNESS_PATHS[i], String.valueOf(brightness));
        }
        return true;
    }
}
