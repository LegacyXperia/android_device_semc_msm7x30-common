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

package com.cyanogenmod.settings.device;

import android.content.Context;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.CheckBoxPreference;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceFragment;
import android.preference.PreferenceManager;
import android.preference.PreferenceScreen;
import android.util.Log;
import android.view.Gravity;
import android.widget.Toast;

import com.cyanogenmod.settings.device.R;

public class GeneralFragmentActivity extends PreferenceFragment {

    private static final String BUTTON_BRIGHTNESS_RGB1_FILE = "/sys/class/leds/button-backlight-rgb1/max_brightness";
    private static final String BUTTON_BRIGHTNESS_RGB2_FILE = "/sys/class/leds/button-backlight-rgb2/max_brightness";

    private static final String KEYBOARD_BRIGHTNESS_RGB1_FILE = "/sys/class/leds/keyboard-backlight-rgb1/max_brightness";
    private static final String KEYBOARD_BRIGHTNESS_RGB2_FILE = "/sys/class/leds/keyboard-backlight-rgb2/max_brightness";
    private static final String KEYBOARD_BRIGHTNESS_RGB3_FILE = "/sys/class/leds/keyboard-backlight-rgb3/max_brightness";
    private static final String KEYBOARD_BRIGHTNESS_RGB4_FILE = "/sys/class/leds/keyboard-backlight-rgb4/max_brightness";

    private static final String PREF_ENABLED = "1";
    private static final String TAG = "DeviceSettings_General";

    private ListPreference mButtonBrightness;
    private ListPreference mKeyboardBrightness;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        addPreferencesFromResource(R.xml.general_preferences);

        PreferenceScreen prefSet = getPreferenceScreen();
        mButtonBrightness = (ListPreference) findPreference(DeviceSettings.BUTTON_BRIGHTNESS);
        mKeyboardBrightness = (ListPreference) findPreference(DeviceSettings.KEYBOARD_BRIGHTNESS);

        mButtonBrightness.setOnPreferenceChangeListener(new Preference.OnPreferenceChangeListener() {
            public boolean onPreferenceChange(Preference preference, Object newValue) {
                if (isSupported(BUTTON_BRIGHTNESS_RGB1_FILE)) {
                    Utils.writeValue(BUTTON_BRIGHTNESS_RGB1_FILE, (String) newValue);
                    Utils.writeValue(BUTTON_BRIGHTNESS_RGB1_FILE.replace("max_",""), (String) newValue);
                }
                if (isSupported(BUTTON_BRIGHTNESS_RGB2_FILE)) {
                    Utils.writeValue(BUTTON_BRIGHTNESS_RGB2_FILE, (String) newValue);
                    Utils.writeValue(BUTTON_BRIGHTNESS_RGB2_FILE.replace("max_",""), (String) newValue);
                }

                return true;
            }
        });

        mKeyboardBrightness.setOnPreferenceChangeListener(new Preference.OnPreferenceChangeListener() {
            public boolean onPreferenceChange(Preference preference, Object newValue) {
                if (isSupported(KEYBOARD_BRIGHTNESS_RGB1_FILE)) {
                    Utils.writeValue(KEYBOARD_BRIGHTNESS_RGB1_FILE, (String) newValue);
                    Utils.writeValue(KEYBOARD_BRIGHTNESS_RGB1_FILE.replace("max_",""), (String) newValue);
                }
                if (isSupported(KEYBOARD_BRIGHTNESS_RGB2_FILE)) {
                    Utils.writeValue(KEYBOARD_BRIGHTNESS_RGB2_FILE, (String) newValue);
                    Utils.writeValue(KEYBOARD_BRIGHTNESS_RGB2_FILE.replace("max_",""), (String) newValue);
                }
                if (isSupported(KEYBOARD_BRIGHTNESS_RGB3_FILE)) {
                    Utils.writeValue(KEYBOARD_BRIGHTNESS_RGB3_FILE, (String) newValue);
                    Utils.writeValue(KEYBOARD_BRIGHTNESS_RGB3_FILE.replace("max_",""), (String) newValue);
                }
                if (isSupported(KEYBOARD_BRIGHTNESS_RGB4_FILE)) {
                    Utils.writeValue(KEYBOARD_BRIGHTNESS_RGB4_FILE, (String) newValue);
                    Utils.writeValue(KEYBOARD_BRIGHTNESS_RGB4_FILE.replace("max_",""), (String) newValue);
                }

                return true;
            }
        });

