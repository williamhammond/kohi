#include "vulkan_device.h"

#include "containers/darray.h"

#include "core/kmemory.h"
#include "core/kstring.h"
#include "core/logger.h"

typedef struct vulkan_physical_device_requirements {
    b8 graphics;
    b8 present;
    b8 compute;
    b8 transfer;
    // darray
    const char** device_extension_names;
    b8 sampler_anisotropy;
    b8 discrete_gpu;
} vulkan_physical_device_requirements;

typedef struct vulkan_physical_device_queue_family_info {
    u32 graphics_family_index;
    u32 present_family_index;
    u32 compute_family_index;
    u32 transfer_family_index;
} vulkan_physical_device_queue_family_info;

b8 select_physical_device(vulkan_context* context);

b8 physical_device_meets_requirements(
    VkPhysicalDevice device,
    VkSurfaceKHR surface,
    const VkPhysicalDeviceProperties* properties,
    const VkPhysicalDeviceFeatures* features,
    const vulkan_physical_device_requirements* requirements,
    vulkan_physical_device_queue_family_info* out_queue_info,
    vulkan_swapchain_support_info* out_swapchain_support);

b8 vulkan_device_create(vulkan_context* context) {
    if (!select_physical_device(context)) {
        return false;
    }

    KINFO("Creating logical GPU device");

    // Do not create additional queues for shared indices
    u32 index_count = 1;
    b8 present_shares_graphics_queue = context->device.graphics_queue_index == context->device.present_queue_index;
    if (!present_shares_graphics_queue) {
        index_count++;
    }
    b8 transfer_shares_graphics_queue = context->device.graphics_queue_index == context->device.transfer_queue_index;
    if (!transfer_shares_graphics_queue) {
        index_count++;
    }

    u32 indices[MAX_QUEUE_COUNT];
    u32 index = 0;
    indices[index] = context->device.graphics_queue_index;
    index++;
    if (!present_shares_graphics_queue) {
        indices[index] = context->device.present_queue_index;
        index++;
    }
    if (!transfer_shares_graphics_queue) {
        indices[index] = context->device.transfer_queue_index;
        index++;
    }

    VkDeviceQueueCreateInfo queue_create_infos[MAX_QUEUE_COUNT];
    for (u32 i = 0; i < index_count; i++) {
        queue_create_infos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_infos[i].queueFamilyIndex = indices[i];
        queue_create_infos[i].queueCount = 1;

        // TODO: Readd and fix when multithreading the renderer
        // if (indices[i] == context->device.graphics_queue_index) {
        //     queue_create_infos[i].queueCount = 2;
        // }
        queue_create_infos[i].flags = 0;
        queue_create_infos[i].pNext = NULL;
        f32 queue_priority = 1.0f;
        queue_create_infos[i].pQueuePriorities = &queue_priority;
    }

    // TODO: Use config for this
    VkPhysicalDeviceFeatures device_features = {};
    device_features.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo device_create_info = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    device_create_info.queueCreateInfoCount = index_count;
    device_create_info.pQueueCreateInfos = queue_create_infos;
    device_create_info.pEnabledFeatures = &device_features;
    device_create_info.enabledExtensionCount = 1;

    const char* extension_names = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    device_create_info.ppEnabledExtensionNames = &extension_names;

    // Deprecated and ignored, so pass nothing
    device_create_info.enabledLayerCount = 0;
    device_create_info.ppEnabledLayerNames = NULL;

    VK_CHECK(vkCreateDevice(
        context->device.physical_device,
        &device_create_info,
        context->allocator,
        &context->device.logical_device));
    KINFO("Logical device created");

    vkGetDeviceQueue(
        context->device.logical_device,
        context->device.graphics_queue_index,
        0,
        &context->device.graphics_queue);
    vkGetDeviceQueue(
        context->device.logical_device,
        context->device.transfer_queue_index,
        0,
        &context->device.transfer_queue);
    vkGetDeviceQueue(
        context->device.logical_device,
        context->device.present_queue_index,
        0,
        &context->device.present_queue);
    KINFO("Queue obtained");

    VkCommandPoolCreateInfo pool_create_info = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    pool_create_info.queueFamilyIndex = context->device.graphics_queue_index;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    VK_CHECK(vkCreateCommandPool(
        context->device.logical_device,
        &pool_create_info,
        context->allocator,
        &context->device.graphics_command_pool));

    KINFO("Graphics command pool created");

    return true;
}

