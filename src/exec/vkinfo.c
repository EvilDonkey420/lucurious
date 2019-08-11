#include <lucom.h>
#include <wlu/vlucur/vkall.h>
#include <exec/vkinfo.h>

const char *device_extensions[] = {
  VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

const char *instance_extensions[] = {
  VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME,
  VK_KHR_SURFACE_EXTENSION_NAME,
  VK_KHR_DISPLAY_EXTENSION_NAME
};

void help_message() {
  fprintf(stdout, "Usage: lucur [options]\n");
  fprintf(stdout, "Example: lucur --pde VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU\n");

  fprintf(stdout, "Options:\n");
  fprintf(stdout, "\t-l, --pgvl\t\t\t Print global validation layers\n");
  fprintf(stdout, "\t-i, --pie\t\t\t Print instance extenstion list\n");
  fprintf(stdout, "\t-d, --pde <VkPhysicalDeviceType> Print device extenstion list\n");
  fprintf(stdout, "\t-v, --version\t\t\t Print lucurious library version\n");
  fprintf(stdout, "\t-h, --help\t\t\t Show this message\n");
}

void version_num() {
  fprintf(stdout, "liblucurious 0.0.1\n");
}

void lower_to_upper(char *s) {
  while (*s != '\0') {
    if (*s >= 'a' && *s <= 'z')
      *s = ('A' + *s - 'a');
    s++;
  }
}

void print_gvalidation_layers() {
  VkResult err;
  vkcomp *app = NULL;
  app = wlu_init_vk();

  err = wlu_set_global_layers(app);
  if (err) {
    wlu_freeup_vk(app);
    fprintf(stdout, "[x] The vulkan sdk must not be installed\n");
    fprintf(stdout, "[x] wlu_set_global_layers failed with error code: %d\n", err);
    return;
  }

  printf("\t   Validation Layers List\n  SpecVersion\t\tLayer Name\t\t\tDescription\n");
  for (uint32_t i = 0; i < app->vk_layer_count; i++) {
    fprintf(stdout, "\t%d\t %s\t   : %s\n",
          app->vk_layer_props[i].specVersion,
          app->vk_layer_props[i].layerName,
          app->vk_layer_props[i].description);
  }
  fprintf(stdout, "\tValidation Layer Count: %d\n", app->vk_layer_count);

  wlu_freeup_vk(app);
}

void print_instance_extensions() {
  VkResult err;
  vkcomp *app = NULL;
  app = wlu_init_vk();

  err = wlu_create_instance(app, "PrintStmt", "PrintStmt", 0, NULL, 3, instance_extensions);
  if (err) {
    fprintf(stdout, "[x] Failed to create instance\n");
    wlu_freeup_vk(app);
    return;
  }

  fprintf(stdout, "\t   Instance Extension List\n  SpecVersion\t\tExtension Name\n\n");
  for (uint32_t i = 0; i < app->ep_instance_count; i++) {
    lower_to_upper(app->ep_instance_props[i].extensionName);
    printf("\t%d\t %s_EXTENSION_NAME\n",
            app->ep_instance_props[i].specVersion,
            app->ep_instance_props[i].extensionName);
  }
  fprintf(stdout, "Instance Extension Count: %d\n", app->ep_instance_count);

  wlu_freeup_vk(app);
}

void print_device_extensions(VkPhysicalDeviceType dt) {
  VkResult err;
  vkcomp *app = NULL;
  app = wlu_init_vk();

  err = wlu_create_instance(app, "PrintStmt", "PrintStmt", 0, NULL, 3, instance_extensions);
  if (err) {
    wlu_freeup_vk(app);
    return;
  }

  err = wlu_enumerate_devices(app, dt);
  if (err) {
    wlu_freeup_vk(app);
    return;
  }

  printf("\t   Device Extension List\n  SpecVersion\t\tExtension Name\n\n");
  for (uint32_t i = 0; i < app->ep_device_count; i++) {
    lower_to_upper(app->ep_device_props[i].extensionName);
    fprintf(stdout, "\t%d\t %s_EXTENSION_NAME\n",
            app->ep_device_props[i].specVersion,
            app->ep_device_props[i].extensionName);
  }
  printf("Device Extension Count: %d\n", app->ep_device_count);

  wlu_freeup_vk(app);
}
