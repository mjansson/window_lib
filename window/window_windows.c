/* window_windows.c  -  Window library  -  Public Domain  -  2014 Mattias Jansson / Rampant Pixels
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

#if FOUNDATION_PLATFORM_WINDOWS

#include <foundation/foundation.h>

#include <stdio.h>

static LRESULT WINAPI
_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	window_t* window;
	LRESULT result;

	if (msg == WM_CREATE) {
		FOUNDATION_STATIC_ASSERT(sizeof(LONG_PTR) == sizeof(window_t*), "Type size mismatch");
		window = (window_t*)((CREATESTRUCT*)lparam)->lpCreateParams;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)window);
		window_event_post(WINDOWEVENT_CREATE, window);
		return 0;
	}

	window = (window_t*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	//log_debugf(HASH_WINDOW, STRING_CONST("WND message 0x%x for hwnd 0x%" PRIfixPTR " : window 0x%" PRIfixPTR), msg, hwnd, window);

	window_event_post_native(WINDOWEVENT_NATIVE, window, hwnd, msg, wparam, lparam);

	if (!window)
		goto default_process;

	switch (msg) {
	case WM_NCHITTEST:
		result = DefWindowProc(hwnd, msg, wparam, lparam);
		if ((result == HTBORDER) || (result == HTLEFT) || (result == HTRIGHT) || (result == HTSIZE) ||
		        (result == HTBOTTOM) || (result == HTBOTTOMLEFT) || (result == HTBOTTOMRIGHT) ||
		        (result == HTTOP) || (result == HTTOPLEFT) || (result == HTTOPRIGHT))
			result = HTCLIENT;
		return result;

	case WM_ENTERSIZEMOVE:
		window->is_resizing = true;
		break;

	case WM_EXITSIZEMOVE:
		window->is_resizing = false;
		break;

	case WM_SIZE:
		if (!window->last_resize != window_event_token)
			window_event_post(WINDOWEVENT_RESIZE, window);
		window->last_resize = window_event_token;
		break;

	case WM_SETFOCUS:
		window_event_post(WINDOWEVENT_GOTFOCUS, window);
		break;

	case WM_KILLFOCUS:
		window_event_post(WINDOWEVENT_LOSTFOCUS, window);
		break;

	case WM_SETCURSOR:
		/*if (window_cursor(window)) {
			window_set_cursor(window, window_cursor(window));
			return TRUE;
		}*/
		break;

	case WM_WINDOWPOSCHANGED: {
			WINDOWPOS* wpos = (WINDOWPOS*)lparam;
			if (wpos->flags & SWP_HIDEWINDOW)
				window_event_post(WINDOWEVENT_HIDE, window);
			else if(wpos->flags & SWP_SHOWWINDOW)
				window_event_post(WINDOWEVENT_SHOW, window);
		}
		break;

	case WM_NCPAINT:
	case WM_PAINT:
		if (window->last_paint != window_event_token)
			window_event_post(WINDOWEVENT_REDRAW, window);
		window->last_paint = window_event_token;
		break;

	case WM_CLOSE:
		window_event_post(WINDOWEVENT_CLOSE, window);
		return TRUE;

	case WM_DESTROY:
		window_event_post(WINDOWEVENT_DESTROY, window);
		break;

	case WM_UNICHAR:
		if (wparam == 0xFFFF)  //UNICODE_NOCHAR
			return TRUE;
		return FALSE;

	default:
		break;
	}

default_process:

	return DefWindowProc(hwnd, msg, wparam, lparam);
}

