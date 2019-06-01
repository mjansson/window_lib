/* event.c  -  Window library events  -  Public Domain  -  2014 Mattias Jansson / Rampant Pixels
 *
 * This library provides a cross-platform window library in C11 providing basic support data types
 * and functions to create and manage windows in a platform-independent fashion. The latest source
 * code is always available at
 *
 * https://github.com/rampantpixels/window_lib
 *
 * This library is put in the public domain; you can redistribute it and/or modify it without any
 * restrictions.
 *
 */

#include <window/window.h>
#include <window/internal.h>

#include <foundation/array.h>
#include <foundation/event.h>
#include <foundation/semaphore.h>
#include <foundation/log.h>

#if FOUNDATION_PLATFORM_WINDOWS
#include <foundation/windows.h>
#endif

tick_t window_event_token;

static event_stream_t* _window_stream = 0;

bool _window_app_started = false;
bool _window_app_paused = true;

#if FOUNDATION_PLATFORM_LINUX
static semaphore_t _windows_lock;
static window_t** _windows;
#endif

int
_window_event_initialize(void) {
	_window_stream = event_stream_allocate(1024);
	window_event_token = 1;
#if FOUNDATION_PLATFORM_LINUX
	semaphore_initialize(&_windows_lock, 1);
	_windows = 0;
#endif
	return 0;
}

void
_window_event_finalize(void) {
#if FOUNDATION_PLATFORM_LINUX
	array_deallocate(_windows);
	semaphore_finalize(&_windows_lock);
#endif
	event_stream_deallocate(_window_stream);
}

#if FOUNDATION_PLATFORM_LINUX

void
_window_event_add(window_t* window) {
	semaphore_wait(&_windows_lock);
	array_push(_windows, window);
	semaphore_post(&_windows_lock);
}

void
_window_event_remove(window_t* window) {
	semaphore_wait(&_windows_lock);
	for (size_t iwin = 0, wsize = array_size(_windows); iwin < wsize; ++iwin) {
		if (_windows[iwin] == window) {
			array_erase(_windows, iwin);
			break;
		}
	}
	semaphore_post(&_windows_lock);
}

#endif

void
window_event_post(window_event_id id, window_t* window) {
	if (_window_stream)
		event_post(_window_stream, (int)id, 0, 0, &window, sizeof(window_t*));
}

#if FOUNDATION_PLATFORM_WINDOWS

void
window_event_post_native(window_event_id id, window_t* window, void* hwnd, unsigned int msg,
                         uintptr_t wparam, uintptr_t lparam) {
	FOUNDATION_UNUSED(msg);
	if (_window_stream)
		event_post_varg(_window_stream, (int)id, 0, 0, &window, sizeof(window_t*), &hwnd,
		                sizeof(void*), &msg, sizeof(unsigned int), &wparam, sizeof(uintptr_t),
		                &lparam, sizeof(uintptr_t), nullptr, nullptr);
}

#elif FOUNDATION_PLATFORM_LINUX

void
window_event_post_native(window_event_id id, window_t* window, void* xevent) {
	if (_window_stream)
		event_post_varg(_window_stream, (int)id, 0, 0, &window, sizeof(window_t*), xevent,
		                sizeof(XEvent), nullptr, nullptr);
}

#endif

const window_t*
window_event_window(const event_t* event) {
	return *(const window_t* const*)&event->payload[0];
}

void
window_event_process(void) {
#if FOUNDATION_PLATFORM_WINDOWS
	MSG msg;
	BOOL got;
	while ((got = PeekMessage(&msg, (HWND)0, 0U, 0U, PM_REMOVE | PM_NOYIELD)) != 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		if (msg.message >= WM_MOUSEFIRST)
			break;
	}
#elif FOUNDATION_PLATFORM_LINUX
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
#endif
	window_event_token++;
}

event_stream_t*
window_event_stream(void) {
	return _window_stream;
}

void
window_event_handle(event_t* event) {
	if (event->id == FOUNDATIONEVENT_START) {
		_window_app_started = true;
		_window_app_paused = false;
	} else if (event->id == FOUNDATIONEVENT_PAUSE)
		_window_app_paused = true;
	else if (event->id == FOUNDATIONEVENT_RESUME)
		_window_app_paused = false;
}
