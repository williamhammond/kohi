#pragma once

#include "renderer/renderer_backend.h"

b8 vulkan_initialize(renderer_backend* backend, const char* application_name);
void vulkan_shutdown(renderer_backend* backend);

void vulkan_resized(renderer_backend* backend, u16 width, u16 height);

b8 vulkan_begin_frame(renderer_backend* backend, f32 delta_time);
void vulkan_renderer_update_global_state(mat4 projection, mat4 view, vec3 view_position, vec4 ambient_colour, i32 mode);
b8 vulkan_end_frame(renderer_backend* backend, f32 delta_time);

void vulkan_update_object(struct renderer_backend* backend, mat4 model);