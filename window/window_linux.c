/* window_linux.c  -  Window library  -  Public Domain  -  2014 Mattias Jansson
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
#include <window/internal.h>

#if FOUNDATION_PLATFORM_LINUX

#include <foundation/foundation.h>

#include <GL/glx.h>

#define _NET_WM_STATE_REMOVE 0
#define _NET_WM_STATE_ADD 1
//#define _NET_WM_STATE_TOGGLE 2

static XVisualInfo*
_get_xvisual(Display* display, int screen, unsigned int color, unsigned int depth, unsigned int stencil) {
#if FOUNDATION_PLATFORM_LINUX_RASPBERRYPI
	return 0;
#else
	int config[13];
	int cbits = (color > 16) ? 8 : 5;
	int dbits = (depth > 0) ? 15 : 0;
	int sbits = (stencil > 0) ? 1 : 0;

	config[0] = GLX_DOUBLEBUFFER;
	config[1] = GLX_RGBA;
	config[2] = GLX_GREEN_SIZE;
	config[3] = cbits;
	config[4] = GLX_RED_SIZE;
	config[5] = cbits;
	config[6] = GLX_BLUE_SIZE;
	config[7] = cbits;
	config[8] = GLX_DEPTH_SIZE;
	config[9] = dbits;
	config[10] = GLX_STENCIL_SIZE;
	config[11] = sbits;
	config[12] = None;

	return glXChooseVisual(display, screen, config);
#endif
}

void
window_allocate(void) {
	window_t* window = memory_allocate(HASH_WINDOW, sizeof(window_t), 0, MEMORY_PERSISTENT | MEMORY_ZERO_INITIALIZED);
	return window;
}

void
window_create(window_t* window, unsigned int adapter, const char* title, size_t length, unsigned int width,
              unsigned int height, unsigned int flags) {
	FOUNDATION_UNUSED(length);

	Display* display = XOpenDisplay(0);
	if (!display) {
		log_error(HASH_WINDOW, ERROR_SYSTEM_CALL_FAIL, STRING_CONST("Unable to open X display"));
		return;
	}

	int screen = (adapter != WINDOW_ADAPTER_DEFAULT) ? (int)adapter : DefaultScreen(display);
	XVisualInfo* visual = _get_xvisual(display, screen, 24, 16, 0);
	if (!visual) {
		log_errorf(HASH_WINDOW, ERROR_SYSTEM_CALL_FAIL, STRING_CONST("Unable to get X visual for screen %d"), screen);
		return;
	}

	Colormap colormap = XCreateColormap(display, XRootWindow(display, screen), visual->visual, AllocNone);

	log_debugf(HASH_WINDOW, STRING_CONST("Creating window on screen %d with dimensions %ux%u"), screen, width, height);

	XSetWindowAttributes attrib;
	attrib.colormap = colormap;
	attrib.background_pixel = 0;
	attrib.border_pixel = 0;
	attrib.event_mask = ExposureMask | StructureNotifyMask | ButtonPressMask | ButtonReleaseMask | EnterWindowMask |
	                    LeaveWindowMask | PointerMotionMask | Button1MotionMask | Button2MotionMask |
	                    Button3MotionMask | Button4MotionMask | Button5MotionMask | ButtonMotionMask | KeyPressMask |
	                    KeyReleaseMask | KeymapStateMask | VisibilityChangeMask | FocusChangeMask;
	Window drawable = XCreateWindow(display, XRootWindow(display, screen), 0, 0, (unsigned int)width,
	                                (unsigned int)height, 0, visual->depth, InputOutput, visual->visual,
	                                CWBackPixel | CWBorderPixel | CWColormap | CWEventMask, &attrib);

	XSizeHints* sizehints = XAllocSizeHints();
	if (sizehints) {
		sizehints->base_width = (int)width;
		sizehints->base_height = (int)height;
		sizehints->flags = PBaseSize;
	}
	XSetStandardProperties(display, drawable, title, title, None, 0, 0, sizehints);
	if (sizehints)
		XFree(sizehints);

	if (!(flags & WINDOW_FLAG_NOSHOW)) {
		XMapWindow(display, drawable);
		XRaiseWindow(display, drawable);
		XFlush(display);
	}

	Atom atom_delete = XInternAtom(display, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(display, drawable, &atom_delete, 1);
	XFlush(display);
	XSync(display, False);

	XIC xic = 0;
	XIM xim = XOpenIM(display, 0, 0, 0);
	if (xim) {
		xic = XCreateIC(xim, XNInputStyle, XIMPreeditNone | XIMStatusNone, XNClientWindow, drawable, nullptr);
		if (xic) {
			/*XGetICValues(ic, XNFilterEvents, &fevent, NULL);
			mask = ExposureMask | KeyPressMask | FocusChangeMask;
			XSelectInput(display, window, mask|fevent);*/
		} else {
			log_warn(HASH_WINDOW, WARNING_SUSPICIOUS, STRING_CONST("Unable to create X input context"));
		}
	} else {
		log_warn(HASH_WINDOW, WARNING_SUSPICIOUS, STRING_CONST("Unable to open X input method"));
	}

	window->display = display;
	window->visual = visual;
	window->screen = (unsigned int)screen;
	window->drawable = drawable;
	window->xim = xim;
	window->xic = xic;
	window->created = true;
	window->atom_delete = atom_delete;

	_window_event_add(window);

	window_event_post(WINDOWEVENT_CREATE, window);
}

