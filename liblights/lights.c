/*
 * Copyright (C) 2008 The Android Open Source Project
 * Copyright (C) 2011 Diogo Ferreira <defer@cyanogenmod.com>
 * Copyright (C) 2012-2013 The CyanogenMod Project <http://www.cyanogenmod.org>
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

#define LOG_TAG "lights.semc"

#include <cutils/log.h>

#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <math.h>

#include <sys/ioctl.h>
#include <sys/types.h>

#include <hardware/lights.h>

char const*const LCD_BACKLIGHT_FILE = "/sys/class/leds/lcd-backlight/brightness";
char const*const RED_LED_FILE       = "/sys/class/leds/red/brightness";
char const*const GREEN_LED_FILE     = "/sys/class/leds/green/brightness";
char const*const BLUE_LED_FILE      = "/sys/class/leds/blue/brightness";

char const*const BUTTON_BACKLIGHT_FILE[] = {
  "/sys/class/leds/button-backlight/brightness",
  "/sys/class/leds/button-backlight-rgb1/brightness",
  "/sys/class/leds/button-backlight-rgb2/brightness"
};

char const*const KEYBOARD_BACKLIGHT_FILE[] = {
  "/sys/class/leds/keyboard-backlight-rgb1/brightness",
  "/sys/class/leds/keyboard-backlight-rgb2/brightness",
  "/sys/class/leds/keyboard-backlight-rgb3/brightness",
  "/sys/class/leds/keyboard-backlight-rgb4/brightness"
};

char const*const ALS_FILE = "/sys/devices/i2c-0/0-0040/als_on";

char const*const LED_FILE_TRIGGER[]  = {
  "/sys/class/leds/red/use_pattern",
  "/sys/class/leds/green/use_pattern",
  "/sys/class/leds/blue/use_pattern"
};

char const*const LED_FILE_PATTERN     = "/sys/devices/i2c-0/0-0040/pattern_data";
char const*const LED_FILE_REPEATDELAY = "/sys/devices/i2c-0/0-0040/pattern_delay";
char const*const LED_FILE_PATTERNLEN  = "/sys/devices/i2c-0/0-0040/pattern_duration_secs";
char const*const LED_FILE_DIMONOFF    = "/sys/devices/i2c-0/0-0040/pattern_use_softdim";
char const*const LED_FILE_DIMTIME     = "/sys/devices/i2c-0/0-0040/dim_time";

const int LCD_BRIGHTNESS_MIN = 1;

char const*const ON  = "1";
char const*const OFF = "0";

/* Synchronization primities */
static pthread_once_t g_init = PTHREAD_ONCE_INIT;
static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;

/* Mini-led state machine */
static struct light_state_t g_notification;
static struct light_state_t g_battery;

/* The leds we have */
enum {
	LED_RED,
	LED_GREEN,
	LED_BLUE,
	LED_BLANK
};

static int write_int (const char *path, int value) {
	int fd;
	static int already_warned = 0;

	fd = open(path, O_RDWR);
	if (fd < 0) {
		if (already_warned == 0) {
			ALOGE("write_int failed to open %s\n", path);
			already_warned = 1;
		}
		return -errno;
	}

	char buffer[20];
	int bytes = snprintf(buffer, sizeof(buffer), "%d\n", value);
	int written = write (fd, buffer, bytes);
	close(fd);

	return written == -1 ? -errno : 0;
}

static int write_string (const char *path, const char *value) {
	int fd;
	static int already_warned = 0;

	fd = open(path, O_RDWR);
	if (fd < 0) {
		if (already_warned == 0) {
			ALOGE("write_string failed to open %s\n", path);
			already_warned = 1;
		}
		return -errno;
	}

	char buffer[20];
	int bytes = snprintf(buffer, sizeof(buffer), "%s\n", value);
	int written = write (fd, buffer, bytes);
	close(fd);

	return written == -1 ? -errno : 0;
}

/* Color tools */
static int is_lit (struct light_state_t const* state) {
	return state->color & 0x00ffffff;
}

