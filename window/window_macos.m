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
#include <foundation/semaphore.h>

#import <QuartzCore/QuartzCore.h>

static volatile int dummy_window_class_reference = 0;
static semaphore_t window_quit_semaphore;
static bool window_exit_loop;

void
window_native_initialize(void) {
	semaphore_initialize(&window_quit_semaphore, 0);
	window_exit_loop = 0;
}

void
window_native_finalize(void) {
	semaphore_finalize(&window_quit_semaphore);
}

@interface WindowDelegate : NSObject <NSWindowDelegate>
@property (nonatomic, assign) window_t* window;
@end

@implementation WindowView

+ (void)referenceClass {
	log_debug(0, STRING_CONST("WindowView class referenced"));
	++dummy_window_class_reference;
}

+ (Class)layerClass {
	return [CAMetalLayer class];
}

- (CALayer*)makeBackingLayer {
	return [CAMetalLayer layer];
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
	++dummy_window_class_reference;
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
	if (window->nswindow) {
		dispatch_sync(dispatch_get_main_queue(), ^{
		  [(__bridge NSWindow*)window->nswindow setDelegate:delegate];
		});
	}
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
	window->delegate = nullptr;
	window->nswindow = nullptr;
}

void
window_deallocate(window_t* window) {
	window_finalize(window);
	memory_deallocate(window);
}

void*
window_view(window_t* window, unsigned int tag) {
	FOUNDATION_UNUSED(tag);
	__block void* view = nullptr;
	dispatch_sync(dispatch_get_main_queue(), ^{
	  view = (__bridge void*)((window && window->nswindow) ? [(__bridge NSWindow*)window->nswindow contentView] : 0);
	});
	return view;
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

	NSWindow* nswindow = (__bridge NSWindow*)window->nswindow;
	dispatch_sync(dispatch_get_main_queue(), ^{
	  if ([nswindow isZoomed])
		  return;

	  [nswindow zoom:nil];
	});
}

void
window_minimize(window_t* window) {
	if (!window || !window->nswindow)
		return;

	dispatch_sync(dispatch_get_main_queue(), ^{
	  [(__bridge NSWindow*)window->nswindow miniaturize:nil];
	});
}

void
window_restore(window_t* window) {
	if (!window || !window->nswindow)
		return;

	dispatch_sync(dispatch_get_main_queue(), ^{
	  NSWindow* nswindow = (__bridge NSWindow*)window->nswindow;
	  if ([nswindow isMiniaturized])
		  [nswindow deminiaturize:nil];
	  else if ([nswindow isZoomed])
		  [nswindow zoom:nil];
	});
}

void
window_resize(window_t* window, int width, int height) {
	if (!window || !window->nswindow)
		return;

	dispatch_sync(dispatch_get_main_queue(), ^{
	  NSWindow* nswindow = (__bridge NSWindow*)window->nswindow;
	  if ([nswindow isZoomed])
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
	});
}

void
window_move(window_t* window, int x, int y) {
	if (!window || !window->nswindow)
		return;

	NSWindow* nswindow = (__bridge NSWindow*)window->nswindow;
	NSPoint pt = {x, y};
	dispatch_sync(dispatch_get_main_queue(), ^{
	  [nswindow setFrameOrigin:pt];
	});
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

	__block bool is_visible = false;
	dispatch_sync(dispatch_get_main_queue(), ^{
	  is_visible = [(__bridge NSWindow*)window->nswindow isVisible];
	});
	return is_visible;
}

bool
window_is_maximized(window_t* window) {
	if (!window || !window->nswindow)
		return false;

	__block bool is_maxi = false;
	dispatch_sync(dispatch_get_main_queue(), ^{
	  is_maxi = [(__bridge NSWindow*)window->nswindow isZoomed];
	});
	return is_maxi;
}

bool
window_is_minimized(window_t* window) {
	if (!window || !window->nswindow)
		return false;

	__block bool is_mini = false;
	dispatch_sync(dispatch_get_main_queue(), ^{
	  is_mini = [(__bridge NSWindow*)window->nswindow isMiniaturized];
	});
	return is_mini;
}

bool
window_has_focus(window_t* window) {
	__block bool has_focus = false;
	dispatch_sync(dispatch_get_main_queue(), ^{
	  @autoreleasepool {
		  bool active = [[NSRunningApplication currentApplication] isActive];
		  has_focus =
			  active && (window && window->nswindow && ([NSApp mainWindow] == (__bridge NSWindow*)(window->nswindow)));
	  }
	});
	return has_focus;
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
		__block NSRect rect;
		dispatch_sync(dispatch_get_main_queue(), ^{
		  rect = [(__bridge NSWindow*)window->nswindow
			  contentRectForFrameRect:[(__bridge NSWindow*)window->nswindow frame]];
		});
		return (uint)rect.size.width;
	}
	return 0;
}

unsigned int
window_height(window_t* window) {
	if (window && window->nswindow) {
		__block NSRect rect;
		dispatch_sync(dispatch_get_main_queue(), ^{
		  rect = [(__bridge NSWindow*)window->nswindow
			  contentRectForFrameRect:[(__bridge NSWindow*)window->nswindow frame]];
		});
		return (uint)rect.size.height;
	}
	return 0;
}

int
window_position_x(window_t* window) {
	if (window && window->nswindow) {
		__block NSRect rect;
		dispatch_sync(dispatch_get_main_queue(), ^{
		  rect = [(__bridge NSWindow*)window->nswindow frame];
		});
		return (int)rect.origin.x;
	}
	return 0;
}

int
window_position_y(window_t* window) {
	if (window && window->nswindow) {
		__block NSRect rect;
		dispatch_sync(dispatch_get_main_queue(), ^{
		  rect = [(__bridge NSWindow*)window->nswindow frame];
		});
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

	dispatch_async(dispatch_get_main_queue(), ^{
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
	});
}

int
window_message_loop(void) {
	while (!window_exit_loop) {
		semaphore_wait(&window_quit_semaphore);
	}
	return 0;
}

void
window_message_quit(void) {
	window_exit_loop = true;
	semaphore_post(&window_quit_semaphore);
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
