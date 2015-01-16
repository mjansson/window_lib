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

#include <foundation/platform.h>

#include <window/build.h>
#include <window/hashstrings.h>


static const unsigned int WINDOW_ADAPTER_DEFAULT    = -1;

typedef struct window_t window_t;


#if FOUNDATION_PLATFORM_MACOSX
#  ifdef __OBJC__

#include <foundation/apple.h>
#import <AppKit/NSView.h>

@interface WindowView : NSView
{
	@public
}
+ (void)referenceClass;
@end

@interface WindowViewController : NSViewController
{
	@public
}
+ (void)referenceClass;
@end

#  endif
#endif

#if FOUNDATION_PLATFORM_IOS
#  ifdef __OBJC__

#include <foundation/apple.h>

@interface WindowView : UIView
{
	@public
	id       display_link;
	CGPoint  begin_touch;
	CGPoint  last_touch;
	tick_t   begin_touch_time;
	UIView*  keyboard_view;
}
+ (void)referenceClass;
@end

@interface WindowViewController : UIViewController
{
	@public
}
@property (nonatomic) BOOL hideStatusBar;
+ (void)referenceClass;
@end

#  endif
#endif


typedef void (* window_draw_fn)( window_t* window );

