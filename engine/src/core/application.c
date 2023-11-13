#include "application.h"

#include "core/clock.h"
#include "core/event.h"
#include "core/input.h"
#include "core/kmemory.h"
#include "core/logger.h"

#include "game_types.h"
#include "memory/linear_allocator.h"
#include "platform/platform.h"
#include "renderer/renderer_frontend.h"

typedef struct application_state {
    game* game_inst;
    b8 is_running;
    b8 is_suspended;
    i16 width;
    i16 height;
    clock clock;
    f64 last_time;
    linear_allocator systems_allocator;

    u64 memory_system_memory_requirement;
    void* memory_system_state;

    u64 logging_system_memory_requirement;
    void* logging_system_state;

    u64 renderer_system_memory_requirement;
    void* renderer_system_state;

    u64 platform_system_memory_requirement;
    void* platform_system_state;
} application_state;

// TODO: Configure this
static b8 limit_frames = false;

static application_state* app_state;

b8 application_on_event(u16 code, void* sender, void* listener_inst, event_context);
b8 application_on_key(u16 code, void* sender, void* listener_inst, event_context);
b8 application_on_resized(u16 code, void* sender, void* listener_inst, event_context context);

b8 application_create(game* game_inst) {
    if (game_inst->application_state) {
        KERROR("application_create called more than once");
        return false;
    }

    game_inst->application_state = kallocate(sizeof(application_state), MEMORY_TAG_APPLICATION);
    app_state = game_inst->application_state;
    app_state->game_inst = game_inst;
    app_state->is_running = false;
    app_state->is_suspended = false;

    u64 systems_allocator_total_size = 64 * 1024 * 1024;  // 64 mb
    linear_allocator_create(systems_allocator_total_size, NULL, &app_state->systems_allocator);

    initialize_memory(&app_state->memory_system_memory_requirement, NULL);
    app_state->memory_system_state = linear_allocator_allocate(&app_state->systems_allocator, app_state->memory_system_memory_requirement);
    initialize_memory(&app_state->memory_system_memory_requirement, app_state->memory_system_state);

    initialize_logging(&app_state->logging_system_memory_requirement, NULL);
    app_state->logging_system_state = linear_allocator_allocate(&app_state->systems_allocator, app_state->logging_system_memory_requirement);
    if (!initialize_logging(&app_state->logging_system_memory_requirement, app_state->logging_system_state)) {
        KERROR("Failed to initialize logging system. Shutting down");
        return false;
    }

    initialize_input();

    if (!event_initialize()) {
        KFATAL("Failed to initialize event system");
        return false;
    }
    platform_startup(&app_state->platform_system_memory_requirement, 0, 0, 0, 0, 0, 0);
    app_state->platform_system_state = linear_allocator_allocate(&app_state->systems_allocator, app_state->platform_system_memory_requirement);
    if (!platform_startup(
            &app_state->platform_system_memory_requirement,
            app_state->platform_system_state,
            game_inst->app_config.name,
            game_inst->app_config.start_pos_x,
            game_inst->app_config.start_pos_y,
            game_inst->app_config.start_width,
            game_inst->app_config.start_height)) {
        return false;
    }

    renderer_initialize(&app_state->renderer_system_memory_requirement, NULL, NULL);
    app_state->renderer_system_state = linear_allocator_allocate(&app_state->systems_allocator, app_state->renderer_system_memory_requirement);
    if (!renderer_initialize(&app_state->renderer_system_memory_requirement, app_state->renderer_system_state, game_inst->app_config.name)) {
        KFATAL("Failed to initialize renderer. Shutting down...");
        return false;
    }

    if (!app_state->game_inst->initialize(game_inst)) {
        KFATAL("Failed to initialize game");
        return false;
    }

    event_register(EVENT_CODE_APPLICATION_QUIT, NULL, application_on_event);
    event_register(EVENT_CODE_KEY_PRESSED, NULL, application_on_key);
    event_register(EVENT_CODE_KEY_RELEASED, NULL, application_on_key);
    event_register(EVENT_CODE_RESIZE, NULL, application_on_resized);

    app_state->game_inst->on_resize(app_state->game_inst, app_state->width, app_state->height);

    return true;
}

