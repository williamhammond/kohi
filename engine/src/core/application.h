#pragma once

#include "defines.h"

// Forward delcare game to avoid circular depdendency with game_types.h
struct game;

typedef struct application_config {
    i16 start_pos_x;
    i16 start_pos_y;

    i16 start_width;
    i16 start_height;

    char* name;
} application_config;

KAPI b8 applicaton_create(struct game* game_inst);

KAPI b8 applicaton_run();

void application_get_framebuffer_size(u32* width, u32* height);
