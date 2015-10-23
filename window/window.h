/* window.h  -  Window library  -  Public Domain  -  2014 Mattias Jansson / Rampant Pixels
 *
 * This library provides a cross-platform window library in C11 providing basic support data types and
 * functions to create and manage windows in a platform-independent fashion. The latest source code is
 * always available at
 *
 * https://github.com/rampantpixels/window_lib
 *
 * This library is put in the public domain; you can redistribute it and/or modify it without any restrictions.
 *
 */

#pragma once

#include <foundation/platform.h>

#include <window/build.h>
#include <window/types.h>
#include <window/event.h>

WINDOW_API int
window_module_initialize(const window_config_t config);

WINDOW_API void
window_module_finalize(void);

WINDOW_API bool
window_module_is_initialized(void);

WINDOW_API version_t
window_module_version(void);

WINDOW_API void
window_finalize(window_t* window);

WINDOW_API void
window_deallocate(window_t* window);

WINDOW_API unsigned int
window_adapter(window_t* window);

WINDOW_API void
window_maximize(window_t* window);

WINDOW_API void
window_minimize(window_t* window);

WINDOW_API void
window_restore(window_t* window);

WINDOW_API void
window_resize(window_t* window, unsigned int width, unsigned int height);

WINDOW_API void
window_move(window_t* window, int x, int y);

WINDOW_API bool
window_is_open(window_t* window);

WINDOW_API bool
window_is_visible(window_t* window);

WINDOW_API bool
window_is_maximized(window_t* window);

WINDOW_API bool
window_is_minimized(window_t* window);

WINDOW_API bool
window_has_focus(window_t* window);

WINDOW_API void
window_show_cursor(window_t* window, bool show, bool lock);

WINDOW_API void
window_set_cursor_pos(window_t* window, int x, int y);

WINDOW_API bool
window_is_cursor_locked(window_t* window);

WINDOW_API void
window_set_title(window_t* window, const char* title);

WINDOW_API int
window_width(window_t* window);

WINDOW_API int
window_height(window_t* window);

WINDOW_API int
window_position_x(window_t* window);

WINDOW_API int
window_position_y(window_t* window);

WINDOW_API void
window_fit_to_screen(window_t* window);

#if FOUNDATION_PLATFORM_WINDOWS

WINDOW_API window_t*
window_create(unsigned int adapter, const char* title, size_t length, unsigned int width,
              unsigned int height, bool show);

WINDOW_API window_t*
window_allocate(void* hwnd);

WINDOW_API void
window_initialize(window_t* window, void* hwnd);

WINDOW_API void*
window_hwnd(window_t* window);

WINDOW_API void*
window_hinstance(window_t* window);

WINDOW_API void*
window_hdc(window_t* window);

WINDOW_API void
window_release_hdc(void* hwnd, void* hdc);

WINDOW_API unsigned int
window_screen_width(unsigned int adapter);

WINDOW_API unsigned int
window_screen_height(unsigned int adapter);

#elif FOUNDATION_PLATFORM_MACOSX

WINDOW_API window_t*
window_allocate(void* nswindow);

WINDOW_API void
window_initialize(window_t* window, void* nswindow);

WINDOW_API void*
window_content_view(window_t* window);   //NSView*

#elif FOUNDATION_PLATFORM_LINUX

WINDOW_API window_t*
window_create(unsigned int adapter, const char* title, unsigned int width, unsigned int height,
              bool show);

WINDOW_API void*
window_display(window_t* window);

WINDOW_API int
window_screen(window_t* window);

WINDOW_API int
window_drawable(window_t* window);

WINDOW_API void*
window_visual(window_t* window);

#elif FOUNDATION_PLATFORM_IOS

WINDOW_API window_t*
window_allocate(void* uiwindow);

WINDOW_API void
window_initialize(window_t* window, void* uiwindow);

WINDOW_API void*
window_view(window_t* window, unsigned int tag);   //UIView*

WINDOW_API void*
window_layer(window_t* window, void* view);   //CAEAGLLayer*

WINDOW_API int
window_view_width(window_t* window, void* view);

WINDOW_API int
window_view_height(window_t* window, void* view);

WINDOW_API void
window_add_displaylink(window_t* window, window_draw_fn drawfn);

WINDOW_API void
window_show_keyboard(window_t* window);

WINDOW_API void
window_hide_keyboard(window_t* window);

#elif FOUNDATION_PLATFORM_ANDROID

WINDOW_API window_t*
window_allocate(void* native);

WINDOW_API void
window_initialize(window_t* window, void* native);

WINDOW_API void*
window_native(window_t* window);

WINDOW_API void*
window_display(window_t* window);

WINDOW_API void
window_show_keyboard(window_t* window);

WINDOW_API void
window_hide_keyboard(window_t* window);

#endif
