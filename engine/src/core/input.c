#include "core/input.h"

#include "core/event.h"
#include "core/kmemory.h"
#include "core/logger.h"

typedef struct keyboard_state {
    // TODO: Might need to + 1 this?
    b8 keys[KEYS_MAX_KEYS];
} keyboard_state;

typedef struct mouse_state {
    i16 x;
    i16 y;
    u8 buttons[BUTTON_MAX_BUTTONS];
} mouse_state;

typedef struct input_state {
    keyboard_state keyboard_current;
    keyboard_state keyboard_previous;

    mouse_state mouse_current;
    mouse_state mouse_previous;
} input_state;

static b8 initialized = false;
static input_state state = {};

// TODO: Add better error handling for initialization
void initialize_input() {
    kzero_memory(&state, sizeof(input_state));
    initialized = true;
    KINFO("input subsystem initialized");
}

void input_shutdown() {
    // TODO: Add shutdown routines when needed
    initialized = false;
}

void input_update(f64 delta_time) {
    if (initialized) {
        return;
    }
    kcopy_memory(&state.keyboard_previous, &state.keyboard_current, sizeof(keyboard_state));
    kcopy_memory(&state.mouse_previous, &state.mouse_current, sizeof(mouse_state));
}

b8 input_is_key_up(keys key) {
    if (!initialized) {
        return false;
    }

    return !state.keyboard_current.keys[key];
}

b8 input_is_key_down(keys key) {
    if (!initialized) {
        return false;
    }

    return state.keyboard_current.keys[key];
}

KAPI b8 input_was_key_up(keys key) {
    if (!initialized) {
        return true;
    }

    return !state.keyboard_previous.keys[key];
}

b8 input_was_key_down(keys key) {
    if (!initialized) {
        return false;
    }

    return state.keyboard_previous.keys[key];
}

void input_process_key(keys key, b8 pressed) {
    if (state.keyboard_current.keys[key] != pressed) {
        state.keyboard_current.keys[key] = pressed;

        event_context context;
        context.data.u16[0] = key;
        event_fire(pressed ? EVENT_CODE_KEY_PRESSED : EVENT_CODE_KEY_RELEASED, NULL, context);
    }
}

b8 input_is_button_down(buttons button) {
    if (!initialized) {
        return false;
    }

    return state.keyboard_current.keys[button];
}

b8 input_is_button_up(buttons button) {
    if (!initialized) {
        return false;
    }

    return !state.keyboard_current.keys[button];
}

b8 input_was_button_down(buttons button) {
    if (!initialized) {
        return false;
    }

    return state.keyboard_previous.keys[button];
}

b8 input_was_button_up(buttons button) {
    if (!initialized) {
        return false;
    }

    return !state.keyboard_previous.keys[button];
}

void input_get_mouse_position(i32* x, i32* y) {
    if (!initialized) {
        return;
    }
    *x = state.mouse_current.x;
    *y = state.mouse_current.y;
}

void input_get_previous_mouse_position(i32* x, i32* y) {
    if (!initialized) {
        return;
    }
    *x = state.mouse_previous.x;
    *y = state.mouse_previous.y;
}

void input_process_mouse_button(buttons button, b8 pressed) {
    if (state.mouse_current.buttons[button] != pressed) {
        state.mouse_current.buttons[button] = pressed;

        event_context context;
        context.data.u16[0] = button;
        event_fire(pressed ? EVENT_CODE_BUTTON_PRESSED : EVENT_CODE_BUTTON_RELEASED, NULL, context);
    }
}

void input_process_mouse_move(i16 x, i16 y) {
    if (state.mouse_current.x != x || state.mouse_current.y != y) {
        state.mouse_current.x = x;
        state.mouse_current.y = y;

        event_context context;
        context.data.i16[0] = x;
        context.data.i16[1] = y;
        event_fire(EVENT_CODE_MOUSE_MOVED, NULL, context);
    }
}

void input_process_mouse_wheel(i16 delta) {
    event_context context;
    context.data.u8[0] = delta;
    event_fire(EVENT_CODE_MOUSE_WHEEL, NULL, context);
}