b8 applicaton_run() {
    app_state->is_running = true;
    clock_start(&app_state->clock);
    clock_update(&app_state->clock);
    app_state->last_time = app_state->clock.elapsed;
    f64 running_time = 0.0f;
    u8 frame_count = 0;
    f64 target_frame_seconds = 1.0f / 60.0f;

    while (app_state->is_running) {
        if (!platform_pump_messages()) {
            app_state->is_running = false;
        }
        if (!app_state->is_suspended) {
            clock_update(&app_state->clock);
            f64 current_time = app_state->clock.elapsed;
            f64 delta_time = current_time - app_state->last_time;
            f64 frame_start_time = platform_get_absolute_time();

            if (!app_state->game_inst->update(app_state->game_inst, delta_time)) {
                KFATAL("Game update failed. shutting down");
                app_state->is_running = false;
                break;
            }

            if (!app_state->game_inst->render(app_state->game_inst, delta_time)) {
                KFATAL("Game render failed. shutting down");
                app_state->is_running = false;
                break;
            }

            // TODO: Handle this in the renderer
            render_packet packet;
            packet.delta_time = delta_time;
            renderer_draw_frame(&packet);

            f64 frame_end_time = platform_get_absolute_time();
            f64 frame_elapsed_time = frame_end_time - frame_start_time;
            running_time += frame_elapsed_time;
            f64 remaining_time = target_frame_seconds - frame_elapsed_time;
            if (remaining_time > 0) {
                u64 remaining_ms = (remaining_time * 1000.0f);
                if (remaining_ms > 0 && limit_frames) {
                    platform_sleep(remaining_ms - 1);
                }
            }

            frame_count++;

            // TODO: Handle input at the top of frame?
            // Input update/state copying should always be handled
            // after any input should be recorded; I.E. before this line.
            // As a safety, input is the last thing to be updated before
            // this frame ends.
            input_update(delta_time);

            // TODO: See if current time should be gotten here
            app_state->last_time = current_time;
        }
    }

    event_unregister(EVENT_CODE_APPLICATION_QUIT, NULL, application_on_event);
    event_unregister(EVENT_CODE_KEY_PRESSED, NULL, application_on_key);
    event_unregister(EVENT_CODE_KEY_RELEASED, NULL, application_on_key);

    event_shutdown();
    input_shutdown();

    renderer_shutdown();

    // TODO: maybe explicitly set is_running to FALSE here?
    // app_state->is_running = FALSE;
    platform_shutdown(&app_state->platform_system_state);

    shutdown_memory(app_state->memory_system_state);

    return true;
}

b8 application_on_event(u16 code, void* sender, void* listener_inst, event_context context) {
    switch (code) {
        case EVENT_CODE_APPLICATION_QUIT: {
            KINFO("EVENT_CODE_APPLICATION_QUIT received, shutting down");
            app_state->is_running = false;
            return true;
        }
    }
    return false;
}

void application_get_framebuffer_size(u32* width, u32* height) {
    *width = app_state->width;
    *height = app_state->height;
}

b8 application_on_key(u16 code, void* sender, void* listener_inst, event_context context) {
    if (code == EVENT_CODE_KEY_PRESSED) {
        u16 key_code = context.data.u16[0];
        if (key_code == KEY_ESCAPE) {
            event_context data = {};
            event_fire(EVENT_CODE_APPLICATION_QUIT, NULL, data);

            return true;
        } else if (key_code == KEY_A) {
            KDEBUG("Explicit - A key pressed!");
        } else {
            KDEBUG("'%c' key pressed in window", key_code);
        }
    } else if (code == EVENT_CODE_KEY_RELEASED) {
        u16 key_code = context.data.u16[0];

        if (key_code == KEY_B) {
            KDEBUG("Explicit - B key released!");
        } else {
            KDEBUG("'%c' key released in window", key_code);
        }
    }
    return false;
}

b8 application_on_resized(u16 code, void* sender, void* listener_inst, event_context context) {
    if (code == EVENT_CODE_RESIZE) {
        u16 width = context.data.u16[0];
        u16 height = context.data.u16[1];

        if (width != app_state->width || height != app_state->height) {
            app_state->width = width;
            app_state->height = height;

            KDEBUG("Window resize: %i, %i", width, height);

            if (width == 0 || height == 0) {
                KINFO("Window minimized, suspending application");
                app_state->is_suspended = true;
                return true;
            } else {
                if (app_state->is_suspended) {
                    KINFO("Window restored, resuming application");
                    app_state->is_suspended = false;
                }
                app_state->game_inst->on_resize(app_state->game_inst, width, height);
                renderer_on_resized(width, height);
            }
        }
    }

    return false;
}