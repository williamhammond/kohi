#include "core/logger.h"
#include "vulkan_backend.h"

#include "vulkan_types.inl"

static vulkan_context context;

b8 vulkan_initialize(renderer_backend* backend, const char* application_name, struct platform_state* plat_state) {
    // TODO: Implement custom allocator
    context.allocator = NULL;

    VkApplicationInfo app_info = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
    app_info.apiVersion = VK_API_VERSION_1_3;
    app_info.pApplicationName = application_name;
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "Kohi Engine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);

    VkInstanceCreateInfo create_info = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    create_info.pApplicationInfo = &app_info;
    create_info.pApplicationInfo = &app_info;
    create_info.enabledExtensionCount = 0;
    create_info.ppEnabledExtensionNames = NULL;
    create_info.enabledLayerCount = 0;
    create_info.ppEnabledLayerNames = NULL;

    VkResult result = vkCreateInstance(&create_info, context.allocator, &context.instance);
    if (result != VK_SUCCESS) {
        KERROR("Failed to create Vulkan instance with result code: %u", result);
        return FALSE;
    }
    KINFO("Vulkan renderer initialized successfully");
    return TRUE;
}

void vulkan_shutdown(renderer_backend* backend) {

}

void vulkan_resized(renderer_backend* backend, u16 width, u16 height) {

}

b8 vulkan_begin_frame(renderer_backend* backend, f32 delta_time) {
    return TRUE;
}

b8 vulkan_end_frame(renderer_backend* backend, f32 delta_time) {
    return TRUE;
}