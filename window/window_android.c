/* window_android.c  -  Window library  -  Public Domain  -  2014 Mattias Jansson
 *
 * This library provides a cross-platform window library in C11 providing basic support data types
 * and functions to create and manage windows in a platform-independent fashion. The latest source
 * code is always available at
 *
 * https://github.com/mjansson/window_lib
 *
 * This library is put in the public domain; you can redistribute it and/or modify it without any
 * restrictions.
 *
 */

#include <window/window.h>
#include <window/internal.h>

#if FOUNDATION_PLATFORM_ANDROID

#include <foundation/foundation.h>
#include <foundation/android.h>

window_t*
window_allocate(void* native) {
	window_t* window = memory_allocate(0, sizeof(window_t), 0, MEMORY_PERSISTENT | MEMORY_ZERO_INITIALIZED);
	window_initialize(window, native);
	return window;
}

void
window_initialize(window_t* window, void* native) {
}

void
window_finalize(window_t* window) {
}

void*
window_native(window_t* window) {
	return 0;
}

void*
window_display(window_t* window) {
	return 0;
}

void
window_show_keyboard(window_t* window) {
}

void
window_hide_keyboard(window_t* window) {
}

void
window_deallocate(window_t* window) {
	window_finalize(window);
	memory_deallocate(window);
}

unsigned int
window_adapter(window_t* window) {
	return WINDOW_ADAPTER_DEFAULT;
}

void
window_maximize(window_t* window) {
}

void
window_minimize(window_t* window) {
}

void
window_restore(window_t* window) {
}

void
window_resize(window_t* window, unsigned int width, unsigned int height) {
}

void
window_move(window_t* window, int x, int y) {
}

bool
window_is_open(window_t* window) {
	return true;
}

bool
window_is_visible(window_t* window) {
	return true;
}

bool
window_is_maximized(window_t* window) {
	return false;
}

bool
window_is_minimized(window_t* window) {
	return false;
}

bool
window_has_focus(window_t* window) {
	return true;
}

void
window_show_cursor(window_t* window, bool show, bool lock) {
}

void
window_set_cursor_pos(window_t* window, int x, int y) {
}

bool
window_is_cursor_locked(window_t* window) {
	return false;
}

void
window_set_title(window_t* window, const char* title, size_t length) {
}

int
window_width(window_t* window) {
	return 0;
}

int
window_height(window_t* window) {
	return 0;
}

int
window_position_x(window_t* window) {
	return 0;
}

int
window_position_y(window_t* window) {
	return 0;
}

void
window_fit_to_screen(window_t* window) {
}

#endif
