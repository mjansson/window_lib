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

#include <foundation/memory.h>
#include <foundation/time.h>
#include <foundation/system.h>
#include <foundation/thread.h>
#include <foundation/log.h>
#include <foundation/atomic.h>
#include <foundation/windows.h>


static LRESULT WINAPI _window_proc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
	if( msg == WM_CREATE )
	{
		SetWindowLongPtr( hwnd, GWLP_USERDATA, (LONG_PTR)((CREATESTRUCT*)lparam)->lpCreateParams );
		return 0;
	}

	//debug_logf( "WND message 0x%x for window 0x%p", msg, hwnd );

	window_t* window = (window_t*)GetWindowLongPtr( hwnd, GWLP_USERDATA );

	//_input_service_process_native( hwnd, msg, wparam, lparam );

	if( window ) switch( msg )
	{
		case WM_NCHITTEST:
		{
			LRESULT result = DefWindowProc( hwnd, msg, wparam, lparam );
			if( ( result == HTBORDER ) || ( result == HTLEFT ) || ( result == HTRIGHT ) || ( result == HTSIZE ) ||
				( result == HTBOTTOM ) || ( result == HTBOTTOMLEFT ) || ( result == HTBOTTOMRIGHT ) ||
				( result == HTTOP ) || ( result == HTTOPLEFT ) || ( result == HTTOPRIGHT ) )
				result = HTCLIENT;
			return result;
		}

		case WM_ENTERSIZEMOVE:
		{
			window->is_resizing = true;
			break;
		}

		case WM_EXITSIZEMOVE:
		{
			RECT rect;
			int width, height;

			window->is_resizing = false;
			if( !window_is_visible( window ) )
				window_event_post( WINDOWEVENT_SHOW, window );
			GetClientRect( hwnd, &rect );
			width  = rect.right  - rect.left;
			height = rect.bottom - rect.top;
			if( ( width != 0 ) || ( height != 0 ) )
			{
				int size_x = window_width( window );
				int size_y = window_height( window );
				if( ( width != size_x ) || ( height != size_y ) )
				{
					window_event_post( WINDOWEVENT_RESIZE, window );
					window_event_post( WINDOWEVENT_REDRAW, window );
				}
			}
			break;
		}

		case WM_SETFOCUS:
		{
			log_infof( HASH_WINDOW, "WM_SETFOCUS: %s", window_has_focus( window ) ? "window already has focus" : "gain focus" );
			//if( !window_has_focus( window ) )
			{
				window_event_post( WINDOWEVENT_GOTFOCUS, window );
				window_event_post( WINDOWEVENT_REDRAW, window );
			}
			break;
		}

		case WM_KILLFOCUS:
		{
			log_infof( HASH_WINDOW, "WM_KILLFOCUS: %s", window_has_focus( window ) ? "window already unfocused" : "lost focus" );
			//if( window_has_focus( window ) )
			{
				window_event_post( WINDOWEVENT_LOSTFOCUS, window );
			}
			break;
		}

		case WM_SETCURSOR:
		{
			/*if( window_cursor() )
			{
				window_set_cursor( window_cursor() );
				return TRUE;
			}*/
			break;
		}

		case WM_SIZE: 
		{
			int width, height;

			if( wparam == SIZE_MINIMIZED )
			{
				window_event_post( WINDOWEVENT_HIDE, window );			
				break;
			}

			if( !window_is_visible( window ) )
			{
				window_event_post( WINDOWEVENT_SHOW, window );			
				window_event_post( WINDOWEVENT_REDRAW, window );			
			}

			width  = LOWORD( lparam );
			height = HIWORD( lparam );
			if( ( !window->is_resizing || ( wparam == SIZE_MAXIMIZED ) ) && ( ( width != 0 ) || ( height != 0 ) ) )
			{
				int size_x = window_width( window );
				int size_y = window_height( window );
				if( ( wparam == SIZE_MAXIMIZED ) || ( width != size_x ) || ( height != size_y ) )
				{
					//resize.setParameter( "width",  core::EventParameter( ( width  > 0 ) ? width  : 1 ) );
					//resize.setParameter( "height", core::EventParameter( ( height > 0 ) ? height : 1 ) );
					window_event_post( WINDOWEVENT_RESIZE, window );
					window_event_post( WINDOWEVENT_REDRAW, window );
				}
			}

			break;
		}

		case WM_ACTIVATEAPP:
		{
			if( !window_has_focus( window ) && ( wparam == TRUE ) )
			{
				window_event_post( WINDOWEVENT_GOTFOCUS, window );
				window_event_post( WINDOWEVENT_REDRAW, window );
			}
			else if( window_has_focus( window ) && ( wparam == FALSE ) )
			{
				window_event_post( WINDOWEVENT_LOSTFOCUS, window );
			}
			break;
		}

		case WM_PAINT:
		{
			window_event_post( WINDOWEVENT_REDRAW, window );
			break;
		}

		case WM_CLOSE:
		{
			window_event_post( WINDOWEVENT_CLOSE, window );
			return TRUE;
		}

		case WM_DESTROY:
		{
			//window_event_post( WINDOWEVENT_CLOSE, window );
			break;
		}

		case WM_UNICHAR:
		{
			if( wparam == 0xFFFF ) //UNICODE_NOCHAR
				return TRUE;
			return FALSE;
		}

		default:
			break;
	}

	return DefWindowProc( hwnd, msg, wparam, lparam );
}


