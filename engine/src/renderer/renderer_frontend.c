#include "renderer_frontend.h"

#include "core/kmemory.h"
#include "core/logger.h"

#include "renderer_backend.h"

static renderer_backend* backend = NULL;

b8 renderer_initialize(const char* application_name, struct platform_state* plat_state) {
    backend = kallocate(sizeof(renderer_backend), MEMORY_TAG_RENDERER);

    // TODO: make this configurable
    renderer_backend_create(RENDERER_BACKEND_TYPE_VULKAN, plat_state, backend);

    if (!backend->initialize(backend, application_name, plat_state)) {
        KFATAL("Failed to initialize renderer backend");
        return FALSE;
    }

    return TRUE;
}

void renderer_shutdown() {
    backend->shutdown(backend);
    renderer_backend_destroy(backend);
    kfree(backend, sizeof(renderer_backend), MEMORY_TAG_RENDERER);
}

b8 renderer_begin_frame(f32 delta_time) {
    return backend->begin_frame(backend, delta_time);
}

b8 renderer_end_frame(f32 delta_time) {
    b8 result = backend->end_frame(backend, delta_time);
    backend->frame_number++;
    return result;
}

b8 renderer_draw_frame(render_packet* packet) {
    // TODO: Figure out why a render frame not beginning is not as serious of an issue as not ending correctly
    if (backend->begin_frame(backend, packet->delta_time)) {
        b8 result = backend->end_frame(backend, packet->delta_time);
        // TODO: Should error handling really be done here?
        if (!result) {
            KFATAL("renderer_end_frame failed. Application is shutting down...");
            return FALSE;
        }
    }
    return TRUE;
}