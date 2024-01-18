#pragma once

#include "defines.h"
#include "math/math_types.h"

typedef enum renderer_backend_type {
    RENDERER_BACKEND_TYPE_VULKAN
} renderer_backend_type;

// Some NVIDIA cards need uniforms to be exactly 256 bytes.
typedef struct global_uniform_object {
    mat4 projection;    // 64 bytes
    mat4 view;          // 64 bytes
    char padding[128];  // 128 bytes
} global_uniform_object;

typedef struct renderer_backend {
    u64 frame_number;

    b8 (*initialize)(struct renderer_backend* backend, const char* application_name);
    void (*shutdown)(struct renderer_backend* backend);

    void (*resized)(struct renderer_backend* backend, u16 width, u16 height);

    b8 (*begin_frame)(struct renderer_backend* backend, f32 delta_time);
    void (*update_global_state)(mat4 projection, mat4 view, vec3 view_position, vec4 ambient_colour, i32 mode);
    b8 (*end_frame)(struct renderer_backend* backend, f32 delta_time);
    void (*update_object)(struct renderer_backend* backend, mat4 model);
} renderer_backend;

typedef struct render_packet {
    f32 delta_time;
} render_packet;