#pragma once

#include "math/math_types.h"
#include "renderer/renderer_types.inl"
#include "renderer/vulkan/vulkan_types.inl"

b8 vulkan_object_shader_create(vulkan_context* context, texture* default_diffuse, vulkan_object_shader* out_shader);

void vulkan_object_shader_destroy(vulkan_context* context, struct vulkan_object_shader* shader);

void vulkan_object_shader_use(vulkan_context* context, struct vulkan_object_shader* shader);

void vulkan_object_shader_update_global_state(vulkan_context* context, struct vulkan_object_shader* shader, f32 delta_time);

void vulkan_object_shader_update_object(vulkan_context* context, struct vulkan_object_shader* shader, geometry_render_data data);

b8 vulkan_object_shader_acquire_resources(vulkan_context* context, struct vulkan_object_shader* shader, u32* out_object_id);
void vulkan_object_shader_release_resources(vulkan_context* context, struct vulkan_object_shader* shader, u32 object_id);
