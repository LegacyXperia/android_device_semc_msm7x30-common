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

#include <stdlib.h>
#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>

#include "vendor_init.h"
#include "property_service.h"
#include "util.h"

/* Serial number */
#define SERIAL_PROP "ro.serialno"

static void import_kernel_nv(char *name,
        __attribute__((unused)) int for_emulator)
{
    char *value = strchr(name, '=');
    int name_len = strlen(name);
    prop_info *pi;

    if (value == 0) return;
    *value++ = 0;
    if (name_len == 0) return;

    if (!strcmp(name, "serialno")) {
        pi = (prop_info*) __system_property_find(SERIAL_PROP);
        if (pi)
            __system_property_update(pi,
                    value, strlen(value));
        else
            __system_property_add(SERIAL_PROP, strlen(SERIAL_PROP),
                    value, strlen(value));
    }
}

void vendor_load_properties()
{
    import_kernel_cmdline(0, import_kernel_nv);
}
