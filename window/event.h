/* event.h  -  Window library events  -  Public Domain  -  2014 Mattias Jansson / Rampant Pixels
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
#include <foundation/types.h>

#include <window/types.h>


WINDOW_API void              window_event_post( window_event_id id, window_t* window );
WINDOW_API void              window_event_process( void );
WINDOW_API event_stream_t*   window_event_stream( void );
WINDOW_API void              window_event_handle_foundation( event_t* event );

