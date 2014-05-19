/* window_ios.m  -  Window library  -  Public Domain  -  2014 Mattias Jansson / Rampant Pixels
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

#include <foundation/foundation.h>

#include <window/window.h>
#include <window/internal.h>

#if FOUNDATION_PLATFORM_IOS

#include <foundation/apple.h>


window_t* window_allocate_from_uiwindow( void* uiwindow )
{
	window_t* window = memory_allocate_zero( sizeof( window_t ), 0, MEMORY_PERSISTENT );
	window->uiwindow = uiwindow;
	return window;
}


void* window_view( window_t* window, unsigned int tag )
{
	return 0;
}


void* window_layer( window_t* window, void* view )
{
	return 0;
}


void window_wait_for_displaylink( window_t* window )
{
}


void window_show_keyboard( window_t* window )
{
}

void window_hide_keyboard( window_t* window )
{
}


void window_deallocate( window_t* window )
{
	memory_deallocate( window );
}


unsigned int window_adapter( window_t* window )
{
	return WINDOW_ADAPTER_DEFAULT;
}


void window_maximize( window_t* window )
{
}


void window_minimize( window_t* window )
{
}


void window_restore( window_t* window )
{
}


void window_resize( window_t* window, unsigned int width, unsigned int height )
{
}


void window_move( window_t* window, int x, int y )
{
}


bool window_is_open( window_t* window )
{
	return true;
}


bool window_is_visible( window_t* window )
{
	return true;
}


bool window_is_maximized( window_t* window )
{
	return false;
}


bool window_is_minimized( window_t* window )
{
	return false;
}


bool window_has_focus( window_t* window )
{
	return true;
}


void window_show_cursor( window_t* window, bool show, bool lock )
{
}


void window_set_cursor_pos( window_t* window, int x, int y )
{
}


bool window_is_cursor_locked( window_t* window )
{
	return false;
}


void window_set_title( window_t* window, const char* title )
{
}


int window_width( window_t* window )
{
	if( window && window->uiwindow )
	{
		NSRect rect = [(UIWindow*)window->nswindow contentRectForFrameRect:[(UIWindow*)window->nswindow frame]];
		return (int)rect.size.width;
	}
	return 0;
}


int window_height( window_t* window )
{
	if( window && window->nswindow )
	{
		NSRect rect = [(NSWindow*)window->nswindow contentRectForFrameRect:[(NSWindow*)window->nswindow frame]];
		return (int)rect.size.height;
	}
	return 0;
}


int window_position_x( window_t* window )
{
	if( window && window->nswindow )
	{
		NSRect rect = [(NSWindow*)window->nswindow contentRectForFrameRect:[(NSWindow*)window->nswindow frame]];
		return (int)rect.origin.x;
	}
	return 0;
}


int window_position_y( window_t* window )
{
	if( window && window->nswindow )
	{
		NSRect rect = [(NSWindow*)window->nswindow contentRectForFrameRect:[(NSWindow*)window->nswindow frame]];
		return (int)rect.origin.y;
	}
	return 0;
}


void window_fit_to_screen( window_t* window )
{
	if( !window || !window->nswindow )
		return;
	
	NSWindow* nswindow = (NSWindow*)window->nswindow;
	NSScreen* screen = [nswindow screen];
	NSRect frame_rect = [nswindow frame];
	
	NSUInteger style_mask = [nswindow styleMask];
	NSUInteger resize_mask = style_mask | NSResizableWindowMask;
	[nswindow setStyleMask:resize_mask];
	
	NSRect new_rect = [nswindow constrainFrameRect:frame_rect toScreen:screen];
	if( ( new_rect.size.width < frame_rect.size.width ) || ( new_rect.size.height < frame_rect.size.height ) )
	{
		//Maintain aspect
		float width_factor = (float)new_rect.size.width / (float)frame_rect.size.width;
		float height_factor = (float)new_rect.size.height / (float)frame_rect.size.height;
		
		if( width_factor < height_factor )
			new_rect.size.height = new_rect.size.height * width_factor;
		else
			new_rect.size.width = new_rect.size.width * height_factor;
		
		[nswindow setFrame:new_rect display:TRUE];
		[nswindow setMinSize:new_rect.size];
		[nswindow setMaxSize:new_rect.size];
	}
	
	[nswindow setStyleMask:style_mask];
}


#endif
