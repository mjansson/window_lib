/* internal.h  -  Window library internals  -  Public Domain  -  2014 Mattias Jansson / Rampant Pixels
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

/*! \file internal.h
    Window internal types */

#include <foundation/platform.h>
#include <window/types.h>


// PRIMITIVE TYPES


// OPAQUE COMPLEX TYPES
struct _window
{
#if FOUNDATION_PLATFORM_WINDOWS
	unsigned int           adapter;
	HWND                   hwnd;
	HINSTANCE              instance;
	bool                   created;
	bool                   cursor_lock;
	uipoint_t              cursor_pos;
	unsigned int           wstyle;
#elif FOUNDATION_PLATFORM_MACOSX
	void*                  nswindow;
#elif FOUNDATION_PLATFORM_LINUX
	unsigned int           adapter;
	bool                   created;
	Display*               display;
	unsigned int           screen;
	XVisualInfo*           visual;
	Window                 drawable;
	Atom                   atom;
	XIM                    xim;
	XIC                    xic;
	bool                   focus;
	bool                   visible;
#elif FOUNDATION_PLATFORM_IOS
	void*                  uiwindow;
	unsigned int           tag;
#elif FOUNDATION_PLATFORM_ANDROID
	unsigned int           adapter;
	uipoint_t              size;
	void*                  native;
#endif
};


// COMPLEX TYPES


// UTILITY FUNCTIONS
WINDOW_EXTERN int          _window_event_initialize( void );
WINDOW_EXTERN void         _window_event_shutdown( void );


// GLOBAL DATA
#if FOUNDATION_PLATFORM_IOS
WINDOW_EXTERN bool         _window_app_started;
WINDOW_EXTERN bool         _window_app_paused;
#endif
