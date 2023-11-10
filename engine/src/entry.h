#pragma once

#include "core/application.h"
#include "core/logger.h"

#include "game_types.h"

extern b8 create_game(game* out_game);

/**
 * The entry point for the application.
 */
int main(void) {
    game game_inst;
    if (!create_game(&game_inst)) {
        KFATAL("Failed to create game");
        return -1;
    }

    if (!game_inst.render || !game_inst.update || !game_inst.initialize || !game_inst.on_resize) {
        KFATAL("game instance has unddefined core function pointers");
        return -2;
    }
    b8 created = application_create(&game_inst);
    if (!created) {
        KFATAL("Failed to create application");
        return 1;
    }

    if (!applicaton_run()) {
        KFATAL("Application did not shutdown successfully");
        return 2;
    }

    return 0;
}