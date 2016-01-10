/*
 * Copyright (C) 2008 The Android Open Source Project
 * Copyright (C) 2011 Diogo Ferreira <defer@cyanogenmod.com>
 * Copyright (C) 2012-2014 The CyanogenMod Project <http://www.cyanogenmod.org>
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

char const*const LCD_BACKLIGHT_FILE      = "/sys/class/leds/lcd-backlight/brightness";

#ifdef HAVE_BUTTON_BACKLIGHT
char const*const BUTTON_BACKLIGHT_FILE   = "/sys/class/leds/button-backlight/brightness";
#endif
#ifdef HAVE_KEYBOARD_BACKLIGHT
char const*const KEYBOARD_BACKLIGHT_FILE = "/sys/class/leds/keyboard-backlight/brightness";
#endif

char const*const RED_LED_FILE            = "/sys/class/leds/red/brightness";
char const*const GREEN_LED_FILE          = "/sys/class/leds/green/brightness";
char const*const BLUE_LED_FILE           = "/sys/class/leds/blue/brightness";

char const*const LED_FILE_TRIGGER[] = {
  "/sys/class/leds/red/trigger",
  "/sys/class/leds/green/trigger",
  "/sys/class/leds/blue/trigger"
};

char const*const LED_FILE_DELAYON[] = {
  "/sys/class/leds/red/delay_on",
  "/sys/class/leds/green/delay_on",
  "/sys/class/leds/blue/delay_on"
};

char const*const LED_FILE_DELAYOFF[] = {
  "/sys/class/leds/red/delay_off",
  "/sys/class/leds/green/delay_off",
  "/sys/class/leds/blue/delay_off"
};

const int LCD_BRIGHTNESS_MIN = 5;

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
	int brightness = rgb_to_brightness(state);

	if (brightness > 0)
		brightness = brightness_apply_gamma(brightness);

	ALOGV("%s brightness = %d", __func__, brightness);
	pthread_mutex_lock(&g_lock);
	err = write_int (LCD_BACKLIGHT_FILE, brightness);
	pthread_mutex_unlock(&g_lock);

	return err;
}

static int set_light_buttons (struct light_device_t *dev, struct light_state_t const* state) {
	int err = 0;
#ifdef HAVE_BUTTON_BACKLIGHT
	size_t i = 0;
	int brightness = rgb_to_brightness(state);

	pthread_mutex_lock(&g_lock);
	ALOGV("%s brightness = %d", __func__, brightness);
	err |= write_int (BUTTON_BACKLIGHT_FILE, brightness);
	pthread_mutex_unlock(&g_lock);
#endif
	return err;
}

static int set_light_keyboard (struct light_device_t* dev, struct light_state_t const* state) {
	int err = 0;
#ifdef HAVE_KEYBOARD_BACKLIGHT
	size_t i = 0;
	int brightness = rgb_to_brightness(state);

	pthread_mutex_lock(&g_lock);
	ALOGV("%s brightness = %d", __func__, brightness);
	err |= write_int (KEYBOARD_BACKLIGHT_FILE, brightness);
	pthread_mutex_unlock(&g_lock);
#endif
	return err;
}

static void set_shared_light_locked (struct light_device_t *dev, struct light_state_t *state) {
	int delayOn, delayOff;
	size_t i = 0;
	unsigned int colorRGB;

	// state->color is an ARGB value, clear the alpha channel
	colorRGB = (0xFFFFFF & state->color);

	ALOGV("colorRGB 0x%x", state->color);

	delayOn = state->flashOnMS;
	delayOff = state->flashOffMS;

	ALOGV("flashOn = %d, flashOff = %d", delayOn, delayOff);

	if (delayOn == 1)
		state->flashMode = LIGHT_FLASH_NONE;

	switch (state->flashMode) {
	case LIGHT_FLASH_TIMED:
	case LIGHT_FLASH_HARDWARE:
		for (i = 0; i < sizeof(LED_FILE_TRIGGER)/sizeof(LED_FILE_TRIGGER[0]); i++) {
			write_string (LED_FILE_TRIGGER[i], "timer");
			write_int (LED_FILE_DELAYON[i], delayOn);
			write_int (LED_FILE_DELAYOFF[i], delayOff);
		}
		break;

	case LIGHT_FLASH_NONE:
		for (i = 0; i < sizeof(LED_FILE_TRIGGER)/sizeof(LED_FILE_TRIGGER[0]); i++) {
			write_string (LED_FILE_TRIGGER[i], "none");
		}
		break;
	}

	write_int (RED_LED_FILE, (colorRGB >> 16) & 0xFF);
	write_int (GREEN_LED_FILE, (colorRGB >> 8) & 0xFF);
	write_int (BLUE_LED_FILE, colorRGB & 0xFF);
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
	.tag = HARDWARE_MODULE_TAG,
	.module_api_version = 1,
	.hal_api_version = HARDWARE_HAL_API_VERSION,
	.id = LIGHTS_HARDWARE_MODULE_ID,
	.name = "SEMC lights module",
	.author = "Diogo Ferreira <defer@cyanogenmod.com>, Andreas Makris <Andreas.Makris@gmail.com>",
	.methods = &lights_module_methods,
};