void
window_create(window_t* window, unsigned int adapter, const char* title, size_t length,
              int width, int height, bool show) {
	wchar_t wndclassname[64];
	WNDCLASSW wc;
	RECT rect;

	memset(window, 0, sizeof(window_t));
	window->instance = GetModuleHandle(0);
	window->created = true;
	window->adapter = adapter;
	window->last_paint = -1;
	window->last_resize = -1;

	do {
		static atomic32_t counter = {0};
		_snwprintf_s(wndclassname, sizeof(wndclassname), _TRUNCATE,
		             L"__window_lib_%" FOUNDATION_PREPROCESSOR_JOIN(L, PRIx64) L"%d", time_current(),
		             atomic_incr32(&counter, memory_order_relaxed));
		wc.lpfnWndProc    = (WNDPROC)_window_proc;
		wc.cbClsExtra     = 0;
		wc.cbWndExtra     = 0;
		wc.hInstance      = (HINSTANCE)window->instance;
		wc.style          = CS_OWNDC;
		wc.hIcon          = LoadIcon((HINSTANCE)window->instance, MAKEINTRESOURCE(102));
		wc.hCursor        = LoadCursor(0, IDC_ARROW);
		wc.hbrBackground  = CreateSolidBrush(RGB(0, 0, 0));
		wc.lpszMenuName   = 0;
		wc.lpszClassName  = wndclassname;
		if (!wc.hIcon)
			wc.hIcon      = LoadIcon(0, IDI_WINLOGO);
	}
	while (!RegisterClassW(&wc));

	window->wstyle = WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	if (true/*!fullscreen*/) {
		//if( _flags & NOSYSTEMMENU )
		//	window->wstyle |= WS_OVERLAPPED;
		//else
		//window->wstyle |= WS_OVERLAPPEDWINDOW;
		window->wstyle |= WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME;
	}
	else {
		window->wstyle |= WS_POPUP;
	}

	rect.left   = 0;
	rect.top    = 0;
	rect.right  = width;
	rect.bottom = height;

	//if( !fullscreen )
	{
		AdjustWindowRect(&rect, window->wstyle, FALSE);
		rect.right  -= rect.left;
		rect.bottom -= rect.top;
		rect.left    = 0;
		rect.top     = 0;

		int pad_x = rect.right - width;
		int pad_y = rect.bottom - height;

		//Constrain to screen and maintain aspect
		int screen_width = window_screen_width(adapter);
		int screen_height = window_screen_height(adapter);

		if ((screen_width < rect.right) || (screen_height < rect.bottom)) {
			int new_width = screen_width - pad_x;
			int new_height = screen_height - pad_y;

			real width_factor = (real)new_width / (real)width;
			real height_factor = (real)new_height / (real)height;

			if ((width_factor < height_factor) && (width_factor < 1)) {
				width = new_width;
				height = (int)((real)height * width_factor);
				rect.right  = width + pad_x;
				rect.bottom = height + pad_y;
			}
			else if (height_factor < 1) {
				width = (int)((real)width * height_factor);
				height = new_height;
				rect.right  = width + pad_x;
				rect.bottom = height + pad_y;
			}
		}
	}

	if (adapter != WINDOW_ADAPTER_DEFAULT) {
		FOUNDATION_ASSERT_FAIL("Not implemented");
		/*MONITORINFOEX info;
		memset( &info, 0, sizeof( MONITORINFOEX ) );
		info.cbSize = sizeof( MONITORINFOEX );
		GetMonitorInfo( (HMONITOR)_adapter.hmonitor, (MONITORINFO*)&info );
		rect.left   += info.rcWork.left;
		rect.top    += info.rcWork.top;*/
		rect.left = CW_USEDEFAULT;
		rect.top  = CW_USEDEFAULT;
	}
	else {
		rect.left = CW_USEDEFAULT;
		rect.top  = CW_USEDEFAULT;
	}

	wchar_t* titlestr = wstring_allocate_from_string(title, length);
	window->hwnd = CreateWindowExW(/*fullscreen ? WS_EX_TOPMOST :*/ 0, wndclassname, titlestr,
	               window->wstyle, rect.left, rect.top, rect.right, rect.bottom, 0, 0,
	               (HINSTANCE)window->instance, window);
	wstring_deallocate(titlestr);
	if (!window->hwnd) {
		int err = system_error();
		string_const_t errmsg = system_error_message(err);
		log_errorf(HASH_WINDOW, ERROR_SYSTEM_CALL_FAIL, "Unable to create window: %.*s (%d)",
		           STRING_FORMAT(errmsg), err);
		window_finalize(window);
		return;
	}

	if (show) {
		ShowWindow((HWND)window->hwnd, SW_SHOW);
		if (thread_is_main())
			window_event_process();
	}
}

window_t*
window_allocate(void* hwnd) {
	window_t* window = memory_allocate(HASH_WINDOW, sizeof(window_t), 0,
	                                   MEMORY_PERSISTENT | MEMORY_ZERO_INITIALIZED);
	window_initialize(window, hwnd);
	return window;
}

void
window_initialize(window_t* window, void* hwnd) {
	window->instance = GetModuleHandle(0);
	window->created = false;
	window->adapter = WINDOW_ADAPTER_DEFAULT; //TODO: Get the corresponding adapter for the window
	window->wstyle = 0; //TODO: Get wstyle from the window
	window->hwnd = hwnd;
	window->last_paint = -1;
	window->last_resize = -1;
}

void*
window_hwnd(window_t* window) {
	return window ? window->hwnd : 0;
}

void*
window_hinstance(window_t* window) {
	return window ? window->instance : 0;
}

void*
window_hdc(window_t* window) {
	return (window && window->hwnd ? GetDC((HWND)window->hwnd) : 0);
}

