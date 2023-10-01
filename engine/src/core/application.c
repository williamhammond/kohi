#include "application.h"
#include "game_types.h"

#include "core/input.h"
#include "core/kmemory.h"
#include "core/logger.h"
#include "core/event.h"

#include "platform/platform.h"

typedef struct application_state {
    game* game_inst;
    b8 is_running;
    b8 is_suspended;
    platform_state platform;
    i16 width;
    i16 height;
    f64 last_time;
} application_state;

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

    KINFO("PI: %f", 3.14159265359f);
    KERROR("PI: %f", 3.14159265359f);
    KFATAL("PI: %f", 3.14159265359f);
    KWARN("PI: %f", 3.14159265359f);
    KDEBUG("PI: %f", 3.14159265359f);
    KTRACE("PI: %f", 3.14159265359f);

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
    KINFO(get_memory_usage_str());
    while (app_state.is_running) {
        if(!platform_pump_messages(&app_state.platform)) {
            app_state.is_running = FALSE;
        }
        if (!app_state.is_suspended) {
            // TODO fix time delta
            if (!app_state.game_inst->update(app_state.game_inst, (f32)0)) {
                KFATAL("Game update failed. shutting down");
                app_state.is_running = FALSE;
                break;
            }

            if (!app_state.game_inst->render(app_state.game_inst, (f32)0)) {
                KFATAL("Game render failed. shutting down");
                app_state.is_running = FALSE;
                break;
            }

            // TODO: Handle input at the top of frame?
            // Input update/state copying should always be handled
            // after any input should be recorded; I.E. before this line.
            // As a safety, input is the last thing to be updated before
            // this frame ends.
            input_update(0);
        }
        platform_sleep(1);
    }

    event_unregister(EVENT_CODE_APPLICATION_QUIT, NULL, application_on_event);
    event_unregister(EVENT_CODE_KEY_PRESSED, NULL, application_on_key);
    event_unregister(EVENT_CODE_KEY_RELEASED, NULL, application_on_key);

    event_shutdown();
    input_shutdown();
    
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