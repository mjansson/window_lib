/* internal.h  -  Window library internals  -  Public Domain  -  2014 Mattias Jansson
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

#include <window/types.h>

#if FOUNDATION_PLATFORM_WINDOWS
#include <foundation/windows.h>
#endif
#if FOUNDATION_PLATFORM_LINUX
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/extensions/xf86vmode.h>
#endif

WINDOW_EXTERN int
window_event_initialize(void);

WINDOW_EXTERN void
window_event_finalize(void);

WINDOW_EXTERN tick_t window_event_token;

#if FOUNDATION_PLATFORM_MACOS || FOUNDATION_PLATFORM_IOS

WINDOW_EXTERN void
window_class_reference(void);

#endif

#if FOUNDATION_PLATFORM_LINUX || FOUNDATION_PLATFORM_APPLE

WINDOW_EXTERN void
window_native_initialize(void);

WINDOW_EXTERN void
window_native_finalize(void);

#endif

WINDOW_EXTERN bool window_app_started;
WINDOW_EXTERN bool window_app_paused;
