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

#ifndef VKALL_H
#define VKALL_H

#include <cglm/types.h>

#define VK_USE_PLATFORM_WAYLAND_KHR 1
#include <vulkan/vulkan.h>

#include "types.h"

#ifdef INCLUDE_MATRIX_H
#include "matrix.h"
#endif

vkcomp *wlu_init_vk();

/* Free up swap chain */
void wlu_freeup_sc(void *data);

void wlu_freeup_vk(void *data);

/* Simple Effective One Time Buffer Allocater */
VkResult wlu_otba(vkcomp *app, uint32_t buff_size, wlu_data_type type);

VkResult wlu_set_debug_message(vkcomp *app);

/*
 * Almost every operation in Vulkan, from submitting command buffers
 * to presenting images to a surface, requires commands to be submitted
 * to a hardware queue. This will create multiple queue families
 * that are supported by the VkQueueFlagBits set and assign the
 * available graphics and present queues
 */
VkBool32 wlu_set_queue_family(vkcomp *app, VkQueueFlagBits vkqfbits);

/*
 * Needed to create the swap chain. This function queries your physical device's
 * capabilities. Mainly used to get minImageCount and the extent/resolution
 * that a particular physical device
 */
VkSurfaceCapabilitiesKHR wlu_q_device_capabilities(vkcomp *app);

/* Create connection between app and the vulkan api */
VkResult wlu_create_instance(
  vkcomp *app,
  char *app_name,
  char *engine_name,
  uint32_t enabledLayerCount,
  const char* const* ppEnabledLayerNames,
  uint32_t enabledExtensionCount,
  const char* const* ppEnabledExtensionNames
);

/*
 * This function will select the physical device of
 * your choosing based off of VkPhysicalDeviceType
 */
VkResult wlu_create_physical_device(
  vkcomp *app,
  VkPhysicalDeviceType vkpdtype,
  VkPhysicalDeviceProperties *device_properties,
  VkPhysicalDeviceFeatures *device_features
);

/*
 * After selecting a physical device to use.
 * Set up a logical device to interface with your physical device
 * This function is also used to set Vulkan Device Level Extensions
 * that entail what a device does
 */
VkResult wlu_create_logical_device(
  vkcomp *app,
  VkPhysicalDeviceFeatures *device_feats,
  uint32_t queue_count,
  uint32_t enabledLayerCount,
  const char *const *ppEnabledLayerNames,
  uint32_t enabledExtensionCount,
  const char *const *ppEnabledExtensionNames
);

/* Create the actual swap chain used to present images to a surface */
VkResult wlu_create_swap_chain(
  vkcomp *app,
  uint32_t cur_scd,
  VkSurfaceCapabilitiesKHR capabilities,
  VkSurfaceFormatKHR surface_fmt,
  VkPresentModeKHR pres_mode,
  uint32_t width,
  uint32_t height
);

/*
 * Create image views which is the way you communicate to vulkan
 * on how you intend to use the images in a swap chain
 */
VkResult wlu_create_img_views(
  vkcomp *app,
  uint32_t cur_scd,
  VkFormat format,
  VkImageViewType type
);

/* Need to depth buffer to render 3D images (only need one) */
VkResult wlu_create_depth_buff(
  vkcomp *app,
  uint32_t cur_scd,
  VkFormat depth_format,
  VkFormatFeatureFlags linearTilingFeatures,
  VkFormatFeatureFlags optimalTilingFeatures,
  VkImageType imageType,
  VkExtent3D extent,
  VkImageUsageFlags usage,
  VkSharingMode sharingMode,
  VkImageLayout initialLayout,
  VkImageViewType viewType
);

/*
 * Function creates buffers like a uniform buffer so that shaders can access
 * in a read-only fashion constant parameter data. Function also
 * creates buffers like a vertex buffer so that it's visible to the CPU
 */
VkResult wlu_create_buffer(
  vkcomp *app,
  uint32_t cur_bd,
  VkDeviceSize size,
  void *data,
  VkBufferCreateFlagBits flags,
  VkBufferUsageFlags usage,
  VkSharingMode sharingMode,
  uint32_t queueFamilyIndexCount,
  const uint32_t *pQueueFamilyIndices,
  char *buff_name,
  VkFlags requirements_mask
);

VkResult wlu_create_framebuffers(
  vkcomp *app,
  uint32_t cur_scd,
  uint32_t cur_gpd,
  uint32_t attachmentCount,
  VkImageView *attachments,
  uint32_t width,
  uint32_t height,
  uint32_t layers
);