window_t* window_create( unsigned int adapter, const char* title, unsigned int width, unsigned int height, bool show )
{
	wchar_t wndclassname[64];
	window_t* window;
	WNDCLASSW wc;
	RECT rect;
	
	window = memory_allocate( HASH_WINDOW, sizeof( window_t ), 0, MEMORY_PERSISTENT | MEMORY_ZERO_INITIALIZED );
	window->instance     = GetModuleHandle(0);
	window->created      = true;
	window->adapter      = adapter;

	do
	{
		static atomic32_t counter = {0};
		wsprintf( wndclassname, L"__neo_wnd_%lld%d", time_current(), atomic_incr32( &counter ) );
		wc.lpfnWndProc    = (WNDPROC)_window_proc;
		wc.cbClsExtra     = 0;
		wc.cbWndExtra     = 0;
		wc.hInstance      = (HINSTANCE)window->instance;
		wc.style          = CS_OWNDC;
		wc.hIcon          = LoadIcon( (HINSTANCE)window->instance, MAKEINTRESOURCE(102) );
		wc.hCursor        = LoadCursor( 0, IDC_ARROW );
		wc.hbrBackground  = CreateSolidBrush( RGB( 0, 0, 0 ) );
		wc.lpszMenuName   = 0;
		wc.lpszClassName  = wndclassname;

		if( !wc.hIcon )
			wc.hIcon      = LoadIcon( 0, IDI_WINLOGO );

	} while( !RegisterClassW( &wc ) );
	
	window->wstyle = WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	if( true/*!fullscreen*/ )
	{
		//if( _flags & NOSYSTEMMENU )
		//	window->wstyle |= WS_OVERLAPPED;
		//else
			//window->wstyle |= WS_OVERLAPPEDWINDOW;
			window->wstyle |= WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME;
	}
	else
	{
		window->wstyle |= WS_POPUP;
	}

	rect.left   = 0;
	rect.top    = 0;
	rect.right  = width;
	rect.bottom = height;

	//if( !fullscreen )
	{
		AdjustWindowRect( &rect, window->wstyle, FALSE );
		rect.right  -= rect.left;
		rect.bottom -= rect.top;
		rect.left    = 0;
		rect.top     = 0;

		unsigned int pad_x = rect.right - width;
		unsigned int pad_y = rect.bottom - height;

		//Constrain to screen and maintain aspect
		unsigned int screen_width = window_screen_width( adapter );
		unsigned int screen_height = window_screen_height( adapter );

		if( ( screen_width < (unsigned int)rect.right ) || ( screen_height < (unsigned int)rect.bottom ) )
		{
			unsigned int new_width = screen_width - pad_x;
			unsigned int new_height = screen_height - pad_y;
				
			real width_factor = (real)new_width / (real)width;
			real height_factor = (real)new_height / (real)height;

			if( ( width_factor < height_factor ) && ( width_factor < 1 ) )
			{
				width = new_width;
				height = (unsigned int)( (real)height * width_factor );
				rect.right  = width + pad_x;
				rect.bottom = height + pad_y;
			}
			else if( height_factor < 1 )
			{
				width = (unsigned int)( (real)width * height_factor );
				height = new_height;
				rect.right  = width + pad_x;
				rect.bottom = height + pad_y;
			}
		}
	}

	if( adapter != WINDOW_ADAPTER_DEFAULT )
	{
		FOUNDATION_ASSERT_FAIL( "Not implemented" );
		/*MONITORINFOEX info;
		memset( &info, 0, sizeof( MONITORINFOEX ) );
		info.cbSize = sizeof( MONITORINFOEX );
		GetMonitorInfo( (HMONITOR)_adapter.hmonitor, (MONITORINFO*)&info );
		rect.left   += info.rcWork.left;
		rect.top    += info.rcWork.top;*/
		rect.left = CW_USEDEFAULT;
		rect.top  = CW_USEDEFAULT;
	}
	else
	{
		rect.left = CW_USEDEFAULT;
		rect.top  = CW_USEDEFAULT;
	}

	if( !title || !strlen( title ) )
		title = "Untitled";
	
	wchar_t* wtitle = wstring_allocate_from_string( title, 0 );
	window->hwnd = CreateWindowExW( /*fullscreen ? WS_EX_TOPMOST :*/ 0, wndclassname, wtitle, window->wstyle, rect.left, rect.top, rect.right, rect.bottom, 0, 0, (HINSTANCE)window->instance, window );
	wstring_deallocate( wtitle );
	if( !window->hwnd )
	{
		log_errorf( HASH_WINDOW, ERROR_SYSTEM_CALL_FAIL, "Unable to create window: %s", system_error_message( GetLastError() ) );
		window_deallocate( window );
		return 0;
	}
		
	if( show )
	{
		ShowWindow( (HWND)window->hwnd, SW_SHOW );
		if( thread_is_main() )
			window_event_process();
	}

	return window;
}