static int rgb_to_brightness (struct light_state_t const* state) {
	int color = state->color & 0x00ffffff;
	return ((77*((color>>16)&0x00ff))
			+ (150*((color>>8)&0x00ff)) + (29*(color&0x00ff))) >> 8;
}

static int brightness_apply_gamma (int brightness) {
	double floatbrt = (double) brightness;
	floatbrt /= 255.0;
	ALOGV("%s: brightness = %d, floatbrt = %f", __func__, brightness, floatbrt);
	floatbrt = pow(floatbrt,2.2);
	ALOGV("%s: gamma corrected floatbrt = %f", __func__, floatbrt);
	floatbrt *= 255.0;
	brightness = (int) floatbrt;
	if (brightness < LCD_BRIGHTNESS_MIN)
		brightness = LCD_BRIGHTNESS_MIN;
	ALOGV("%s: gamma corrected brightness = %d", __func__, brightness);
	return brightness;
}

/* The actual lights controlling section */
static int set_light_backlight (struct light_device_t *dev, struct light_state_t const *state) {
	int err = 0;
	int enable = 0;
	int brightness = rgb_to_brightness(state);

	if (brightness > 0)
		brightness = brightness_apply_gamma(brightness);

	if ((state->brightnessMode == BRIGHTNESS_MODE_SENSOR) && (brightness > 0))
		enable = 1;

	ALOGV("%s brightness = %d", __func__, brightness);
	pthread_mutex_lock(&g_lock);
	err = write_int (ALS_FILE, enable);
	err |= write_int (LCD_BACKLIGHT_FILE, brightness);
	pthread_mutex_unlock(&g_lock);

	return err;
}

static int set_light_buttons (struct light_device_t *dev, struct light_state_t const* state) {
	size_t i = 0;
	int on = is_lit(state);

	pthread_mutex_lock(&g_lock);
	for (i = 0; i < sizeof(BUTTON_BACKLIGHT_FILE)/sizeof(BUTTON_BACKLIGHT_FILE[0]); i++) {
		write_int (BUTTON_BACKLIGHT_FILE[i], on ? 255 : 0);
	}
	pthread_mutex_unlock(&g_lock);

	return 0;
}

static int set_light_keyboard (struct light_device_t* dev, struct light_state_t const* state) {
	size_t i = 0;
	int on = is_lit(state);

	pthread_mutex_lock(&g_lock);
	for (i = 0; i < sizeof(KEYBOARD_BACKLIGHT_FILE)/sizeof(KEYBOARD_BACKLIGHT_FILE[0]); i++) {
		write_int (KEYBOARD_BACKLIGHT_FILE[i], on ? 255 : 0);
	}
	pthread_mutex_unlock(&g_lock);

	return 0;
}

static void set_shared_light_locked (struct light_device_t *dev, struct light_state_t *state) {
	int r, g, b;
	size_t i = 0;

	uint32_t pattern = 0;
	uint32_t patbits = 0;
	uint32_t numbits, delayshift;

	char patternstr[11];

	ALOGV("color 0x%x", state->color);

	r = (state->color >> 16) & 0xFF;
	g = (state->color >> 8) & 0xFF;
	b = (state->color) & 0xFF;

	ALOGV("flashOn = %d, flashOff = %d", state->flashOnMS, state->flashOffMS);

	if (state->flashOnMS == 1)
		state->flashMode = LIGHT_FLASH_NONE;
	else {
		numbits = state->flashOnMS / 250;
		delayshift = state->flashOffMS / 250;

		// Make sure we never do 0 on time
		if (numbits == 0)
			numbits = 1;

		// Always make sure period is >2x the on time, we don't support
		// more than 50% duty cycle
		if (delayshift < numbits * 2)
			delayshift = numbits * 2;

		ALOGV("numbits = %d, delayshift = %d", numbits, delayshift);

		patbits = ((uint32_t)1 << numbits) - 1;
		ALOGV("patbits = 0x%x", patbits);

		for (i = 0; i < 32; i += delayshift) {
			pattern = pattern | (patbits << i);
		}

		ALOGV("pattern = 0x%x", pattern);

		snprintf(patternstr, 11, "0x%x", pattern);

		ALOGV("patternstr = %s", patternstr);
	}

	switch (state->flashMode) {
	case LIGHT_FLASH_TIMED:
	case LIGHT_FLASH_HARDWARE:
		for (i = 0; i < sizeof(LED_FILE_TRIGGER)/sizeof(LED_FILE_TRIGGER[0]); i++) {
			write_string (LED_FILE_TRIGGER[i], ON);
		}
		write_string (LED_FILE_DIMONOFF, ON);
		write_int (LED_FILE_DIMTIME, numbits * 125);
		write_string (LED_FILE_PATTERN, patternstr);
		write_int (LED_FILE_PATTERNLEN, 8);
		write_int (LED_FILE_REPEATDELAY, 0);
		break;

	case LIGHT_FLASH_NONE:
		for (i = 0; i < sizeof(LED_FILE_TRIGGER)/sizeof(LED_FILE_TRIGGER[0]); i++) {
			write_string (LED_FILE_TRIGGER[i], OFF);
		}
		write_string (LED_FILE_DIMONOFF, OFF);
		break;
	}

	write_int (RED_LED_FILE, r);
	write_int (GREEN_LED_FILE, g);
	write_int (BLUE_LED_FILE, b);
}

