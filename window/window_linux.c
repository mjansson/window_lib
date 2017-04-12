/* window_linux.c  -  Window library  -  Public Domain  -  2014 Mattias Jansson / Rampant Pixels
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

#if FOUNDATION_PLATFORM_LINUX

#include <foundation/foundation.h>

#include <GL/glx.h>

static XVisualInfo*
_get_xvisual(Display* display, unsigned int screen, unsigned int color, unsigned int depth, unsigned int stencil) {
#if FOUNDATION_PLATFORM_LINUX_RASPBERRYPI
	return 0;
#else
	int config[13];
	int cbits  = (color > 16) ? 8 : 5;
	int dbits  = (depth > 0) ? 15 : 0;
	int sbits  = (stencil > 0) ? 1 : 0;

	config[0]  = GLX_DOUBLEBUFFER;
	config[1]  = GLX_RGBA;
	config[2]  = GLX_GREEN_SIZE  ; config[3]  = cbits;
	config[4]  = GLX_RED_SIZE    ; config[5]  = cbits;
	config[6]  = GLX_BLUE_SIZE   ; config[7]  = cbits;
	config[8]  = GLX_DEPTH_SIZE  ; config[9]  = dbits;
	config[10] = GLX_STENCIL_SIZE; config[11] = sbits;
	config[12] = None;

	return glXChooseVisual(display, (int)screen, config);
#endif
}

window_t*
window_create(unsigned int adapter, const char* title, size_t length, unsigned int width, unsigned int height,
              bool show) {
	FOUNDATION_UNUSED(length);

	Display* display = XOpenDisplay(0);
	if (!display) {
		log_error(HASH_WINDOW, ERROR_SYSTEM_CALL_FAIL, STRING_CONST("Unable to open X display"));
		goto fail;
	}

	unsigned int screen = adapter;
	if (!screen)
		screen = (unsigned int)DefaultScreen(display);

	XVisualInfo* visual = _get_xvisual(display, screen, 24, 16, 0);
	if (!visual) {
		log_errorf(HASH_WINDOW, ERROR_SYSTEM_CALL_FAIL, STRING_CONST("Unable to get X visual"));
		goto fail;
	}

	Colormap colormap = XCreateColormap(display, XRootWindow(display, (int)screen), visual->visual, AllocNone);

	log_debugf(HASH_WINDOW, STRING_CONST("Creating window on screen %u with dimensions %ux%u"), screen, width, height);

	XSetWindowAttributes attrib;
	attrib.colormap         = colormap;
	attrib.background_pixel = 0;
	attrib.border_pixel     = 0;
	attrib.event_mask       = ExposureMask | StructureNotifyMask | ButtonPressMask | ButtonReleaseMask | EnterWindowMask | LeaveWindowMask | PointerMotionMask | Button1MotionMask | Button2MotionMask | Button3MotionMask | Button4MotionMask | Button5MotionMask | ButtonMotionMask | KeyPressMask | KeyReleaseMask | KeymapStateMask | VisibilityChangeMask | FocusChangeMask;
	Window drawable = XCreateWindow(display, XRootWindow(display, (int)screen), 0, 0, width, height, 0, visual->depth, InputOutput, visual->visual, CWBackPixel | CWBorderPixel | CWColormap | CWEventMask, &attrib);

	XSizeHints sizehints;
	sizehints.x      = 0;
	sizehints.y      = 0;
	sizehints.width  = (int)width;
	sizehints.height = (int)height;
	sizehints.flags  = USSize | USPosition;
	XSetNormalHints(display, drawable, &sizehints);
	XSetStandardProperties(display, drawable, title, title, None, 0, 0, &sizehints);

	if (show) {
		XMapWindow(display, drawable);
		XFlush(display);
	}

	Atom atom_delete = XInternAtom(display, "WM_DELETE_WINDOW", False);

	XSetWMProtocols(display, drawable, &atom_delete, 1);
	XFlush(display);
	XSync(display, False);

	XIC xic = 0;
	XIM xim = XOpenIM(display, 0, 0, 0);
	if(xim) {
		xic = XCreateIC(xim, XNInputStyle, XIMPreeditNone | XIMStatusNone, XNClientWindow, drawable, nullptr);
		if(xic) {
			/*XGetICValues(ic, XNFilterEvents, &fevent, NULL);
			mask = ExposureMask | KeyPressMask | FocusChangeMask;
			XSelectInput(display, window, mask|fevent);*/
		}
		else {
			log_warn(HASH_WINDOW, WARNING_SUSPICIOUS, STRING_CONST("Unable to create X input context"));
		}
	}
	else {
		log_warn(HASH_WINDOW, WARNING_SUSPICIOUS, STRING_CONST("Unable to open X input method"));
	}

	window_t* window = memory_allocate(HASH_WINDOW, sizeof(window_t), 0, MEMORY_PERSISTENT | MEMORY_ZERO_INITIALIZED);
	window->display  = display;
	window->visual   = visual;
	window->screen   = screen;
	window->drawable = drawable;
	window->atom     = atom_delete;
	window->xim      = xim;
	window->xic      = xic;

	return window;

fail:

	return 0;
}

void*
window_display(window_t* window) {
	FOUNDATION_UNUSED(window);
	return 0;
}

int
window_screen(window_t* window) {
	FOUNDATION_UNUSED(window);
	return -1;
}

int
window_drawable(window_t* window) {
	FOUNDATION_UNUSED(window);
	return 0;
}

void*
window_visual(window_t* window) {
	FOUNDATION_UNUSED(window);
	return 0;
}

void
window_deallocate(window_t* window) {
	memory_deallocate(window);
}

unsigned int
window_adapter(window_t* window) {
	FOUNDATION_UNUSED(window);
	return WINDOW_ADAPTER_DEFAULT;
}

void
window_maximize(window_t* window) {
	FOUNDATION_UNUSED(window);
}

void
window_minimize(window_t* window) {
	FOUNDATION_UNUSED(window);
}

void
window_restore(window_t* window) {
	FOUNDATION_UNUSED(window);
}

void
window_resize(window_t* window, unsigned int width, unsigned int height) {
	FOUNDATION_UNUSED(window);
	FOUNDATION_UNUSED(width);
	FOUNDATION_UNUSED(height);
}

void
window_move(window_t* window, int x, int y) {
	FOUNDATION_UNUSED(window);
	FOUNDATION_UNUSED(x);
	FOUNDATION_UNUSED(y);
}

bool
window_is_open(window_t* window) {
	FOUNDATION_UNUSED(window);
	return true;
}

bool
window_is_visible(window_t* window) {
	FOUNDATION_UNUSED(window);
	return true;
}

bool
window_is_maximized(window_t* window) {
	FOUNDATION_UNUSED(window);
	return false;
}

bool
window_is_minimized(window_t* window) {
	FOUNDATION_UNUSED(window);
	return false;
}

bool
window_has_focus(window_t* window) {
	FOUNDATION_UNUSED(window);
	return true;
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
window_set_title(window_t* window, const char* title) {
	FOUNDATION_UNUSED(window);
	FOUNDATION_UNUSED(title);
}

int
window_width(window_t* window) {
	FOUNDATION_UNUSED(window);
	return 0;
}

int
window_height(window_t* window) {
	FOUNDATION_UNUSED(window);
	return 0;
}

int
window_position_x(window_t* window) {
	FOUNDATION_UNUSED(window);
	return 0;
}

int
window_position_y(window_t* window) {
	FOUNDATION_UNUSED(window);
	return 0;
}

void
window_fit_to_screen(window_t* window) {
	FOUNDATION_UNUSED(window);
}

#endif
