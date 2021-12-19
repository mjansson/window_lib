/* window_osx.m  -  Window library  -  Public Domain  -  2014 Mattias Jansson
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

#include <window/window.h>

#if FOUNDATION_PLATFORM_MACOS

#include <window/internal.h>
#include <foundation/apple.h>

static volatile int _dummy_window_class_reference = 0;

@interface WindowDelegate : NSObject <NSWindowDelegate>
@property (nonatomic, assign) window_t* window;
@end

@implementation WindowView

+ (void)referenceClass {
	log_debug(0, STRING_CONST("WindowView class referenced"));
	++_dummy_window_class_reference;
}

- (BOOL)acceptsFirstResponder {
	return YES;
}

- (BOOL)acceptsFirstMouse {
	return YES;
}

- (BOOL)isOpaque {
	return YES;
}

@end

@implementation WindowViewController

+ (void)referenceClass {
	log_debug(0, STRING_CONST("WindowViewController class referenced"));
	++_dummy_window_class_reference;
}

@end

window_t*
window_allocate(void* nswindow) {
	window_t* window = memory_allocate(0, sizeof(window_t), 0, MEMORY_PERSISTENT | MEMORY_ZERO_INITIALIZED);
	window_initialize(window, nswindow);
	return window;
}

void
window_initialize(window_t* window, void* nswindow) {
	window->nswindow = nswindow;

	WindowDelegate* delegate = [[WindowDelegate alloc] init];
	delegate.window = window;
	window->delegate = (__bridge_retained void*)delegate;
	if (window->nswindow)
		[(__bridge NSWindow*)window->nswindow setDelegate:delegate];
	window_event_post(WINDOWEVENT_CREATE, window);

	if (window_is_visible(window))
		window_event_post(WINDOWEVENT_SHOW, window);
	if (window_has_focus(window))
		window_event_post(WINDOWEVENT_GOTFOCUS, window);
	window_event_post(WINDOWEVENT_REDRAW, window);
}

void
window_finalize(window_t* window) {
	if (window->delegate)
		CFRelease(window->delegate);
}

void
window_deallocate(window_t* window) {
	window_finalize(window);
	memory_deallocate(window);
}

void*
window_view(window_t* window, unsigned int tag) {
	FOUNDATION_UNUSED(tag);
	return (__bridge void*)((window && window->nswindow) ? [(__bridge NSWindow*)window->nswindow contentView] : 0);
}

unsigned int
window_adapter(window_t* window) {
	FOUNDATION_UNUSED(window);
	return (unsigned int)WINDOW_ADAPTER_DEFAULT;
}

void
window_maximize(window_t* window) {
	if (!window || !window->nswindow)
		return;

	if (window_is_maximized(window))
		return;

	[(__bridge NSWindow*)window->nswindow zoom:nil];
}

void
window_minimize(window_t* window) {
	if (!window || !window->nswindow)
		return;

	[(__bridge NSWindow*)window->nswindow miniaturize:nil];
}

void
window_restore(window_t* window) {
	if (!window || !window->nswindow)
		return;

	NSWindow* nswindow = (__bridge NSWindow*)window->nswindow;
	if (window_is_minimized(window))
		[nswindow deminiaturize:nil];
	else if (window_is_maximized(window))
		[nswindow zoom:nil];
}

void
window_resize(window_t* window, int width, int height) {
	if (!window || !window->nswindow)
		return;

	NSWindow* nswindow = (__bridge NSWindow*)window->nswindow;
	if (window_is_maximized(window))
		[nswindow zoom:nil];

	NSRect frame_rect = [nswindow frame];

	NSRect new_rect = frame_rect;
	new_rect.size.width = width;
	new_rect.size.height = height;
	new_rect = [nswindow frameRectForContentRect:new_rect];
	if (!math_real_eq((real)new_rect.size.width, (real)frame_rect.size.width, 100) ||
	    !math_real_eq((real)new_rect.size.height, (real)frame_rect.size.height, 100)) {
		NSUInteger style_mask = [nswindow styleMask];
		NSUInteger resize_mask = style_mask | NSWindowStyleMaskResizable;
		[nswindow setStyleMask:resize_mask];
		[nswindow setFrame:new_rect display:TRUE];
		[nswindow setStyleMask:style_mask];
	}
}

void
window_move(window_t* window, int x, int y) {
	if (!window || !window->nswindow)
		return;

	NSWindow* nswindow = (__bridge NSWindow*)window->nswindow;
	NSPoint pt = {x, y};
	[nswindow setFrameOrigin:pt];
}

bool
window_is_open(window_t* window) {
	if (!window || !window->nswindow)
		return false;
	return true;
}

bool
window_is_visible(window_t* window) {
	if (!window || !window->nswindow)
		return false;

	return [(__bridge NSWindow*)window->nswindow isVisible];
}

bool
window_is_maximized(window_t* window) {
	if (!window || !window->nswindow)
		return false;

	return [(__bridge NSWindow*)window->nswindow isZoomed];
}

bool
window_is_minimized(window_t* window) {
	if (!window || !window->nswindow)
		return false;

	return [(__bridge NSWindow*)window->nswindow isMiniaturized];
}

bool
window_has_focus(window_t* window) {
	return window && window->nswindow && ([NSApp mainWindow] == (__bridge NSWindow*)(window->nswindow));
}

void
window_show_cursor(window_t* window, bool show, bool lock) {
	FOUNDATION_UNUSED(window);
	FOUNDATION_UNUSED(show);
	FOUNDATION_UNUSED(lock);
}

void
window_set_cursor_pos(window_t* window, int x, int y) {
	FOUNDATION_UNUSED(window);
	FOUNDATION_UNUSED(x);
	FOUNDATION_UNUSED(y);
}

bool
window_is_cursor_locked(window_t* window) {
	FOUNDATION_UNUSED(window);
	return false;
}

void
window_set_title(window_t* window, const char* title, size_t length) {
	if (!window || !window->nswindow)
		return;

	@autoreleasepool {
		NSString* nsstr = [[NSString alloc] initWithBytes:title length:length encoding:NSUTF8StringEncoding];
		if (nsstr)
			[(__bridge NSWindow*)window->nswindow setTitle:nsstr];
	}
}

unsigned int
window_width(window_t* window) {
	if (window && window->nswindow) {
		NSRect rect =
		    [(__bridge NSWindow*)window->nswindow contentRectForFrameRect:[(__bridge NSWindow*)window->nswindow frame]];
		return (uint)rect.size.width;
	}
	return 0;
}

unsigned int
window_height(window_t* window) {
	if (window && window->nswindow) {
		NSRect rect =
		    [(__bridge NSWindow*)window->nswindow contentRectForFrameRect:[(__bridge NSWindow*)window->nswindow frame]];
		return (uint)rect.size.height;
	}
	return 0;
}

int
window_position_x(window_t* window) {
	if (window && window->nswindow) {
		NSRect rect = [(__bridge NSWindow*)window->nswindow frame];
		return (int)rect.origin.x;
	}
	return 0;
}

int
window_position_y(window_t* window) {
	if (window && window->nswindow) {
		NSRect rect = [(__bridge NSWindow*)window->nswindow frame];
		return (int)rect.origin.y;
	}
	return 0;
}

int
window_screen_width(unsigned int adapter) {
	FOUNDATION_UNUSED(adapter);
	return 0;
}

int
window_screen_height(unsigned int adapter) {
	FOUNDATION_UNUSED(adapter);
	return 0;
}

void
window_fit_to_screen(window_t* window) {
	if (!window || !window->nswindow)
		return;

	NSWindow* nswindow = (__bridge NSWindow*)window->nswindow;
	NSScreen* screen = [nswindow screen];
	NSRect frame_rect = [nswindow frame];

	NSUInteger style_mask = [nswindow styleMask];
	NSUInteger resize_mask = style_mask | NSWindowStyleMaskResizable;
	[nswindow setStyleMask:resize_mask];

	NSRect new_rect = [nswindow constrainFrameRect:frame_rect toScreen:screen];
	if ((new_rect.size.width < frame_rect.size.width) || (new_rect.size.height < frame_rect.size.height)) {
		// Maintain aspect
		double width_factor = new_rect.size.width / frame_rect.size.width;
		double height_factor = new_rect.size.height / frame_rect.size.height;

		if (width_factor < height_factor)
			new_rect.size.height = new_rect.size.height * width_factor;
		else
			new_rect.size.width = new_rect.size.width * height_factor;

		[nswindow setFrame:new_rect display:TRUE];
	}

	[nswindow setStyleMask:style_mask];
}

int
window_message_loop(void) {
	return 0;
}

void
window_message_quit(void) {
}

void
window_class_reference(void) {
	[WindowView referenceClass];
	[WindowViewController referenceClass];
}

@implementation WindowDelegate

@synthesize window;

- (void)windowDidResize:(NSNotification*)notification {
	FOUNDATION_UNUSED(notification);
}

- (void)windowDidExpose:(NSNotification*)notification {
	FOUNDATION_UNUSED(notification);
	window_event_post(WINDOWEVENT_SHOW, self.window);
}

- (void)windowDidMove:(NSNotification*)notification {
	FOUNDATION_UNUSED(notification);
}

- (void)windowDidBecomeKey:(NSNotification*)notification {
	FOUNDATION_UNUSED(notification);
}

- (void)windowDidResignKey:(NSNotification*)notification {
	FOUNDATION_UNUSED(notification);
}

- (void)windowDidBecomeMain:(NSNotification*)notification {
	FOUNDATION_UNUSED(notification);
}

- (void)windowDidResignMain:(NSNotification*)notification {
	FOUNDATION_UNUSED(notification);
}

- (void)windowDidMiniaturize:(NSNotification*)notification {
	FOUNDATION_UNUSED(notification);
	window_event_post(WINDOWEVENT_RESIZE, self.window);
	window_event_post(WINDOWEVENT_LOSTFOCUS, self.window);
}

- (void)windowDidDeminiaturize:(NSNotification*)notification {
	FOUNDATION_UNUSED(notification);
	window_event_post(WINDOWEVENT_RESIZE, self.window);
	window_event_post(WINDOWEVENT_REDRAW, self.window);
	if (window_has_focus(self.window))
		window_event_post(WINDOWEVENT_GOTFOCUS, self.window);
}

- (void)windowDidEndLiveResize:(NSNotification*)notification {
	FOUNDATION_UNUSED(notification);
	window_event_post(WINDOWEVENT_RESIZE, self.window);
	window_event_post(WINDOWEVENT_REDRAW, self.window);
}

- (void)windowDidEnterFullScreen:(NSNotification*)notification {
	FOUNDATION_UNUSED(notification);
}

- (void)windowDidExitFullScreen:(NSNotification*)notification {
	FOUNDATION_UNUSED(notification);
}

- (void)windowWillClose:(NSNotification*)notification {
	FOUNDATION_UNUSED(notification);
	window_event_post(WINDOWEVENT_CLOSE, self.window);
}

@end

#endif
