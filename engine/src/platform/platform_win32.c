#include "platform/platform.h"

#if K_PLATFORM_WINDOWS

#include "containers/darray.h"

#include "core/event.h"
#include "core/input.h"
#include "core/logger.h"

#include <stdlib.h>
#include <windows.h>
#include <windowsx.h>

// TODO: Find a way to keep all this vulkan stuff out of the platform code
#include "renderer/vulkan/vulkan_types.inl"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

typedef struct platform_state {
    HINSTANCE h_instance;
    HWND window;
    VkSurfaceKHR surface;
} platform_state;

static platform_state* state_ptr;

static f64 clock_frequency;
static LARGE_INTEGER start_time;

LRESULT CALLBACK win32_process_message(HWND window, UINT message, WPARAM w_param, LPARAM l_param);

void clock_setup() {
    LARGE_INTEGER frequency;
    QueryPerformanceCounter(&frequency);
    clock_frequency = 1.0 / (f64)frequency.QuadPart;
    QueryPerformanceCounter(&start_time);
}

b8 platform_startup(
    u64* memory_requirement,
    void* state,
    const char* application_name,
    i32 x,
    i32 y,
    i32 width,
    i32 height) {
    *memory_requirement = sizeof(platform_state);
    if (state == 0) {
        return true;
    }
    state_ptr = state;
    state_ptr->h_instance = GetModuleHandleA(0);

    HICON icon = LoadIcon(state_ptr->h_instance, IDI_APPLICATION);
    WNDCLASSA window_class;
    memset(&window_class, 0, sizeof(window_class));
    window_class.style = CS_DBLCLKS;  // Get double clicks
    window_class.lpfnWndProc = win32_process_message;
    window_class.cbClsExtra = 0;
    window_class.cbWndExtra = 0;
    window_class.hInstance = state_ptr->h_instance;
    window_class.hIcon = icon;
    window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
    window_class.hbrBackground = NULL;
    window_class.lpszClassName = "kohi_window_class";

    if (!RegisterClassA(&window_class)) {
        MessageBoxA(NULL, "Failed to register window class", "Error", MB_OK | MB_ICONEXCLAMATION);
        KFATAL("Failed to register window class");
        return false;
    }

    u32 client_x = x;
    u32 client_y = y;
    u32 client_width = width;
    u32 client_height = height;

    u32 window_x = client_x;
    u32 window_y = client_y;
    u32 window_width = client_width;
    u32 window_height = client_height;

    u32 window_style = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;
    u32 window_ex_style = WS_EX_APPWINDOW;

    window_style |= WS_MAXIMIZEBOX;
    window_style |= WS_MINIMIZEBOX;
    window_style |= WS_THICKFRAME;

    RECT border_rect = {0, 0, 0, 0};
    AdjustWindowRectEx(&border_rect, window_style, false, window_ex_style);

    // In this case, the border rectangle is negative
    window_x += border_rect.left;
    window_y += border_rect.top;

    // Grow by the size of the OS border
    window_width += border_rect.right - border_rect.left;
    window_height += border_rect.bottom - border_rect.top;

    HWND handle = CreateWindowExA(
        window_ex_style,
        window_class.lpszClassName,
        application_name,
        window_style,
        window_x,
        window_y,
        window_width,
        window_height,
        NULL,
        NULL,
        state_ptr->h_instance,
        NULL);

    if (!handle) {
        MessageBoxA(NULL, "Window creation failed!", "Error", MB_OK | MB_ICONEXCLAMATION);
        KFATAL("Window creation failed!");
        return false;
    }
    state_ptr->window = handle;

    b32 should_activate = 1;  // TODO: if the window should not accept input this should be false
    i32 show_window_command_flags = should_activate ? SW_SHOW : SW_SHOWNOACTIVATE;
    ShowWindow(state_ptr->window, show_window_command_flags);

    clock_setup();

    return true;
}

void platform_shutdown(void* plat_state) {
    if (state_ptr && state_ptr->window) {
        DestroyWindow(state_ptr->window);
        state_ptr->window = NULL;
    }
}

b8 platform_pump_messages(platform_state* plat_state) {
    if (state_ptr) {
        MSG message;
        while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&message);
            DispatchMessageA(&message);
        }
    }
    return true;
}

// TODO: replace all of these with custom allocators
void* platform_allocate(u64 size, b8 aligned) {
    return malloc(size);
}

void platform_free(void* block, b8 aligned) {
    free(block);
}

void* platform_zero_memory(void* block, u64 size) {
    return memset(block, 0, size);
}

void* platform_copy_memory(void* dest, const void* source, u64 size) {
    return memcpy(dest, source, size);
}

void* platform_set_memory(void* dest, i32 value, u64 size) {
    return memset(dest, value, size);
}

