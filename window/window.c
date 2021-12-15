/* window.c  -  Window library  -  Public Domain  -  2014 Mattias Jansson
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

#include <foundation/foundation.h>
#include <foundation/stacktrace.h>

static bool window_initialized = false;

#if FOUNDATION_PLATFORM_LINUX

static int
x11_error_handler(Display* display, XErrorEvent* event) {
	char errmsg[512];
	XGetErrorText(display, event->error_code, errmsg, sizeof(errmsg));
	log_warnf(HASH_WINDOW, WARNING_SYSTEM_CALL_FAIL, STRING_CONST("X error event occurred: %s"), errmsg);

	void* frame[64];
	size_t frame_count = stacktrace_capture(frame, sizeof(frame) / sizeof(frame[0]), 0);
	if (frame_count) {
		size_t capacity = 8 * 1024;
		char* buffer = memory_allocate(HASH_WINDOW, capacity, 0, MEMORY_PERSISTENT);
		string_t stacktrace = stacktrace_resolve(buffer, capacity, frame, frame_count, 0);
		log_infof(HASH_WINDOW, STRING_CONST("Stack trace:\n%.*s"), STRING_FORMAT(stacktrace));
		memory_deallocate(buffer);
	}
	return 0;
}

#endif

int
window_module_initialize(const window_config_t config) {
	FOUNDATION_UNUSED(config);
	if (window_initialized)
		return 0;

	if (window_event_initialize() < 0)
		return -1;

#if FOUNDATION_PLATFORM_MACOS || FOUNDATION_PLATFORM_IOS
	window_class_reference();
#endif

#if FOUNDATION_PLATFORM_LINUX
	XInitThreads();
	XSetErrorHandler(x11_error_handler);
#endif

#if FOUNDATION_PLATFORM_IOS || FOUNDATION_PLATFORM_LINUX
	window_native_initialize();
#endif

	window_initialized = true;

	return 0;
}

void
window_module_finalize(void) {
#if FOUNDATION_PLATFORM_IOS || FOUNDATION_PLATFORM_LINUX
	window_native_finalize();
#endif

	window_event_finalize();

	window_initialized = false;
}

bool
window_module_is_initialized(void) {
	return window_initialized;
}
