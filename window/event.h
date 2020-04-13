/* event.h  -  Window library events  -  Public Domain  -  2014 Mattias Jansson
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

#pragma once

#include <foundation/platform.h>
#include <foundation/types.h>

#include <window/types.h>

WINDOW_API void
window_event_post(window_event_id id, window_t* window);

WINDOW_API event_stream_t*
window_event_stream(void);

/*! Handle foundation events. Do not pass in events from any other
event namespace to this function.
\param event Foundation event */
WINDOW_API void
window_event_handle(event_t* event);

/*! Get window related to the event
\param event Window event
\return Window */
WINDOW_API const window_t*
window_event_window(const event_t* event);

#if FOUNDATION_PLATFORM_WINDOWS

WINDOW_API void
window_event_post_native(window_event_id id, window_t* window, void* hwnd, uintptr_t msg, uintptr_t wparam,
                         uintptr_t lparam, void* buffer, size_t size);

#elif FOUNDATION_PLATFORM_LINUX

WINDOW_API void
window_event_post_native(window_event_id id, window_t* window, void* xevent);

#endif