/*
 * Allows for your app to create a command pool to store your
 * command buffers before being committed to main memory
 */
VkResult wlu_create_cmd_pool(
  vkcomp *app,
  uint32_t cur_scd,
  uint32_t cur_cmdd,
  uint32_t queueFamilyIndex,
  VkCommandPoolCreateFlagBits flags
);

/*
 * Allows for your app to put commands into a buffer to later
 * be submitted to one of the hardware queues
 */
VkResult wlu_create_cmd_buffs(
  vkcomp *app,
  uint32_t cur_pool,
  uint32_t cur_scd,
  VkCommandBufferLevel level
);

/*
 * Can find in Vulkan SDK samples/API-Samples/10-init_render_pass
 * A semaphore (or fence) is required in order to acquire a
 * swapchain image to prepare it for use in a render pass.
 * The semaphore is normally used to hold back the rendering
 * operation until the image is actually available. This function
 * creates semaphores
 */
VkResult wlu_create_semaphores(vkcomp *app, uint32_t cur_scd);

VkResult wlu_exec_begin_cmd_buffs(
  vkcomp *app,
  uint32_t cur_pool,
  uint32_t cur_scd,
  VkCommandBufferUsageFlags flags,
  const VkCommandBufferInheritanceInfo *pInheritanceInfo
);

VkResult wlu_exec_stop_cmd_buffs(vkcomp *app, uint32_t cur_pool, uint32_t cur_scd);

/*
 * How Vulkan establishes connection with window system.
 * Through the use of Window System Integration (WSI).
 * This fuction exposes a VkSurfaceKHR object that represents
 * a surface to present rendered images to.
 */
VkResult wlu_vkconnect_surfaceKHR(vkcomp *app, void *wl_display, void *wl_surface);


/*
 * Needed to create the swap chain. This will specify the format and
 * the surace of an image. The "format" variable refers to the pixel
 * formats and the "colorSpace" variable refers to the Color Depth
 */
VkSurfaceFormatKHR wlu_choose_swap_surface_format(vkcomp *app, VkFormat format, VkColorSpaceKHR colorSpace);

/*
 * Needed to create the swap chain
 * This function chooses the best presentation mode for swapchain
 * (Conditions required for swapping images to the screen)
 */
VkPresentModeKHR wlu_choose_swap_present_mode(vkcomp *app);

/*
 * Needed to create the swap chain. Pick the resolution
 * for the images in the swap chain
 */
VkExtent2D wlu_choose_2D_swap_extent(VkSurfaceCapabilitiesKHR capabilities, uint32_t width, uint32_t height);

/*
 * Needed to create the swap chain. Pick the resolution
 * for the images in the swap chain along with the depth
 * of the 3D object
 */
VkExtent3D wlu_choose_3D_swap_extent(VkSurfaceCapabilitiesKHR capabilities, uint32_t width, uint32_t height, uint32_t depth);

/* Acquire the swapchain image in order to set its layout */
VkResult wlu_retrieve_swapchain_img(vkcomp *app, uint32_t *cur_buff, uint32_t cur_scd);

VkResult wlu_queue_graphics_queue(
  vkcomp *app,
  uint32_t commandBufferCount,
  VkCommandBuffer *pCommandBuffers,
  uint32_t waitSemaphoreCount,
  const VkSemaphore *pWaitSemaphores,
  const VkPipelineStageFlags *pWaitDstStageMask,
  uint32_t signalSemaphoreCount,
  const VkSemaphore *pSignalSemaphores
);

/* Submit results back to the swap chain */
VkResult wlu_queue_present_queue(
  vkcomp *app,
  uint32_t waitSemaphoreCount,
  const VkSemaphore *pWaitSemaphores,
  uint32_t swapchainCount,
  const VkSwapchainKHR *Swapchains,
  const uint32_t *pImageIndices,
  VkResult *pResults
);

VkResult wlu_copy_buffer(
  vkcomp *app,
  uint32_t cur_pool,
  VkBuffer src_buffer,
  VkBuffer dst_buffer,
  VkDeviceSize size
);

VkCommandBufferInheritanceInfo wlu_set_cmd_buff_inheritance_info(
  VkRenderPass renderPass,
  uint32_t subpass,
  VkFramebuffer framebuffer,
  VkBool32 occlusionQueryEnable,
  VkQueryControlFlags queryFlags,
  VkQueryPipelineStatisticFlags pipelineStatistics
);

#endif
