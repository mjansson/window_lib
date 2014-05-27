/* types.h  -  Window library types  -  Public Domain  -  2014 Mattias Jansson / Rampant Pixels
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
#include <window/build.h>
#include <window/hashstrings.h>


// CONSTANTS
static const unsigned int WINDOW_ADAPTER_DEFAULT    = -1;


// PRIMITIVE TYPES


// OPAQUE COMPLEX TYPES
typedef struct _window window_t;


// COMPLEX TYPES

#if FOUNDATION_PLATFORM_MACOSX
#  ifdef __OBJC__

#include <foundation/apple.h>
#import <AppKit/NSView.h>

@interface WindowGLView : NSView
{
@public
}

@end

#  endif
#endif

#if FOUNDATION_PLATFORM_IOS
#  ifdef __OBJC__

#include <foundation/apple.h>

/*! This class wraps the CAEAGLLayer from CoreAnimation into a convenient UIView subclass.
    The view content is basically an EAGL surface you render your OpenGL scene into.
    Note that setting the view non-opaque will only work if the EAGL surface has an alpha channel. */
@interface WindowGLView : UIView
{
@public
	id       display_link;
	CGPoint  begin_touch;
	CGPoint  last_touch;
	tick_t   begin_touch_time;
	UIView*  keyboard_view;
}

@end

#  endif
#endif


// FUNCTION TYPES

typedef void (* window_draw_fn)( window_t* window );


// UTILITY FUNCTIONS