void vulkan_device_destroy(vulkan_context* context) {
    context->device.present_queue = 0;
    context->device.graphics_queue = 0;
    context->device.transfer_queue = 0;

    KINFO("Destroying command pools");
    vkDestroyCommandPool(
        context->device.logical_device,
        context->device.graphics_command_pool,
        context->allocator);

    KINFO("Destroying logical device");
    if (context->device.logical_device) {
        vkDestroyDevice(context->device.logical_device, context->allocator);
        context->device.logical_device = NULL;
    }

    KINFO("Releasing physical device resources");
    context->device.physical_device = NULL;

    if (context->device.swapchain_support.formats) {
        kfree(
            context->device.swapchain_support.formats,
            sizeof(VkSurfaceFormatKHR) * context->device.swapchain_support.format_count,
            MEMORY_TAG_RENDERER);
        context->device.swapchain_support.format_count = 0;
        context->device.swapchain_support.formats = NULL;
    }

    if (context->device.swapchain_support.present_modes) {
        kfree(
            context->device.swapchain_support.present_modes,
            sizeof(VkSurfaceFormatKHR) * context->device.swapchain_support.format_count,
            MEMORY_TAG_RENDERER);
        context->device.swapchain_support.present_mode_count = 0;
        context->device.swapchain_support.present_modes = NULL;
    }

    kzero_memory(
        &context->device.properties,
        sizeof(context->device.swapchain_support.capabilities));

    context->device.graphics_queue_index = -1;
    context->device.present_queue_index = -1;
    context->device.transfer_queue_index = -1;
}

void vulkan_device_query_swapchain_support(
    VkPhysicalDevice physical_device,
    VkSurfaceKHR surface,
    vulkan_swapchain_support_info* out_support_info) {
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        physical_device,
        surface,
        &out_support_info->capabilities));

    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
        physical_device,
        surface,
        &out_support_info->format_count,
        0));

    if (out_support_info->format_count != 0) {
        if (!out_support_info->formats) {
            out_support_info->formats = kallocate(sizeof(VkSurfaceFormatKHR) * out_support_info->format_count, MEMORY_TAG_RENDERER);
        }
        VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
            physical_device,
            surface,
            &out_support_info->format_count,
            out_support_info->formats));
    }

    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
        physical_device,
        surface,
        &out_support_info->present_mode_count,
        0));

    if (out_support_info->present_mode_count != 0) {
        if (!out_support_info->present_modes) {
            out_support_info->present_modes = kallocate(sizeof(VkPresentModeKHR) * out_support_info->present_mode_count, MEMORY_TAG_RENDERER);
        }
        VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
            physical_device,
            surface,
            &out_support_info->present_mode_count,
            out_support_info->present_modes));
    }
}

b8 vulkan_device_detect_depth_format(vulkan_device* device) {
    const u64 candidate_count = 3;
    VkFormat candidates[3] = {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT,
    };

    u32 flags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
    for (u64 i = 0; i < candidate_count; i++) {
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(device->physical_device, candidates[i], &properties);

        if ((properties.linearTilingFeatures & flags) == flags) {
            device->depth_format = candidates[i];
            return true;
        } else if ((properties.optimalTilingFeatures & flags) == flags) {
            device->depth_format = candidates[i];
            return true;
        }
    }
    return false;
}

