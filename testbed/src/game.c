#include "game.h"

#include <core/input.h>
#include <core/kmemory.h>
#include <core/logger.h>
#include <core/event.h>
#include <math/kmath.h>

// Hack: Remove this, it should not be available outside the engine
#include <renderer/renderer_frontend.h>

// Function that prints a mat4 matrix.
void print_mat4(mat4 m) {
    KDEBUG("\n");
    KDEBUG("| %1.8f  %1.8f  %1.8f  %1.8f |", m.data[0], m.data[1], m.data[2], m.data[3]);
    KDEBUG("| %1.8f  %1.8f  %1.8f  %1.8f |", m.data[4], m.data[5], m.data[6], m.data[7]);
    KDEBUG("| %1.8f  %1.8f  %1.8f  %1.8f |", m.data[8], m.data[9], m.data[10], m.data[11]);
    KDEBUG("| %1.8f  %1.8f  %1.8f  %1.8f |", m.data[12], m.data[13], m.data[14], m.data[15]);
    KDEBUG("\n");
}

void recalculate_view_matrix(game_state* state) {
    if (!state->camera_view_dirty) {
        return;
    }
    mat4 rotation = mat4_euler_xyz(state->camera_euler.x, state->camera_euler.y, state->camera_euler.z);
    print_mat4(rotation);
    mat4 translation = mat4_translation(state->camera_position);

    state->view = mat4_mul(rotation, translation);
    state->view = mat4_inverse(state->view);
    state->camera_view_dirty = false;
}

void camera_pitch(game_state* state, f32 amount) {
    state->camera_euler.x += amount;

    // clamp to avoid gimbal lock
    f32 limit = deg_to_rad(89.0f);
    state->camera_euler.x = KCLAMP(state->camera_euler.x, -limit, limit);
    state->camera_view_dirty = true;
}

void camera_yaw(game_state* state, f32 amount) {
    state->camera_euler.y += amount;
    state->camera_view_dirty = true;
}

b8 game_initialize(game* game_inst) {
    KDEBUG("game_initialize() called");

    game_state* state = (game_state*)game_inst->state;
    state->camera_position = (vec3){0, 0, 30.0f};
    state->camera_euler = vec3_zero();

    state->view = mat4_translation(state->camera_position);
    state->view = mat4_inverse(state->view);
    state->camera_view_dirty = true;

    return true;
}

b8 game_update(game* game_inst, f32 delta_time) {
    static u64 alloc_count = 0;
    u64 prev_alloc_count = alloc_count;
    alloc_count = get_memory_alloc_count();
    b8 key_up = input_is_key_up('M');
    b8 key_down = input_was_button_down('M');
    if (input_is_key_up('M') && input_was_key_down('M')) {
        KDEBUG("Allocations: %llu (%llu this frame)", alloc_count, alloc_count - prev_alloc_count);
    }

    if (input_is_key_up('T') && input_was_key_down('T')) {
        KDEBUG("Swapping texture!");
        event_context context = {};
        event_fire(EVENT_CODE_DEBUG0, game_inst, context);
    }

    game_state* state = (game_state*)game_inst->state;

    // HACK: Temporary controls for camera
    if (input_is_key_down(KEY_UP)) {
        camera_pitch(state, 10000.0f * delta_time);
    }
    if (input_is_key_down(KEY_LEFT)) {
        camera_yaw(state, 10000.0f * delta_time);
    }
    if (input_is_key_down(KEY_RIGHT)) {
        camera_yaw(state, -10000.0f * delta_time);
    }
    if (input_is_key_down(KEY_DOWN)) {
        camera_pitch(state, -10000.0f * delta_time);
    }

    vec3 velocity = vec3_zero();
    f32 temp_move_speed = 100000.0f;

    if (input_is_key_down('W')) {
        // TODO: Calculate this in a way that doesn't make it a frame behind
        vec3 forward = mat4_forward(state->view);
        velocity = vec3_add(velocity, forward);
    }
    if (input_is_key_down('A')) {
        // TODO: Calculate this in a way that doesn't make it a frame behind
        vec3 left = mat4_left(state->view);
        velocity = vec3_add(velocity, left);
    }
    if (input_is_key_down('S')) {
        vec3 backward = mat4_backward(state->view);
        velocity = vec3_add(velocity, backward);
    }
    if (input_is_key_down('D')) {
        // TODO: Calculate this in a way that doesn't make it a frame behind
        vec3 right = mat4_right(state->view);
        velocity = vec3_add(velocity, right);
    }
    if (input_is_key_down(KEY_SPACE)) {
        velocity.y += 1.0f;
    }
    if (input_is_key_down('X')) {
        velocity.y -= 1.0f;
    }

    vec3 z = vec3_zero();
    if (!vec3_compare(z, velocity, 0.0002f)) {
        vec3_normalize(&velocity);
        state->camera_position.x += velocity.x * temp_move_speed * delta_time;
        state->camera_position.y += velocity.y * temp_move_speed * delta_time;
        state->camera_position.z += velocity.z * temp_move_speed * delta_time;
        state->camera_view_dirty = true;
    }

    if (input_is_key_down(KEY_ENTER)) {
        state->camera_position = (vec3){0, 0, 30.0f};
        state->camera_euler = vec3_zero();
        state->camera_view_dirty = true;
    }

    recalculate_view_matrix(state);
    renderer_set_view(state->view);
    return true;
}

b8 game_render(game* game_inst, f32 delta_time) {
    return true;
}

void game_on_resize(game* game_inst, u32 width, u32 height) {
}