        if (!isSupported(BUTTON_BRIGHTNESS_RGB1_FILE)) {
            mButtonBrightness.setEnabled(false);
        }

        if (!isSupported(KEYBOARD_BRIGHTNESS_RGB1_FILE)) {
            mKeyboardBrightness.setEnabled(false);
        }

    }

    public static boolean isSupported(String FILE) {
        return Utils.fileExists(FILE);
    }

    public static void restore(Context context) {
        SharedPreferences sharedPrefs = PreferenceManager.getDefaultSharedPreferences(context);
        if (isSupported(BUTTON_BRIGHTNESS_RGB1_FILE)) {
            Utils.writeValue(BUTTON_BRIGHTNESS_RGB1_FILE, sharedPrefs.getString(DeviceSettings.BUTTON_BRIGHTNESS, "0"));
            Utils.writeValue(BUTTON_BRIGHTNESS_RGB1_FILE.replace("max_",""), sharedPrefs.getString(DeviceSettings.BUTTON_BRIGHTNESS, "0"));
        }
        if (isSupported(BUTTON_BRIGHTNESS_RGB2_FILE)) {
            Utils.writeValue(BUTTON_BRIGHTNESS_RGB2_FILE, sharedPrefs.getString(DeviceSettings.BUTTON_BRIGHTNESS, "0"));
            Utils.writeValue(BUTTON_BRIGHTNESS_RGB2_FILE.replace("max_",""), sharedPrefs.getString(DeviceSettings.BUTTON_BRIGHTNESS, "0"));
        }
        if (isSupported(KEYBOARD_BRIGHTNESS_RGB1_FILE)) {
            Utils.writeValue(KEYBOARD_BRIGHTNESS_RGB1_FILE, sharedPrefs.getString(DeviceSettings.KEYBOARD_BRIGHTNESS, "0"));
            Utils.writeValue(KEYBOARD_BRIGHTNESS_RGB1_FILE.replace("max_",""), sharedPrefs.getString(DeviceSettings.KEYBOARD_BRIGHTNESS, "0"));
        }
        if (isSupported(KEYBOARD_BRIGHTNESS_RGB2_FILE)) {
            Utils.writeValue(KEYBOARD_BRIGHTNESS_RGB2_FILE, sharedPrefs.getString(DeviceSettings.KEYBOARD_BRIGHTNESS, "0"));
            Utils.writeValue(KEYBOARD_BRIGHTNESS_RGB2_FILE.replace("max_",""), sharedPrefs.getString(DeviceSettings.KEYBOARD_BRIGHTNESS, "0"));
        }
        if (isSupported(KEYBOARD_BRIGHTNESS_RGB3_FILE)) {
            Utils.writeValue(KEYBOARD_BRIGHTNESS_RGB3_FILE, sharedPrefs.getString(DeviceSettings.KEYBOARD_BRIGHTNESS, "0"));
            Utils.writeValue(KEYBOARD_BRIGHTNESS_RGB3_FILE.replace("max_",""), sharedPrefs.getString(DeviceSettings.KEYBOARD_BRIGHTNESS, "0"));
        }
        if (isSupported(KEYBOARD_BRIGHTNESS_RGB4_FILE)) {
            Utils.writeValue(KEYBOARD_BRIGHTNESS_RGB4_FILE, sharedPrefs.getString(DeviceSettings.KEYBOARD_BRIGHTNESS, "0"));
            Utils.writeValue(KEYBOARD_BRIGHTNESS_RGB4_FILE.replace("max_",""), sharedPrefs.getString(DeviceSettings.KEYBOARD_BRIGHTNESS, "0"));
        }
    }
}
