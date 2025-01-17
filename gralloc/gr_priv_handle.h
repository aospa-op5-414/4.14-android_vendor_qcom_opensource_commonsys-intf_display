/*
 * Copyright (c) 2011-2019, The Linux Foundation. All rights reserved.
 * Not a Contribution
 *
 * Copyright (C) 2008 The Android Open Source Project
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

/*
* Changes from Qualcomm Innovation Center are provided under the following license:
*
* Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted (subject to the limitations in the
* disclaimer below) provided that the following conditions are met:
*
*    * Redistributions of source code must retain the above copyright
*      notice, this list of conditions and the following disclaimer.
*
*    * Redistributions in binary form must reproduce the above
*      copyright notice, this list of conditions and the following
*      disclaimer in the documentation and/or other materials provided
*      with the distribution.
*
*    * Neither the name of Qualcomm Innovation Center, Inc. nor the
*      names of its contributors may be used to endorse or promote
*      products derived from this software without specific prior
*      written permission.
*
* NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
* GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
* HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
* GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
* IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
* IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __GR_PRIV_HANDLE_H__
#define __GR_PRIV_HANDLE_H__

#include <log/log.h>
#include <hardware/gralloc.h>
#include <hardware/gralloc1.h>
#ifdef __cplusplus
#include <cinttypes>
#endif

#define GRALLOC1_FUNCTION_PERFORM 0x00001000

#define DBG_HANDLE false

typedef gralloc1_error_t (*GRALLOC1_PFN_PERFORM)(gralloc1_device_t *device, int operation, ...);

#define PRIV_HANDLE_CONST(exp) static_cast<const private_handle_t *>(exp)

#pragma pack(push, 4)
#ifdef __cplusplus
struct private_handle_t : public native_handle_t {
#else
struct private_handle_t {
        native_handle_t nativeHandle;
#endif
  enum {
    PRIV_FLAGS_FRAMEBUFFER = 0x00000001,
    PRIV_FLAGS_USES_ION = 0x00000008,
    PRIV_FLAGS_NEEDS_FLUSH = 0x00000020,
    PRIV_FLAGS_INTERNAL_ONLY = 0x00000040,
    PRIV_FLAGS_NON_CPU_WRITER = 0x00000080,
    PRIV_FLAGS_CACHED = 0x00000200,
    PRIV_FLAGS_SECURE_BUFFER = 0x00000400,
    PRIV_FLAGS_EXTERNAL_ONLY = 0x00002000,
    PRIV_FLAGS_PROTECTED_BUFFER = 0x00004000,
    PRIV_FLAGS_VIDEO_ENCODER = 0x00010000,
    PRIV_FLAGS_CAMERA_WRITE = 0x00020000,
    PRIV_FLAGS_CAMERA_READ = 0x00040000,
    PRIV_FLAGS_HW_COMPOSER = 0x00080000,
    PRIV_FLAGS_HW_TEXTURE = 0x00100000,
    PRIV_FLAGS_ITU_R_601 = 0x00200000,     // Unused from display
    PRIV_FLAGS_ITU_R_601_FR = 0x00400000,  // Unused from display
    PRIV_FLAGS_ITU_R_709 = 0x00800000,     // Unused from display
    PRIV_FLAGS_SECURE_DISPLAY = 0x01000000,
    PRIV_FLAGS_TILE_RENDERED = 0x02000000,
    PRIV_FLAGS_CPU_RENDERED = 0x04000000,
    PRIV_FLAGS_UBWC_ALIGNED = 0x08000000,
    PRIV_FLAGS_DISP_CONSUMER = 0x10000000,
    PRIV_FLAGS_CLIENT_ALLOCATED = 0x20000000,  // Ion buffer allocated outside of gralloc
    PRIV_FLAGS_UBWC_ALIGNED_PI = 0x40000000,  // PI format
  };

  // file-descriptors dup'd over IPC
  int fd;
  int fd_metadata;

  // values sent over IPC
  int magic;
  int flags;
  int width;             // holds width of the actual buffer allocated
  int height;            // holds height of the  actual buffer allocated
  int unaligned_width;   // holds width client asked to allocate
  int unaligned_height;  // holds height client asked to allocate
  int format;
  int buffer_type;

  // Target-specific members
#include <private_handle_members.h>

#ifdef __cplusplus
  // Legacy constructor used by some clients
  private_handle_t(int fd, unsigned int size, int usage, int buf_type, int format, int w, int h)
      : private_handle_t(fd, -1, PRIV_FLAGS_CLIENT_ALLOCATED, w, h, 0, 0, format, buf_type, size,
                         static_cast<uint64_t>(usage)) {}

  ~private_handle_t() {
    magic = 0;
    ALOGE_IF(DBG_HANDLE, "Deleting buffer handle %p", this);
  }

  static int validate(const native_handle *h) {
    auto *hnd = static_cast<const private_handle_t *>(h);
    if (!h || h->version != sizeof(native_handle) || h->numInts != NumInts() ||
        h->numFds != kNumFds) {
      ALOGE("Invalid gralloc handle (at %p): ver(%d/%zu) ints(%d/%d) fds(%d/%d)",
          h, h ? h->version : -1, sizeof(native_handle), h ? h->numInts : -1, NumInts(),
          h ? h->numFds : -1, kNumFds);
      return -EINVAL;
    }
    if (hnd->magic != kMagic) {
      ALOGE("handle = %p  invalid  magic(%c%c%c%c/%c%c%c%c)",hnd,
          hnd ? (((hnd->magic >> 24) & 0xFF) ? ((hnd->magic >> 24) & 0xFF) : '-') : '?',
          hnd ? (((hnd->magic >> 16) & 0xFF) ? ((hnd->magic >> 16) & 0xFF) : '-') : '?',
          hnd ? (((hnd->magic >> 8) & 0xFF) ? ((hnd->magic >> 8) & 0xFF) : '-') : '?',
          hnd ? (((hnd->magic >> 0) & 0xFF) ? ((hnd->magic >> 0) & 0xFF) : '-') : '?',
          (kMagic >> 24) & 0xFF, (kMagic >> 16) & 0xFF, (kMagic >> 8) & 0xFF, (kMagic >> 0) & 0xFF);
      return -EINVAL;
    }

    return 0;
  }

  static void Dump(const private_handle_t *hnd) {
    ALOGD("handle id:%" PRIu64
          " wxh:%dx%d uwxuh:%dx%d size: %d fd:%d fd_meta:%d flags:0x%x "
          "usage:0x%" PRIx64 "  format:0x%x layer_count: %d",
          hnd->id, hnd->width, hnd->height, hnd->unaligned_width, hnd->unaligned_height, hnd->size,
          hnd->fd, hnd->fd_metadata, hnd->flags, hnd->usage, hnd->format, hnd->layer_count);
  }

  int GetUnalignedWidth() const { return unaligned_width; }

  int GetUnalignedHeight() const { return unaligned_height; }

  int GetColorFormat() const { return format; }

  unsigned int GetLayerCount() const { return layer_count; }

  int GetStride() const {
    // In handle we currently store aligned width after allocation.
    return width;
  }

  uint64_t GetUsage() const { return usage; }

  uint64_t GetBackingstore() const { return id; }
#endif
};
#pragma pack(pop)

#endif  // __GR_PRIV_HANDLE_H__