window_t* window_allocate_from_hwnd( void* hwnd )
{
	window_t* window = memory_allocate( HASH_WINDOW, sizeof( window_t ), 0, MEMORY_PERSISTENT | MEMORY_ZERO_INITIALIZED );
	window->instance     = GetModuleHandle(0);
	window->created      = false;
	window->adapter      = WINDOW_ADAPTER_DEFAULT; //TODO: Get the corresponding adapter for the window
	window->wstyle       = 0; //TODO: Get wstyle from the window
	window->hwnd         = hwnd;

	return window;
}


void* window_hwnd( window_t* window )
{
	return window ? window->hwnd : 0;
}


void* window_hinstance( window_t* window )
{
	return window ? window->instance : 0;
}


void* window_hdc( window_t* window )
{
	return ( window && window->hwnd ? GetDC( (HWND)window->hwnd ) : 0 );
}


void window_release_hdc( void* hwnd, void* hdc )
{
	if( hdc )
		ReleaseDC( (HWND)hwnd, (HDC)hdc );
}


unsigned int window_screen_width( unsigned int adapter )
{
	//if( !adapter )
		return GetSystemMetrics( SM_CXSCREEN );
	//else not implemented, use GetDeviceCaps?
}


unsigned int window_screen_height( unsigned int adapter )
{
	//if( !adapter )
		return GetSystemMetrics( SM_CYSCREEN );
	//else not implemented, use GetDeviceCaps?
}


void window_deallocate( window_t* window )
{
	if ( window && window->created )
	{
		void* hwnd = window->hwnd;
		window->hwnd = 0;
		if( hwnd )
			DestroyWindow( (HWND)hwnd );
	}

	memory_deallocate( window );
}


unsigned int window_adapter( window_t* window )
{
	return window ? window->adapter : WINDOW_ADAPTER_DEFAULT;
}


void window_maximize( window_t* window )
{
	if( window && window->hwnd  )
		ShowWindow( (HWND)window->hwnd, SW_MAXIMIZE );
}


void window_minimize( window_t* window )
{
	if( window && window->hwnd )
		ShowWindow( (HWND)window->hwnd, SW_MINIMIZE );
}


void window_restore( window_t* window )
{
	if( window && window->hwnd )
		ShowWindow( (HWND)window->hwnd, SW_RESTORE );
}


