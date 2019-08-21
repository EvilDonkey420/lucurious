#include <lucom.h>
#include <wlu/vlucur/vkall.h>
#include <wlu/utils/log.h>

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

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_report_callbackFN(
  VkDebugReportFlagsEXT flags,
  VkDebugReportObjectTypeEXT objectType,
  uint64_t object,
  size_t  location,
  int32_t messageCode,
  const char *pLayerPrefix,
  const char *pMessage,
  void *pUserData
) {

  ALL_UNUSED(flags, objectType, object,
             messageCode, location,
             pLayerPrefix, pUserData);

  wlu_log_me(WLU_DANGER, "%s", pMessage);

  return VK_FALSE;
}

VkResult wlu_set_debug_message(vkcomp *app, uint32_t size) {
  VkResult res = VK_RESULT_MAX_ENUM;
  VkDebugReportCallbackEXT debug_report_callback;

  if (!app->vk_layer_props) {
    wlu_log_me(WLU_DANGER, "[x] Vulkan Validation Layers must be created");
    wlu_log_me(WLU_DANGER, "[x] Must make a call to wlu_set_global_layers(3)");
    wlu_log_me(WLU_DANGER, "[x] See man pages for further details");
    return res;
  }

  if (!app->instance) {
    wlu_log_me(WLU_DANGER, "[x] A Vulkan Instance must be created");
    wlu_log_me(WLU_DANGER, "[x] Must make a call to wlu_create_instance(3)");
    wlu_log_me(WLU_DANGER, "[x] See man pages for further details");
    return res;
  }

  app->dbg_size = size;
  app->dbg_create_report_callback = (PFN_vkCreateDebugReportCallbackEXT) \
      vkGetInstanceProcAddr(app->instance, "vkCreateDebugReportCallbackEXT");
  if (!app->dbg_create_report_callback) {
    wlu_log_me(WLU_DANGER, "GetInstanceProcAddr: Unable to find vkCreateDebugReportCallbackEXT function");
    return VK_ERROR_INITIALIZATION_FAILED;
  }

  wlu_log_me(WLU_SUCCESS, "Got dbg_create_report_callback function");

  app->debug_messenger = (PFN_vkDebugReportMessageEXT) \
      vkGetInstanceProcAddr(app->instance, "vkDebugReportMessageEXT");
  if (!app->debug_messenger) {
    wlu_log_me(WLU_DANGER, "GetInstanceProcAddr: Unable to find vkDebugReportMessageEXT function.");
    return VK_ERROR_INITIALIZATION_FAILED;
  }

  wlu_log_me(WLU_SUCCESS, "Got debug_messenger function");

  app->dbg_destroy_report_callback = (PFN_vkDestroyDebugReportCallbackEXT) \
    vkGetInstanceProcAddr(app->instance, "vkDestroyDebugReportCallbackEXT");
  if (!app->dbg_destroy_report_callback) {
    wlu_log_me(WLU_DANGER, "GetInstanceProcAddr: Unable to find vkDestroyDebugReportCallbackEXT function.");
    return VK_ERROR_INITIALIZATION_FAILED;
  }

  wlu_log_me(WLU_SUCCESS, "Got dbg_destroy_report_callback function");

  VkDebugReportCallbackCreateInfoEXT create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
  create_info.pNext = NULL;
  create_info.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT   |
                     VK_DEBUG_REPORT_WARNING_BIT_EXT |
                     VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
  create_info.pfnCallback = debug_report_callbackFN;
  create_info.pUserData = NULL;

  res = app->dbg_create_report_callback(app->instance, &create_info, NULL, &debug_report_callback);

  wlu_log_me(WLU_SUCCESS, "Successfully created debug report callback object");

  app->debug_report_callbacks = (VkDebugReportCallbackEXT *) calloc(
      sizeof(VkDebugReportCallbackEXT), app->dbg_size * sizeof(VkDebugReportCallbackEXT));
  if (!app->debug_report_callbacks) {
    wlu_log_me(WLU_DANGER, "[x] calloc VkDebugReportCallbackEXT *debug_report_callbacks failed");
    return VK_RESULT_MAX_ENUM;
  }

  for (uint32_t i = 0; i < app->dbg_size; i++)
    app->debug_report_callbacks[i] = debug_report_callback;

  return res;
}