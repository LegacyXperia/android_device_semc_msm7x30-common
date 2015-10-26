/*
 * Copyright (C) 2015 The CyanogenMod Project
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

#include <stdint.h>
#include <ui/Rect.h>
#include <hardware/gralloc.h>

/* status_t GraphicBufferMapper::lock */
extern "C" android::status_t _ZN7android19GraphicBufferMapper4lockEPK13native_handlejRKNS_4RectEPPv(
        buffer_handle_t handle, uint32_t usage, const android::Rect& bounds, void** vaddr);
extern "C" android::status_t _ZN7android19GraphicBufferMapper4lockEPK13native_handleiRKNS_4RectEPPv(
        buffer_handle_t handle, int usage, const android::Rect& bounds, void** vaddr) {
    return _ZN7android19GraphicBufferMapper4lockEPK13native_handlejRKNS_4RectEPPv(
            handle, (uint32_t)usage, bounds, vaddr);
}
