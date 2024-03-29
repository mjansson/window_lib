/* main.c  -  Window test  -  Public Domain  -  2013 Mattias Jansson
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
	app.company = string_const(STRING_CONST(""));
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

static int got_create, got_destroy, got_show, got_hide, got_focus, got_unfocus, got_redraw, got_resize, got_other;

static void
on_test_fail(void) {
	window_message_quit();
}

static void*
createdestroy_thread(void* arg) {
	FOUNDATION_UNUSED(arg);
	thread_sleep(500);

	event_stream_t* stream = window_event_stream();
	event_block_t* block = event_stream_process(stream);
	event_t* event = 0;
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

	EXPECT_INTEQ(got_create, 1);
	EXPECT_INTEQ(got_show, 1);
	EXPECT_INTEQ(got_focus, 1);
	EXPECT_INTGE(got_redraw, 1);

	window_message_quit();

	return 0;
}

DECLARE_TEST(window, createdestroy) {
	window_t window;
	thread_t thread;

	test_set_fail_hook(on_test_fail);

	thread_sleep(100);

#if FOUNDATION_PLATFORM_WINDOWS || FOUNDATION_PLATFORM_LINUX || FOUNDATION_PLATFORM_BSD
	window_create(&window, WINDOW_ADAPTER_DEFAULT, STRING_CONST("Window test"), 800, 600, 0);
#elif FOUNDATION_PLATFORM_MACOS || FOUNDATION_PLATFORM_IOS
	window_initialize(&window, delegate_window());
#endif

	EXPECT_TRUE(window_is_open(&window));

	got_create = got_destroy = got_show = got_hide = got_focus = got_unfocus = got_redraw = got_resize = got_other = 0;

	thread_initialize(&thread, createdestroy_thread, &window, STRING_CONST("createdestroy_thread"),
	                  THREAD_PRIORITY_NORMAL, 0);
	thread_start(&thread);

	EXPECT_EQ(window_message_loop(), 0);

	void* ret = thread_join(&thread);

	window_finalize(&window);
	thread_finalize(&thread);

	if (ret)
		return ret;

	got_create = got_destroy = got_show = got_hide = got_focus = got_unfocus = got_redraw = got_resize = got_other = 0;

	event_stream_t* stream = window_event_stream();
	event_block_t* block = event_stream_process(stream);
	event_t* event = 0;
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

#if FOUNDATION_PLATFORM_MACOS
	EXPECT_INTEQ(got_destroy, 0);  // Does not destroy actual NS window
#else
	EXPECT_INTEQ(got_destroy, 1);
#endif
	EXPECT_INTLE(got_hide, 1);     // Potential event
	EXPECT_INTLE(got_unfocus, 1);  // Potential event

	EXPECT_FALSE(window_is_open(&window));

	return 0;
}

static void*
sizemove_thread(void* arg) {
	window_t* window = arg;

	thread_sleep(100);

	event_stream_t* stream = window_event_stream();
	event_block_t* block = event_stream_process(stream);
	// Ignore initial batch of events in this test

	EXPECT_NE(window, 0);
	EXPECT_TRUE(window_is_open(window));

	EXPECT_TRUE(window_is_visible(window));
	EXPECT_TRUE(window_has_focus(window));
#if FOUNDATION_PLATFORM_IOS || FOUNDATION_PLATFORM_ANDROID
	EXPECT_TRUE(window_is_maximized(window));
#else
	// EXPECT_FALSE(window_is_maximized(window));
#endif

	block = event_stream_process(stream);
	event_t* event = 0;
	while ((event = event_next(block, event))) {
	}

	window_maximize(window);
	thread_sleep(1000);

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
				// log_warnf(HASH_TEST, WARNING_INVALID_VALUE, STRING_CONST("Got invalid window event: %d"), event->id);
				got_other++;
				break;
		}
	}
	EXPECT_INTEQ(got_resize, 1);
	EXPECT_INTEQ(got_redraw, 1);
#endif

	EXPECT_TRUE(window_is_maximized(window));
	EXPECT_TRUE(window_has_focus(window));

#if !FOUNDATION_PLATFORM_IOS && !FOUNDATION_PLATFORM_ANDROID
	window_restore(window);
	thread_sleep(1000);

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

	EXPECT_FALSE(window_is_maximized(window));
	EXPECT_TRUE(window_has_focus(window));
#endif

	window_maximize(window);
	thread_sleep(1000);

	block = event_stream_process(stream);
	EXPECT_TRUE(window_is_maximized(window));

#if !FOUNDATION_PLATFORM_IOS && !FOUNDATION_PLATFORM_ANDROID
	window_resize(window, 150, 100);
	thread_sleep(1000);

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
	// Can get two resize && redraw for restore and resize
	EXPECT_INTGE(got_resize, 1);
	EXPECT_INTLE(got_resize, 2);
	EXPECT_INTGE(got_redraw, 1);
	EXPECT_INTLE(got_redraw, 2);

	EXPECT_INTEQ(window_width(window), 150);
	EXPECT_INTEQ(window_height(window), 100);
	EXPECT_FALSE(window_is_maximized(window));
	EXPECT_TRUE(window_has_focus(window));

	window_move(window, 200, 300);
	thread_sleep(1000);

	int base_x = window_position_x(window);
	int base_y = window_position_y(window);
	window_move(window, 300, 500);
	thread_sleep(1000);

	block = event_stream_process(stream);

	EXPECT_INTEQ(window_position_x(window), base_x + 100);
	EXPECT_INTEQ(window_position_y(window), base_y + 200);
	EXPECT_FALSE(window_is_maximized(window));
	EXPECT_TRUE(window_has_focus(window));

	window_minimize(window);
	thread_sleep(1000);

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
	EXPECT_INTEQ(got_resize, 0);
	EXPECT_INTEQ(got_redraw, 0);
	EXPECT_INTEQ(got_unfocus, 1);

	EXPECT_FALSE(window_is_maximized(window));
	EXPECT_FALSE(window_has_focus(window));

	window_restore(window);
	thread_sleep(1000);

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
	EXPECT_INTEQ(got_resize, 0);
	EXPECT_INTGE(got_redraw, 1);
	EXPECT_INTEQ(got_focus, 1);

	EXPECT_FALSE(window_is_maximized(window));
	EXPECT_FALSE(window_is_minimized(window));
	EXPECT_TRUE(window_has_focus(window));

	window_minimize(window);
	thread_sleep(1000);

	EXPECT_FALSE(window_is_maximized(window));
	EXPECT_TRUE(window_is_minimized(window));
#endif

	window_message_quit();

	return 0;
}

DECLARE_TEST(window, sizemove) {
	window_t window;
	thread_t thread;

	test_set_fail_hook(on_test_fail);

	thread_sleep(100);

#if FOUNDATION_PLATFORM_WINDOWS || FOUNDATION_PLATFORM_LINUX || FOUNDATION_PLATFORM_BSD
	window_create(&window, WINDOW_ADAPTER_DEFAULT, STRING_CONST("Window test"), 800, 600, 0);
#elif FOUNDATION_PLATFORM_MACOS || FOUNDATION_PLATFORM_IOS
	window_initialize(&window, delegate_window());
#endif

	EXPECT_TRUE(window_is_open(&window));

	got_create = got_destroy = got_show = got_hide = got_focus = got_unfocus = got_redraw = got_resize = got_other = 0;

	thread_initialize(&thread, sizemove_thread, &window, STRING_CONST("sizemove_thread"), THREAD_PRIORITY_NORMAL, 0);
	thread_start(&thread);

	EXPECT_EQ(window_message_loop(), 0);

	void* ret = thread_join(&thread);

	window_finalize(&window);
	thread_finalize(&thread);
	window_finalize(&window);

	EXPECT_FALSE(window_is_open(&window));

	return ret;
}

static void
test_window_declare(void) {
	ADD_TEST(window, createdestroy);
	ADD_TEST(window, sizemove);
}

static test_suite_t test_window_suite = {test_window_application,
                                         test_window_memory_system,
                                         test_window_config,
                                         test_window_declare,
                                         test_window_initialize,
                                         test_window_finalize,
                                         0};

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