b8 select_physical_device(vulkan_context* context) {
    u32 physical_device_count = 0;
    VK_CHECK(vkEnumeratePhysicalDevices(context->instance, &physical_device_count, 0));
    if (physical_device_count == 0) {
        KFATAL("No devices which support Vulkan were found");
        return false;
    }

    VkPhysicalDevice physical_devices[MAX_PHYSICAL_DEVICE_COUNT];
    VK_CHECK(vkEnumeratePhysicalDevices(context->instance, &physical_device_count, physical_devices));

    for (u32 i = 0; i < physical_device_count; i++) {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(physical_devices[i], &properties);

        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(physical_devices[i], &features);

        VkPhysicalDeviceMemoryProperties memory;
        vkGetPhysicalDeviceMemoryProperties(physical_devices[i], &memory);

        b8 supports_device_local_host_visible = false;
        for (u32 i = 0; i < memory.memoryTypeCount; i++) {
            if (
                (memory.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) &&
                (memory.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
            ) {
                supports_device_local_host_visible = true;
                break;
            }
        }

        vulkan_physical_device_requirements requirements = {};
        requirements.graphics = true;
        requirements.present = true;
        requirements.transfer = true;
        // NOTE: Enable this if compute will be required
        // requirements.compute = TRUE;
        requirements.sampler_anisotropy = true;
        requirements.discrete_gpu = true;
        requirements.device_extension_names = darray_create(const char*);
        darray_push(requirements.device_extension_names, &VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        vulkan_physical_device_queue_family_info queue_info = {};
        b8 result = physical_device_meets_requirements(
            physical_devices[i],
            context->surface,
            &properties,
            &features,
            &requirements,
            &queue_info,
            &context->device.swapchain_support);
        if (result) {
            KINFO("Selected device: %s", properties.deviceName);
            switch (properties.deviceType) {
                case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                    KINFO("Device type: Discrete GPU");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                    KINFO("Device type: Integrated GPU");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                    KINFO("Device type: Virtual GPU");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_CPU:
                    KINFO("Device type: CPU");
                    break;
                default:
                    KINFO("Device type: Unknown");
                    break;
            }

            KINFO(
                "GPU Driver Version: %d.%d.%d",
                VK_VERSION_MAJOR(properties.driverVersion),
                VK_VERSION_MINOR(properties.driverVersion),
                VK_VERSION_PATCH(properties.driverVersion));

            KINFO(
                "Vulkan API VERSION: %d.%d.%d",
                VK_VERSION_MAJOR(properties.apiVersion),
                VK_VERSION_MINOR(properties.apiVersion),
                VK_VERSION_PATCH(properties.apiVersion));

            for (u32 j = 0; j < memory.memoryHeapCount; j++) {
                f32 memory_size_gib = memory.memoryHeaps[j].size / (1024.0f * 1024.0f * 1024.0f);
                if (memory.memoryHeaps[j].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
                    KINFO("Device local memory: %.2f GiB", memory_size_gib);
                } else {
                    KINFO("Host visible memory: %.2f GiB", memory_size_gib);
                }
            }

            context->device.physical_device = physical_devices[i];
            context->device.graphics_queue_index = queue_info.graphics_family_index;
            context->device.present_queue_index = queue_info.present_family_index;
            context->device.transfer_queue_index = queue_info.transfer_family_index;

            context->device.properties = properties;
            context->device.features = features;
            context->device.memory = memory;

            context->device.supports_device_local_host_visible = supports_device_local_host_visible;
            break;

        }
    }

    if (!context->device.physical_device) {
        KERROR("No suitable devices found");
        return false;
    }

    KINFO("Physical device selected");
    return true;
}

b8 physical_device_meets_requirements(
    VkPhysicalDevice device,
    VkSurfaceKHR surface,
    const VkPhysicalDeviceProperties* properties,
    const VkPhysicalDeviceFeatures* features,
    const vulkan_physical_device_requirements* requirements,
    vulkan_physical_device_queue_family_info* out_queue_info,
    vulkan_swapchain_support_info* out_swapchain_support) {
    out_queue_info->graphics_family_index = -1;
    out_queue_info->present_family_index = -1;
    out_queue_info->compute_family_index = -1;
    out_queue_info->transfer_family_index = -1;

    if (requirements->discrete_gpu) {
        if (properties->deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            KINFO("Device is not a discrete GPU, and one is required.");
        }
    }

    u32 queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, 0);
    VkQueueFamilyProperties queue_familes[MAX_QUEUE_COUNT];
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_familes);

    KINFO("Graphics | Present | Compute | Transfer | Name");
    u8 min_transfer_score = 255;
    for (u32 i = 0; i < queue_family_count; i++) {
        u8 current_transfer_score = 0;

        if (queue_familes[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            out_queue_info->graphics_family_index = i;
            current_transfer_score++;
        }

        if (queue_familes[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
            out_queue_info->compute_family_index = i;
            current_transfer_score++;
        }

        if (queue_familes[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
            if (current_transfer_score <= min_transfer_score) {
                min_transfer_score = current_transfer_score;
                out_queue_info->transfer_family_index = i;
            }
        }

        VkBool32 supports_present = VK_FALSE;
        VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &supports_present));
        if (supports_present) {
            out_queue_info->present_family_index = i;
        }
    }
    // Print out some info about the device
    KINFO("%8s | %7s | %7s | %8s | %s",
          out_queue_info->graphics_family_index != -1 ? "Yes" : "No",
          out_queue_info->present_family_index != -1 ? "Yes" : "No",
          out_queue_info->compute_family_index != -1 ? "Yes" : "No",
          out_queue_info->transfer_family_index != -1 ? "Yes" : "No",
          properties->deviceName);

    if (
        (!requirements->graphics || out_queue_info->graphics_family_index != -1) &&
        (!requirements->present || out_queue_info->present_family_index != -1) &&
        (!requirements->compute || out_queue_info->compute_family_index != -1) &&
        (!requirements->transfer || out_queue_info->transfer_family_index != -1)) {
        KINFO("Device meets requirements");
        KTRACE("Graphics family index: %i", out_queue_info->graphics_family_index);
        KTRACE("Present family index: %i", out_queue_info->present_family_index);
        KTRACE("Compute family index: %i", out_queue_info->compute_family_index);
        KTRACE("Transfer family index: %i", out_queue_info->transfer_family_index);
    }

    vulkan_device_query_swapchain_support(device, surface, out_swapchain_support);
    if (out_swapchain_support->format_count < 1 || out_swapchain_support->present_mode_count < 1) {
        if (out_swapchain_support->formats) {
            kfree(out_swapchain_support->formats, sizeof(VkSurfaceFormatKHR) * out_swapchain_support->format_count, MEMORY_TAG_RENDERER);
        }
        if (out_swapchain_support->present_modes) {
            kfree(out_swapchain_support->present_modes, sizeof(VkSurfaceFormatKHR) * out_swapchain_support->present_mode_count, MEMORY_TAG_RENDERER);
        }
        KINFO("Required swapchain support not present. skippping device.");
        return false;
    }

    if (requirements->device_extension_names) {
        u32 available_extension_count = 0;
        VkExtensionProperties* available_extensions = 0;
        VK_CHECK(vkEnumerateDeviceExtensionProperties(
            device,
            NULL,
            &available_extension_count,
            NULL));
        if (available_extension_count != 0) {
            available_extensions = kallocate(sizeof(VkExtensionProperties) * available_extension_count, MEMORY_TAG_RENDERER);
            VK_CHECK(vkEnumerateDeviceExtensionProperties(
                device,
                NULL,
                &available_extension_count,
                available_extensions));
            u32 required_extension_count = darray_length(requirements->device_extension_names);
            for (u32 i = 0; i < required_extension_count; i++) {
                b8 found = false;
                for (u32 j = 0; j < available_extension_count; j++) {
                    if (string_equal(requirements->device_extension_names[i], available_extensions[j].extensionName)) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    KINFO("Required device extension %s not found. Skipping device.", requirements->device_extension_names[i]);
                    kfree(available_extensions, sizeof(VkExtensionProperties) * available_extension_count, MEMORY_TAG_RENDERER);
                    return false;
                }
                kfree(available_extensions, sizeof(VkExtensionProperties) * available_extension_count, MEMORY_TAG_RENDERER);
            }
        }
        if (requirements->sampler_anisotropy && !features->samplerAnisotropy) {
            KINFO("Required sampler anisotropy not supported. Skipping device.");
            return false;
        }
        return true;
    }
    return false;
}