static void handle_shared_battery_locked (struct light_device_t *dev) {
	if (is_lit (&g_notification))
		set_shared_light_locked (dev, &g_notification);
	else
		set_shared_light_locked (dev, &g_battery);
}

static int set_light_battery (struct light_device_t *dev, struct light_state_t const* state) {
	pthread_mutex_lock (&g_lock);
	g_battery = *state;
	handle_shared_battery_locked(dev);
	pthread_mutex_unlock (&g_lock);
	return 0;
}

static int set_light_notifications (struct light_device_t *dev, struct light_state_t const* state) {
	pthread_mutex_lock (&g_lock);
	g_notification = *state;
	handle_shared_battery_locked(dev);
	pthread_mutex_unlock (&g_lock);
	return 0;
}

/* Initializations */
void init_globals () {
	pthread_mutex_init (&g_lock, NULL);
}

/* Glueing boilerplate */
static int close_lights (struct light_device_t *dev) {
	if (dev)
		free(dev);

	return 0;
}

static int open_lights (const struct hw_module_t* module, char const* name,
						struct hw_device_t** device) {
	int (*set_light)(struct light_device_t* dev,
					 struct light_state_t const *state);

	if (0 == strcmp(LIGHT_ID_BACKLIGHT, name))
		set_light = set_light_backlight;
	else if (0 == strcmp(LIGHT_ID_KEYBOARD, name))
		set_light = set_light_keyboard;
	else if (0 == strcmp(LIGHT_ID_BUTTONS, name))
		set_light = set_light_buttons;
	else if (0 == strcmp(LIGHT_ID_BATTERY, name))
		set_light = set_light_battery;
	else if (0 == strcmp(LIGHT_ID_NOTIFICATIONS, name))
		set_light = set_light_notifications;
	else
		return -EINVAL;

	pthread_once (&g_init, init_globals);
	struct light_device_t *dev = malloc(sizeof (struct light_device_t));
	memset(dev, 0, sizeof(*dev));

	dev->common.tag		= HARDWARE_DEVICE_TAG;
	dev->common.version	= 0;
	dev->common.module 	= (struct hw_module_t*)module;
	dev->common.close 	= (int (*)(struct hw_device_t*))close_lights;
	dev->set_light 		= set_light;

	*device = (struct hw_device_t*)dev;
	return 0;
}

static struct hw_module_methods_t lights_module_methods = {
	.open = open_lights,
};

struct hw_module_t HAL_MODULE_INFO_SYM = {
	.tag		= HARDWARE_MODULE_TAG,
	.version_major	= 1,
	.version_minor	= 0,
	.id		= LIGHTS_HARDWARE_MODULE_ID,
	.name		= "SEMC lights module",
	.author		= "Diogo Ferreira <defer@cyanogenmod.com>, Andreas Makris <Andreas.Makris@gmail.com>",
	.methods	= &lights_module_methods,
};
