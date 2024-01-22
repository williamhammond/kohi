#pragma once

#include "defines.h"
#include "math/math_types.h"
#include "resources/resource_types.h"

typedef enum renderer_backend_type {
    RENDERER_BACKEND_TYPE_VULKAN
} renderer_backend_type;

// Some NVIDIA cards need uniforms to be exactly 256 bytes.
typedef struct global_uniform_object {
    mat4 projection;  // 64 bytes
    mat4 view;        // 64 bytes
    mat4 padding_0;   // 64 bytes
    mat4 padding_1;   // 64 bytes
} global_uniform_object;

typedef struct object_uniform_object {
    vec4 diffuse_color;  // 16 bytes
    vec4 padding_0;      // 16 bytes
    vec4 padding_1;      // 16 bytes
    vec4 padding_2;      // 16 bytes
} object_uniform_object;

typedef struct geometry_render_data {
    u32 object_id;
    mat4 model;
    texture* textures[16];
} geometry_render_data;

typedef struct renderer_backend {
    u64 frame_number;

    b8 (*initialize)(struct renderer_backend* backend, const char* application_name);
    void (*shutdown)(struct renderer_backend* backend);

    void (*resized)(struct renderer_backend* backend, u16 width, u16 height);

    b8 (*begin_frame)(struct renderer_backend* backend, f32 delta_time);
    void (*update_global_state)(mat4 projection, mat4 view, vec3 view_position, vec4 ambient_colour, i32 mode);
    b8 (*end_frame)(struct renderer_backend* backend, f32 delta_time);
    void (*update_object)(geometry_render_data data);
    void (*create_texture)(
        const char* name,
        b8 auto_release,
        i32 width,
        i32 height,
        i32 channel_count,
        const u8* pixels,
        b8 has_transparency,
        struct texture* out_texture);
    void (*destroy_texture)(struct texture* texture);
} renderer_backend;

typedef struct render_packet {
    f32 delta_time;
} render_packet;
