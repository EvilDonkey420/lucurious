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

#ifndef DLU_DRM_SCREEN_H
#define DLU_DRM_SCREEN_H

bool dlu_drm_do_modeset(dlu_drm_core *core, uint32_t cur_bi);

bool dlu_drm_do_page_flip(dlu_drm_core *core, uint32_t cur_bi, void *user_data);

int dlu_drm_do_handle_event(int fd, drmEventContext *ev);

void *dlu_drm_gbm_bo_map(dlu_drm_core *core, uint32_t cur_bi, void **data, uint32_t flags);

void dlu_drm_gbm_bo_unmap(struct gbm_bo *bo, void *map_data);

int dlu_drm_gbm_bo_write(struct gbm_bo *bo, const void *buff, size_t count);

bool dlu_drm_do_atomic_req(dlu_drm_core *core, uint32_t cur_bd, drmModeAtomicReq *req);

int dlu_drm_do_atomic_commit(dlu_drm_core *core, drmModeAtomicReq *req, bool allow_modeset);

drmModeAtomicReq *dlu_drm_do_atomic_alloc();
void dlu_drm_do_atomic_free(drmModeAtomicReq *req);

#endif