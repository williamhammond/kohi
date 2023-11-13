#pragma once

#include "renderer/renderer_backend.h"

b8 vulkan_initialize(renderer_backend* backend, const char* application_name);
void vulkan_shutdown(renderer_backend* backend);

void vulkan_resized(renderer_backend* backend, u16 width, u16 height);

b8 vulkan_begin_frame(renderer_backend* backend, f32 delta_time);
b8 vulkan_end_frame(renderer_backend* backend, f32 delta_time);