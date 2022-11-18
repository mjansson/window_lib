/* event.c  -  Window library events  -  Public Domain  -  2014 Mattias Jansson
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

#include <foundation/array.h>
#include <foundation/event.h>
#include <foundation/semaphore.h>
#include <foundation/log.h>

#if FOUNDATION_PLATFORM_WINDOWS
#include <foundation/windows.h>
#endif

tick_t window_event_token;

static event_stream_t* window_stream = 0;

bool window_app_started = false;
bool window_app_paused = true;

#if FOUNDATION_PLATFORM_LINUX
static semaphore_t windows_lock;
static window_t** windows;
#endif

int
window_event_initialize(void) {
	window_stream = event_stream_allocate(1024);
	window_event_token = 1;
#if FOUNDATION_PLATFORM_LINUX
	semaphore_initialize(&windows_lock, 1);
	windows = 0;
#endif
	return 0;
}

void
window_event_finalize(void) {
#if FOUNDATION_PLATFORM_LINUX
	array_deallocate(windows);
	semaphore_finalize(&windows_lock);
#endif
	event_stream_deallocate(window_stream);
	window_stream = nullptr;
}

void
window_event_post(window_event_id id, window_t* window) {
	if (window_stream)
		event_post(window_stream, (int)id, 0, 0, &window, sizeof(window_t*));
}

#if FOUNDATION_PLATFORM_WINDOWS

void
window_event_post_native(window_event_id id, window_t* window, void* hwnd, uintptr_t msg, uintptr_t wparam,
                         uintptr_t lparam, void* buffer, size_t size) {
	if (window_stream)
		event_post_varg(window_stream, (int)id, 0, 0, &window, sizeof(window_t*), &hwnd, sizeof(void*), &msg,
		                sizeof(uintptr_t), &wparam, sizeof(uintptr_t), &lparam, sizeof(uintptr_t),
		                size ? buffer : nullptr, size, nullptr, nullptr);
}

#elif FOUNDATION_PLATFORM_LINUX

void
window_event_post_native(window_event_id id, window_t* window, void* xevent) {
	if (window_stream)
		event_post_varg(window_stream, (int)id, 0, 0, &window, sizeof(window_t*), xevent, sizeof(XEvent), nullptr,
		                nullptr);
}

#endif

const window_t*
window_event_window(const event_t* event) {
	return *(const window_t* const*)&event->payload[0];
}

event_stream_t*
window_event_stream(void) {
	return window_stream;
}

void
window_event_handle(event_t* event) {
	if (event->id == FOUNDATIONEVENT_START) {
		window_app_started = true;
		window_app_paused = false;
	} else if (event->id == FOUNDATIONEVENT_PAUSE)
		window_app_paused = true;
	else if (event->id == FOUNDATIONEVENT_RESUME)
		window_app_paused = false;
}
