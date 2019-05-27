#include <vlucur/vlucur.h>
#include <vlucur/errors.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cglm/call.h>

const int WIDTH = 800;
const int HEIGHT = 600;

/* All of the useful standard validation is
  bundled into a layer included in the SDK */
const char *validation_layers = { "VK_LAYER_KHRONOS_validation" };

#define NDEBUG // Defined here becuase I don't have the vulkan sdk installed

#ifdef NDEBUG
  const bool enable_validation_layers = false;
#else
  const bool enable_validation_layers = true;
#endif

struct htapp {
  GLFWwindow *window;

  /* Connection between application and the Vulkan library */
  VkInstance instance;

  VkExtensionProperties *vkprops;
  VkLayerProperties *available_layers;

  const char **glfw_extensions;
  uint32_t glfw_extension_count;

  VkDebugUtilsMessengerEXT debug_messenger;
};

void set_required_extension(struct htapp *app) {

  app->glfw_extensions = glfwGetRequiredInstanceExtensions(&app->glfw_extension_count);
  assert(app->glfw_extensions != NULL);

  // if (enable_validation_layers)
  //   result.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

}

bool check_validation_layer_support(struct htapp *app) {
  VkResult err;
  uint32_t layer_count;
  err = vkEnumerateInstanceLayerProperties(&layer_count, NULL);
  assert(!err);

  app->available_layers = calloc(sizeof(VkLayerProperties), layer_count * sizeof(VkLayerProperties));
  assert(app->available_layers != NULL);

  err = vkEnumerateInstanceLayerProperties(&layer_count, app->available_layers);
  assert(!err);

  bool layer_found = false;

  for (unsigned int j = 0; j < layer_count; j++) {
    if (!strcmp(&validation_layers[0], app->available_layers[j].layerName)) {
      layer_found = true;
      break;
    }
  }

  return (layer_found) ? true : false;
}

void init_window(struct htapp *app) {
  if(!glfwInit()) return;
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  app->window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", NULL, NULL);
  glfwMakeContextCurrent(app->window);
}

void main_loop(struct htapp *app) {
  printf("glfwWindow %d\n", glfwWindowShouldClose(app->window));

  // Still working on why window not showing up
  // Could be the wayland compositor I'm using
  return;
  while (!glfwWindowShouldClose(app->window))
    glfwPollEvents();
}

void create_instance(struct htapp *app) {
  if (enable_validation_layers && !check_validation_layer_support(app)) {
    perror("[x] validation layers requested, but not available!\n");
    return;
  }

  VkResult err;

  VkApplicationInfo app_info = {};
  app_info.pApplicationName = "Hello Triangle";
  app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.pEngineName = "No Engine";
  app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.apiVersion = VK_API_VERSION_1_0;

  /* tells the Vulkan driver which global extensions
    and validation layers we want to use */
  VkInstanceCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pApplicationInfo = &app_info;

  /* glfw3 extesion to interface vulkan with the window system */
  set_required_extension(app);

  create_info.enabledExtensionCount = app->glfw_extension_count;
  create_info.ppEnabledExtensionNames = app->glfw_extensions;

  if (enable_validation_layers) {
    create_info.enabledLayerCount = (uint32_t) (sizeof(validation_layers));
    create_info.ppEnabledLayerNames = &validation_layers;
  } else {
    create_info.enabledLayerCount = 0;
  }

  /* Create the instance */
  VkResult result = vkCreateInstance(&create_info, NULL, &app->instance);

  if (result != VK_SUCCESS) {
    perror("[x] failed to created instance");
    return;
  }

  uint32_t extension_count = 0;
  err = vkEnumerateInstanceExtensionProperties(NULL, &extension_count, NULL);
  assert(!err);

  if (extension_count > 0) {

    app->vkprops = calloc(sizeof(VkExtensionProperties), extension_count * sizeof(VkExtensionProperties));
    assert(app->vkprops != NULL);

    err = vkEnumerateInstanceExtensionProperties(NULL, &extension_count, app->vkprops);
    assert(!err);

    printf("Instance created\navailable extesions: %d\n", extension_count);

    for (unsigned int i = 0; i < extension_count; i++)
      printf("%s\n", app->vkprops[i].extensionName);

  }
}

void setup_debug_messenger(struct htapp *app) {
  if (!enable_validation_layers) return;

  VkDebugUtilsMessengerCreateInfoEXT create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

  /* specify all the types of severities you
    would like your callback to be called for */
  create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | \
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | \
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

  /* filter which types of messages your callback is
  notified about. all types are enabled here */
  create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | \
                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | \
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  create_info.pfnUserCallback = debug_callback;
  create_info.pUserData = NULL; // Optional

  if (CreateDebugUtilsMessengerEXT(app->instance, &create_info, NULL, &app->debug_messenger) != VK_SUCCESS) {
    perror("[x] failed to set up debug messenger!");
    return;
  }
}

void init_vulkan(struct htapp *app) {
  create_instance(app);
  setup_debug_messenger(app);
}

void cleanup(struct htapp *app) {

  if (enable_validation_layers)
    DestroyDebugUtilsMessengerEXT(app->instance, app->debug_messenger, NULL);

  free(app->vkprops);
  app->vkprops = NULL;

  free(app->available_layers);
  app->available_layers = NULL;

  vkDestroyInstance(app->instance, NULL);
  glfwDestroyWindow(app->window);
  glfwTerminate();
}

void run() {
  struct htapp app;

  app.window = NULL;
  app.instance = 0;
  app.vkprops = NULL;
  app.available_layers = NULL;
  app.glfw_extensions = NULL;
  app.glfw_extension_count = 0;

  init_window(&app);
  init_vulkan(&app);
  main_loop(&app);
  cleanup(&app);
}
