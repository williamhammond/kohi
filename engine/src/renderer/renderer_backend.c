#include "renderer_backend.h"

#include "vulkan/vulkan_backend.h"

b8 renderer_backend_create(renderer_backend_type type, struct platform_state* plat_state, renderer_backend* out_renderer_backend) {
    out_renderer_backend->frame_number = 0;

    if (type == RENDERER_BACKEND_TYPE_VULKAN) {
        out_renderer_backend->initialize = vulkan_initialize;
        out_renderer_backend->shutdown = vulkan_shutdown;
        out_renderer_backend->begin_frame = vulkan_begin_frame;
        out_renderer_backend->end_frame = vulkan_end_frame;
        out_renderer_backend->resized = vulkan_resized;

        return true;
    }
    return false;
}

void renderer_backend_destroy(renderer_backend* backend) {
    backend->initialize = NULL;
    backend->shutdown = NULL;
    backend->begin_frame = NULL;
    backend->end_frame = NULL;
    backend->resized = NULL;
}