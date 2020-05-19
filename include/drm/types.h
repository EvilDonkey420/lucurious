
/**
* The MIT License (MIT)
*
* Copyright (c) 2019-2020 Vincent Davis Jr.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*/

#ifndef DLU_DRM_TYPES_H
#define DLU_DRM_TYPES_H

/* These headers allow for the use of ioctl calls directly */
#include <drm.h>
#include <drm_fourcc.h>
#include <drm_mode.h>

/**
* These headers are DRM API user space headers. Mainly used
* for device and resource enumeration
*/
#include <xf86drm.h>
#include <xf86drmMode.h>

/* GBM allocates buffers that are used with use KMS */
#include <gbm.h>

typedef enum _dlu_drm_plane_type {
  DLU_DRM_PLANE_TYPE_PRIMARY = 0x0000, /* Store background image or graphics content */
  DLU_DRM_PLANE_TYPE_CURSOR = 0x0001,  /* Used to display a cursor plane (mouse) */
  DLU_DRM_PLANE_TYPE_OVERLAY = 0x0002  /* Used to display any image over a background */
} dlu_drm_plane_type;

typedef struct _dlu_drm_core {
  uint32_t drmfd;
  drmModeRes *dms;
} dlu_drm_core;

#endif
