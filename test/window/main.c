/* main.c  -  Window test  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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
#include <foundation/delegate.h>
#include <window/window.h>
#include <test/test.h>

static application_t
test_window_application(void) {
	application_t app;
	memset(&app, 0, sizeof(app));
	app.name = string_const(STRING_CONST("Window tests"));
	app.short_name = string_const(STRING_CONST("test_window"));
	app.company = string_const(STRING_CONST("Rampant Pixels"));
	app.version = window_module_version();
	app.exception_handler = test_exception_handler;
	return app;
}

static memory_system_t
test_window_memory_system(void) {
	return memory_system_malloc();
}

static foundation_config_t
test_window_config(void) {
	foundation_config_t config;
	memset(&config, 0, sizeof(config));
	return config;
}

static int
test_window_initialize(void) {
	window_config_t config;
	memset(&config, 0, sizeof(config));
	return window_module_initialize(config);
}

static void
test_window_finalize(void) {
	window_module_finalize();
}

DECLARE_TEST(window, createdestroy) {
	window_t* window = 0;
	event_stream_t* stream;
	event_block_t* block;
	event_t* event;
	int got_create, got_destroy, got_show, got_hide, got_focus, got_unfocus, got_redraw, got_resize;
	int got_other;
#if FOUNDATION_PLATFORM_WINDOWS
	window = window_create(WINDOW_ADAPTER_DEFAULT, STRING_CONST("Window test"), 800, 600, true);
#elif FOUNDATION_PLATFORM_LINUX
	window = window_create(WINDOW_ADAPTER_DEFAULT, STRING_CONST("Window test"), 800, 600, true);
#elif FOUNDATION_PLATFORM_MACOSX
	window = window_allocate(delegate_nswindow());
#elif FOUNDATION_PLATFORM_IOS
	window = window_allocate(delegate_uiwindow());
#endif

	EXPECT_NE(window, 0);
	EXPECT_TRUE(window_is_open(window));

	window_event_process();
	stream = window_event_stream();
	block = event_stream_process(stream);
	event = 0;
	got_create = got_show = got_focus = got_redraw = got_resize = 0;
	got_other = 0;
	while ((event = event_next(block, event))) {
		switch (event->id) {
		case WINDOWEVENT_CREATE:
			got_create++;
			break;
		case WINDOWEVENT_GOTFOCUS:
			got_focus++;
			break;
		case WINDOWEVENT_SHOW:
			got_show++;
			break;
		case WINDOWEVENT_REDRAW:
			got_redraw++;
			break;
		case WINDOWEVENT_RESIZE:
			got_resize++;
			break;
		default:
			got_other++;
			break;
		}
	}
	EXPECT_INTEQ(got_create, 1);
	EXPECT_INTEQ(got_show, 1);
	EXPECT_INTEQ(got_focus, 1);
	EXPECT_INTEQ(got_redraw, 1);
	EXPECT_INTEQ(got_other, 0);

	window_deallocate(window);
	window = 0;

	window_event_process();
	block = event_stream_process(stream);
	event = 0;
	got_destroy = got_hide = got_unfocus = 0;
	got_other = 0;
	while ((event = event_next(block, event))) {
		switch (event->id) {
		case WINDOWEVENT_DESTROY:
			got_destroy++;
			break;
		case WINDOWEVENT_HIDE:
			got_hide++;
			break;
		case WINDOWEVENT_LOSTFOCUS:
			got_unfocus++;
			break;
		default:
			got_other++;
			break;
		}
	}
	EXPECT_INTEQ(got_destroy, 1);
	EXPECT_INTLE(got_unfocus, 1); //Potential event
	EXPECT_INTEQ(got_hide, 1);
	EXPECT_INTEQ(got_other, 0);

	EXPECT_FALSE(window_is_open(window));

	return 0;
}

DECLARE_TEST(window, sizemove) {
	window_t* window = 0;
	event_stream_t* stream;
	event_block_t* block;
	event_t* event;
#if !FOUNDATION_PLATFORM_IOS && !FOUNDATION_PLATFORM_ANDROID
	int got_resize, got_redraw, got_focus, got_unfocus;
	int got_other;
#endif

#if FOUNDATION_PLATFORM_WINDOWS
	window = window_create(WINDOW_ADAPTER_DEFAULT, STRING_CONST("Window test"), 800, 600, true);
#elif FOUNDATION_PLATFORM_MACOSX
	window = window_allocate(delegate_nswindow());
#elif FOUNDATION_PLATFORM_IOS
	window = window_allocate(delegate_uiwindow());
#endif

	window_event_process();
	stream = window_event_stream();
	block = event_stream_process(stream);

	EXPECT_NE(window, 0);
	EXPECT_TRUE(window_is_open(window));

	thread_sleep(1000);

#if FOUNDATION_PLATFORM_IOS || FOUNDATION_PLATFORM_ANDROID
	EXPECT_TRUE(window_is_maximized(window));
#else
	EXPECT_FALSE(window_is_maximized(window));
#endif

	EXPECT_TRUE(window_is_visible(window));
	EXPECT_TRUE(window_has_focus(window));

	window_maximize(window);
	window_event_process();

#if !(FOUNDATION_PLATFORM_IOS || FOUNDATION_PLATFORM_ANDROID)
	block = event_stream_process(stream);
	event = 0;
	got_resize = got_redraw = 0;
	got_other = 0;
	while ((event = event_next(block, event))) {
		switch (event->id) {
		case WINDOWEVENT_RESIZE:
			got_resize++;
			break;
		case WINDOWEVENT_REDRAW:
			got_redraw++;
			break;
		default:
			got_other++;
			break;
		}
	}
	EXPECT_INTEQ(got_resize, 1);
	EXPECT_INTEQ(got_redraw, 1);
	EXPECT_INTEQ(got_other, 0);
#endif

	thread_sleep(1000);
	EXPECT_TRUE(window_is_maximized(window));
	EXPECT_TRUE(window_has_focus(window));

#if !FOUNDATION_PLATFORM_IOS && !FOUNDATION_PLATFORM_ANDROID
	window_restore(window);
	window_event_process();
	block = event_stream_process(stream);
	event = 0;
	got_resize = got_redraw = 0;
	got_other = 0;
	while ((event = event_next(block, event))) {
		switch (event->id) {
		case WINDOWEVENT_RESIZE:
			got_resize++;
			break;
		case WINDOWEVENT_REDRAW:
			got_redraw++;
			break;
		default:
			got_other++;
			break;
		}
	}
	EXPECT_INTEQ(got_resize, 1);
	EXPECT_INTEQ(got_redraw, 1);
	EXPECT_INTEQ(got_other, 0);

	thread_sleep(1000);
	EXPECT_FALSE(window_is_maximized(window));
	EXPECT_TRUE(window_has_focus(window));
#endif

	window_maximize(window);
	window_event_process();
	block = event_stream_process(stream);
	thread_sleep(1000);
	EXPECT_TRUE(window_is_maximized(window));

#if !FOUNDATION_PLATFORM_IOS && !FOUNDATION_PLATFORM_ANDROID
	window_resize(window, 150, 100);
	window_event_process();
	block = event_stream_process(stream);
	event = 0;
	got_resize = got_redraw = 0;
	got_other = 0;
	while ((event = event_next(block, event))) {
		switch (event->id) {
		case WINDOWEVENT_RESIZE:
			got_resize++;
			break;
		case WINDOWEVENT_REDRAW:
			got_redraw++;
			break;
		default:
			got_other++;
			break;
		}
	}
	//Can get two resize && redraw for restore and resize
	EXPECT_INTGE(got_resize, 1);
	EXPECT_INTLE(got_resize, 2);
	EXPECT_INTGE(got_redraw, 1);
	EXPECT_INTLE(got_redraw, 2);
	EXPECT_INTEQ(got_other, 0);

	thread_sleep(1000);
	EXPECT_INTEQ(window_width(window), 150);
	EXPECT_INTEQ(window_height(window), 100);
	EXPECT_FALSE(window_is_maximized(window));
	EXPECT_TRUE(window_has_focus(window));

	window_move(window, 10, 20);
	window_event_process();
	block = event_stream_process(stream);
	thread_sleep(1000);
	EXPECT_INTEQ(window_position_x(window), 10);
	EXPECT_INTEQ(window_position_y(window), 20);
	EXPECT_FALSE(window_is_maximized(window));
	//EXPECT_TRUE(window_has_focus(window));

	window_minimize(window);
	window_event_process();
	block = event_stream_process(stream);
	event = 0;
	got_resize = got_redraw = got_unfocus = 0;
	got_other = 0;
	while ((event = event_next(block, event))) {
		switch (event->id) {
		case WINDOWEVENT_RESIZE:
			got_resize++;
			break;
		case WINDOWEVENT_REDRAW:
			got_redraw++;
			break;
		case WINDOWEVENT_LOSTFOCUS:
			got_unfocus++;
			break;
		default:
			got_other++;
			break;
		}
	}
	EXPECT_INTEQ(got_resize, 1);
	EXPECT_INTEQ(got_redraw, 0);
	EXPECT_INTEQ(got_unfocus, 1);
	EXPECT_INTEQ(got_other, 0);

	thread_sleep(1000);
	EXPECT_FALSE(window_is_maximized(window));
	EXPECT_FALSE(window_has_focus(window));

	window_restore(window);
	window_event_process();
	block = event_stream_process(stream);
	event = 0;
	got_resize = got_redraw = got_focus = 0;
	got_other = 0;
	while ((event = event_next(block, event))) {
		switch (event->id) {
		case WINDOWEVENT_RESIZE:
			got_resize++;
			break;
		case WINDOWEVENT_REDRAW:
			got_redraw++;
			break;
		case WINDOWEVENT_GOTFOCUS:
			got_focus++;
			break;
		default:
			got_other++;
			break;
		}
	}
	EXPECT_INTEQ(got_resize, 1);
	EXPECT_INTEQ(got_redraw, 1);
	EXPECT_INTEQ(got_focus, 1);
	EXPECT_INTEQ(got_other, 0);

	thread_sleep(1000);
	EXPECT_FALSE(window_is_maximized(window));
	EXPECT_FALSE(window_is_minimized(window));
	EXPECT_TRUE(window_has_focus(window));

	window_minimize(window);
	window_event_process();
	thread_sleep(1000);
	EXPECT_FALSE(window_is_maximized(window));
	EXPECT_TRUE(window_is_minimized(window));
#endif

	window_deallocate(window);
	window = 0;

	EXPECT_FALSE(window_is_open(window));

	return 0;
}

static void
test_window_declare(void) {
	ADD_TEST(window, createdestroy);
	ADD_TEST(window, sizemove);
}

test_suite_t test_window_suite = {
	test_window_application,
	test_window_memory_system,
	test_window_config,
	test_window_declare,
	test_window_initialize,
	test_window_finalize
};


#if FOUNDATION_PLATFORM_ANDROID || FOUNDATION_PLATFORM_IOS

int
test_window_run(void);

int
test_window_run(void) {
	test_suite = test_window_suite;
	return test_run_all();
}

#else

test_suite_t
test_suite_define(void);

test_suite_t
test_suite_define(void) {
	return test_window_suite;
}

#endif

