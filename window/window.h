/* window  -  Window library  -  Public Domain  -  2014 Mattias Jansson / Rampant Pixels
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

/*! \file window
    Wrapper for window library headers and main entry/exit points */

#include <foundation/platform.h>

#include <window/build.h>
#include <window/types.h>


/*! Main entry point. Call this to bootstrap the window library
    and initialize all functionality.
    \return                0 if initialization successful, <0 if error */
WINDOW_API int             window_initialize( void );

/*! Main exit point. Call this to cleanup the window library
    and terminate all functionality. */
WINDOW_API void            window_shutdown( void );

/*! Query if window library is initialized properly
    \return                true if initialized, false if not */
WINDOW_API bool            window_is_initialized( void );