void*
window_display(window_t* window) {
	return window->display;
}

int
window_screen(window_t* window) {
	return (int)window->screen;
}

unsigned long
window_drawable(window_t* window) {
	return window->drawable;
}

void*
window_visual(window_t* window) {
	return window->visual;
}

void
window_finalize(window_t* window) {
	if (window->created)
		_window_event_remove(window);

	if (window->created && window->drawable) {
		XDestroyWindow(window->display, window->drawable);
		XFlush(window->display);
		XSync(window->display, False);
		window_event_post(WINDOWEVENT_DESTROY, window);
	}
	window->drawable = 0;

	if (window->created && window->visual) {
		XFree(window->visual);
		XSync(window->display, True);
	}
	window->visual = 0;

	if (window->display)
		XCloseDisplay(window->display);
	window->display = 0;
}

void
window_deallocate(window_t* window) {
	window_finalize(window);
	memory_deallocate(window);
}

unsigned int
window_adapter(window_t* window) {
	FOUNDATION_UNUSED(window);
	return WINDOW_ADAPTER_DEFAULT;
}

int
window_screen_width(unsigned int adapter) {
	FOUNDATION_UNUSED(adapter);
	return 800;
}

int
window_screen_height(unsigned int adapter) {
	FOUNDATION_UNUSED(adapter);
	return 600;
}

