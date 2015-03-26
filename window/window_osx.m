/* window_osx.m  -  Window library  -  Public Domain  -  2014 Mattias Jansson / Rampant Pixels
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

#if FOUNDATION_PLATFORM_MACOSX

#include <foundation/apple.h>


volatile int _dummy_window_class_reference = 0;


@implementation WindowView

+ (void)referenceClass
{
	log_debugf( 0, "WindowView class referenced" );
	++_dummy_window_class_reference;
}

- (BOOL)acceptsFirstResponder
{
	return YES;
}

- (BOOL)acceptsFirstMouse
{
	return YES;
}

- (BOOL)isOpaque
{
    return YES;
}

@end


@implementation WindowViewController

+ (void)referenceClass
{
	log_debugf( 0, "WindowViewController class referenced" );
	++_dummy_window_class_reference;
}

@end


window_t* window_allocate_from_nswindow( void* nswindow )
{
	window_t* window = memory_allocate( 0, sizeof( window_t ), 0, MEMORY_PERSISTENT | MEMORY_ZERO_INITIALIZED );
	window->nswindow = nswindow;
	return window;
}


void* window_content_view( window_t* window )
{
	return (__bridge void *)(window && window->nswindow ? [(__bridge NSWindow*)window->nswindow contentView] : 0);
}


void window_deallocate( window_t* window )
{
	memory_deallocate( window );
}


unsigned int window_adapter( window_t* window )
{
	FOUNDATION_UNUSED( window );
	return WINDOW_ADAPTER_DEFAULT;
}


void window_maximize( window_t* window )
{
    if( !window || !window->nswindow )
        return;

	if( window_is_maximized( window ) )
		return;

	NSWindow* nswindow = (__bridge NSWindow*)window->nswindow;
	[nswindow zoom:nil];
}


void window_minimize( window_t* window )
{
    if( !window || !window->nswindow )
        return;

	NSWindow* nswindow = (__bridge NSWindow*)window->nswindow;
	[nswindow miniaturize:nil];
}


void window_restore( window_t* window )
{
    if( !window || !window->nswindow )
        return;

	NSWindow* nswindow = (__bridge NSWindow*)window->nswindow;
	if( window_is_minimized( window ) )
		[nswindow deminiaturize:nil];
	else if( window_is_maximized( window ) )
		[nswindow zoom:nil];
}


void window_resize( window_t* window, unsigned int width, unsigned int height )
{
	if( !window || !window->nswindow )
		return;

	NSWindow* nswindow = (__bridge NSWindow*)window->nswindow;
	if( window_is_maximized( window ) )
		[nswindow zoom:nil];

	NSRect frame_rect = [nswindow frame];

	NSRect new_rect = frame_rect;
	new_rect.size.width = width;
	new_rect.size.height = height;
	new_rect = [nswindow frameRectForContentRect:new_rect];
	if( ( new_rect.size.width != frame_rect.size.width ) || ( new_rect.size.height != frame_rect.size.height ) )
	{
		NSUInteger style_mask = [nswindow styleMask];
		NSUInteger resize_mask = style_mask | NSResizableWindowMask;
		[nswindow setStyleMask:resize_mask];

		[nswindow setFrame:new_rect display:TRUE];

		[nswindow setStyleMask:style_mask];
	}
}


void window_move( window_t* window, int x, int y )
{
	if( !window || !window->nswindow )
		return;

	NSWindow* nswindow = (__bridge NSWindow*)window->nswindow;
	NSPoint pt = { x, y };
	[nswindow setFrameOrigin:pt];
}


bool window_is_open( window_t* window )
{
	if( !window || !window->nswindow )
		return false;
	return true;
}


bool window_is_visible( window_t* window )
{
	if( !window || !window->nswindow )
		return false;

	NSWindow* nswindow = (__bridge NSWindow*)window->nswindow;
	return [nswindow isVisible];
}


bool window_is_maximized( window_t* window )
{
	if( !window || !window->nswindow )
		return false;

	NSWindow* nswindow = (__bridge NSWindow*)window->nswindow;
	return [nswindow isZoomed];
}


bool window_is_minimized( window_t* window )
{
	if( !window || !window->nswindow )
		return false;

	NSWindow* nswindow = (__bridge NSWindow*)window->nswindow;
	return [nswindow isMiniaturized];
}


bool window_has_focus( window_t* window )
{
	return window && window->nswindow && ( [NSApp mainWindow] == (__bridge NSWindow *)(window->nswindow) );
}


void window_show_cursor( window_t* window, bool show, bool lock )
{
	FOUNDATION_UNUSED( window );
	FOUNDATION_UNUSED( show );
	FOUNDATION_UNUSED( lock );
}


void window_set_cursor_pos( window_t* window, int x, int y )
{
	FOUNDATION_UNUSED( window );
	FOUNDATION_UNUSED( x );
	FOUNDATION_UNUSED( y );
}


bool window_is_cursor_locked( window_t* window )
{
	FOUNDATION_UNUSED( window );
	return false;
}


void window_set_title( window_t* window, const char* title )
{
	if( !window || !window->nswindow )
		return;

	NSWindow* nswindow = (__bridge NSWindow*)window->nswindow;
	@autoreleasepool
	{
		[nswindow setTitle:[NSString stringWithUTF8String:title]];
	}
}


int window_width( window_t* window )
{
	if( window && window->nswindow )
	{
		NSRect rect = [(__bridge NSWindow*)window->nswindow contentRectForFrameRect:[(__bridge NSWindow*)window->nswindow frame]];
		return (int)rect.size.width;
	}
	return 0;
}


int window_height( window_t* window )
{
	if( window && window->nswindow )
	{
		NSRect rect = [(__bridge NSWindow*)window->nswindow contentRectForFrameRect:[(__bridge NSWindow*)window->nswindow frame]];
		return (int)rect.size.height;
	}
	return 0;
}


int window_position_x( window_t* window )
{
	if( window && window->nswindow )
	{
		NSRect rect = [(__bridge NSWindow*)window->nswindow frame];
		return (int)rect.origin.x;
	}
	return 0;
}


int window_position_y( window_t* window )
{
	if( window && window->nswindow )
	{
		NSRect rect = [(__bridge NSWindow*)window->nswindow frame];
		return (int)rect.origin.y;
	}
	return 0;
}


void window_fit_to_screen( window_t* window )
{
	if( !window || !window->nswindow )
		return;

	NSWindow* nswindow = (__bridge NSWindow*)window->nswindow;
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
	}

	[nswindow setStyleMask:style_mask];
}


void _window_class_reference( void )
{
	[WindowView referenceClass];
	[WindowViewController referenceClass];
}


#endif
