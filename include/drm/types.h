/**
* Parts of this file contain functionality similar to what is in kms-quads kms-quads.h:
* https://gitlab.freedesktop.org/daniels/kms-quads/-/blob/master/kms-quads.h
* If you want a tutorial got to the link above
*/

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

/* GBM allocates buffers that are used with KMS */
#include <gbm.h>

#include <systemd/sd-bus.h>
#include <systemd/sd-login.h>

struct drm_prop_enum_info {
  const char *name; /**< name as string (static, not freed) */
  bool valid; /**< true if value is supported; ignore if false */
  uint64_t value; /**< raw value */
};

struct drm_prop_info {
  const char *name; /**< name as string (static, not freed) */
  uint32_t prop_id; /**< KMS property object ID */
  unsigned enum_values_cnt; /**< number of enum values */
  struct drm_prop_enum_info *enum_values; /**< array of enum values */
};

/**
* DLU_DRM_PLANE_TYPE_PRIMARY: Store background image or graphics content
* DLU_DRM_PLANE_TYPE_CURSOR: Used to display a cursor plane (mouse)
* DLU_DRM_PLANE_TYPE_OVERLAY: Used to display any surface (window) over a background
*/
typedef enum _dlu_drm_plane_type {
  DLU_DRM_PLANE_TYPE_PRIMARY = 0x0000,
  DLU_DRM_PLANE_TYPE_CURSOR = 0x0001,
  DLU_DRM_PLANE_TYPE_OVERLAY = 0x0002,
  DLU_DRM_PLANE_TYPE__CNT
} dlu_drm_plane_type;

typedef enum _dlu_drm_connector_props {
  DLU_DRM_CONNECTOR_EDID = 0x0000,
  DLU_DRM_CONNECTOR_DPMS = 0x0001,
  DLU_DRM_CONNECTOR_CRTC_ID = 0x0002,
  DLU_DRM_CONNECTOR_NON_DESKTOP = 0x0003,
  DLU_DRM_CONNECTOR__CNT
} dlu_drm_connector_props;

/* Display Power Management Signaling */
typedef enum _dlu_drm_dpms_state {
  DLU_DRM_DPMS_STATE_OFF = 0x0000,
  DLU_DRM_DPMS_STATE_ON = 0x0001,
  DLU_DRM_DPMS_STATE__CNT
} dlu_drm_dpms_state;

typedef enum _dlu_drm_crtc_props {
  DLU_DRM_CRTC_MODE_ID = 0x0000,
  DLU_DRM_CRTC_ACTIVE = 0x0001,
  DLU_DRM_CRTC_OUT_FENCE_PTR = 0x0002,
  DLU_DRM_CRTC__CNT
} dlu_drm_crtc_props;

/* Properties attached to DRM planes */
typedef enum _dlu_drm_plane_props {
  DLU_DRM_PLANE_TYPE = 0x0000,
  DLU_DRM_PLANE_SRC_X = 0x0001,
  DLU_DRM_PLANE_SRC_Y = 0x0002,
  DLU_DRM_PLANE_SRC_W = 0x0003,
  DLU_DRM_PLANE_SRC_H = 0x0004,
  DLU_DRM_PLANE_CRTC_X = 0x0005,
  DLU_DRM_PLANE_CRTC_Y = 0x0006,
  DLU_DRM_PLANE_CRTC_W = 0x0007,
  DLU_DRM_PLANE_CRTC_H = 0x0008,
  DLU_DRM_PLANE_FB_ID = 0x0009,
  DLU_DRM_PLANE_CRTC_ID = 0x000A,
  DLU_DRM_PLANE_IN_FORMATS = 0x000B,
  DLU_DRM_PLANE_IN_FENCE_FD = 0x000C,
  DLU_DRM_PLANE__CNT
} dlu_drm_plan_props;

typedef struct _dlu_drm_core {
  struct _dlu_device {
    /* KMS API Device node */
    uint32_t kmsfd;

    uint32_t vtfd; /* Virtual Terminal File Descriptor */
    uint32_t bkbm; /* Backup Keyboard mode */

    /* A GBM device is used to create gbm_bo (it's a buffer allocator) */
    struct gbm_device *gbm_device;

    /**
    * Output Data struct contains information of a given
    * Plane, CRTC, Encoder, Connector pair
    */
    uint32_t odc; /* Output data count */
    struct _output_data {
      uint32_t modifiers_cnt;
      uint32_t *modifiers;

      /* A friendly name. */
      char name[32];

      uint32_t mode_blob_id;
      drmModeModeInfo mode;
      uint64_t refresh; /* Refresh rate for a pair store in nanoseconds */

      uint32_t pp_id;   /* Primary Plane ID */
      uint32_t crtc_id; /* CTRC ID */
      uint32_t conn_id; /* Connector ID */
      uint32_t enc_id;  /* Keeping encoder ID just because */

      /**
      * Encoders are deprecated and unused KMS objects
      * The plane -> CRTC -> connector chain construct
      */
      drmModePlane *plane;
      drmModeCrtc *crtc;
      drmModeEncoder *enc;
      drmModeConnector *conn;

      struct {
        struct drm_prop_info plane[DLU_DRM_PLANE__CNT];
        struct drm_prop_info crtc[DLU_DRM_CRTC__CNT];
        struct drm_prop_info conn[DLU_DRM_CONNECTOR__CNT];
      } props;
    } *output_data;
  } device;

  struct _dlu_logind {
    /* For open D-Bus connection */
    sd_bus *bus;

    /* Session id and path */
    char *id, *path;

    bool has_drm;
  } session;
} dlu_drm_core;

#endif
