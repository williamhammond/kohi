#include "core/event.h"

#include "containers/darray.h"

#include "core/kmemory.h"

typedef struct registered_event {
    void* listener;
    PFN_on_event callback;
} registered_event;

typedef struct event_code_entry {
    registered_event* events;
} event_code_entry;

#define MAX_MESSAGE_CODES 16384
typedef struct event_system_state {
    event_code_entry registered[MAX_MESSAGE_CODES];
} event_system_state;

static event_system_state* state_ptr;

void event_initialize(u64* memory_requirement, void* state) {
    *memory_requirement = sizeof(event_system_state);
    if (state == 0) {
        return;
    }
    kzero_memory(state, sizeof(state));
    state_ptr = state;
}

void event_shutdown(void* state) {
    if (state_ptr) {
        // Free the events arrays. And objects pointed to should be destroyed on their own.
        for (u16 i = 0; i < MAX_MESSAGE_CODES; ++i) {
            if (state_ptr->registered[i].events != 0) {
                darray_destroy(state_ptr->registered[i].events);
                state_ptr->registered[i].events = NULL;
            }
        }
    }
    state_ptr = NULL;
}

b8 event_register(u16 code, void* listener, PFN_on_event on_event) {
    if (!state_ptr) {
        return false;
    }

    if (state_ptr->registered[code].events == 0) {
        state_ptr->registered[code].events = darray_create(registered_event);
    }

    u64 registered_count = darray_length(state_ptr->registered[code].events);
    for (u64 i = 0; i < registered_count; i++) {
        if (state_ptr->registered[code].events[i].listener == listener) {
            // TODO: Warning
            return false;
        }
    }

    registered_event event;
    event.listener = listener;
    event.callback = on_event;
    darray_push(state_ptr->registered[code].events, event);

    return true;
}

b8 event_unregister(u16 code, void* listener, PFN_on_event on_event) {
    if (!state_ptr) {
        return false;
    }

    if (state_ptr->registered[code].events == 0) {
        // TODO: Warning
        return false;
    }

    u64 registered_count = darray_length(state_ptr->registered[code].events);
    for (u64 i = 0; i < registered_count; i++) {
        registered_event event = state_ptr->registered[code].events[i];
        if (event.listener == listener && event.callback == on_event) {
            registered_event popped_event;
            darray_pop_at(state_ptr->registered[code].events, i, &popped_event);
            return true;
        }
    }
    return false;
}

b8 event_fire(u16 code, void* sender, event_context context) {
    if (!state_ptr) {
        return false;
    }

    if (state_ptr->registered[code].events == 0) {
        return false;
    }

    u64 registered_count = darray_length(state_ptr->registered[code].events);
    for (u64 i = 0; i < registered_count; i++) {
        registered_event event = state_ptr->registered[code].events[i];
        if (event.callback(code, sender, event.listener, context)) {
            return true;
        }
    }

    return false;
}