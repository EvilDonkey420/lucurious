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

#ifndef DLU_VKCOMP_BIND_H
#define DLU_VKCOMP_BIND_H

void dlu_bind_pipeline(
  vkcomp *app,
  uint32_t cur_pool,
  uint32_t cur_buff,
  uint32_t cur_gpd,
  uint32_t cur_pl,
  VkPipelineBindPoint pipelineBindPoint
);

void dlu_bind_desc_sets(
  vkcomp *app,
  uint32_t cur_pool,
  uint32_t cur_buff,
  uint32_t cur_gpd,
  uint32_t cur_dd,
  VkPipelineBindPoint pipelineBindPoint,
  uint32_t dynamicOffsetCount,
  const uint32_t *pDynamicOffsets
);

void dlu_bind_vertex_buffs_to_cmd_buff(
  vkcomp *app,
  uint32_t cur_pool,
  uint32_t cur_buff,
  uint32_t cur_bd,
  uint32_t firstBinding,
  const VkDeviceSize *offsets
);

void dlu_bind_index_buff_to_cmd_buff(
  vkcomp *app,
  uint32_t cur_pool,
  uint32_t cur_buff,
  uint32_t cur_bd,
  VkDeviceSize offset,
  VkIndexType indexType
);

#endif
