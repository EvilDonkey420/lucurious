/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Lucurious Labs
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

#include <wlu/vkall.h>
#include <wlu/client.h>
#include <wlu/errors.h>
#include <wlu/log.h>
#include <wlu/shade.h>
#include <wlu/gp.h>
#include <wlu/matrix.h>

#include <signal.h>
#include <unistd.h>
#include <stdbool.h>

#include "simple_example.h"

#define NUM_DESCRIPTOR_SETS 1
#define WIDTH 1920
#define HEIGHT 1080
#define DEPTH 1

void freeme(vkcomp *app, wclient *wc, wlu_shader_info *shinfo, wlu_shader_info *shinfo_two) {
  wlu_freeup_shi(shinfo);
  wlu_freeup_shi(shinfo_two);
  wlu_freeup_vk(app);
  wlu_freeup_wc(wc);
}

int main(void) {
  VkResult err;

  wclient *wc = wlu_init_wc();
  if (!wc) {
    wlu_log_me(WLU_DANGER, "[x] wlu_init_wc failed!!");
    return EXIT_FAILURE;
  }

  vkcomp *app = wlu_init_vk();
  if (!app) {
    freeme(NULL, wc, NULL, NULL);
    wlu_log_me(WLU_DANGER, "[x] wlu_init_vk failed!!");
    return EXIT_FAILURE;
  }

  /* Signal handler for this process */
  err = wlu_watch_me(SIGSEGV, getpid());
  if (err) {
    freeme(app, wc, NULL, NULL);
    return EXIT_FAILURE;
  }

  wlu_add_watchme_info(1, app, 1, wc, 0, NULL, 0, NULL);

  err = wlu_set_global_layers(app);
  if (err) {
    freeme(app, wc, NULL, NULL);
    wlu_log_me(WLU_DANGER, "[x] checking and setting validation layers failed");
    return EXIT_FAILURE;
  }

  err = wlu_create_instance(app, "Draw Cube", "Desktop Engine", 3, enabled_validation_layers, 4, instance_extensions);
  if (err) {
    freeme(app, wc, NULL, NULL);
    wlu_log_me(WLU_DANGER, "[x] failed to create vulkan instance");
    return EXIT_FAILURE;
  }

  err = wlu_set_debug_message(app, 1);
  if (err) {
    freeme(app, wc, NULL, NULL);
    wlu_log_me(WLU_DANGER, "[x] failed to setup debug message");
    return EXIT_FAILURE;
  }

  err = wlu_enumerate_devices(app, VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU);
  if (err) {
    freeme(app, wc, NULL, NULL);
    wlu_log_me(WLU_DANGER, "[x] failed to find physical device");
    return EXIT_FAILURE;
  }

  if (wlu_connect_client(wc)) {
    freeme(app, wc, NULL, NULL);
    wlu_log_me(WLU_DANGER, "[x] failed to connect client");
    return EXIT_FAILURE;
  }

  /* initialize vulkan app surface */
  err = wlu_vkconnect_surfaceKHR(app, wc->display, wc->surface);
  if (err) {
    freeme(app, wc, NULL, NULL);
    wlu_log_me(WLU_DANGER, "[x] failed to connect to vulkan surfaceKHR");
    return EXIT_FAILURE;
  }

  err = wlu_set_queue_family(app, VK_QUEUE_GRAPHICS_BIT);
  if (err) {
    freeme(app, wc, NULL, NULL);
    wlu_log_me(WLU_DANGER, "[x] failed to set device queue family");
    return EXIT_FAILURE;
  }

  err = wlu_create_logical_device(app, 3, enabled_validation_layers, 1, device_extensions);
  if (err) {
    freeme(app, wc, NULL, NULL);
    wlu_log_me(WLU_DANGER, "[x] failed to initialize logical device to physical device");
    return EXIT_FAILURE;
  }

  wlu_retrieve_device_queue(app);

  VkSurfaceCapabilitiesKHR capabilities = wlu_q_device_capabilities(app);
  if (capabilities.minImageCount == UINT32_MAX) {
    freeme(app, wc, NULL, NULL);
    return EXIT_FAILURE;
  }
  /*
  * VK_FORMAT_B8G8R8A8_UNORM will store the B, G, R and alpha channels
  * in that order with an 8 bit unsigned integer and a total of 32 bits per pixel.
  * SRGB if used for colorSpace if available, because it
  * results in more accurate perceived colors
  */

  VkSurfaceFormatKHR surface_fmt = wlu_choose_swap_surface_format(app, VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);
  if (surface_fmt.format == VK_FORMAT_UNDEFINED) {
    freeme(app, wc, NULL, NULL);
    return EXIT_FAILURE;
  }

  VkPresentModeKHR pres_mode = wlu_choose_swap_present_mode(app);
  if (pres_mode == VK_PRESENT_MODE_MAX_ENUM_KHR) {
    freeme(app, wc, NULL, NULL);
    return EXIT_FAILURE;
  }

  VkExtent3D extent3D = wlu_choose_3D_swap_extent(capabilities, WIDTH, HEIGHT, DEPTH);
  if (extent3D.width == UINT32_MAX) {
    freeme(app, wc, NULL, NULL);
    wlu_log_me(WLU_DANGER, "[x] choose_swap_extent failed, extent3D.width equals %d", extent3D.width);
    return EXIT_FAILURE;
  }

  err = wlu_create_swap_chain(app, capabilities, surface_fmt, pres_mode, extent3D.width, extent3D.height);
  if (err) {
    freeme(app, wc, NULL, NULL);
    wlu_log_me(WLU_DANGER, "[x] failed to create swap chain");
    return EXIT_FAILURE;
  }

  err = wlu_create_cmd_pool(app, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
  if (err) {
    freeme(app, wc, NULL, NULL);
    wlu_log_me(WLU_DANGER, "[x] failed to create command pool, ERROR CODE: %d", err);
    return EXIT_FAILURE;
  }

  err = wlu_create_cmd_buffs(app, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
  if (err) {
    freeme(app, wc, NULL, NULL);
    wlu_log_me(WLU_DANGER, "[x] failed to create command buffers, ERROR CODE: %d", err);
    return EXIT_FAILURE;
  }

  err = wlu_exec_begin_cmd_buffs(app, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT, NULL);
  if (err) {
    freeme(app, wc, NULL, NULL);
    wlu_log_me(WLU_DANGER, "[x] failed to start command buffer recording");
    return EXIT_FAILURE;
  }

  err = wlu_create_img_views(app, surface_fmt.format, VK_IMAGE_VIEW_TYPE_2D);
  if (err) {
    freeme(app, wc, NULL, NULL);
    wlu_log_me(WLU_DANGER, "[x] failed to create image views");
    return EXIT_FAILURE;
  }

  err = wlu_create_depth_buff(app, VK_FORMAT_D16_UNORM,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_IMAGE_TYPE_2D, extent3D,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_SHARING_MODE_EXCLUSIVE, VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_VIEW_TYPE_2D);
  if (err) {
    freeme(app, wc, NULL, NULL);
    wlu_log_me(WLU_DANGER, "[x] wlu_create_depth_buff failed");
    return EXIT_FAILURE;
  }

  float fovy = wlu_set_fovy(45.0f);
  float hw = extent3D.height / (float) extent3D.width;
  if (extent3D.width > extent3D.height) fovy *= hw;

  wlu_set_perspective(app, fovy, hw, 0.1f, 100.0f);

  float dir[3] = {-5, 3, -10};
  float eye[3] = {0, 0, 0};
  float up[3] = {0, -1, 0};
  wlu_set_lookat(app, dir, eye, up);

  wlu_set_model_matrix(app, 1.0f);

  float clip_matrix[4][4] = {
    { 1.0f, 0.0f, 0.0f, 0.0f },
    { 0.0f,-1.0f, 0.0f, 0.0f },
    { 0.0f, 0.0f, 0.5f, 0.0f },
    { 0.0f, 0.0f, 0.5f, 1.0f }
  };
  wlu_set_clip_matrix(app, clip_matrix);
  wlu_set_mvp_matrix(app);
  wlu_print_matrices(app);

  err = wlu_create_uorv_buff(app, sizeof(app->mvp), &app->mvp, 0, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
  if (err) {
    freeme(app, wc, NULL, NULL);
    wlu_log_me(WLU_DANGER, "[x] wlu_create_uniform_buff failed");
    return EXIT_FAILURE;
  }

  VkDescriptorSetLayoutBinding desc_set = wlu_set_desc_set(0,
    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, NULL
  );

  VkDescriptorSetLayoutCreateInfo desc_set_info = wlu_set_desc_set_info(app, 0, NUM_DESCRIPTOR_SETS, &desc_set);

  err = wlu_create_desc_set_layout(app, &desc_set_info);
  if (err) {
    freeme(app, wc, NULL, NULL);
    wlu_log_me(WLU_DANGER, "[x] wlu_set_desc_set_info failed");
    return EXIT_FAILURE;
  }

  /* This is where creation of the graphics pipeline begins */

  err = wlu_create_pipeline_layout(app, 0, NULL);
  if (err) {
    freeme(app, wc, NULL, NULL);
    wlu_log_me(WLU_DANGER, "[x] wlu_create_pipeline_layout failed");
    return EXIT_FAILURE;
  }

  err = wlu_create_desc_set(app, 1, 0, 0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
  if (err) {
    freeme(app, wc, NULL, NULL);
    wlu_log_me(WLU_DANGER, "[x] wlu_create_desc_set failed");
    return EXIT_FAILURE;
  }

  /* start of render pass creation */
  wlu_log_me(WLU_INFO, "Start of render pass creation");

  VkAttachmentDescription attachments[2];
  /* Create render pass color attachment for swapchain images */
  attachments[0] = wlu_set_attachment_desc(surface_fmt.format,
    VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
    VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
  );

  /* Create render pass stencil/depth attachment for depth buffer */
  attachments[1] = wlu_set_attachment_desc(app->depth.format,
    VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE,
    VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
  );

  VkAttachmentReference color_ref = wlu_set_attachment_ref(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
  VkAttachmentReference depth_ref = wlu_set_attachment_ref(1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
  VkSubpassDescription subpass = wlu_set_subpass_desc(0, NULL, 1, &color_ref, NULL, &depth_ref, 0, NULL);

  err = wlu_create_render_pass(app, 2, attachments, 1, &subpass, 0, NULL);
  if (err) {
    freeme(app, wc, NULL, NULL);
    wlu_log_me(WLU_DANGER, "[x] wlu_create_render_pass failed");
    return EXIT_FAILURE;
  }

  wlu_log_me(WLU_SUCCESS, "Successfully created the render pass!!!");
  /* End of render pass creation */

  wlu_log_me(WLU_INFO, "Start of shader creation");

  wlu_log_me(WLU_WARNING, "Compiling the frag code to spirv shader");
  wlu_shader_info shi_frag = wlu_compile_to_spirv(VK_SHADER_STAGE_FRAGMENT_BIT,
                             shader_frag_src, "frag.spv", "main");
  if (!shi_frag.bytes) {
    freeme(app, wc, NULL, NULL);
    wlu_log_me(WLU_DANGER, "[x] wlu_compile_to_spirv failed");
    return EXIT_FAILURE;
  }

  wlu_log_me(WLU_WARNING, "Compiling the vert code to spirv shader");
  wlu_shader_info shi_vert = wlu_compile_to_spirv(VK_SHADER_STAGE_VERTEX_BIT,
                             shader_vert_src, "vert.spv", "main");
  if (!shi_vert.bytes) {
    freeme(app, wc, &shi_frag, NULL);
    wlu_log_me(WLU_DANGER, "[x] wlu_compile_to_spirv failed");
    return EXIT_FAILURE;
  }

  wlu_add_watchme_info(0, NULL, 0, NULL, 0, NULL, 1, &shi_frag);
  wlu_add_watchme_info(0, NULL, 0, NULL, 0, NULL, 2, &shi_vert);
  wlu_log_me(WLU_SUCCESS, "vert.spv and frag.spv officially created");
  wlu_log_me(WLU_INFO, "End of shader creation");

  VkImageView vkimg_attach[2];
  vkimg_attach[1] = app->depth.view;
  err = wlu_create_framebuffers(app, 2, vkimg_attach, extent3D.width, extent3D.height, 1);
  if (err) {
    freeme(app, wc, &shi_frag, &shi_vert);
    wlu_log_me(WLU_DANGER, "[x] wlu_create_framebuffers failed");
    return EXIT_FAILURE;
  }

  err = wlu_create_pipeline_cache(app, 0, NULL);
  if (err) {
    freeme(app, wc, NULL, NULL);
    wlu_log_me(WLU_DANGER, "[x] wlu_create_pipeline_cache failed");
    return EXIT_FAILURE;
  }

  /* Start of vertex buffer */
  err = wlu_create_uorv_buff(app, sizeof(g_vb_solid_face_colors_Data), g_vb_solid_face_colors_Data, 0, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
  if (err) {
    freeme(app, wc, &shi_frag, &shi_vert);
    wlu_log_me(WLU_DANGER, "[x] wlu_create_uniform_buff failed");
    return EXIT_FAILURE;
  }

  VkVertexInputAttributeDescription vi_attribs[2];
  vi_attribs[0] = wlu_set_vertex_input_attrib_desc(0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, 0);
  vi_attribs[1] = wlu_set_vertex_input_attrib_desc(0, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 16);

  VkVertexInputBindingDescription vi_binding = wlu_set_vertex_input_binding_desc(
    0, VK_VERTEX_INPUT_RATE_VERTEX, sizeof(g_vb_solid_face_colors_Data[0])
  );

  VkPipelineVertexInputStateCreateInfo vertext_input_info = wlu_set_vertex_input_state_info(
    1, &vi_binding, 2, vi_attribs
  );

  /* End of vertex buffer */

  VkShaderModule vert_shader_module = wlu_create_shader_module(app, shi_vert.bytes, shi_vert.byte_size);
  if (!vert_shader_module) {
    freeme(app, wc, &shi_frag, &shi_vert);
    wlu_log_me(WLU_DANGER, "[x] failed to create shader module");
    return EXIT_FAILURE;
  }

  VkShaderModule frag_shader_module = wlu_create_shader_module(app, shi_frag.bytes, shi_frag.byte_size);
  if (!frag_shader_module) {
    wlu_freeup_shader(app, &vert_shader_module);
    freeme(app, wc, &shi_frag, &shi_vert);
    wlu_log_me(WLU_DANGER, "[x] failed to create shader module");
    return EXIT_FAILURE;
  }

  wlu_add_watchme_info(1, app, 0, NULL, 1, &vert_shader_module, 0, NULL);
  wlu_add_watchme_info(1, app, 0, NULL, 2, &frag_shader_module, 0, NULL);

  VkPipelineShaderStageCreateInfo vert_shader_stage_info = wlu_set_shader_stage_info(
    vert_shader_module, "main", VK_SHADER_STAGE_VERTEX_BIT, NULL
  );

  VkPipelineShaderStageCreateInfo frag_shader_stage_info = wlu_set_shader_stage_info(
    frag_shader_module, "main", VK_SHADER_STAGE_FRAGMENT_BIT, NULL
  );

  VkPipelineShaderStageCreateInfo shader_stages[] = {
    vert_shader_stage_info, frag_shader_stage_info
  };

  VkDynamicState dynamic_states[2] = {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR
  };

  VkPipelineDynamicStateCreateInfo dynamic_state = wlu_set_dynamic_state_info(2, dynamic_states);

  VkPipelineInputAssemblyStateCreateInfo input_assembly = wlu_set_input_assembly_state_info(
    VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE
  );

  VkPipelineRasterizationStateCreateInfo rasterizer = wlu_set_rasterization_state_info(
    VK_TRUE, VK_FALSE, VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT,
    VK_FRONT_FACE_CLOCKWISE, VK_FALSE, 0.0f, 0.0f, 0.0f, 1.0f
  );

  VkPipelineColorBlendAttachmentState color_blend_attachment = wlu_set_color_blend_attachment_state(
    VK_FALSE, VK_BLEND_FACTOR_ZERO, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD,
    VK_BLEND_FACTOR_ZERO, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD,
    0xf
  );

  float blend_const[4] = {1.0f, 1.0f, 1.0f, 1.0f};
  VkPipelineColorBlendStateCreateInfo color_blending = wlu_set_color_blend_attachment_state_info(
    VK_FALSE, VK_LOGIC_OP_NO_OP, 1, &color_blend_attachment, blend_const
  );

  VkViewport viewport = wlu_set_view_port(0.0f, 0.0f, (float) extent3D.width, (float) extent3D.height, 0.0f, 1.0f);
  VkRect2D scissor = wlu_set_rect2D(0, 0, extent3D.width, extent3D.height);
  VkPipelineViewportStateCreateInfo view_port_info = wlu_set_view_port_state_info(1, &viewport, 1, &scissor);

  VkStencilOpState back = wlu_set_stencil_op_state(
    VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP,
    VK_COMPARE_OP_ALWAYS, 0, 0, 0
  );

  VkPipelineDepthStencilStateCreateInfo ds_info = wlu_set_depth_stencil_state(
    VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL,
    VK_FALSE, VK_FALSE, back, back, 0, 0
  );

  VkPipelineMultisampleStateCreateInfo multisampling = wlu_set_multisample_state_info(
    VK_SAMPLE_COUNT_1_BIT, VK_FALSE, 0.0f, NULL, VK_FALSE, VK_FALSE
  );

  err = wlu_create_graphics_pipeline(app, 2, shader_stages,
    &vertext_input_info, &input_assembly, VK_NULL_HANDLE, &view_port_info,
    &rasterizer, &multisampling, &ds_info, &color_blending,
    &dynamic_state, 0, VK_NULL_HANDLE, UINT32_MAX);
  if (err) {
    wlu_freeup_shader(app, &frag_shader_module);
    wlu_freeup_shader(app, &vert_shader_module);
    freeme(app, wc, &shi_frag, &shi_vert);
    wlu_log_me(WLU_DANGER, "[x] failed to create graphics pipeline");
    return EXIT_FAILURE;
  }

  wlu_log_me(WLU_SUCCESS, "Successfully created graphics pipeline");

  err = wlu_create_semaphores(app);
  if (err) {
    wlu_freeup_shader(app, &frag_shader_module);
    wlu_freeup_shader(app, &vert_shader_module);
    freeme(app, wc, &shi_frag, &shi_vert);
    wlu_log_me(WLU_DANGER, "[x] wlu_create_semaphores failed");
    return EXIT_FAILURE;
  }

  uint32_t cur_buff;
  /* Acquire the swapchain image in order to set its layout */
  err = wlu_retrieve_swapchain_img(app, &cur_buff);
  if (err) {
    wlu_freeup_shader(app, &frag_shader_module);
    wlu_freeup_shader(app, &vert_shader_module);
    freeme(app, wc, &shi_frag, &shi_vert);
    wlu_log_me(WLU_DANGER, "[x] wlu_retrieve_swapchain_img failed");
    return EXIT_FAILURE;
  }

  /* We cannot bind the vertex buffer until we begin a renderpass */
  VkClearValue clear_values[2];
  clear_values[0].color.float32[0] = 0.2f;
  clear_values[0].color.float32[1] = 0.2f;
  clear_values[0].color.float32[2] = 0.2f;
  clear_values[0].color.float32[3] = 0.2f;
  clear_values[1].depthStencil.depth = 1.0f;
  clear_values[1].depthStencil.stencil = 0;

  wlu_exec_begin_render_pass(app, 0, 0, extent3D.width, extent3D.height,
                             2, clear_values, VK_SUBPASS_CONTENTS_INLINE);
  wlu_bind_gp(app, cur_buff, VK_PIPELINE_BIND_POINT_GRAPHICS);
  wlu_bind_desc_set(app, cur_buff, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, 0, NULL);

  const VkDeviceSize offsets[1] = {0};
  wlu_bind_vertex_buff_to_cmd_buffs(app, cur_buff, 0, 1, offsets);

  wlu_cmd_set_viewport(app, viewport, cur_buff, 0, 1);
  // wlu_cmd_draw(app, cur_buff, 12 * 3, 1, 0, 0);

  wlu_exec_stop_render_pass(app);
  wlu_exec_stop_cmd_buffs(app);

  VkPipelineStageFlags pipe_stage_flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  err = wlu_queue_graphics_queue(app, 1, cur_buff, 0, NULL, &pipe_stage_flags, 0, NULL);
  if (err) {
    wlu_freeup_shader(app, &frag_shader_module);
    wlu_freeup_shader(app, &vert_shader_module);
    freeme(app, wc, &shi_frag, &shi_vert);
    wlu_log_me(WLU_DANGER, "[x] wlu_exec_queue_cmd_buff failed");
    return EXIT_FAILURE;
  }

  err = wlu_queue_present_queue(app, 0, NULL, 1, &app->swap_chain, &cur_buff, NULL);
  if (err) {
    wlu_freeup_shader(app, &frag_shader_module);
    wlu_freeup_shader(app, &vert_shader_module);
    freeme(app, wc, &shi_frag, &shi_vert);
    wlu_log_me(WLU_DANGER, "[x] wlu_exec_queue_cmd_buff failed");
    return EXIT_FAILURE;
  }

  wait_seconds(1);

  wlu_freeup_shader(app, &frag_shader_module);
  wlu_freeup_shader(app, &vert_shader_module);
  freeme(app, wc, &shi_frag, &shi_vert);

  return EXIT_SUCCESS;
}