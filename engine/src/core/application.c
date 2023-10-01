#include "application.h"
#include "core/kmemory.h"
#include "game_types.h"
#include "logger.h"
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

b8 applicaton_create(game* game_inst) {
    if (initialized) {
        KERROR("application_create called more than once");
        return FALSE;
    }

    app_state.game_inst = game_inst;

    initialize_logging();

    KINFO("PI: %f", 3.14159265359f);
    KERROR("PI: %f", 3.14159265359f);
    KFATAL("PI: %f", 3.14159265359f);
    KWARN("PI: %f", 3.14159265359f);
    KDEBUG("PI: %f", 3.14159265359f);
    KTRACE("PI: %f", 3.14159265359f);

    app_state.is_running = TRUE;
    app_state.is_suspended = FALSE;

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
        }
        platform_sleep(1);
    }
    
    // TODO: maybe explicitly set is_running to FALSE here?
    // app_state.is_running = FALSE;
    platform_shutdown(&app_state.platform);

    return TRUE;
}