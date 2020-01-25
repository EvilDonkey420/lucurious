/**
* The MIT License (MIT)
*
* Copyright (c) 2019 Vincent Davis Jr.
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

#define LUCUR_VKCOMP_API
#include <lucom.h>

void wlu_exec_begin_render_pass(
  vkcomp *app,
  uint32_t cur_pool,
  uint32_t cur_scd,
  uint32_t cur_gpd,
  uint32_t x,
  uint32_t y,
  uint32_t width,
  uint32_t height,
  uint32_t clearValueCount,
  const VkClearValue *pClearValues,
  VkSubpassContents contents
) {

  if (!app->sc_data[cur_scd].sc_buffs) { PERR(WLU_BUFF_NOT_ALLOC, 0, "WLU_SC_DATA_MEMS"); return; }

  VkRenderPassBeginInfo render_pass_info = {};
  render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  render_pass_info.pNext = NULL;
  render_pass_info.renderPass = app->gp_data[cur_gpd].render_pass;
  render_pass_info.renderArea.offset.x = x;
  render_pass_info.renderArea.offset.y = y;
  render_pass_info.renderArea.extent.width = width;
  render_pass_info.renderArea.extent.height = height;
  render_pass_info.clearValueCount = clearValueCount;
  render_pass_info.pClearValues = pClearValues;

  for (uint32_t i = 0; i < app->sc_data[cur_scd].sic; i++) {
    render_pass_info.framebuffer = app->sc_data[cur_scd].sc_buffs[i].fb;
    /* Instert render pass into command buffer */
    vkCmdBeginRenderPass(app->cmd_data[cur_pool].cmd_buffs[i], &render_pass_info, contents);
  }
}

void wlu_exec_stop_render_pass(vkcomp *app, uint32_t cur_pool, uint32_t cur_scd) {
  for (uint32_t i = 0; i < app->sc_data[cur_scd].sic; i++)
    vkCmdEndRenderPass(app->cmd_data[cur_pool].cmd_buffs[i]);
}