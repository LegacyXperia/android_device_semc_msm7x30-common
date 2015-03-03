/*
 * Copyright (C) 2013-2015 The CyanogenMod Project
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

    private static final File auo = new File(AUO_SYSFS);
    private static final File hitachi = new File(HITACHI_SYSFS);
    private static final File sii = new File(SII_SYSFS);
    private static final File sony = new File(SONY_SYSFS);

    public static boolean isSupported() {
        return (auo.exists() || hitachi.exists() || sii.exists() || sony.exists());
    }

    public static boolean isEnabled() {
        if (auo.exists()) {
            return Boolean.parseBoolean(FileUtils.readOneLine(AUO_SYSFS));
        } else if (hitachi.exists()) {
            return Boolean.parseBoolean(FileUtils.readOneLine(HITACHI_SYSFS));
        } else if (sii.exists()) {
            return Boolean.parseBoolean(FileUtils.readOneLine(SII_SYSFS));
        } else if (sony.exists()) {
            return Boolean.parseBoolean(FileUtils.readOneLine(SONY_SYSFS));
        } else {
            return false;
        }
    }

    public static boolean setEnabled(boolean status) {
        if (auo.exists()) {
            return FileUtils.writeLine(AUO_SYSFS, status ? "1" : "0");
        } else if (hitachi.exists()) {
            return FileUtils.writeLine(HITACHI_SYSFS, status ? "1" : "0");
        } else if (sii.exists()) {
            return FileUtils.writeLine(SII_SYSFS, status ? "1" : "0");
        } else if (sony.exists()) {
            return FileUtils.writeLine(SONY_SYSFS, status ? "1" : "0");
        } else {
            return false;
        }
    }
}
