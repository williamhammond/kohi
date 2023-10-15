#pragma once

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef signed char i8;
typedef signed short i16;
typedef signed int i32;
typedef signed long long i64;

typedef float f32;
typedef double f64;

typedef int b32;
typedef char b8;

#ifndef NULL
#define NULL ((void *)0)
#endif

#if defined(__clang__) || defined(__gcc__)
#define STATIC_ASSERT _Static_assert
#else
#define STATIC_ASSERT static_assert
#endif

STATIC_ASSERT(sizeof(u8) == 1, "u8 is not 1 byte");
STATIC_ASSERT(sizeof(u16) == 2, "u16 is not 2 byte");
STATIC_ASSERT(sizeof(u32) == 4, "u32 is not 4 byte");
STATIC_ASSERT(sizeof(u64) == 8, "u64 is not 8 byte");

STATIC_ASSERT(sizeof(i8) == 1, "i8 is not 1 byte");
STATIC_ASSERT(sizeof(i16) == 2, "i16 is not 2 byte");
STATIC_ASSERT(sizeof(i32) == 4, "i32 is not 4 byte");
STATIC_ASSERT(sizeof(i64) == 8, "i64 is not 8 byte");

STATIC_ASSERT(sizeof(f32) == 4, "f32 is not 4 byte");
STATIC_ASSERT(sizeof(f64) == 8, "f64 is not 8 byte");

#define TRUE 1
#define FALSE 0

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define K_PLATFORM_WINDOWS 1
#ifndef _WIN64
#error "64-bit is required for Windows"
#endif

#elif defined(__linux__) || defined(__gnu_linux__)
#define K_PLATFORM_LINUX 1
#if defined(__ANDROID__)
#define K_PLATFORM_ANDROID 1
#endif

#elif (__unix__)
#define K_PLATFORM_UNIX 1

#elif (__POSIX_VERSION)
#define K_PLATFORM_POSIX 1

#elif defined(__APPLE__) || defined(__MACH__)
#define K_PLATFORM_APPLE 1
// #include<TargetConditionals.h>
#if TARGET_IPHONE_SIMULATOR == 1
#define K_PLATFORM_IOS 1
#define K_PLATFORM_IOS_SIMULATOR 1

#elif TARGET_OS_MAC
#else
#error "Unknown Apple platform"
#endif
#else
#error "Unknown platform"

#endif

#ifdef KEXPORT

#ifdef _MSC_VER
#define KAPI __declspec(dllexport)
#else
#define KAPI __attribute__((visibility("default")))
#endif

#else

#ifdef _MSC_VER
#define KAPI __declspec(dllimport)
#else
#define KAPI
#endif

#endif

#define KCLAMP(value, min, max) (value <= min) ? min : (value >= max) ? max \
                                                                      : value;