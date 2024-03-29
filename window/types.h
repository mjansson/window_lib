/* types.h  -  Window library types  -  Public Domain  -  2014 Mattias Jansson
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

#pragma once

#include <foundation/platform.h>

#include <window/build.h>
#include <window/hashstrings.h>

#if FOUNDATION_PLATFORM_LINUX
#include <X11/Xutil.h>
#endif

typedef enum window_event_id {
	/*! Window was created */
	WINDOWEVENT_CREATE = 1,
	/*! Window was resized */
	WINDOWEVENT_RESIZE,
	/*! Window was moved */
	WINDOWEVENT_MOVE,
	/*! Window close requested */
	WINDOWEVENT_CLOSE,
	/*! Window was destroyed */
	WINDOWEVENT_DESTROY,
	/*! Window was shown */
	WINDOWEVENT_SHOW,
	/*! Window was hidden */
	WINDOWEVENT_HIDE,
	/*! Window got focus */
	WINDOWEVENT_GOTFOCUS,
	/*! Window lost focus */
	WINDOWEVENT_LOSTFOCUS,
	/*! Window needs to be redrawn */
	WINDOWEVENT_REDRAW,
	/*! Native event */
	WINDOWEVENT_NATIVE
} window_event_id;

#define WINDOW_ADAPTER_DEFAULT ((unsigned int)-1)

#define WINDOW_FLAG_NOSHOW 0x0001
#define WINDOW_FLAG_NOSYSTEMMENU 0x0002
#define WINDOW_FLAG_FULLSCREEN 0x0004
#define WINDOW_FLAG_NORESIZE 0x0008

typedef struct window_config_t window_config_t;
typedef struct window_t window_t;

struct window_config_t {
	int unused;
};

struct window_t {
#if FOUNDATION_PLATFORM_WINDOWS
	unsigned int adapter;
	void* hwnd;
	void* instance;
	bool created;
	bool cursor_lock;
	int cursor_pos_x;
	int cursor_pos_y;
	unsigned int wstyle;
	bool is_resizing;
#elif FOUNDATION_PLATFORM_MACOS
	void* nswindow;
	void* delegate;
#elif FOUNDATION_PLATFORM_LINUX
	unsigned int adapter;
	bool created;
	Display* display;
	unsigned int screen;
	XVisualInfo* visual;
	Window drawable;
	Atom atom_delete;
	XIM xim;
	XIC xic;
	bool focus;
	bool visible;
#elif FOUNDATION_PLATFORM_IOS
	void* uiwindow;
	unsigned int tag;
#elif FOUNDATION_PLATFORM_ANDROID
	unsigned int adapter;
	int width;
	int height;
	void* native;
#endif
	unsigned int flags;
	bool is_visible;
	tick_t last_paint;
	tick_t last_resize;
};

#if FOUNDATION_PLATFORM_MACOS
#ifdef __OBJC__

#include <foundation/apple.h>
#import <AppKit/NSView.h>

@interface WindowView : NSView {
   @public
}
+ (void)referenceClass;
@end

@interface WindowViewController : NSViewController {
   @public
}
+ (void)referenceClass;
@end

#endif
#endif

#if FOUNDATION_PLATFORM_IOS
#ifdef __OBJC__

#include <foundation/apple.h>

@interface WindowView : UIView {
   @public
	id display_link;
	CGPoint begin_touch;
	CGPoint last_touch;
	tick_t begin_touch_time;
	UIView* keyboard_view;
}
+ (void)referenceClass;
@end

IB_DESIGNABLE
@interface WindowViewController : UIViewController {
   @public
}
@property (nonatomic) IBInspectable BOOL hideStatusBar;
+ (void)referenceClass;
@end

#endif
#endif

typedef void (*window_draw_fn)(window_t* window);
