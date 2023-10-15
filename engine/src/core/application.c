#include "application.h"
#include "game_types.h"

#include "core/clock.h"
#include "core/input.h"
#include "core/kmemory.h"
#include "core/logger.h"
#include "core/event.h"

#include "platform/platform.h"

#include "renderer/renderer_frontend.h"

typedef struct application_state {
    game* game_inst;
    b8 is_running;
    b8 is_suspended;
    platform_state platform;
    i16 width;
    i16 height;
    clock clock;
    f64 last_time;
} application_state;

// TODO: Configure this
static b8 limit_frames = FALSE;

static b8 initialized = FALSE;
static application_state app_state;

b8 application_on_event(u16 code, void* sender, void* listener_inst, event_context);
b8 application_on_key(u16 code, void* sender, void* listener_inst, event_context);

b8 applicaton_create(game* game_inst) {
    if (initialized) {
        KERROR("application_create called more than once");
        return FALSE;
    }

    app_state.game_inst = game_inst;

    initialize_logging();
    initialize_input();

    app_state.is_running = TRUE;
    app_state.is_suspended = FALSE;

    if (!event_initialize()) {
        KFATAL("Failed to initialize event system");
        return FALSE;
    }
    if (!platform_startup(&app_state.platform,
        game_inst->app_config.name,
        game_inst->app_config.start_pos_x, 
        game_inst->app_config.start_pos_y, 
        game_inst->app_config.start_width, 
        game_inst->app_config.start_height)) {

        return FALSE;
    }

    if (!renderer_initialize(game_inst->app_config.name, &app_state.platform)) {
        KFATAL("Failed to initialize renderer. Shutting down...");
        return FALSE;
    }

    if (!app_state.game_inst->initialize(game_inst)) {
        KFATAL("Failed to initialize game");
        return FALSE;
    }

    event_register(EVENT_CODE_APPLICATION_QUIT, NULL, application_on_event);
    event_register(EVENT_CODE_KEY_PRESSED, NULL, application_on_key);
    event_register(EVENT_CODE_KEY_RELEASED, NULL, application_on_key);

    app_state.game_inst->on_resize(app_state.game_inst, app_state.width, app_state.height);

    initialized = TRUE;

    return TRUE;
}

b8 applicaton_run() {
    clock_start(&app_state.clock);
    clock_update(&app_state.clock);
    app_state.last_time = app_state.clock.elapsed;
    f64 running_time = 0.0f;
    u8 frame_count = 0;
    f64 target_frame_seconds = 1.0f / 60.0f;

    while (app_state.is_running) {
        if(!platform_pump_messages(&app_state.platform)) {
            app_state.is_running = FALSE;
        }
        if (!app_state.is_suspended) {
            clock_update(&app_state.clock);
            f64 current_time = app_state.clock.elapsed;
            f64 delta_time = current_time - app_state.last_time;
            f64 frame_start_time = platform_get_absolute_time();

            if (!app_state.game_inst->update(app_state.game_inst, delta_time)) {
                KFATAL("Game update failed. shutting down");
                app_state.is_running = FALSE;
                break;
            }

            if (!app_state.game_inst->render(app_state.game_inst, delta_time)) {
                KFATAL("Game render failed. shutting down");
                app_state.is_running = FALSE;
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
            app_state.last_time = current_time;
        }
    }

    event_unregister(EVENT_CODE_APPLICATION_QUIT, NULL, application_on_event);
    event_unregister(EVENT_CODE_KEY_PRESSED, NULL, application_on_key);
    event_unregister(EVENT_CODE_KEY_RELEASED, NULL, application_on_key);

    event_shutdown();
    input_shutdown();

    renderer_shutdown();
    
    // TODO: maybe explicitly set is_running to FALSE here?
    // app_state.is_running = FALSE;
    platform_shutdown(&app_state.platform);

    return TRUE;
}

b8 application_on_event(u16 code, void* sender, void* listener_inst, event_context context) {
    switch(code) {
        case EVENT_CODE_APPLICATION_QUIT: {
            KINFO("EVENT_CODE_APPLICATION_QUIT received, shutting down");
            app_state.is_running = FALSE;
            return TRUE;
        }
    }
    return FALSE;
}

void application_get_framebuffer_size(u32* width, u32* height) {
    *width = app_state.width;
    *height = app_state.height;
}

b8 application_on_key(u16 code, void* sender, void* listener_inst, event_context context) {
    if (code == EVENT_CODE_KEY_PRESSED) {
        u16 key_code = context.data.u16[0];
        if (key_code == KEY_ESCAPE) {
            event_context data = {};
            event_fire(EVENT_CODE_APPLICATION_QUIT, NULL, data);

            return TRUE;
        } else if (key_code == KEY_A) {
            KDEBUG("Explicit - A key pressed!");
        } else {
            KDEBUG("'%c' key pressed in window", key_code);
        }
    } else if (code == EVENT_CODE_KEY_RELEASED){
        u16 key_code = context.data.u16[0];

        if (key_code == KEY_B) {
            KDEBUG("Explicit - B key released!");
        } else {
            KDEBUG("'%c' key released in window", key_code);
        }
    }
    return FALSE;
}