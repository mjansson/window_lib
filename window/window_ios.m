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
#include <objc/runtime.h>

#import <Foundation/NSRunLoop.h>
#import <UIKit/UIScreen.h>
#import <UIKit/UIScreenMode.h>


bool _window_app_started = false;
bool _window_app_paused = true;


@interface WindowKeyboardView : UIView <UIKeyInput>
@end


window_t* window_allocate_from_uiwindow( void* uiwindow )
{
	window_t* window = memory_allocate_zero( sizeof( window_t ), 0, MEMORY_PERSISTENT );
	window->uiwindow = uiwindow;
	return window;
}


void* window_view( window_t* window, unsigned int tag )
{
	if( !window || !window->uiwindow )
		return 0;
	
	UIWindow* uiwindow = window->uiwindow;
	UIView* view = [uiwindow viewWithTag:tag];
	if( !view )
		view = [[uiwindow subviews] objectAtIndex:0];
	return view;
}


void* window_layer( window_t* window, void* view )
{
	return view ? [(UIView*)view layer] : 0;
}


@interface WindowDisplayLink : NSObject
{
@public
window_t*      target_window;
window_draw_fn draw_fn;
}

+(id)fromDrawFn:(window_draw_fn)fn window:(window_t*)window;
-(id)initWithDrawFn:(window_draw_fn)fn window:(window_t*)window;;
-(void)callDraw:(id)obj;

@end

@implementation WindowDisplayLink


+ (id)fromDrawFn:(window_draw_fn)fn window:(window_t *)window
{
    return [[self alloc] initWithDrawFn:fn window:window];
}


- (id)initWithDrawFn:(window_draw_fn)fn window:(window_t*)window;
{
	draw_fn = fn;
	target_window = window;
	return self;
}


- (void)callDraw:(id)obj
{
	draw_fn( target_window );
}

@end


void window_add_displaylink( window_t* window, window_draw_fn drawfn )
{
	id wrapper = [WindowDisplayLink fromDrawFn:drawfn window:window];
	id display_link = [NSClassFromString(@"CADisplayLink") displayLinkWithTarget:wrapper selector:@selector(callDraw:)];
	[display_link setFrameInterval:1];
	[display_link addToRunLoop:[NSRunLoop mainRunLoop] forMode:NSDefaultRunLoopMode];
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
	return _window_app_started;
}


bool window_is_visible( window_t* window )
{
	return !_window_app_paused;
}


bool window_is_maximized( window_t* window )
{
	return !_window_app_paused;
}


bool window_is_minimized( window_t* window )
{
	return _window_app_paused;
}


bool window_has_focus( window_t* window )
{
	return !_window_app_paused;
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
		UIWindow* uiwindow = window->uiwindow;
		CGRect rect = [uiwindow frame];
		CGFloat scale = [uiwindow contentScaleFactor];
		return (int)( rect.size.width * scale );
	}
	return 0;
}


int window_height( window_t* window )
{
	if( window && window->uiwindow )
	{
		UIWindow* uiwindow = window->uiwindow;
		CGRect rect = [uiwindow frame];
		CGFloat scale = [uiwindow contentScaleFactor];
		return (int)( rect.size.width * scale );
	}
	return 0;
}


int window_position_x( window_t* window )
{
	return 0;
}


int window_position_y( window_t* window )
{
	return 0;
}


void window_fit_to_screen( window_t* window )
{
}


@implementation WindowKeyboardView

- (void)insertText:(NSString*)text
{
	/*log_debugf( HASH_WINDOW, "iOS keyboard text: %s", [text UTF8String] );
	unsigned int glyph = [text characterAtIndex:0];
	input_event_post_key( INPUTEVENT_CHAR, glyph, 0 );
	if( ( glyph == 10 ) || ( glyph == 13 ) )
	{
		input_event_post_key( INPUTEVENT_KEYDOWN, KEY_RETURN, 0 );
		input_event_post_key( INPUTEVENT_KEYUP, KEY_RETURN, 0 );
	}*/
}

- (void)deleteBackward
{
	/*log_debugf( HASH_WINDOW, "iOS keyboard backspace" );
	input_event_post_key( INPUTEVENT_KEYDOWN, KEY_BACKSPACE, 0 );
	input_event_post_key( INPUTEVENT_KEYUP, KEY_BACKSPACE, 0 );*/
}

- (BOOL)hasText { return YES; }
- (BOOL)canBecomeFirstResponder { return YES; }

@end


@implementation WindowGLView


+ (Class) layerClass
{
    return [CAEAGLLayer class];
}


- (BOOL)canBecomeFirstResponder { return YES; }


- (id) initWithFrame:(CGRect)frame
{
    if( ( self = [super initWithFrame:frame] ) )
	{
		UIScreen* main_screen = [UIScreen mainScreen];
		CGFloat screen_width = main_screen.currentMode.size.width;
		CGFloat screen_height = main_screen.currentMode.size.height;
        
        if( ( frame.size.width >= ( screen_width * 2 ) ) && ( frame.size.height >= ( screen_height * 2 ) ) )
			self.contentScaleFactor = 2;
		
		log_debugf( HASH_WINDOW, "WindowGLView initWithFrame, setting up layer (class %s) %dx%d (%.1f)", class_getName( [[self layer] class] ), screen_width, screen_height, self.contentScaleFactor );
        CAEAGLLayer* layer = (CAEAGLLayer*)self.layer;
        layer.opaque = TRUE;
        if( self.contentScaleFactor > 1 )
            layer.contentsScale = self.contentScaleFactor;
        layer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithBool:FALSE], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];
		
		keyboard_view = 0;
	}
    return self;
}


- (id) initWithCoder:(NSCoder*)coder
{
    if( ( self = [super initWithCoder:coder] ) )
	{
		UIScreen* main_screen = [UIScreen mainScreen];
		CGFloat screen_width = main_screen.currentMode.size.width;
		CGFloat screen_height = main_screen.currentMode.size.height;
        
		CGRect frame = [self frame];
        if( ( frame.size.width >= ( screen_width * 2 ) ) && ( frame.size.height >= ( screen_height * 2 ) ) )
			self.contentScaleFactor = 2;
        
		log_debugf( HASH_WINDOW, "WindowGLView initWithCoder, setting up layer (class %s) %dx%d (%.1f)", class_getName( [[self layer] class] ), screen_width, screen_height, self.contentScaleFactor );
        CAEAGLLayer* layer = (CAEAGLLayer*)self.layer;
        layer.opaque = TRUE;
        if( self.contentScaleFactor > 1 )
            layer.contentsScale = self.contentScaleFactor;
        layer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithBool:FALSE], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];
		
		keyboard_view = 0;
	}
    return self;
}


@end


#endif