void window_resize( window_t* window, unsigned int width, unsigned int height )
{
	if( window && window->hwnd )
	{
		RECT rect = {0};
		if( window_is_maximized( window ) )
			window_restore( window );
		rect.right = width;
		rect.bottom = height;
		AdjustWindowRect( &rect, window->wstyle, FALSE );
		SetWindowPos( (HWND)window->hwnd, 0, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOZORDER );
	}
}


void window_move( window_t* window, int x, int y )
{
	if( window && window->hwnd )
	{
		if( window_is_maximized( window ) )
			window_restore( window );
		SetWindowPos( (HWND)window->hwnd, 0, x, y, 0, 0, SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOZORDER );
	}
}


bool window_is_open( window_t* window )
{
	return ( window && window->hwnd );
}


bool window_is_visible( window_t* window )
{
	return ( window && window->hwnd && IsWindowVisible( (HWND)window->hwnd ) );
}


bool window_is_maximized( window_t* window )
{
	if( window && window->hwnd )
	{
		WINDOWPLACEMENT plc;
		memset( &plc, 0, sizeof( WINDOWPLACEMENT ) );
		plc.length = sizeof( WINDOWPLACEMENT );
		GetWindowPlacement( (HWND)window->hwnd, &plc );
		return ( plc.showCmd == SW_SHOWMAXIMIZED );
	}
	return false;
}


bool window_is_minimized( window_t* window )
{
	if( window && window->hwnd )
	{
		WINDOWPLACEMENT plc;
		memset( &plc, 0, sizeof( WINDOWPLACEMENT ) );
		plc.length = sizeof( WINDOWPLACEMENT );
		GetWindowPlacement( (HWND)window->hwnd, &plc );
		return ( plc.showCmd == SW_MINIMIZE ) || ( plc.showCmd == SW_SHOWMINIMIZED ) || ( plc.showCmd == SW_HIDE );
	}
	return false;
}


bool window_has_focus( window_t* window )
{
	return ( window && window->hwnd && ( GetForegroundWindow() == window->hwnd ) );
}


void window_show_cursor( window_t* window, bool show, bool lock )
{
	if( !window || !window->hwnd )
		return;

	ShowCursor( show ? TRUE : FALSE );

	/*if( show && window_cursor( window ) )
		window_set_cursor( window, window_cursor( window ) );*/

	if( !window->cursor_lock && lock )
	{
		POINT pt;

		window->cursor_lock = lock;

		GetCursorPos( &pt );
		ScreenToClient( (HWND)window->hwnd, &pt );

		window->cursor_pos_x = pt.x;
		window->cursor_pos_y = pt.y;
	}
}


void window_set_cursor_pos( window_t* window, int x, int y )
{
	if( !window || !window->hwnd )
		return;

	POINT pt;
	pt.x = x;
	pt.y = y;
	ClientToScreen( (HWND)window->hwnd, &pt );
	SetCursorPos( pt.x, pt.y );
}


bool window_is_cursor_locked( window_t* window )
{
	return ( window && window->hwnd && window->cursor_lock );
}


void window_set_title( window_t* window, const char* title )
{
	if( window && window->hwnd )
	{
		wchar_t* wstr = wstring_allocate_from_string( title, 0 );
		SetWindowTextW( window->hwnd, wstr );
		wstring_deallocate( wstr );
	}
}


int window_width( window_t* window )
{
	if( window && window->hwnd )
	{
		RECT rect;
		GetClientRect( (HWND)window->hwnd, &rect );
		return rect.right - rect.left;
	}

	return 0;
}


int window_height( window_t* window )
{
	if( window && window->hwnd )
	{
		RECT rect;
		GetClientRect( (HWND)window->hwnd, &rect );
		return rect.bottom - rect.top;
	}

	return 0;
}


int window_position_x( window_t* window )
{
	if( window && window->hwnd )
	{
		RECT rect;
		GetWindowRect( (HWND)window->hwnd, &rect );
		return rect.left;
	}

	return 0;
}


int window_position_y( window_t* window )
{
	if( window && window->hwnd )
	{
		RECT rect;
		GetWindowRect( (HWND)window->hwnd, &rect );
		return rect.top;
	}

	return 0;
}


void window_fit_to_screen( window_t* window )
{
}


#endif
