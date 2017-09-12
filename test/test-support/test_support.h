/**
 * @file
 * @brief Header file declaring common unit test functionality
 *
 * @copyright Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.  No license to Wind River intellectual property rights is
 * granted herein.  All rights not licensed by Wind River are reserved by Wind
 * River.
 */

#ifndef UNIT_TEST_H
#define UNIT_TEST_H

/* header files to include before cmocka */
/* clang-format off */
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>
/* clang-format on */

#include <stdio.h> /* for snprintf */

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

#ifndef __GNUC__
#define __attribute__( x ) /* gcc specific */
#endif                     /* ifndef __GNUC__ */

/* In Windows some signatures of functions are slightly different */
#if defined( _WIN32 ) && !defined( snprintf )
#define snprintf _snprintf
#endif /* if defined( _WIN32 ) && !defined( snprintf ) */

/* functions */
/**
 * @brief Called to destory test support system
 *
 * @param[in]      argc                number of command-line arguments
 * @param[in]      argv                array of command-line arguments
 */
void test_finalize( int argc, char **argv );

/**
 * @brief Generates a random string for testing
 * @note The returned string is null terminated
 * @param[out] dest       destination to put the generated string
 * @param[in]  len        length of the string to be generated
 */
void test_generate_random_string( char *dest, size_t len );

/**
 * @brief Called to initialize test support system
 *
 * @param[in]      argc                number of command-line arguments
 * @param[in]      argv                array of command-line arguments
 */
void test_initialize( int argc, char **argv );

/* macros */
/**
 * @def FUNCTION_NAME
 * @brief Macro that will hold the name of the current function
 */
#ifndef FUNCTION_NAME
#if defined __func__
#define FUNCTION_NAME __func__
#elif defined __PRETTY_FUNCTION__
#define FUNCTION_NAME __PRETTY_FUNCTION__
#elif defined __FUNCTION__
#define FUNCTION_NAME __FUNCTION__
#else
#define FUNCTION_NAME ""
#endif
#endif

/**
 * @brief Macro test displays the name of the test case
 * @param[in]  x   name of the test case
 */
#define test_case( x ) test_case_out( FUNCTION_NAME, x )
/**
 * @brief Macro that displays the name of the test case
 * @param[in] x    name of the test case in printf format
 * @param[in] ...  printf format flags for the test case name
 */
#define test_case_printf( x, ... ) test_case_out( FUNCTION_NAME, x, __VA_ARGS__ )

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

/**
 * @brief Whether low-level system function mocking is currently enabled
 */
extern int MOCK_SYSTEM_ENABLED;

#endif /* ifndef UNIT_TEST_H */
