/* window.c  -  Window library  -  Public Domain  -  2014 Mattias Jansson / Rampant Pixels
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

#include <window/window.h>
#include <window/internal.h>

#include <foundation/foundation.h>

static bool _window_initialized = false;

#if FOUNDATION_PLATFORM_LINUX

static int
_x11_error_handler(Display* display, XErrorEvent* event) {
	char errmsg[512];
	XGetErrorText(display, event->error_code, errmsg, sizeof(errmsg));
	log_warnf(HASH_WINDOW, WARNING_SYSTEM_CALL_FAIL, STRING_CONST("X error event occurred: %s"), errmsg);
	return 0;
}

#endif

int
window_module_initialize(const window_config_t config) {
	FOUNDATION_UNUSED(config);
	if (_window_initialized)
		return 0;

	if (_window_event_initialize() < 0)
		return -1;

	_window_initialized = true;

#if FOUNDATION_PLATFORM_MACOSX || FOUNDATION_PLATFORM_IOS
	_window_class_reference();
#endif

#if FOUNDATION_PLATFORM_IOS
	_window_native_initialize();
#endif

#if FOUNDATION_PLATFORM_LINUX
	XSetErrorHandler(_x11_error_handler);
#endif

	return 0;
}

void
window_module_finalize(void) {
#if FOUNDATION_PLATFORM_IOS
	_window_native_finalize();
#endif

	_window_event_finalize();

	_window_initialized = false;
}

bool
window_module_is_initialized(void) {
	return _window_initialized;
}