void platform_console_write(const char* message, u8 color) {
    HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    // FATAL, ERROR, WARN, INFO, DEBUG, TRACE
    static u8 levels[6] = {64, 4, 6, 2, 1, 8};
    SetConsoleTextAttribute(console_handle, levels[color]);

    OutputDebugStringA(message);
    u64 length = strlen(message);
    LPDWORD number_written = 0;
    WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), message, (DWORD)length, number_written, NULL);
}

void platform_console_write_error(const char* message, u8 color) {
    HANDLE console_handle = GetStdHandle(STD_ERROR_HANDLE);
    // FATAL, ERROR, WARN, INFO, DEBUG, TRACE
    static u8 levels[6] = {64, 4, 6, 2, 1, 8};
    SetConsoleTextAttribute(console_handle, levels[color]);

    OutputDebugStringA(message);
    u64 length = strlen(message);
    LPDWORD number_written = 0;
    WriteConsoleA(GetStdHandle(STD_ERROR_HANDLE), message, (DWORD)length, number_written, NULL);
}

f64 platform_get_absolute_time() {
    if (!clock_frequency) {
        clock_setup();
    }

    LARGE_INTEGER now_time;
    QueryPerformanceCounter(&now_time);
    return (f64)now_time.QuadPart * clock_frequency;
}

void platform_sleep(u64 ms) {
    Sleep(ms);
}

// declared in vulkan_platform.h
void platform_get_required_extension_names(const char*** names_darray) {
    darray_push(*names_darray, &"VK_KHR_win32_surface");
}

b8 platform_create_vulkan_surface(vulkan_context* context) {
    if (!state_ptr) {
        return false;
    }

    VkWin32SurfaceCreateInfoKHR create_info = {VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR};
    create_info.hinstance = state_ptr->h_instance;
    create_info.hwnd = state_ptr->window;

    VkResult result = vkCreateWin32SurfaceKHR(context->instance, &create_info, context->allocator, &state_ptr->surface);
    if (result != VK_SUCCESS) {
        KFATAL("Vulkan surface creation failed.");
        return false;
    }
    context->surface = state_ptr->surface;
    return true;
}

LRESULT CALLBACK win32_process_message(HWND window, UINT message, WPARAM w_param, LPARAM l_param) {
    switch (message) {
        case WM_ERASEBKGND:
            // Notify the os that erasing will be handled by the application to prevent flickering
            return 1;
        case WM_CLOSE:
            event_context data = {};
            event_fire(EVENT_CODE_APPLICATION_QUIT, 0, data);
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_SIZE: {
            RECT r;
            GetClientRect(window, &r);
            u32 width = r.right - r.left;
            u32 height = r.bottom - r.top;
            event_context context;
            context.data.u16[0] = (u16)width;
            context.data.u16[1] = (u16)height;
            event_fire(EVENT_CODE_RESIZE, 0, context);
        } break;
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP: {
            b8 pressed = (message == WM_KEYDOWN) || (message == WM_SYSKEYDOWN);
            keys key = (u16)w_param;

            // learn.microsoft.com/en-us/windows/win32/learnwin32/keyboard-input#keyboard-state
            if (w_param == VK_MENU) {
                if (GetKeyState(VK_RMENU) & 0x8000) {
                    key = KEY_RALT;
                } else if (GetKeyState(VK_LMENU) & 0x8000) {
                    key = KEY_LALT;
                }
            } else if (w_param == VK_SHIFT) {
                if (GetKeyState(VK_RSHIFT) & 0x8000) {
                    key = KEY_RSHIFT;
                } else if (GetKeyState(VK_LSHIFT) & 0x8000) {
                    key = KEY_LSHIFT;
                }
            } else if (w_param == VK_CONTROL) {
                if (GetKeyState(VK_RCONTROL) & 0x8000) {
                    key = KEY_RCONTROL;
                } else if (GetKeyState(VK_LCONTROL) & 0x8000) {
                    key = KEY_LCONTROL;
                }
            }

            input_process_key(key, pressed);
        } break;
        case WM_MOUSEMOVE: {
            i32 x_position = GET_X_LPARAM(l_param);
            i32 y_position = GET_Y_LPARAM(l_param);
            input_process_mouse_move(x_position, y_position);
        } break;
        case WM_MOUSEWHEEL: {
            i32 z_delta = GET_WHEEL_DELTA_WPARAM(w_param);
            if (z_delta != 0) {
                // Some mouse wheels take into account sensitivity on the wheel.
                // We only care about up or down, so we normalize to
                // a platform indepdendent -1 or 1
                z_delta = (z_delta < 0) ? -1 : 1;
                input_process_mouse_wheel(z_delta);
            }
        } break;
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
            input_process_mouse_button(BUTTON_LEFT, (b8)(message == WM_LBUTTONDOWN));
            break;

        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
            input_process_mouse_button(BUTTON_RIGHT, message == WM_RBUTTONDOWN);
            break;

        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
            input_process_mouse_button(BUTTON_MIDDLE, message == WM_MBUTTONDOWN);
            break;
    }

    return DefWindowProcA(window, message, w_param, l_param);
}

#endif  // K_PLATFORM_WINDOWS