void
window_maximize(window_t* window) {
	XEvent event = {0};
	Atom atom_wmstate = XInternAtom(window->display, "_NET_WM_STATE", False);
	Atom atom_horizontal = XInternAtom(window->display, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
	Atom atom_vertical = XInternAtom(window->display, "_NET_WM_STATE_MAXIMIZED_VERT", False);

	event.type = ClientMessage;
	event.xclient.window = window->drawable;
	event.xclient.message_type = atom_wmstate;
	event.xclient.format = 32;
	event.xclient.data.l[0] = _NET_WM_STATE_ADD;
	event.xclient.data.l[1] = (long)atom_horizontal;
	event.xclient.data.l[2] = (long)atom_vertical;

	XSendEvent(window->display, XRootWindow(window->display, (int)window->screen), False, SubstructureNotifyMask,
	           &event);
	XFlush(window->display);
	XSync(window->display, False);
}

void
window_minimize(window_t* window) {
	if (window_is_minimized(window))
		return;
	XIconifyWindow(window->display, window->drawable, (int)window->screen);
	XFlush(window->display);
	XSync(window->display, False);
	window_event_post(WINDOWEVENT_RESIZE, window);
}

void
window_restore(window_t* window) {
	if (window_is_minimized(window)) {
		XEvent event = {0};
		Atom atom_changestate = XInternAtom(window->display, "WM_CHANGE_STATE", False);

		event.type = ClientMessage;
		event.xclient.window = window->drawable;
		event.xclient.message_type = atom_changestate;
		event.xclient.format = 32;
		event.xclient.data.l[0] = NormalState;

		XSendEvent(window->display, XRootWindow(window->display, (int)window->screen), False,
		           SubstructureRedirectMask | SubstructureNotifyMask, &event);
		XFlush(window->display);
		XSync(window->display, False);
		window_event_post(WINDOWEVENT_RESIZE, window);
		window_event_post(WINDOWEVENT_REDRAW, window);

		XSetInputFocus(window->display, window->drawable, RevertToParent, CurrentTime);
		XFlush(window->display);
		XSync(window->display, False);

	} else if (window_is_maximized(window)) {
		XEvent event = {0};
		Atom atom_wmstate = XInternAtom(window->display, "_NET_WM_STATE", False);
		Atom atom_horizontal = XInternAtom(window->display, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
		Atom atom_vertical = XInternAtom(window->display, "_NET_WM_STATE_MAXIMIZED_VERT", False);

		event.type = ClientMessage;
		event.xclient.window = window->drawable;
		event.xclient.message_type = atom_wmstate;
		event.xclient.format = 32;
		event.xclient.data.l[0] = _NET_WM_STATE_REMOVE;
		event.xclient.data.l[1] = (long)atom_horizontal;
		event.xclient.data.l[2] = (long)atom_vertical;

		XSendEvent(window->display, XRootWindow(window->display, (int)window->screen), False, SubstructureNotifyMask,
		           &event);
		XFlush(window->display);
		XSync(window->display, False);
	}
}

void
window_resize(window_t* window, int width, int height) {
	window_restore(window);
	XResizeWindow(window->display, window->drawable, (unsigned int)width, (unsigned int)height);
	XFlush(window->display);
	XSync(window->display, False);
}

void
window_move(window_t* window, int x, int y) {
	window_restore(window);
	XMoveWindow(window->display, window->drawable, x, y);
	XFlush(window->display);
	XSync(window->display, False);
}

bool
window_is_open(window_t* window) {
	return window && (window->drawable != 0);
}

bool
window_is_visible(window_t* window) {
	FOUNDATION_UNUSED(window);
	return true;
}

bool
window_is_maximized(window_t* window) {
	Atom atom_wmstate = XInternAtom(window->display, "_NET_WM_STATE", False);
	Atom atom_horizontal = XInternAtom(window->display, "_NET_WM_STATE_MAXIMIZED_HORZ", False);

	Atom actual_type;
	int actual_format;
	unsigned long i, items_count, bytes_after;
	Atom* atoms = 0;
	bool is_maximized = false;

	XGetWindowProperty(window->display, window->drawable, atom_wmstate, 0, 32, False, XA_ATOM, &actual_type,
	                   &actual_format, &items_count, &bytes_after, (unsigned char**)&atoms);
	for (i = 0; i < items_count; ++i) {
		if (atoms[i] == atom_horizontal) {
			is_maximized = true;
			break;
		}
	}

	if (atoms)
		XFree(atoms);
	return is_maximized;
}

bool
window_is_minimized(window_t* window) {
	Atom atom_wmstate = XInternAtom(window->display, "_NET_WM_STATE", False);
	Atom atom_hidden = XInternAtom(window->display, "_NET_WM_STATE_HIDDEN", False);

	Atom actual_type;
	int actual_format;
	unsigned long i, items_count, bytes_after;
	Atom* atoms = 0;
	bool is_minimized = false;

	XGetWindowProperty(window->display, window->drawable, atom_wmstate, 0, 32, False, XA_ATOM, &actual_type,
	                   &actual_format, &items_count, &bytes_after, (unsigned char**)&atoms);
	for (i = 0; i < items_count; ++i) {
		if (atoms[i] == atom_hidden) {
			is_minimized = true;
			break;
		}
	}

	if (atoms)
		XFree(atoms);
	return is_minimized;
}

bool
window_has_focus(window_t* window) {
	Window focus;
	int revert;
	XGetInputFocus(window->display, &focus, &revert);
	return focus == window->drawable;
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
	FOUNDATION_UNUSED(window);
	FOUNDATION_UNUSED(title);
	FOUNDATION_UNUSED(length);
}

unsigned int
window_width(window_t* window) {
	Window root;
	int x, y;
	unsigned int width, height, border, depth;
	if (XGetGeometry(window->display, window->drawable, &root, &x, &y, &width, &height, &border, &depth))
		return width;
	return 0;
}

unsigned int
window_height(window_t* window) {
	Window root;
	int x, y;
	unsigned int width, height, border, depth;
	if (XGetGeometry(window->display, window->drawable, &root, &x, &y, &width, &height, &border, &depth))
		return height;
	return 0;
}

int
window_position_x(window_t* window) {
	Window child;
	int x, y;
	Window root = XRootWindow(window->display, (int)window->screen);
	XTranslateCoordinates(window->display, window->drawable, root, 0, 0, &x, &y, &child);
	return x;
}

int
window_position_y(window_t* window) {
	Window child;
	int x, y;
	Window root = XRootWindow(window->display, (int)window->screen);
	XTranslateCoordinates(window->display, window->drawable, root, 0, 0, &x, &y, &child);
	return y;
}

void
window_fit_to_screen(window_t* window) {
	FOUNDATION_UNUSED(window);
}

int
window_event_loop(void) {
	// TODO: Reimplement as blocking loop
	if (!semaphore_try_wait(&_windows_lock, 0))
		return;
	for (size_t iwin = 0, wsize = array_size(_windows); iwin < wsize; ++iwin) {
		window_t* window = _windows[iwin];
		while (XPending(window->display)) {
			XEvent event;
			XNextEvent(window->display, &event);
			if (True == XFilterEvent(&event, window->drawable))
				continue;

			window_event_post_native(WINDOWEVENT_NATIVE, window, &event);

			XVisibilityEvent* visibility;
			switch (event.type) {
				case ClientMessage:
					if (event.xclient.data.l[0] == (long)window->atom_delete)
						window_event_post(WINDOWEVENT_CLOSE, window);
					break;

				case ConfigureNotify:
					if (window->last_resize != window_event_token) {
						window_event_post(WINDOWEVENT_RESIZE, window);
						window->last_resize = window_event_token;
					}
					if (window->last_paint != window_event_token) {
						window_event_post(WINDOWEVENT_REDRAW, window);
						window->last_paint = window_event_token;
					}
					break;

				case VisibilityNotify:
					visibility = (XVisibilityEvent*)&event;
					if (visibility->state == VisibilityFullyObscured) {
						if (window->visible)
							window_event_post(WINDOWEVENT_HIDE, window);
						window->visible = false;
					} else {
						if (!window->visible) {
							window_event_post(WINDOWEVENT_SHOW, window);
							if (window->last_paint != window_event_token) {
								window_event_post(WINDOWEVENT_REDRAW, window);
								window->last_paint = window_event_token;
							}
						}
						window->visible = true;
					}
					break;

				case FocusIn:
					if (!window->focus)
						window_event_post(WINDOWEVENT_GOTFOCUS, window);
					window->focus = true;
					break;

				case FocusOut:
					if (window->focus)
						window_event_post(WINDOWEVENT_LOSTFOCUS, window);
					window->focus = false;
					break;
			}
		}
	}
	semaphore_post(&_windows_lock);
}

#endif