void
window_release_hdc(void* hwnd, void* hdc) {
	if (hdc)
		ReleaseDC((HWND)hwnd, (HDC)hdc);
}

int
window_screen_width(unsigned int adapter) {
	//if( !adapter )
	return GetSystemMetrics(SM_CXSCREEN);
	//else not implemented, use GetDeviceCaps?
}

int
window_screen_height(unsigned int adapter) {
	//if( !adapter )
	return GetSystemMetrics(SM_CYSCREEN);
	//else not implemented, use GetDeviceCaps?
}

void
window_finalize(window_t* window) {
	if (window->created) {
		void* hwnd = window->hwnd;
		window->hwnd = 0;
		if (hwnd)
			DestroyWindow((HWND)hwnd);
	}
}

void
window_deallocate(window_t* window) {
	window_finalize(window);
	memory_deallocate(window);
}

unsigned int
window_adapter(window_t* window) {
	return window->adapter;
}

void
window_maximize(window_t* window) {
	ShowWindow((HWND)window->hwnd, SW_MAXIMIZE);
}

void
window_minimize(window_t* window) {
	ShowWindow((HWND)window->hwnd, SW_MINIMIZE);
}

void
window_restore(window_t* window) {
	ShowWindow((HWND)window->hwnd, SW_RESTORE);
}

void
window_resize(window_t* window, int width, int height) {
	RECT rect = {0};
	if (window_is_maximized(window))
		window_restore(window);
	rect.right = width;
	rect.bottom = height;
	AdjustWindowRect(&rect, window->wstyle, FALSE);
	SetWindowPos((HWND)window->hwnd, 0, 0, 0, rect.right - rect.left, rect.bottom - rect.top,
	             SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOZORDER);
}

void
window_move(window_t* window, int x, int y) {
	if (window_is_maximized(window))
		window_restore(window);
	SetWindowPos((HWND)window->hwnd, 0, x, y, 0, 0,
	             SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOZORDER);
}

bool
window_is_open(window_t* window) {
	return (window && window->hwnd);
}

bool
window_is_visible(window_t* window) {
	return (window->hwnd && IsWindowVisible((HWND)window->hwnd));
}

bool
window_is_maximized(window_t* window) {
	WINDOWPLACEMENT plc;
	memset(&plc, 0, sizeof(WINDOWPLACEMENT));
	plc.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement((HWND)window->hwnd, &plc);
	return (plc.showCmd == SW_SHOWMAXIMIZED);
}

bool
window_is_minimized(window_t* window) {
	WINDOWPLACEMENT plc;
	memset(&plc, 0, sizeof(WINDOWPLACEMENT));
	plc.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement((HWND)window->hwnd, &plc);
	return (plc.showCmd == SW_MINIMIZE) || (plc.showCmd == SW_SHOWMINIMIZED) ||
	       (plc.showCmd == SW_HIDE);
}

bool
window_has_focus(window_t* window) {
	return (GetForegroundWindow() == window->hwnd);
}

void
window_show_cursor(window_t* window, bool show, bool lock) {
	ShowCursor(show ? TRUE : FALSE);

	/*if( show && window_cursor( window ) )
		window_set_cursor( window, window_cursor( window ) );*/

	if (!window->cursor_lock && lock) {
		POINT pt;

		window->cursor_lock = lock;

		GetCursorPos(&pt);
		ScreenToClient((HWND)window->hwnd, &pt);

		window->cursor_pos_x = pt.x;
		window->cursor_pos_y = pt.y;
	}
}

void
window_set_cursor_pos(window_t* window, int x, int y) {
	POINT pt;
	pt.x = x;
	pt.y = y;
	ClientToScreen((HWND)window->hwnd, &pt);
	SetCursorPos(pt.x, pt.y);
}

bool
window_is_cursor_locked(window_t* window) {
	return window->cursor_lock;
}

void
window_set_title(window_t* window, const char* title, size_t length) {
	wchar_t* wstr = wstring_allocate_from_string(title, length);
	SetWindowTextW(window->hwnd, wstr);
	wstring_deallocate(wstr);
}

int
window_width(window_t* window) {
	RECT rect;
	GetClientRect((HWND)window->hwnd, &rect);
	return rect.right - rect.left;
}

int
window_height(window_t* window) {
	RECT rect;
	GetClientRect((HWND)window->hwnd, &rect);
	return rect.bottom - rect.top;
}

int
window_position_x(window_t* window) {
	RECT rect;
	GetWindowRect((HWND)window->hwnd, &rect);
	return rect.left;
}

int
window_position_y(window_t* window) {
	RECT rect;
	GetWindowRect((HWND)window->hwnd, &rect);
	return rect.top;
}

void
window_fit_to_screen(window_t* window) {
}

#endif
