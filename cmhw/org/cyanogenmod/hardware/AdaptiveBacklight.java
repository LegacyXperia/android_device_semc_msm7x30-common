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

public class AdaptiveBacklight {

    private static final String AUO_SYSFS =
            "/sys/devices/platform/mddi_auo_s6d05a1_hvga/dbc_ctrl";
    private static final String HITACHI_SYSFS =
            "/sys/devices/platform/mddi_hitachi_r61529_hvga/dbc_ctrl";
    private static final String SII_SYSFS =
            "/sys/devices/platform/mddi_sii_r61529_hvga/dbc_ctrl";
    private static final String SONY_SYSFS =
            "/sys/devices/platform/mddi_sony_s6d05a1_hvga/dbc_ctrl";

    public static boolean isSupported() {
        File f1 = new File(AUO_SYSFS);
        File f2 = new File(HITACHI_SYSFS);
        File f3 = new File(SII_SYSFS);
        File f4 = new File(SONY_SYSFS);

        if (f1.exists() || f2.exists() || f3.exists() || f4.exists()) {
            return true;
        } else {
            return false;
        }
    }

    public static boolean isEnabled() {
        return isSupported();
    }

    public static boolean setEnabled(boolean status) {
        File f1 = new File(AUO_SYSFS);
        File f2 = new File(HITACHI_SYSFS);
        File f3 = new File(SII_SYSFS);
        File f4 = new File(SONY_SYSFS);

        if (f1.exists()) {
            return FileUtils.writeLine(AUO_SYSFS, status ? "1" : "0");
        } else if (f2.exists()) {
            return FileUtils.writeLine(HITACHI_SYSFS, status ? "1" : "0");
        } else if (f3.exists()) {
            return FileUtils.writeLine(SII_SYSFS, status ? "1" : "0");
        } else if (f4.exists()) {
            return FileUtils.writeLine(SONY_SYSFS, status ? "1" : "0");
        } else {
            return false;
        }
    }
}
