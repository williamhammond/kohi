#include "vulkan_object_shader.h"

#include "core/kmemory.h"
#include "core/logger.h"

#include "math/math_types.h"
#include "renderer/vulkan/vulkan_pipeline.h"
#include "renderer/vulkan/vulkan_shader_utils.h"

#define BUILTIN_SHADER_NAME_OBJECT "Builtin.ObjectShader"

b8 vulkan_object_shader_create(vulkan_context* context, vulkan_object_shader* out_shader) {
    char stage_type_strs[OBJECT_SHADER_STAGE_COUNT][5] = {"vert", "frag"};
    VkShaderStageFlagBits stage_types[OBJECT_SHADER_STAGE_COUNT] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
    for (u32 i = 0; i < OBJECT_SHADER_STAGE_COUNT; i++) {
        if (!create_shader_module(context, BUILTIN_SHADER_NAME_OBJECT, stage_type_strs[i], stage_types[i], i, out_shader->stages)) {
            KERROR("Unable to create %s shader module for '%s'.", stage_type_strs[i], BUILTIN_SHADER_NAME_OBJECT);
            return false;
        }
    }

    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = (f32)context->framebuffer_height;
    viewport.width = (f32)context->framebuffer_width;
    viewport.height = (f32)context->framebuffer_height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor;
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent.width = context->framebuffer_width;
    scissor.extent.height = context->framebuffer_height;

    u32 offset = 0;
    // TODO figure out why the compiler thinks using attribute_count is a VLA
    const i32 attribute_count = 1;
    VkVertexInputAttributeDescription attribute_descriptions[1];
    VkFormat formats[1] = {VK_FORMAT_R32G32B32_SFLOAT};
    u64 sizes[1] = {sizeof(vec3)};
    for (u32 i = 0; i < 1; i++) {
        attribute_descriptions[i].binding = 0;
        attribute_descriptions[i].location = i;
        attribute_descriptions[i].format = formats[i];
        attribute_descriptions[i].offset = offset;
    }

    VkPipelineShaderStageCreateInfo stage_create_infos[OBJECT_SHADER_STAGE_COUNT];
    kzero_memory(stage_create_infos, sizeof(stage_create_infos));
    for (int i = 0; i < OBJECT_SHADER_STAGE_COUNT; i++) {
        stage_create_infos[i].sType = out_shader->stages[i].shader_stage_create_info.sType;
        stage_create_infos[i] = out_shader->stages[i].shader_stage_create_info;
    }

    if (!vulkan_graphics_pipeline_create(
            context,
            &context->main_renderpass,
            attribute_count,
            attribute_descriptions,
            0,
            0,
            OBJECT_SHADER_STAGE_COUNT,
            stage_create_infos,
            viewport,
            scissor,
            false,
            &out_shader->pipeline)) {
        KERROR("Failed to load graphics pipeline for object shader");
        return false;
    }

    return true;
}

void vulkan_object_shader_destroy(vulkan_context* context, struct vulkan_object_shader* shader) {
    vulkan_pipeline_destroy(context, &shader->pipeline);
    for (u32 i = 0; i < OBJECT_SHADER_STAGE_COUNT; i++) {
        vkDestroyShaderModule(context->device.logical_device, shader->stages[i].handle, context->allocator);
        shader->stages[i].handle = NULL;
    }
}

void vulkan_object_shader_use(vulkan_context* context, struct vulkan_object_shader* shader) {
}
