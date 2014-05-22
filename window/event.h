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

/*! \file types.h
    Window types */

#include <foundation/platform.h>
#include <foundation/types.h>

#include <window/types.h>


/*! Process window events. You should call this once every loop iteration in your main loop */
WINDOW_API void                   window_event_process( void );

/*! Get window event stream
    \return                       System event stream */
WINDOW_API event_stream_t*        window_event_stream( void );

/*! Send a foundation system event to the window library to handle. You should ideally let the window
    system listen to all foundation events for full functionality.
    \param event                  Event */
WINDOW_API void                   window_event_handle_foundation( event_t* event );

