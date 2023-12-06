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

static input_state* state_ptr;

// TODO: Add better error handling for initialization
void initialize_input(u64* memory_requirement, void* state) {
    *memory_requirement = sizeof(input_state);
    if (state == 0) {
        return;
    }
    kzero_memory(state, sizeof(input_state));
    state_ptr = state;
    KINFO("Input subsystem initialized.");
}

void input_shutdown(void* state) {
    state_ptr = NULL;
}

void input_update(f64 delta_time) {
    if (!state_ptr) {
        return;
    }
    kcopy_memory(&state_ptr->keyboard_previous, &state_ptr->keyboard_current, sizeof(keyboard_state));
    kcopy_memory(&state_ptr->mouse_previous, &state_ptr->mouse_current, sizeof(mouse_state));
}

b8 input_is_key_up(keys key) {
    if (!state_ptr) {
        return false;
    }

    return !state_ptr->keyboard_current.keys[key];
}

b8 input_is_key_down(keys key) {
    if (!state_ptr) {
        return false;
    }

    return state_ptr->keyboard_current.keys[key];
}

KAPI b8 input_was_key_up(keys key) {
    if (!state_ptr) {
        return true;
    }

    return !state_ptr->keyboard_previous.keys[key];
}

b8 input_was_key_down(keys key) {
    if (!state_ptr) {
        return false;
    }

    return state_ptr->keyboard_previous.keys[key];
}

void input_process_key(keys key, b8 pressed) {
    if (state_ptr && state_ptr->keyboard_current.keys[key] != pressed) {
        state_ptr->keyboard_current.keys[key] = pressed;

        event_context context;
        context.data.u16[0] = key;
        event_fire(pressed ? EVENT_CODE_KEY_PRESSED : EVENT_CODE_KEY_RELEASED, NULL, context);
    }
}

b8 input_is_button_down(buttons button) {
    if (!state_ptr) {
        return false;
    }

    return state_ptr->keyboard_current.keys[button];
}

b8 input_is_button_up(buttons button) {
    if (!state_ptr) {
        return false;
    }

    return !state_ptr->keyboard_current.keys[button];
}

b8 input_was_button_down(buttons button) {
    if (!state_ptr) {
        return false;
    }

    return state_ptr->keyboard_previous.keys[button];
}

b8 input_was_button_up(buttons button) {
    if (!state_ptr) {
        return false;
    }

    return !state_ptr->keyboard_previous.keys[button];
}

void input_get_mouse_position(i32* x, i32* y) {
    if (!state_ptr) {
        return;
    }
    *x = state_ptr->mouse_current.x;
    *y = state_ptr->mouse_current.y;
}

void input_get_previous_mouse_position(i32* x, i32* y) {
    if (!state_ptr) {
        return;
    }
    *x = state_ptr->mouse_previous.x;
    *y = state_ptr->mouse_previous.y;
}

void input_process_mouse_button(buttons button, b8 pressed) {
    if (state_ptr->mouse_current.buttons[button] != pressed) {
        state_ptr->mouse_current.buttons[button] = pressed;

        event_context context;
        context.data.u16[0] = button;
        event_fire(pressed ? EVENT_CODE_BUTTON_PRESSED : EVENT_CODE_BUTTON_RELEASED, NULL, context);
    }
}

void input_process_mouse_move(i16 x, i16 y) {
    if (state_ptr->mouse_current.x != x || state_ptr->mouse_current.y != y) {
        state_ptr->mouse_current.x = x;
        state_ptr->mouse_current.y = y;

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