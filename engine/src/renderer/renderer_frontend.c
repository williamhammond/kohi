#include "renderer_frontend.h"

#include "core/kmemory.h"
#include "core/logger.h"

#include "math/kmath.h"
#include "renderer_backend.h"

typedef struct renderer_system_state {
    renderer_backend backend;
    mat4 projection;
    f32 near_clip;
    f32 far_clip;
} renderer_system_state;

static renderer_system_state* state_ptr;

b8 renderer_initialize(u64* memory_requirement, void* state, const char* application_name) {
    *memory_requirement = sizeof(renderer_system_state);
    if (state == 0) {
        return true;
    }

    state_ptr = state;
    // TODO: make this configurable
    renderer_backend_create(RENDERER_BACKEND_TYPE_VULKAN, &state_ptr->backend);

    state_ptr->near_clip = 0.1f;
    state_ptr->far_clip = 1000.0f;
    state_ptr->projection = mat4_perspective(deg_to_rad(45.0f), 1200 / 720.0f, state_ptr->near_clip, state_ptr->far_clip);

    if (!state_ptr->backend.initialize(&state_ptr->backend, application_name)) {
        KFATAL("Failed to initialize renderer backend");
        return false;
    }

    return true;
}

void renderer_shutdown() {
    if (state_ptr) {
        state_ptr->backend.shutdown(&state_ptr->backend);
    }
    state_ptr = NULL;
}

b8 renderer_begin_frame(f32 delta_time) {
    if (!state_ptr) {
        return false;
    }
    return state_ptr->backend.begin_frame(&state_ptr->backend, delta_time);
}

b8 renderer_end_frame(f32 delta_time) {
    if (!state_ptr) {
        return false;
    }
    b8 result = state_ptr->backend.end_frame(&state_ptr->backend, delta_time);
    state_ptr->backend.frame_number++;
    return true;
}

b8 renderer_draw_frame(render_packet* packet) {
    // TODO: Figure out why a render frame not beginning is not as serious of an issue as not ending correctly
    if (renderer_begin_frame(packet->delta_time)) {
        mat4 view = mat4_translation((vec3){0, 0, -3.0f});
        state_ptr->backend.update_global_state(state_ptr->projection, view, vec3_zero(), vec4_one(), 0);

        static f32 angle = 0.01f;
        angle += 0.01f;
        quat rotation = quat_from_axis_angle(vec3_forward(), angle, false);
        mat4 model = quat_to_rotation_matrix(rotation, vec3_zero());
        state_ptr->backend.update_object(&state_ptr->backend, model);

        b8 result = renderer_end_frame(packet->delta_time);
        // TODO: Should error handling really be done here?
        if (!result) {
            KFATAL("renderer_end_frame failed. Application is shutting down...");
            return false;
        }
    }
    return true;
}

void renderer_on_resized(u16 width, u16 height) {
    if (state_ptr) {
        state_ptr->backend.resized(&state_ptr->backend, width, height);
        state_ptr->projection = mat4_perspective(deg_to_rad(45.0f), width / (f32)(height), state_ptr->near_clip, state_ptr->far_clip);
    } else {
        KWARN("renderer backend does not exist to accept resize: %i %i", width, height);
    }
}
