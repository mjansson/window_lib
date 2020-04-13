/* build.h  -  Window library build setup  -  Public Domain  -  2014 Mattias Jansson
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

#if defined(WINDOW_COMPILE) && WINDOW_COMPILE
#ifdef __cplusplus
#define WINDOW_EXTERN extern "C"
#define WINDOW_API extern "C"
#else
#define WINDOW_EXTERN extern
#define WINDOW_API extern
#endif
#else
#ifdef __cplusplus
#define WINDOW_EXTERN extern "C"
#define WINDOW_API extern "C"
#else
#define WINDOW_EXTERN extern
#define WINDOW_API extern
#endif
#endif
