/**
 * @file
 * @brief declares functions for the operating system abstraction layer
 *
 * @copyright Copyright (C) 2017-2018 Wind River Systems, All Rights Reserved.
 *
 * @license Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed
 * under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
 * OR CONDITIONS OF ANY KIND, either express or implied."
 */
#ifndef OS_H
#define OS_H

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

#include <stdint.h>

/**
 * @brief Possible return code values
 */
typedef enum os_status
{
	/** @brief Success */
	OS_STATUS_SUCCESS = 0,
	/** @brief Action successfully invoked (fire & forget) */
	OS_STATUS_INVOKED,
	/** @brief Invalid parameter passed */
	OS_STATUS_BAD_PARAMETER,
	/** @brief Bad request received */
	OS_STATUS_BAD_REQUEST,
	/** @brief Error executing the requested action */
	OS_STATUS_EXECUTION_ERROR,
	/** @brief Already exists */
	OS_STATUS_EXISTS,
	/** @brief File open failed */
	OS_STATUS_FILE_OPEN_FAILED,
	/** @brief Full storage */
	OS_STATUS_FULL,
	/** @brief Input/output error */
	OS_STATUS_IO_ERROR,
	/** @brief No memory */
	OS_STATUS_NO_MEMORY,
	/** @brief No permission */
	OS_STATUS_NO_PERMISSION,
	/** @brief Not executable */
	OS_STATUS_NOT_EXECUTABLE,
	/** @brief Not found */
	OS_STATUS_NOT_FOUND,
	/** @brief Not Initialized */
	OS_STATUS_NOT_INITIALIZED,
	/** @brief Parameter out of range */
	OS_STATUS_OUT_OF_RANGE,
	/** @brief Failed to parse a message */
	OS_STATUS_PARSE_ERROR,
	/** @brief Timed out */
	OS_STATUS_TIMED_OUT,
	/** @brief Try again */
	OS_STATUS_TRY_AGAIN,
	/** @brief Not supported in this version of the api */
	OS_STATUS_NOT_SUPPORTED,

	/**
	 * @brief General failure
	 * @note This must be the last state
	 */
	OS_STATUS_FAILURE
} os_status_t;

/** @brief Version the library as a string */
#define OS_LIB_VERSION                          "0.9.0"
/** @brief Major version number of the library */
#define OS_LIB_VERSION_MAJOR                    0
/** @brief Minor version number of the library */
#define OS_LIB_VERSION_MINOR                    9
/** @brief Patch version number of the library */
#define OS_LIB_VERSION_PATCH                    0
/** @brief Tweak version number of the library */
#define OS_LIB_VERSION_TWEAK                    0

/** @brief False */
#define OS_FALSE                                (os_bool_t)(0 == 1)
/** @brief True */
#define OS_TRUE                                 (os_bool_t)(1 == 1)

/**
 * @defgroup os_types List of basic operating system types
 * @{
 */
/** @brief boolean (true or false) */
typedef int                                      os_bool_t;
/** @brief 32-bit floating-point */
typedef float                                    os_float32_t;
/** @brief 64-bit floating-point */
typedef double                                   os_float64_t;
/** @brief 8-bit signed integer */
typedef int8_t                                   os_int8_t;
/** @brief 16-bit signed integer */
typedef int16_t                                  os_int16_t;
/** @brief 32-bit signed integer */
typedef int32_t                                  os_int32_t;
/** @brief 64-bit signed integer */
typedef int64_t                                  os_int64_t;
/** @brief 8-bit unsigned integer */
typedef uint8_t                                  os_uint8_t;
/** @brief 16-bit unsigned integer */
typedef uint16_t                                 os_uint16_t;
/** @brief 32-bit unsigned integer */
typedef uint32_t                                 os_uint32_t;
/** @brief 64-bit unsigned integer */
typedef uint64_t                                 os_uint64_t;
/** @brief time interval in milliseconds */
typedef uint32_t                                 os_millisecond_t;
/** @brief time stamp in milliseconds */
typedef uint64_t                                 os_timestamp_t;

/**
 * @}
 */

/** @brief Number of microseconds in millisecond */
#define OS_MICROSECONDS_IN_MILLISECOND 1000u
/** @brief Number of milliseconds in a second */
#define OS_MILLISECONDS_IN_SECOND     1000u
/** @brief Number of nanoseconds in a millisecond */
#define OS_NANOSECONDS_IN_MILLISECOND 1000000u
/** @brief Number of nanoseconds in a second */
#define OS_NANOSECONDS_IN_SECOND      1000000000uL
/** @brief Number of seconds in a minute */
#define OS_SECONDS_IN_MINUTE          60u
/** @brief Number of minutes in an hour */
#define OS_MINUTES_IN_HOUR            60u
/** @brief Number of hours in a day */
#define OS_HOURS_IN_DAY               24u

/**
 * @def OS_API
 * @brief Macro to add compiler specifications for external/internal functions
 */
#if defined( ARDUINO )
#	define OS_API __attribute__((section(".irom0.text")))
#elif defined( OSAL_STATIC )
#	define OS_API
#else /* ifdef OSAL_STATIC */
#	if defined _WIN32 || defined __CYGWIN__
#		if defined(OSAL_EXPORT) && OSAL_EXPORT
#			ifdef __GNUC__
#				define OS_API __attribute__((dllexport))
#			else
#				define OS_API __declspec(dllexport)
#			endif
#		else /* if defined(OSAL_EXPORT) && OSAL_EXPORT */
#			ifdef __GNUC__
#				define OS_API __attribute__((dllimport))
#			else
#				define OS_API __declspec(dllimport)
#			endif
#		endif /* if defined(OSAL_EXPORT) && OSAL_EXPORT */
#	else /* if defined _WIN32 || defined __CYGWIN__ */
#		if __GNUC__ >= 4
#			define OS_API __attribute__((visibility("default")))
#		else
#			define OS_API
#		endif
#	endif /* if defined _WIN32 || defined __CYGWIN__ */
#endif /* ifdef OSAL_STATIC */

/**
 * @def __attribute__
 * @brief GCC specific tag, suppressed for other compilers
 */
/**
 * @def UNUSED
 * @brief Macro used to define an unused attribute to the compiler
 */
#ifdef __GNUC__
#	define UNUSED(x) PARAM_ ## x __attribute((__unused__))
#else
#	define  __attribute__(x) /* gcc specific */
#	define UNUSED(x) x
#endif /* ifdef __GNUC__ */

/** @brief Signature of the function to handle operating systems signals */
typedef void (*os_sighandler_t)( int );

/**
 * @brief Prototype for the main function of a service
 *
 * @param[in]      argc                number of arguments passed in
 * @param[in]      argv                array of arguments passed in
 *
 * @retval EXIT_SUCCESS                service completed successfully
 * @retval EXIT_FAILURE                service failed to complete
 */
typedef int (*os_service_main_t)( int argc, char** argv );

/**
 * @brief Structure used for storing date and time information
 */
typedef struct
{
	/** @brief Milliseconds (0-999) */
	os_uint16_t millisecond;
	/** @brief Seconds (0-59) */
	os_uint8_t second;
	/** @brief Minutes (0-59) */
	os_uint8_t minute;
	/** @brief Hours (0-23) */
	os_uint8_t hour;
	/** @brief Day (1-31) */
	os_uint8_t day;
	/** @brief Month (1-12) */
	os_uint8_t month;
	/** @brief Year (1970-2999) */
	os_uint16_t year;
} os_date_time_t;

/**
 * @def DIRECTORY_CREATE_MAX_TIMEOUT
 * @brief Amount of time in milliseconds to wait for a directory to be
 *        created
 */
#define DIRECTORY_CREATE_MAX_TIMEOUT 300u

/* flags */
/** @brief Time stamps are relative to uptime */
#define OS_FLAG_RELATIVE_TIME  0x1

/** @brief maximum length for a field in system info structure */
#define OS_SYSTEM_INFO_MAX_LEN 64u

/** @brief Type for a list of adapters */
typedef struct os_adapters os_adapters_t;

/** @brief Type for a directory entry */
typedef struct os_dir os_dir_t;

/** @brief Type for a socket */
typedef struct os_socket os_socket_t;

/** @brief operating system information structure */
typedef struct os_system_info
{
	/** @brief operating system vendor */
	char vendor_name[OS_SYSTEM_INFO_MAX_LEN + 1u];
	/** @brief host name */
	char host_name[OS_SYSTEM_INFO_MAX_LEN + 1u];
	/** @brief operating system name */
	char system_name[OS_SYSTEM_INFO_MAX_LEN + 1u];
	/** @brief operating system platform */
	char system_platform[OS_SYSTEM_INFO_MAX_LEN + 1u];
	/** @brief operating system release */
	char system_release[OS_SYSTEM_INFO_MAX_LEN + 1u];
	/** @brief operating system version */
	char system_version[OS_SYSTEM_INFO_MAX_LEN + 1u];
	/** @brief kernel version */
	char kernel_version[OS_SYSTEM_INFO_MAX_LEN + 1u];
	/** @brief operating system flag */
	os_uint8_t system_flags;
} os_system_info_t;

/**
 * @file
 * @brief Function definitions for POSIX operating systems
 *
 * @copyright Copyright (C) 2016-2017 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license Licensed under the Apache License, Version 2.0 (the "License");

 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed
 * under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
 * OR CONDITIONS OF ANY KIND, either express or implied."
 */


#include <pthread.h> /* for: pthread_t, pthread_cond_t, pthread_mutex_t, pthread_rwlock_t */

#include <stdarg.h> /* for: va_list */
#include <limits.h> /* for: PATH_MAX definition */
#include <stdio.h> /* for: FILE *, feof, fgets, fread, fputs, fwrite, fprintf, printf, snprintf, vfprintf */
#include <signal.h> /* for SIGINT, ... */

#ifdef __VXWORKS__
#define remainder fmod
#endif

/**
 * @brief Handle to an open file
 */
typedef FILE *os_file_t;

/**
 * @brief Handle to an open shared library
 */
typedef void *os_lib_handle;

/**
 * @brief Handle to a thread
 */
typedef pthread_t os_thread_t;

/**
 * @brief Thread condition lock
 */
typedef pthread_cond_t os_thread_condition_t;

/**
 * @brief Thread mutually exclusive (mutex) lock
 */
typedef pthread_mutex_t os_thread_mutex_t;

/**
 * @brief Thread read/write lock
 */
#ifdef __VXWORKS__
typedef SEM_ID os_thread_rwlock_t;
#else
typedef pthread_rwlock_t os_thread_rwlock_t;
#endif


/** @brief type representing a UUID for an operating system */
typedef char os_uuid_t[16u];

/**
 * @brief Directory seperator character
 */
#define OS_DIR_SEP                     '/'

/**
 * @brief Character used to split environment variables
 */
#define OS_ENV_SPLIT                   ':'

/**
 * @brief Defines type for invalid file handle
 */
#define OS_FILE_INVALID                NULL

/**
 * @brief Character sequence for a line break
 */
#define OS_FILE_LINE_BREAK             "\n"

/**
 * @brief Seek from start of file
 * @see os_file_seek
 */
#define OS_FILE_SEEK_START             SEEK_SET

/**
 * @brief Seek from current position in file
 * @see os_file_seek
 */
#define OS_FILE_SEEK_CURRENT           SEEK_CUR

/**
 * @brief Seek from end of file
 * @see os_file_seek
 */
#define OS_FILE_SEEK_END               SEEK_END

/**
 * @brief Null device, used to discard data
 */
#define OS_NULL_DEVICE                 "/dev/null"

/**
 * @brief Invalid socket symbol
 */
#define OS_SOCKET_INVALID              -1

/**
 * @brief Symbol to use when thread linking
 */
#define OS_THREAD_LINK
/**
 * @brief Return type for a thread main function
 */
#define OS_THREAD_RETURN               void*
/**
 * @brief Symbol to correct declare a thread on all platforms
 */
#define OS_THREAD_DECL OS_THREAD_RETURN OS_THREAD_LINK
/**
 * @brief Type defining the starting point for a thread
 */
typedef OS_THREAD_RETURN
	(OS_THREAD_LINK *os_thread_main_t)( void *arg );


/**
 * @brief Defines the symbol for standard error
 */
#define OS_STDERR                      stderr

/**
 * @brief Defines the symbol for standard in
 */
#define OS_STDIN                       stdin

/**
 * @brief Defines the symbol for standard out
 */
#define OS_STDOUT                      stdout

/**
 * @brief Converts a character to lower case
 *
 * @param[in]      c                   character to convert
 *
 * @return the lower-case value of the character, or @p c if not possible
 */
OS_API char os_char_tolower(
	char c
);


/**
 * @brief Converts a character to upper case
 *
 * @param[in]      c                   character to convert
 *
 * @return the upper-case value of the character, or @p c if not possible
 */
OS_API char os_char_toupper(
	char c
);


/**
 * @brief check if at end-of-file
 *
 * @param[in]      stream              stream to write to
 *
 * @retval         OS_FALSE            end of file bit not set
 * @retval         OS_TRUE             end of file bit is set
 */
OS_API os_bool_t os_file_eof(
	os_file_t stream
);


/**
 * @brief Read bytes from a file into a char array
 *
 * @note Stops when encountering max read, EOF, or null terminator
 *
 * @param[out]     str                 Pointer to array to write to
 * @param[in]      size                Max number of bytes to read
 * @param[in,out]  stream              Pointer file to read from
 *
 * @retval         !NULL               char array written to
 * @retval         NULL                encountered EOF or error
 *
 * @see os_file_fputs
 */
OS_API char *os_file_gets(
	char *str,
	size_t size,
	os_file_t stream
);


/**
 * @brief Read bytes from a file into an array
 *
 * @note Stops when encountering max read, EOF, or null terminator
 *
 * @param[in,out]  ptr                 Pointer to array to write to
 * @param[in]      size                Size of each item to read
 * @param[in]      nmemb               Number of items to read
 * @param[in,out]  stream              Pointer to file to read from
 *
 * @return         Number of items read
 */
OS_API size_t os_file_read(
	void *ptr,
	size_t size,
	size_t nmemb,
	os_file_t stream
);


/**
 * @brief Write bytes from an array into a file stream
 *
 * @note Stops writing when encountering a null terminator
 * @note Does not write the null terminator to the stream
 *
 * @param[in]      str                 Pointer to array to write from
 * @param[in,out]  stream              Pointer to file to write to
 *
 * @return         Number of bytes written
 *
 * @see os_file_fgets
 */
OS_API size_t os_file_puts(
	char *str,
	os_file_t stream
);


/**
 * @brief Returns the current position in a file stream
 *
 * @param[in,out]  stream              stream to pointer of open file
 *
 * @retval OS_STATUS_BAD_PARAMETER     bad parameter passed to the function
 * @retval OS_STATUS_FAILURE           failed to move file pointer
 * @retval OS_STATUS_SUCCESS           on success
 *
 * @see os_file_seek
 */
OS_API long int os_file_tell(
	os_file_t stream
);


/**
 * @brief Write bytes from an array into a file stream
 *
 * @param[in]      ptr                 Pointer to array to write from
 * @param[in]      size                Size of each item to be written
 * @param[in]      nmemb               Number of items to be written
 * @param[in,out]  stream              Pointer to file to write to
 *
 * @return         Number of items written
 */
OS_API size_t os_file_write(
	const void *ptr,
	size_t size,
	size_t nmemb,
	os_file_t stream
);


/**
 * @brief Locate the first occurance of a character in a string
 *
 * @param[in]      s                   string to be searched
 * @param[in]      c                   character to search for as an int
 *
 * @retval         !NULL               pointer to last occurance of c in s
 * @retval         NULL                c was not found in s
 */
OS_API char *os_strchr(
	const char *s,
	char c
);


/**
 * @brief Compares two strings
 *
 * @param[in]      s1                  first string to compare
 * @param[in]      s2                  second string to compare
 *
 * @retval         <0                  the first non-matching character has a
 *                                     lower value in s1 than in s2
 * @retval         0                   the two strings are equal
 * @retval         >0                  the first non-matching character has a
 *                                     higher value in s1 than in s2
 */
OS_API int os_strcmp(
	const char *s1,
	const char *s2
);


/**
 * @brief Get the length of a string
 *
 * @param[in]      s                   string to get length from
 *
 * @return         Number of characters until a terminating null character
 */
OS_API size_t os_strlen(
	const char *s
);


/**
 * @brief Compares two strings up to a certain number of characters
 *
 * @param[in]      s1                  first string to compare
 * @param[in]      s2                  second string to compare
 * @param[in]      len                 maximum number of characters to compare
 *
 * @retval         <0                  the first non-matching character has a
 *                                     lower value in s1 than in s2
 * @retval         0                   the two strings are equal up to len
 *                                     characters
 * @retval         >0                  the first non-matching character has a
 *                                     higher value in s1 than in s2
 */
OS_API int os_strncmp(
	const char *s1,
	const char *s2,
	size_t len
);


/**
 * @brief Copy characters from string
 *
 * @param[out]     dest                string to copy characters to
 * @param[in]      src                 string to copy character from
 * @param[in]      num                 maximum number of characters to copy
 *
 * @return a pointer to the destination string
 */
OS_API char *os_strncpy(
	char *dest,
	const char *src,
	size_t num
);


/**
 * @brief Finds the first occurrence of some specified characters in a string
 *
 * @param[in]      str1                String to search
 * @param[in]      str2                Characters to search for
 *
 * @retval         !NULL               Pointer to the first occurrence of
 *                                     one of the characters in str2 in str1
 * @retval         NULL                None of the characters in str2 appear
 */
OS_API char *os_strpbrk(
	const char *str1,
	const char *str2
);


/**
 * @brief Locate the last occurance of a character in a string
 *
 * @param[in]      s                   string to be searched
 * @param[in]      c                   character to search for as an int
 *
 * @retval         !NULL               pointer to last occurance of c in s
 * @retval         NULL                c was not found in s
 */
OS_API char *os_strrchr(
	const char *s,
	char c
);


/**
 * @brief Locate a substring
 *
 * @param[in]      str1                string to be searched
 * @param[in]      str2                string to search for
 *
 * @retval         !NULL               pointer to the substring of str2 in str1
 * @retval         NULL                no substring found
 */
OS_API char *os_strstr(
	const char *str1,
	const char *str2
);


/**
 * @brief Parse a string to retrieve a double
 *
 * @param[in]      str                 string to parse for double
 * @param[out]     endptr              (optional)pointer to character 
 *                                     immediately following double in string
 * @return value of double parsed
 */
OS_API double os_strtod(
	const char *str,
	char **endptr
);


/**
 * @brief Parse a string to retrieve a long
 *
 * @note This implementation only supports base-ten numbers
 *
 * @param[in]      str                 string to parse for long
 * @param[out]     endptr              (optional)pointer to character
 *                                     immediately following long in string
 * @return value of long parsed
 */
OS_API long os_strtol(
	const char *str,
	char **endptr
);


/**
 * @brief Parse a string to retrieve an unsigned long
 *
 * @param[in]      str                 string to parse for unsigned long
 * @param[out]     endptr              (optional)pointer to character
 *                                     immediately following unsigned long in
 *                                     string
 * @return value of unsigned long parsed
 */
OS_API unsigned long os_strtoul(
	const char *str,
	char **endptr
);


/* memory functions */
/**
 * @brief Compares two blocks of memory
 *
 * @param[in]      ptr1                pointer to first block of memory
 * @param[in]      ptr2                pointer to second block of memory
 * @param[in]      num                 amount of bytes to compare
 *
 * @retval         <0                  first non-matching byte is lower in ptr1
 * @retval         0                   all bytes are equal
 * @retval         >0                  first non-matching byte is lower in ptr2
 */
OS_API int os_memcmp(
	const void *ptr1,
	const void *ptr2,
	size_t num
);


/**
 * @brief Copy a block of memory
 *
 * @warning The destination and source memory block must not overlap
 *
 * @param[out]     dest                destination to write to
 * @param[in]      src                 source block of memory
 * @param[in]      len                 amount of data to copy
 */
OS_API void *os_memcpy(
	void *dest,
	const void *src,
	size_t len
);


/**
 * @brief Moves a block of memory
 *
 * @param[out]     dest                destination to write to
 * @param[in]      src                 source block of memory
 * @param[in]      len                 amount of data to move
 */
OS_API void *os_memmove(
	void *dest,
	const void *src,
	size_t len
);


/**
 * @brief Sets a block of memory to a specific byte
 *
 * @param[out]     dest                destination to write to
 * @param[in]      c                   byte to set
 * @param[in]      len                 amount of data to set
 */
OS_API void os_memset(
	void *dest,
	int c,
	size_t len
);


/**
 * @brief Zeroizes block of memory
 *
 * @param[out]     dest                destination to write to
 * @param[in]      len                 amount of data to zeroize
 */
OS_API void os_memzero(
	void *dest,
	size_t len
);


/**
 * @brief Writes output to an open file stream
 *
 * @param[in]      stream              stream to write to
 * @param[in]      format              string format
 * @param[in]      ...                 items to replace based on @p format
 *
 * @return the number of characters printed including the null-terminator
 *
 * @see os_printf
 * @see os_sprintf
 * @see os_vfprintf
 */
OS_API int os_fprintf(
	os_file_t stream,
	const char *format,
	...
) __attribute__((format(printf,2,3)));


/**
 * @brief Writes output to standard out
 *
 * @param[in]      format              string format
 * @param[in]      ...                 items to replace based on @p format
 *
 * @return the number of characters printed including the null-terminator
 *
 * @see os_fprintf
 * @see os_sprintf
 */
OS_API int os_printf(
	const char *format,
	...
) __attribute__((format(printf,1,2)));


/**
 * @brief Writes output to an open file stream using a va_list
 *
 * @param[in]      stream              stream to write to
 * @param[in]      format              string format
 * @param[in]      args                variable containing values for @p format
 *
 * @return the number of characters printed including the null-terminator
 *
 * @see os_fprintf
 * @see os_printf
 * @see os_sprintf
 * @see os_vsnprintf
 */
OS_API int os_vfprintf(
	os_file_t stream,
	const char *format,
	va_list args
) __attribute__((format(printf,2,0)));


/**
 * @brief Writes output to a string with a maximum size using a va_list
 *
 * @param[out]     str                 string to output to
 * @param[in]      size                maximum size of buffer
 * @param[in]      format              string format
 * @param[in]      args                variable containing values for @p format
 *
 * @return the number of characters printed including the null-terminator,
 *         if the output is truncated then the return value -1
 *
 * @see os_snprintf
 * @see os_vfprintf
 */
OS_API int os_vsnprintf(
	char *str,
	size_t size,
	const char *format,
	va_list args
) __attribute__((format(printf,3,0)));

/**
 * @brief Allocates memory for an array of elements
 *
 * The memory returned is NOT initialized. Any allocated memory should be
 * deallocated with the corrosponding @p os_free command
 *
 * @note Specifying either 0 elements or elements with a size of 0 may return a
 *       valid memory pointer (that can be later freed) or NULL
 *
 * @param[in]      nmemb               number of elements to allocate memory for
 * @param[in]      size                size of each element in bytes
 *
 * @retval NULL    the specified amount of memory is not continously available
 * @retval !NULL   a pointer to the allocated memory
 *
 * @see os_free
 * @see os_malloc
 * @see os_realloc
 */
OS_API void *os_calloc(
	size_t nmemb,
	size_t size
) __attribute__((malloc));


/**
 * @brief Frees previously allocated memory
 *
 * @param[in]      ptr            pointer to the allocated memory to free
 *
 * @warning @c ptr is not checked for NULL prior to deallocation
 *
 * @see os_calloc
 * @see os_free_null
 * @see os_malloc
 * @see os_realloc
 */
OS_API void os_free(
	void *ptr
);


/**
 * @brief Frees previously allocated memory, setting the variable to NULL
 *
 * @param[in,out]  ptr            pointer to the pointer to the allocated
 *                                memory to free, and set the result value
 *
 * @see os_calloc
 * @see os_free
 * @see os_malloc
 * @see os_realloc
 */
OS_API void os_free_null(
	void **ptr
);


/**
 * @brief Allocates the specified amount of bytes
 *
 * The memory returned is NOT initialized. Any allocated memory should be
 * deallocated with the corrosponding @p os_free command
 *
 * @param[in]      size                amount of memory to allocate
 *
 * @retval NULL    the specified amount of memory is not continously available
 * @retval !NULL   a pointer to the allocated memory
 *
 * @see os_calloc
 * @see os_free
 * @see os_free_null
 * @see os_realloc
 */
OS_API void *os_malloc(
	size_t size
) __attribute__((malloc));


/**
 * @brief Change the size of an allocated memory block
 *
 * The contents of the previously allocated memory will be unchanged in the
 * range from the start up to the minimum of the old and new sizes.  If hte new
 * size is larger than the old size the added memory will not be initialized.
 *
 * @param[in]      ptr                 pointer to the previously allocated
 *                                     block, if ptr is NULL the call is
 *                                     equivilant to os_malloc(size).
 *                                     This value must be a value returned by an
 *                                     earlier call to os_calloc,
 *                                     os_malloc or os_realloc
 * @param[in]      size                new size for the allocated block
 *
 * @retval NULL    the specified amount of memory is not continously available
 * @retval !NULL   a pointer to the allocated memory
 *
 * @see os_calloc
 * @see os_free
 * @see os_malloc
 */
OS_API void *os_realloc(
	void *ptr,
	size_t size
) __attribute__((malloc));


/**
 * @brief Returns the operating system code for the last system error
 *        encountered
 *
 * @return The operating system code for the last error encountered
 */
OS_API int os_system_error_last( void );


/**
 * @brief Returns the process id of the current process
 *
 * @return the process id of the current process
 */
OS_API os_uint32_t os_system_pid( void );


/**
 * @brief Closes an open runtime library
 *
 * @param[in]      lib                 open library handle to close
 *
 * @retval OS_STATUS_FAILURE           on failure
 * @retval OS_STATUS_SUCCESS           on success
 *
 * @see os_library_find
 * @see os_library_open
 */
OS_API os_status_t os_library_close(
	os_lib_handle lib
);

/**
 * @brief Finds a function within an open runtime library
 *
 * @param[in]      lib                 open library handle
 * @param[in]      function            function to find
 *
 * @return a pointer to the function in memory, NULL if symbol is not found
 *
 * @see os_library_close
 * @see os_library_open
 */
OS_API void *os_library_find(
	os_lib_handle lib,
	const char *function
);


/**
 * @brief Opens a runtime library
 *
 * @param[in]      path                library to open
 *
 * @return a valid library handle on success, NULL on failure
 *
 * @see os_library_close
 * @see os_library_find
 */
OS_API os_lib_handle os_library_open(
	const char *path
);


/* thread support */
/**
 * @brief Wakes up all threads waiting on a condition variable
 *
 * @param[in,out]  cond                condition variable to wake up
 *
 * @retval OS_STATUS_BAD_PARAMETER     invalid parameter passed to function
 * @retval OS_STATUS_FAILURE           function failed
 * @retval OS_STATUS_SUCCESS           on success
 */
OS_API os_status_t os_thread_condition_broadcast(
	os_thread_condition_t *cond
);

/**
 * @brief Creates a new condition variable
 *
 * @param[in,out]  cond                newly created condition variable
 *
 * @retval OS_STATUS_BAD_PARAMETER     invalid parameter passed to function
 * @retval OS_STATUS_FAILURE           function failed
 * @retval OS_STATUS_SUCCESS           on success
 */
OS_API os_status_t os_thread_condition_create(
	os_thread_condition_t *cond
);

/**
 * @brief Destroys a previously created condition variable
 *
 * @param[in,out]  cond                previously created condition variable
 *
 * @retval OS_STATUS_BAD_PARAMETER     invalid parameter passed to function
 * @retval OS_STATUS_FAILURE           function failed
 * @retval OS_STATUS_SUCCESS           on success
 */
OS_API os_status_t os_thread_condition_destroy(
	os_thread_condition_t *cond
);

/**
 * @brief Signals a thread waiting on a condition variable to wake up
 *
 * @param[in,out]  cond                condition variable to signal
 * @param[in,out]  lock                lock prototecting condition variable
 *
 * @retval OS_STATUS_BAD_PARAMETER     invalid parameter passed to function
 * @retval OS_STATUS_FAILURE           function failed
 * @retval OS_STATUS_SUCCESS           on success
 */
OS_API os_status_t os_thread_condition_signal(
	os_thread_condition_t *cond,
	os_thread_mutex_t *lock
);

/**
 * @brief Waits a specified amount of time for a condition variable
 *
 * @param[in,out]  cond                condition variable to wait on
 * @param[in,out]  lock                lock protecting condition variable
 * @param[in]      max_time_out        maximum amount of time to wait
 *
 * @retval OS_STATUS_BAD_PARAMETER     invalid parameter passed to function
 * @retval OS_STATUS_FAILURE           function failed
 * @retval OS_STATUS_SUCCESS           on success
 * @retval OS_STATUS_TIMED_OUT         maximum wait time reached
 */
OS_API os_status_t os_thread_condition_timed_wait(
	os_thread_condition_t *cond,
	os_thread_mutex_t *lock,
	os_millisecond_t max_time_out
);

/**
 * @brief Creates a new thread
 *
 * @param[in,out]  thread              newly created thread object
 * @param[in]      main                main method to call for the thread
 * @param[in]      arg                 user specific data
 *
 * @retval OS_STATUS_BAD_PARAMETER     invalid parameter passed to function
 * @retval OS_STATUS_FAILURE           function failed
 * @retval OS_STATUS_SUCCESS           on success
 */
OS_API os_status_t os_thread_create(
	os_thread_t *thread,
	os_thread_main_t main,
	void *arg
);

/**
 * @brief Destroys a previously created thread
 *
 * @param[in,out]  thread              thread object to destroy
 *
 * @retval OS_STATUS_BAD_PARAMETER     invalid parameter passed to function
 * @retval OS_STATUS_FAILURE           function failed
 * @retval OS_STATUS_SUCCESS           on success
 */
OS_API os_status_t os_thread_destroy(
	os_thread_t *thread
);

/**
 * @brief Creates a new mutally exclusive lock
 *
 * @param[in,out]  lock                newly created lock
 *
 * @retval OS_STATUS_BAD_PARAMETER     invalid parameter passed to function
 * @retval OS_STATUS_FAILURE           function failed
 * @retval OS_STATUS_SUCCESS           on success
 */
OS_API os_status_t os_thread_mutex_create(
	os_thread_mutex_t *lock
);

/**
 * @brief Destroys a mutally exclusive lock
 *
 * @param[in,out]  lock                previously created lock
 *
 * @retval OS_STATUS_BAD_PARAMETER     invalid parameter passed to function
 * @retval OS_STATUS_FAILURE           function failed
 * @retval OS_STATUS_SUCCESS           on success
 */
OS_API os_status_t os_thread_mutex_destroy(
	os_thread_mutex_t *lock
);

/**
 * @brief Obtains a mutally exclusive lock (waits until lock is available)
 *
 * @param[in,out]  lock                previously created lock
 *
 * @retval OS_STATUS_BAD_PARAMETER     invalid parameter passed to function
 * @retval OS_STATUS_FAILURE           function failed
 * @retval OS_STATUS_SUCCESS           on success
 */
OS_API os_status_t os_thread_mutex_lock(
	os_thread_mutex_t *lock
);

/**
 * @brief Releases a mutally exclusive lock
 *
 * @param[in,out]  lock                previously created lock
 *
 * @retval OS_STATUS_BAD_PARAMETER     invalid parameter passed to function
 * @retval OS_STATUS_FAILURE           function failed
 * @retval OS_STATUS_SUCCESS           on success
 */
OS_API os_status_t os_thread_mutex_unlock(
	os_thread_mutex_t *lock
);

/**
 * @brief Creates a new read/write lock
 *
 * @param[in,out]  lock                newly created lock
 *
 * @retval OS_STATUS_BAD_PARAMETER     invalid parameter passed to function
 * @retval OS_STATUS_FAILURE           function failed
 * @retval OS_STATUS_SUCCESS           on success
 */
OS_API os_status_t os_thread_rwlock_create(
	os_thread_rwlock_t *lock
);

/**
 * @brief Destroys a previously created read/write lock
 *
 * @param[in,out]  lock                previously created lock
 *
 * @retval OS_STATUS_BAD_PARAMETER     invalid parameter passed to function
 * @retval OS_STATUS_FAILURE           function failed
 * @retval OS_STATUS_SUCCESS           on success
 */
OS_API os_status_t os_thread_rwlock_destroy(
	os_thread_rwlock_t *lock
);

/**
 * @brief Obtains a read/write lock for reading
 *
 * @param[in,out]  lock                previously created lock
 *
 * @retval OS_STATUS_BAD_PARAMETER     invalid parameter passed to function
 * @retval OS_STATUS_FAILURE           function failed
 * @retval OS_STATUS_SUCCESS           on success
 */
OS_API os_status_t os_thread_rwlock_read_lock(
	os_thread_rwlock_t *lock
);

/**
 * @brief Releases read/write lock from reading
 *
 * @param[in,out]  lock                previously created lock
 *
 * @retval OS_STATUS_BAD_PARAMETER     invalid parameter passed to function
 * @retval OS_STATUS_FAILURE           function failed
 * @retval OS_STATUS_SUCCESS           on success
 */
OS_API os_status_t os_thread_rwlock_read_unlock(
	os_thread_rwlock_t *lock
);

/**
 * @brief Obtains a read/write lock for writing
 *
 * @param[in,out]  lock                previously created lock
 *
 * @retval OS_STATUS_BAD_PARAMETER     invalid parameter passed to function
 * @retval OS_STATUS_FAILURE           function failed
 * @retval OS_STATUS_SUCCESS           on success
 */
OS_API os_status_t os_thread_rwlock_write_lock(
	os_thread_rwlock_t *lock
);

/**
 * @brief Releases read/write lock from writing
 *
 * @param[in,out]  lock                previously created lock
 *
 * @retval OS_STATUS_BAD_PARAMETER     invalid parameter passed to function
 * @retval OS_STATUS_FAILURE           function failed
 * @retval OS_STATUS_SUCCESS           on success
 */
OS_API os_status_t os_thread_rwlock_write_unlock(
	os_thread_rwlock_t *lock
);

/**
 * @brief Waits for a thread to complete
 *
 * @param[in,out]  thread              thread object to wait on
 *
 * @retval OS_STATUS_BAD_PARAMETER     invalid parameter passed to function
 * @retval OS_STATUS_FAILURE           function failed
 * @retval OS_STATUS_SUCCESS           on success
 */
OS_API os_status_t os_thread_wait(
	os_thread_t *thread
);

/**
 * @brief Wait indefinitely on a condition variable
 *
 * @param[in,out]  cond                condition variable to wait on
 * @param[in,out]  lock                mutex protecting condition variable
 *
 * @retval OS_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval OS_STATUS_FAILURE          function failed
 * @retval OS_STATUS_SUCCESS          on success
 */
OS_API os_status_t os_thread_condition_wait(
	os_thread_condition_t *cond,
	os_thread_mutex_t *lock
);




/* if there is no file system */
/**
 * @def PATH_MAX
 * @brief Maximum path length
 */
#ifndef PATH_MAX
#	define PATH_MAX 0
#endif


/* adapter information (for obtaining MAC addresses) */
/**
 * @brief Get general information of the current adapter in the the list
 *
 * @param[in,out]  adapters            adapter list to retrieve information from
 * @param[out]     family              family the adapter belongs to
 * @param[out]     flags               flags set on the adapter
 * @param[out]     address             buffer to fill with the address
 * @param[in]      address_len         size of the address buffer
 *
 * @retval OS_STATUS_BAD_PARAMETER     invalid parameter passed to the function
 * @retval OS_STATUS_FAILURE           system error
 * @retval OS_STATUS_SUCCESS           on success
 *
 * @see os_adapters_next
 * @see os_adapters_obtain
 * @see os_adapters_release
 */
OS_API os_status_t os_adapters_address(
	os_adapters_t *adapters,
	int *family,
	int *flags,
	char *address,
	size_t address_len
);

/**
 * @brief Get the index of the current adapter in the the list
 *
 * @param[in,out]  adapters            adapter list to retrieve information from
 * @param[out]     index               index of the adapter
 *
 * @retval OS_STATUS_BAD_PARAMETER     invalid parameter passed to the function
 * @retval OS_STATUS_FAILURE           system error
 * @retval OS_STATUS_SUCCESS           on success
 *
 * @see os_adapters_next
 * @see os_adapters_obtain
 * @see os_adapters_release
 */
OS_API os_status_t os_adapters_index(
	os_adapters_t *adapters,
	unsigned int *index
);

/**
 * @brief Get the mac address of the current adapter in the list
 *
 * @param[in,out]  adapters            adapter list to retrieve information from
 * @param[out]     mac                 buffer to fill with the mac address
 * @param[in]      mac_len             size of the buffer
 *
 * @retval OS_STATUS_BAD_PARAMETER     invalid parameter passed to the function
 * @retval OS_STATUS_FAILURE           system error
 * @retval OS_STATUS_SUCCESS           on success
 *
 * @see os_adapters_next
 * @see os_adapters_obtain
 * @see os_adapters_release
 */
OS_API os_status_t os_adapters_mac(
	os_adapters_t *adapters,
	char *mac,
	size_t mac_len
);

/**
 * @brief Moves to the next adapter in the list
 *
 * @param[in,out]  adapters            adapter list to iterate
 *
 * @retval OS_STATUS_BAD_PARAMETER     invalid parameter passed to the function
 * @retval OS_STATUS_FAILURE           system error
 * @retval OS_STATUS_SUCCESS           on success
 *
 * @see os_adapters_obtain
 * @see os_adapters_release
 */
OS_API os_status_t os_adapters_next(
	os_adapters_t *adapters
);

/**
 * @brief Obtains the list of adapters connected to the system
 *
 * @param[out]     adapters            list of adapters
 *
 * @retval OS_STATUS_BAD_PARAMETER     invalid parameter passed to the function
 * @retval OS_STATUS_FAILURE           system error
 * @retval OS_STATUS_NO_MEMORY         out of memory
 * @retval OS_STATUS_SUCCESS           on success
 *
 * @see os_adapters_next
 * @see os_adapters_release
 */
OS_API os_status_t os_adapters_obtain(
	os_adapters_t **adapters
);

/**
 * @brief Release memory allocated for the adapters
 *
 * @param[in,out]  adapters            adapter list to release
 *
 * @retval OS_STATUS_BAD_PARAMETER     invalid parameter passed to the function
 * @retval OS_STATUS_FAILURE           system error
 * @retval OS_STATUS_SUCCESS           on success
 *
 * @see os_adapters_next
 * @see os_adapters_obtain
 */
OS_API os_status_t os_adapters_release(
	os_adapters_t *adapters
);

/* character testing support */
/**
 * @brief Checks whether @p c is an alphabetical or numerical character
 *
 * @retval OS_FALSE                    character is not a alphabetical or
 *                                     numerical character
 * @retval OS_TRUE                     character is a alphabetical or  numerical
 *                                     character
 */
OS_API os_bool_t os_char_isalnum(
	char c
);

/**
 * @brief Checks whether @p c is a hexadecimal digit
 *
 * @retval OS_FALSE                   character is not a hexadecimal digit
 * @retval OS_TRUE                    character is hexadecimal digit
 */
OS_API os_bool_t os_char_isxdigit(
	char c
);

/** @brief flag indicating read operations */
#define OS_READ         0x01
/** @brief flag indicating write operations */
#define OS_WRITE        0x02
/** @brief flag indicating read & write operations */
#define OS_READ_WRITE   0x03
/** @brief flag indicating to append to end of the file */
#define OS_APPEND       0x04
/** @brief flag indicating to truncate the current contents */
#define OS_TRUNCATE     0x08
/** @brief flag indicating to create the file if it doesn't exist */
#define OS_CREATE       0x10
/** @brief flag indicating exclusive access to the file  */
#define OS_EXCLUSIVE    0x20
/** @brief flag indicating to that the file must not exist and to create it */
#define OS_CREATE_ONLY  0x30

/**
 * @brief Create a directory at the path specified with max_time_out in milliseconds
 *
 * @param[in]      path                path to directory to be created
 * @param[in]      timeout             maximum timeout for operation to complete
 *
 * @retval OS_STATUS_BAD_PARAMETER     invalid parameter passed to the function
 * @retval OS_STATUS_FAILURE           system error
 * @retval OS_STATUS_SUCCESS           on success
 */
OS_API os_status_t os_directory_create(
	const char *path,
	os_millisecond_t timeout
);

/**
 * @brief Create a directory at the path specified
 *
 * @param[in]      path                path to directory to be created
 *
 * @retval OS_STATUS_BAD_PARAMETER     invalid parameter passed to the function
 * @retval OS_STATUS_FAILURE           system error
 * @retval OS_STATUS_SUCCESS           on success
 */
OS_API os_status_t os_directory_create_nowait(
	const char *path
);

/**
 * @brief Stores the current directory into the buffer specified
 *
 * @param[out]     buffer              output destination
 * @param[in]      size                maximum size of destination buffer
 *
 * @retval OS_STATUS_BAD_PARAMETER     invalid parameter passed to the function
 * @retval OS_STATUS_FAILURE           system error or the buffer is too small
 * @retval OS_STATUS_SUCCESS           on success
 */
OS_API os_status_t os_directory_current(
	char *buffer,
	size_t size
);

/**
 * @brief Change the specified directory as current working directory
 *
 * @param[in]      path                path to directory to be changed
 *
 * @retval OS_STATUS_BAD_PARAMETER     invalid parameter passed to the function
 * @retval OS_STATUS_FAILURE           failed to change current directory
 * @retval OS_STATUS_SUCCESS           directory was successfully changed
 */
OS_API os_status_t os_directory_change(
	const char *path
);

/**
 * @brief Opens the specified directory for exploration
 *
 * @param[in]      dir_path            path to directory to open
 *
 * @retval NULL                        failed to open directory
 * @retval !NULL                       directory was successfully opened
 */
OS_API os_dir_t *os_directory_open(
	const char *dir_path
);

/**
 * @brief Closes a previously opened directory stream
 *
 * @param[in]      dir                 directory to close
 *
 * @retval OS_STATUS_FAILURE           failed to close directory
 * @retval OS_STATUS_SUCCESS           successfully closed directory
 */
OS_API os_status_t os_directory_close(
	os_dir_t *dir
);

/**
 * @brief Deletes all files (and empty directories) matching a regular
 *        expression from the given directory
 *
 * @note This only works in the directory specified, i.e. it doesn't recurse down
 * @note This function doesn't remove the base directory
 *
 * @param[in]      path                path to directory to delete
 * @param[in]      regex               regular expression for a sub-directory of
 *                                     file to match (a value of NULL will also
 *                                     delete this directory; a value of "*"
 *                                     deletes all files in the directory
 *                                     but leaves directory)
 * @param[in]      recursive           go recursively through subdirectories
 *
 * @retval OS_STATUS_BAD_PARAMETER     invalid parameter passed to function
 * @retval OS_STATUS_BAD_REQUEST       regular expression contains invalid
 *                                     character(s)
 * @retval OS_STATUS_FAILURE           on failure
 * @retval OS_STATUS_NO_MEMORY         out of memory
 * @retval OS_STATUS_SUCCESS           on success
 * @retval OS_STATUS_TRY_AGAIN         directory is not empty
 *
 * @see os_directory_delete
 * @see os_directory_exists
 * @see os_file_delete
 */
OS_API os_status_t os_directory_delete(
	const char *path,
	const char *regex,
	os_bool_t recursive
);

/**
 * @brief Returns whether or not a directory exists
 *
 * @param[in]      dir_path            path to directory to check existence
 *
 * @retval OS_TRUE                     directory exists
 * @retval OS_FALSE                    directory does not exist
 */
OS_API os_bool_t os_directory_exists(
	const char *dir_path
);

/**
 * @brief Get amount of free space on partition containing path
 *
 * @param[in]       path               path of file indicating which partition
 *                                     to query
 *
 * @retval          >=0                free space on partition in bytes
 * @retval          -1                 on failure
 */
OS_API os_uint64_t os_directory_free_space(
	const char* path
);

/**
 * @brief Get temp directory path
 *
 * @param[in]       dest               destination buffer
 * @param[in]       size               buffer size
 *
 * @return the os temp directory path
 *
 */
OS_API const char *os_directory_get_temp_dir(
	char * dest, size_t size
);

/**
 * @brief Get next file in opened directory
 *
 * @param[in]      dir                 directory to look in
 * @param[in]      files_only          only return the next file, ignoring all
 *                                     sub-directories
 * @param[out]     path                buffer to write the path of next file to
 * @param[in]      path_len            size of buffer
 *
 * @retval OS_STATUS_FAILURE           failed to retrieve next file, might have
 *                                     reached the end of the directory
 * @retval OS_STATUS_SUCCESS           successfully retrieved the next file
 */
OS_API os_status_t os_directory_next(
	os_dir_t *dir,
	os_bool_t files_only,
	char *path,
	size_t path_len
);

/**
 * @brief Reset a directory stream to the beginning of its contents
 *
 * @param[in,out]  dir                 directory to reset
 *
 * @retval OS_STATUS_FAILURE           failed to rewind directory
 * @retval OS_STATUS_SUCCESS           successfully rewound directory
 *
 *
 */
OS_API os_status_t os_directory_rewind(
	os_dir_t *dir
);

/**
 * @brief changes the owner and group of a file
 *
 * @param[in]      path                path to the specified file
 * @param[in]      user                new owner for the file
 *
 * @retval OS_STATUS_BAD_PARAMETER     invalid parameter passed to function
 * @retval OS_STATUS_FAILURE           failed to change ownership of the file
 * @retval OS_STATUS_SUCCESS           successfully changed ownership of the
 *                                     file
 */
OS_API os_status_t os_file_chown(
	const char *path,
	const char *user
);

/**
 * @brief Close file stream
 *
 * @param[in,out]  handle              pointer to file stream to close
 *
 * @retval         OS_STATUS_SUCCESS   successfully closed file stream
 * @retval         OS_STATUS_FAILURE   failed to close file stream
 *
 * @see os_file_open
 */
OS_API os_status_t os_file_close(
	os_file_t handle
);

/**
 * @brief Copies a file in the file system
 *
 * @param[in]      old_path            path of the file to copy
 * @param[in]      new_path            location to copy file to
 *
 * @retval OS_STATUS_BAD_PARAMETER     invalid parameter passed to function
 * @retval OS_STATUS_FAILURE           on failure
 * @retval OS_STATUS_SUCCESS           on success
 *
 * @see os_file_move
 */
OS_API os_status_t os_file_copy(
	const char *old_path,
	const char *new_path
);

/**
 * @brief Deletes a file from the system
 *
 * @param[in]      path                path to file to delete
 *
 * @retval OS_STATUS_BAD_PARAMETER     invalid parameter passed to function
 * @retval OS_STATUS_FAILURE           on failure
 * @retval OS_STATUS_SUCCESS           on success
 *
 * @see os_directory_delete
 * @see os_file_exists
 */
OS_API os_status_t os_file_delete(
	const char *path
);

/**
 * @brief Determines whether a specified file exists
 *
 * @param[in]      file_path           path to specified file
 *
 * @retval         OS_TRUE             file exists
 * @retval         OS_FALSE            file does not exist
 *
 * @see os_file_remove
 */
OS_API os_bool_t os_file_exists(
	const char *file_path
);

/**
 * @brief Moves a file in the file system
 *
 * @param[in]      old_path            path of the file to move
 * @param[in]      new_path            location to move file to
 *
 * @retval OS_STATUS_BAD_PARAMETER     invalid parameter passed to function
 * @retval OS_STATUS_FAILURE           on failure
 * @retval OS_STATUS_SUCCESS           on success
 *
 * @see os_file_copy
 */
OS_API os_status_t os_file_move(
	const char *old_path,
	const char *new_path
);

/**
 * @brief Open a file stream for reading or writing
 *
 * @param[in]      file_path           path to file to open
 * @param[in]      flags               access mode flags for file stream
 *
 * @note flags are set as a bitwise OR operation of the following:
 *       \n OS_READ
 *       \n OS_WRITE
 *       \n OS_APPEND
 *       \n OS_EXCLUSIVE
 *       \n OS_CREATE_ONLY
 *       \n OS_TRUNCATE
 *
 * @retval         !NULL               on success
 * @retval         NULL                on failure
 *
 * @see os_file_close
 */
OS_API os_file_t os_file_open(
	const char *file_path,
	int flags
);

/**
 * @brief Move the current position in an file stream
 *
 * @param[in,out]  stream              stream to pointer of open file
 * @param[in]      offset              amount to remove file pointer
 * @param[in]      whence              where to apply the offset at
 *
 * @retval OS_STATUS_BAD_PARAMETER     bad parameter passed to the function
 * @retval OS_STATUS_FAILURE           failed to move file pointer
 * @retval OS_STATUS_SUCCESS           on success
 *
 * @see os_file_tell
 */
OS_API os_status_t os_file_seek(
	os_file_t stream,
	long offset,
	int whence
);

/**
 * @brief Get size of file in bytes
 *
 * @param[in]      file_path           path to file
 *
 * @return         File size in bytes
 *
 * @see os_file_size_handle
 */
OS_API os_uint64_t os_file_size(
	const char *file_path
);

/**
 * @brief Get size of file in bytes
 *
 * @param[in]      file_handle         file handle
 *
 * @return         File size in bytes
 *
 * @see os_file_size
 */
OS_API os_uint64_t os_file_size_handle(
	os_file_t file_handle
);

/**
 * @brief Synchronize a file's in-core state with storage device
 *
 * @param[in]      file_path           path to file to synchronize
 *
 * @retval OS_STATUS_BAD_PARAMETER     bad parameter passed to the function
 * @retval OS_STATUS_SUCCESS           successfully sync the file
 * @retval OS_STATUS_FAILURE           failed to sync file stream
 *
 */
OS_API os_status_t os_file_sync(
	const char *file_path
);

/**
 * @brief Generates a temporary file based on the specified prototype
 *
 * @param[in,out]  prototype           prototype to use / temporary file name
 * @param[in]      suffix_len          suffix length
 *
 * @retval OS_STATUS_FAILURE           on failure
 * @retval OS_STATUS_SUCCESS           on success
 */
OS_API os_status_t os_file_temp(
	char *prototype,
	size_t suffix_len
);

/**
 * @brief Waits for the user to press a key on a console window
 *
 * @return the character that was received
 */
OS_API char os_key_wait( void );

/**
 * @brief Builds the path to a file for the operating system
 *
 * @param[in]      path                destination to store result
 * @param[in]      len                 size of destination buffer
 * @param[in]      ...                 directories and file name passed in as
 *                                     strings, until NULL is passed.
 *
 * @retval OS_STATUS_BAD_PARAMETER     invalid parameter passed to function
 * @retval OS_STATUS_FULL              the buffer specified is too small to hold
 *                                     the path
 * @retval OS_STATUS_SUCCESS           on success
 */
OS_API os_status_t os_make_path(
	char *path,
	size_t len,
	...
);

/* string functions */
/**
 * @brief Convert a string to an integer
 *
 * @param[in]      str                 string to convert
 *
 * @return an integer value of the string converted (0 is the default)
 */
OS_API int os_atoi(
	const char *str
);

/**
 * @brief Convert an integer to a string
 *
 * @param[in]      value               number to convert
 * @param[out]     str                 output buffer
 * @param[in]      str_len             output buffer length
 * @param[in]      base                Numerical base used to represent the
 *                                     value as a string, between 2 and 36,
 *                                     where 10 means decimal base,
 *                                     16 hexadecimal, 8 octal, and 2 binary.
 *
 * @retval         NULL                @p base is not in a valid range,
 *                                     @p str is NULL or the buffer is not large
 *                                     enough
 * @retval         !NULL               a pointer to the resulting
 *                                     null-terminated string, same as @p str
 */
OS_API char *os_itoa(
	int value,
	char *str,
	size_t str_len,
	int base );

/**
 * @brief Compares two strings ignoring case
 *
 * @param[in]      s1                  first string to compare
 * @param[in]      s2                  second string to compare
 *
 * @retval         <0                  the first non-matching character has a
 *                                     lower value in s1 than in s2
 * @retval         0                   the two strings are equal
 * @retval         >0                  the first non-matching character has a
 *                                     higher value in s1 than in s2
 */
OS_API int os_strcasecmp(
	const char *s1,
	const char *s2
);

/**
 * @brief Finds the length of the initial portion of str1 before the first occurrence
 *        of any characters that are part of str2
 *
 * @param[in]      str1                String to search
 * @param[in]      str2                String containing characters to match
 *
 * @return The length of the initial portion of str1 containing only characters
 *         that do not appear in str2
 */
size_t os_strcspn(
	const char *str1,
	const char *str2
);

/**
 * @brief Compares two strings up to a certain number of characters ignoring case
 *
 * @param[in]      s1                  first string to compare
 * @param[in]      s2                  second string to compare
 * @param[in]      len                 maximum number of characters to compare
 *
 * @retval         <0                  the first non-matching character has a
 *                                     lower value in s1 than in s2
 * @retval         0                   the two strings are equal up to len
 *                                     characters
 * @retval         >0                  the first non-matching character has a
 *                                     higher value in s1 than in s2
 */
OS_API int os_strncasecmp(
	const char *s1,
	const char *s2,
	size_t len
);

/**
 * @brief Concatenates two strings
 *
 * @param[in]      s1                  destination string
 * @param[in]      s2                  string to add to s1
 * @param[in]      count               number of characters to concatenate from s2 to s1
 *
 * @retval         !NULL               pointer to the successfully concatenated string
 * @retval         NULL                indicates an error occurred
 */
OS_API char *os_strncat(
	char *s1,
	const char *s2,
	size_t count
	);

/**
 * @brief Finds the length of the initial portion of str1 which consists only
 *        of characters that are part of str2
 *
 * @param[in]      str1                String to search
 * @param[in]      str2                String containing characters to match
 *
 * @return The length of the initial portion of str1 containing only characters
 *         that appear in str2
 */
OS_API size_t os_strspn(
	const char *str1,
	const char *str2
);

/**
 * @brief Get the next token from a string
 *
 * @param[in,out]  str                 string to truncate, if it is set to NULL,
 *                                     it will use the last str value
 * @param[in]      delimiters          string containing delimiter characters
 *
 * @retval         !NULL               token is found
 * @retval         NULL                no token found
 */
OS_API char *os_strtok(
	char *str,
	const char *delimiters
);

/* print functions */
/**
 * @brief Extracts any environment variables in the source string
 *
 * @note Enviroment variables in the source string are in the format as defined
 *       by the operating system (i.e. for Windows: %VAR%; for POSIX: $VAR )
 *
 * @param[in,out]  src                 source string (containing environment
 *                                     variable as defined by the operating
 *                                     system)
 * @param[in]      in_len              input buffer size (0 if null-terminated)
 * @param[in]      out_len             output buffer size
 *                                     (will be null-terminated, 0 if wanting to
 *                                     know output size)
 *
 * @retval 0       if src is NULL or the input length is > the output length
 * @retval >0      the size of the output string not including the
 *                 null-terminating character (or the required size if:
 *                 out_len == 0)
 *
 * @see os_env_get
 */
OS_API size_t os_env_expand(
	char *src,
	size_t in_len,
	size_t out_len
);

/**
 * @brief Obtains the value for a single environment variable
 *
 * @note The string is guaranteed to be null-terminated (if @c dest is supplied
 *       and @c len > 0u);  If the buffer is not large enough then result
 *       will equal len
 *
 * @param[in]      env                 environment variable
 * @param[in,out]  dest                destination string
 * @param[in]      len                 destination size
 *
 * @retval 0       if either @c env or @c dest is @c NULL
 * @retval >0      the size of the output string
 *
 * @see os_env_expand
 */
OS_API size_t os_env_get(
	const char *env,
	char *dest,
	size_t len
);

/**
 * @brief Writes output to a string
 *
 * @warning this function may write pass the end of the string, for safety
 *          use os_snprintf instead
 *
 * @param[out]     str                 string to output to
 * @param[in]      format              string format
 * @param[in]      ...                 items to replace based on @p format
 *
 * @retval >=0 the number of characters printed not including the
 *             null-terminator
 * @retval <0  an error occurred
 *
 * @see os_snprintf
 */
OS_API int os_sprintf(
	char *str,
	const char *format,
	...
) __attribute__((format(printf,2,3)));

/**
 * @brief Writes output to a string with a maximum size
 *
 * @param[out]     str                 string to output to
 * @param[in]      size                maximum size of buffer
 * @param[in]      format              string format
 * @param[in]      ...                 items to replace based on @p format
 *
 * @retval >=0     the number of characters printed not including the
 *                 null-terminator
 * @retval -1      an error occurred or the buffer is too small
 *
 * @see os_sprintf
 * @see os_vsnprintf
 */
OS_API int os_snprintf(
	char *str,
	size_t size,
	const char *format,
	...
) __attribute__((format(printf,3,4)));

/**
 * @brief Flushes a stream to ensure a buffer is transmitted
 *
 * @param[in]      stream              stream to flush
 *
 * @retval OS_FALSE                    on failure
 * @retval OS_TRUE                     on success
 */
OS_API os_bool_t os_flush(
	os_file_t stream
);

/**
 * @brief Determines if a given path is an absolute path
 *
 * @note This function does NOT check if the path contains invalid characters
 *
 * @param[in]      path                path to test
 *
 * @retval OS_TRUE     path is an absolute path
 * @retval OS_FALSE    path is not an absolute path (or PATH is NULL)
 */
OS_API os_bool_t os_path_is_absolute(
	const char *path
);

/**
 * @brief Get the full path to executable
 *
 * @param[out]     path                buffer to put full path to executable
 * @param[in]      size                size of the buffer
 *
 * @retval OS_STATUS_BAD_PARAMETER     invalid parameter
 * @retval OS_STATUS_FAILURE           on failure
 * @retval OS_STATUS_SUCCESS           on success
 */
OS_API os_status_t os_path_executable(
	char *path,
	size_t size
);

/* process functions */
/**
 * @brief Release resources when a child process exists
 *
 * @retval OS_STATUS_FAILURE           on failure
 * @retval OS_STATUS_SUCCESS           on success
 */
OS_API os_status_t os_process_cleanup( void );

/* service functions */
/**
 * @brief Function to call if running as a service
 *
 * @param[in]      id                  id identifying the service
 * @param[in]      service_function    function pointer for service controller
 *                                     to run
 * @param[in]      argc                parameter count for @c service_function
 * @param[in,out]  argv                array of parameters for
 *                                     @c service_function
 * @param[in]      remove_argc         size of remove argument array
 * @param[in]      remove_argv         array of arguments to remove before
 *                                     passing to the @c service_function
 * @param[in]      handler             service signal handler
 * @param[in]      logdir              directory where log files will be placed
 *
 * @retval OS_STATUS_SUCCESS           on success
 * @retval OS_STATUS_BAD_PARAMETER     on bad parameters
 * @retval OS_STATUS_FAILURE           on failure to start service
 */
OS_API os_status_t os_service_run(
	const char *id,
	os_service_main_t service_function,
	int argc,
	char *argv[],
	int remove_argc,
	const char *remove_argv[],
	os_sighandler_t handler,
	const char *logdir
);

/**
 * @brief Install an executable as a service
 *
 * @param[in]      id                  id identifying the service
 * @param[in]      executable          full path to the executable
 * @param[in]      args                arguments to pass to the executable
 * @param[in]      name                name (or short description) of the
 *                                     service
 * @param[in]      description         (optional) brief description of the
 *                                     service
 * @param[in]      dependencies        (optional) list of dependencies,
 *                                     seperated by semi-colons
 * @param[in]      max_time_out        maximum time out for operation to
 *                                     complete
 *
 * @retval OS_STATUS_SUCCESS           on success
 * @retval OS_STATUS_BAD_PARAMETER     on bad mandatory parameter(s)
 * @retval OS_STATUS_EXISTS            service already exists
 * @retval OS_STATUS_FAILURE           on failure
 * @retval OS_STATUS_NOT_SUPPORTED     if not supported on current OS
 *
 * @note Forward-slashes (/) and Backslashes (\) are not valid within a
 *       service id
 */
OS_API os_status_t os_service_install(
	const char *id,
	const char *executable,
	const char *args,
	const char *name,
	const char *description,
	const char *dependencies,
	os_millisecond_t max_time_out
);

/**
 * @brief Uninstall a service
 *
 * @param[in]      id                  id identifying the service
 * @param[in]      max_time_out        maximum time out for operation to complete
 *
 * @retval OS_STATUS_SUCCESS           on success
 * @retval OS_STATUS_BAD_PARAMETER     on bad mandatory parameter(s)
 * @retval OS_STATUS_FAILURE           on failure
 * @retval OS_STATUS_NOT_SUPPORTED     if not supported on current OS
 *
 * @note Forward-slashes (/) and Backslashes (\) are not valid within a
 *       service id
 */
OS_API os_status_t os_service_uninstall(
	const char *id,
	os_millisecond_t max_time_out
);

/**
 * @brief Start a service
 *
 * @param[in]      id                  id identifying the service
 * @param[in]      max_time_out        maximum time out for operation to complete
 *
 * @retval OS_STATUS_SUCCESS           on success
 * @retval OS_STATUS_BAD_PARAMETER     on bad mandatory parameter(s)
 * @retval OS_STATUS_FAILURE           on failure
 * @retval OS_STATUS_NOT_SUPPORTED     if not supported on current OS
 *
 * @note Forward-slashes (/) and Backslashes (\) are not valid within a
 *       service id.
 */
OS_API os_status_t os_service_start(
	const char *id,
	os_millisecond_t max_time_out
);

/**
 * @brief Stop a service
 *
 * @param[in]      id                  id identifying the service
 * @param[in]      exe                 filename used by the service
 * @param[in]      max_time_out        maximum time out for operation to complete
 *
 * @retval OS_STATUS_SUCCESS           on success
 * @retval OS_STATUS_BAD_PARAMETER     on bad mandatory parameter(s)
 * @retval OS_STATUS_FAILURE           on failure
 * @retval OS_STATUS_NOT_SUPPORTED     if not supported on current OS
 *
 * @note Forward-slashes (/) and Backslashes (\) are not valid within a
 *       service id.
 */
OS_API os_status_t os_service_stop(
	const char *id,
	const char *exe,
	os_millisecond_t max_time_out
);

/**
 * @brief Restart a service
 *
 * @param[in]      id                  id identifying the service
 * @param[in]      exe                 filename used by the service
 * @param[in]      max_time_out        maximum time out for operation to complete
 *
 * @retval OS_STATUS_SUCCESS           on success
 * @retval OS_STATUS_BAD_PARAMETER     on bad mandatory parameter(s)
 * @retval OS_STATUS_FAILURE           on failure
 * @retval OS_STATUS_NOT_SUPPORTED     if not supported on current OS
 *
 * @note Forward-slashes (/) and Backslashes (\) are not valid within a
 *       service id.
 */
OS_API os_status_t os_service_restart(
	const char *id,
	const char *exe,
	os_millisecond_t max_time_out
);

/**
 * @brief Queries a service for its current status
 *
 * @param[in]      id                  id identifying the service
 * @param[in]      max_time_out        maximum time out for operation to complete
 *
 * @retval OS_STATUS_BAD_PARAMETER     invalid parameter(s) passed to function
 * @retval OS_STATUS_FAILURE           failed to query service or in a failed state
 * @retval OS_STATUS_NOT_FOUND         service is not found
 * @retval OS_STATUS_NOT_INITIALIZED   service is not active (or connecting)
 * @retval OS_STATUS_NOT_SUPPORTED     if not supported on current OS
 * @retval OS_STATUS_SUCCESS           service is active & running
 * @retval OS_STATUS_TIMED_OUT         the query timed out before completion
 *
 * @note Forward-slashes (/) and Backslashes (\) are not valid within a
 *       service id.
 */
OS_API os_status_t os_service_query(
	const char *id,
	os_millisecond_t max_time_out
);

/* socket functions */
/**
 * @brief Get the IP address that a host resolves to
 *
 * @param[in]      host                hostname to resolve
 * @param[in]      service             service or port to use for resolution
 * @param[out]     address             the resulting IP address that the host
 *                                     resolves to
 * @param[in]      address_len         length of the IP address char array
 * @param[in]      family              address family to use for resolution
 *
 * @retval 0       on success
 * @retval !0      on failure (see getaddrinfo() return codes)
 */
 OS_API int os_get_host_address(
	const char *host,
	const char *service,
	char *address,
	size_t address_len,
	int family
);

/**
 * @brief Accepts an incoming connection
 *
 * @param[in]      socket              socket to accept connection on
 * @param[out]     out                 accepted socket connection
 * @param[in]      max_time_out        maximum time to wait for operation to
 *                                     complete
 *
 * @retval OS_STATUS_BAD_PARAMETER     invalid parameter(s) passed to function
 * @retval OS_STATUS_FAILURE           failed to accept connection
 * @retval OS_STATUS_NO_MEMORY         out of memory
 * @retval OS_STATUS_SUCCESS           successfully bound to socket
 * @retval OS_STATUS_TIMED_OUT         time out exceeded
 *
 * @see os_socket_bind
 */
OS_API os_status_t os_socket_accept(
	const os_socket_t *socket,
	os_socket_t **out,
	os_millisecond_t max_time_out
);

/**
 * @brief Binds to a socket to listen and accept connections on
 *
 * @param[in]      socket              socket to bind to
 * @param[in]      queue_size          size of queue to hold incoming
 *
 * @retval OS_STATUS_BAD_PARAMETER     invalid parameter(s) passed to function
 * @retval OS_STATUS_FAILURE           failed to bind to socket
 * @retval OS_STATUS_SUCCESS           successfully bound to socket
 *
 * @see os_socket_accept
 * @see os_socket_open
 * @see os_socket_close
 */
OS_API os_status_t os_socket_bind(
	const os_socket_t *socket,
	int queue_size
);

/**
 * @brief Sends a broadcast packet on the socket specified
 *
 * @param[in,out]  socket              socket to broadcast on
 * @param[in]      buf                 raw data to broadcast
 * @param[in]      len                 size of raw data to broadcast
 * @param[in]      ttl                 time-to-live for broadcast packet
 * @param[out]     bytes_written       number of bytes successfully written
 * @param[in]      max_time_out        maximum time to wait for operation to
 *                                     complete
 *
 * @retval OS_STATUS_BAD_PARAMETER     invalid parameter(s) passed to function
 * @retval OS_STATUS_FAILURE           failed to accept connection
 * @retval OS_STATUS_SUCCESS           successfully bound to socket
 * @retval OS_STATUS_TIMED_OUT         time out exceeded
 */
OS_API os_status_t os_socket_broadcast(
	const os_socket_t *socket,
	const void *buf,
	size_t len,
	int ttl,
	size_t *bytes_written,
	os_millisecond_t max_time_out
);

/**
 * @brief Closes an open socket
 *
 * @param[in,out]  socket              socket to close
 *
 * @retval OS_STATUS_BAD_PARAMETER     invalid parameter passed to the function
 * @retval OS_STATUS_FAILURE           on failure
 * @retval OS_STATUS_SUCCESS           on success
 *
 * @see os_socket_open
 */
OS_API os_status_t os_socket_close(
	os_socket_t *socket
);

/**
 * @brief Connects to a socket to send and receive data
 *
 * @param[in]      socket              socket to connect to
 *
 * @retval OS_STATUS_BAD_PARAMETER     invalid parameter passed to the function
 * @retval OS_STATUS_FAILURE           on failure
 * @retval OS_STATUS_SUCCESS           on success
 */
OS_API os_status_t os_socket_connect(
	const os_socket_t *socket
);

/**
 * @brief Initializes resources for raw socket communicationa within a process
 *
 * @retval OS_STATUS_FAILURE           on failure
 * @retval OS_STATUS_SUCCESS           on success
 *
 * @see os_socket_terminate
 */
OS_API os_status_t os_socket_initialize( void );

/**
 * @brief Opens an open socket
 *
 * @param[out]     out                 a pointer to the open socket
 * @param[in]      address             IPv4 or IPv6 host address to open
 * @param[in]      port                port to open socket on
 * @param[in]      type                socket type (SOCKET_STREAM or SOCKET_DGRAM)
 * @param[in]      protocol            protocol index to use in family
 * @param[in]      max_time_out        maximum time out for operation to complete
 *
 * @retval OS_STATUS_BAD_PARAMETER     invalid parameter passed to function
 * @retval OS_STATUS_FAILURE           on failure
 * @retval OS_STATUS_NO_MEMORY         out of memory
 * @retval OS_STATUS_SUCCESS           on success
 * @retval OS_STATUS_TIMED_OUT         time out exceeded
 *
 * @see os_socket_close
 */
OS_API os_status_t os_socket_open(
	os_socket_t **out,
	const char *address,
	os_uint16_t port,
	int type,
	int protocol,
	os_millisecond_t max_time_out
);

/**
 * @brief Sets options on a socket
 *
 * @param[in]      socket              socket to set option on
 * @param[in]      level               level of the socket
 * @param[in]      optname             option name
 * @param[in]      optval              array of option values
 * @param[in]      optlen              number of items in the array
 *
 * @retval OS_STATUS_BAD_PARAMETER     invalid parameter passed to function
 * @retval OS_STATUS_FAILURE           on failure
 * @retval OS_STATUS_SUCCESS           on success
 */
OS_API os_status_t os_socket_option(
	const os_socket_t *socket,
	int level,
	int optname,
	const void *optval,
	size_t optlen
);

/**
 * @brief reads data for an open socket
 *
 * @param[in]      socket              socket to read from
 * @param[out]     buf                 destination buffer
 * @param[in]      len                 size of buffer
 * @param[out]     bytes_read          amount of data read in bytes
 * @param[in]      max_time_out        maximum time out for operation to
 *                                     complete
 *
 * @retval OS_STATUS_BAD_PARAMETER     invalid parameter passed to function
 * @retval OS_STATUS_FAILURE           on failure
 * @retval OS_STATUS_SUCCESS           on success
 * @retval OS_STATUS_TIMED_OUT         time out exceeded
 * @retval OS_STATUS_TRY_AGAIN         socket not connected
 *
 * @see os_socket_write
 */
OS_API os_status_t os_socket_read(
	const os_socket_t *socket,
	void *buf,
	size_t len,
	size_t* bytes_read,
	os_millisecond_t max_time_out
);

/**
 * @brief Receives data on a socket
 *
 * @param[in]      socket              socket to receive data on
 * @param[out]     buf                 destination buffer
 * @param[in]      len                 size of destination buffer
 * @param[out]     src_addr            address of source (optional)
 * @param[in]      src_addr_len        lenth of source address buffer (optional)
 * @param[out]     port                port data received on (optional)
 * @param[in]      max_time_out        maximum time to wait (optional)
 *
 * @retval OS_STATUS_BAD_PARAMETER     invalid parameter passed to function
 * @retval OS_STATUS_FAILURE           on failure
 * @retval OS_STATUS_SUCCESS           on success
 * @retval OS_STATUS_TIMED_OUT         time out exceeded
 * @retval OS_STATUS_TRY_AGAIN         socket not connected
 */
OS_API ssize_t os_socket_receive(
	const os_socket_t *socket,
	void *buf,
	size_t len,
	char *src_addr,
	size_t src_addr_len,
	os_uint16_t *port,
	os_millisecond_t max_time_out
);

/**
 * @brief Sends data on a socket
 *
 * @param[in]      socket              socket to send data on
 * @param[in]      buf                 source buffer
 * @param[in]      len                 size of source buffer
 * @param[in]      dest_addr           destination address
 * @param[in]      port                destination port
 * @param[in]      max_time_out        maximum time to wait (optional)
 *
 * @retval OS_STATUS_BAD_PARAMETER     invalid parameter passed to function
 * @retval OS_STATUS_FAILURE           on failure
 * @retval OS_STATUS_SUCCESS           on success
 * @retval OS_STATUS_TIMED_OUT         time out exceeded
 * @retval OS_STATUS_TRY_AGAIN         socket not connected
 */
OS_API ssize_t os_socket_send(
	const os_socket_t *socket,
	const void *buf,
	size_t len,
	const char *dest_addr,
	os_uint16_t port,
	os_millisecond_t max_time_out
);

/**
 * @brief Cleans up resources utilized for raw socket communication
 *        within a process
 *
 * @retval OS_STATUS_FAILURE           on failure
 * @retval OS_STATUS_SUCCESS           on success
 *
 * @see os_socket_initialize
 */
OS_API os_status_t os_socket_terminate( void );

/**
 * @brief Writes data to an open socket
 *
 * @param[in]      socket              socket to write to
 * @param[in]      buf                 destination buffer
 * @param[in]      len                 size of buffer
 * @param[out]     bytes_written       number of bytes written in bytes
 * @param[in]      max_time_out        maximum amount of time to wait
 *
 * @retval OS_STATUS_BAD_PARAMETER     invalid parameter passed to function
 * @retval OS_STATUS_FAILURE           on failure
 * @retval OS_STATUS_SUCCESS           on success
 * @retval OS_STATUS_TIMED_OUT         time out exceeded
 *
 * @see os_socket_read
 */
OS_API os_status_t os_socket_write(
	const os_socket_t *socket,
	const void *buf,
	size_t len,
	size_t *bytes_written,
	os_millisecond_t max_time_out
);

/**
 * @brief Enable or disable echo in a stream
 *
 * @param[in]      stream              handle to stream to set
 * @param[in]      enable              true to enable, false to disable
 *
 * @retval OS_STATUS_BAD_PARAMETER     invalid parameter
 * @retval OS_STATUS_FAILURE           on failure
 * @retval OS_STATUS_SUCCESS           on success
 */
OS_API os_status_t os_stream_echo_set(
	os_file_t stream,
	os_bool_t enable );

/* operating system information */

/**
 * @brief Returns the error message for indicated operating system error number
 *
 * @warning The function is not thread-safe.
 *
 * @param[in]      error_number        error number to lookup (-1 for last error)
 *
 * @return a string representing the last operating system error
 */
OS_API const char *os_system_error_string(
	int error_number
);

/**
 * @brief Extracts information about the the operating system
 *
 * @param[in,out]  sys_info            structure to fill with operating system
 *                                     information
 *
 * @retval OS_STATUS_FAILURE           on failure
 * @retval OS_STATUS_SUCCESS           on success
 */
OS_API os_status_t os_system_info(
	os_system_info_t *sys_info
);

/**
 * @brief run an executable on the operating system asynchronously
 *
 * @param[in]      command             command string to be run
 * @param[out]     exit_status         return code of the executable (optional)
 * @param[in]      pipe_files          files/pipes to write 'standard out' &
 *                                     'standard error' to
 *
 * @retval OS_STATUS_INVOKED           command triggered as a background process
 * @retval OS_STATUS_NOT_EXECUTABLE    the path provided can not be executed
 */
OS_API os_status_t os_system_run(
	const char *command,
	int *exit_status,
	os_file_t pipe_files[2u] );

/**
 * @brief run an executable on the operating system and wait for it to complete
 *
 * @param[in]      command             command string to be run
 * @param[out]     exit_status         return code of the executable (optional)
 * @param[out]     out_buf             pointer to buffers to write
 *                                     'standard out' & 'standard error' to
 * @param[in,out]  out_len             size of buffers for 'standard out' and
 *                                     'standard error'
 * @param[in]      max_time_out        maximum amount of time to wait
 *
 * @retval OS_STATUS_IO_ERROR          failed to setup error & output capture io
 * @retval OS_STATUS_NOT_EXECUTABLE    the path provided can not be executed
 * @retval OS_STATUS_SUCCESS           on success
 * @retval OS_STATUS_TIMED_OUT         time out exceeded
 */
OS_API os_status_t os_system_run_wait(
	const char *command,
	int *exit_status,
	char *out_buf[2u],
	size_t out_len[2u],
	os_millisecond_t max_time_out );

/**
 * @brief Shutdown or reboot the device
 *
 * @param[in]      reboot              specify whether to reboot or shutdown the
 *                                     device
 * @param[in]      delay               time delay in minutes to reboot or shutdown
 *
 * @retval OS_STATUS_INVOKED           shutdown triggered
 * @retval OS_STATUS_NOT_EXECUTABLE    shutdown failed to be triggered
 */
OS_API os_status_t os_system_shutdown(
	os_bool_t reboot, unsigned int delay
);

/**
 * @brief tests the provided steram to determine if it supports vt-100
 *        terminal codes
 *
 * @param[in]      stream              stream to test
 *
 * @retval OS_TRUE                     terminal supports vt-100 codes
 * @retval OS_FALSE                    terminal does not support vt-100 codes
 */
OS_API os_bool_t os_terminal_vt100_support(
	os_file_t stream
);

/**
 * @brief Function to setup a signal handler
 *
 * @param[in]      signal_handler      callback function to be called when a
 *                                     signal is encountered, NULL to remove a
 *                                     previously registered signal handler
 *
 * @retval OS_STATUS_FAILURE           on failure
 * @retval OS_STATUS_SUCCESS           on success
 */
OS_API os_status_t os_terminate_handler(
	os_sighandler_t signal_handler
);

/* time functions */
/**
 * @brief Generate a random number between a minimum and maximum
 *
 * @param[in]     min                  minimum value
 * @param[in]     max                  maximum value
 *
 * @return a random number between the minimum and maximum specified
 */
OS_API double os_random(
	double min,
	double max
);

/**
 * @brief Returns the system time
 *
 * @note This function will return either an absolute time or up-time (in
 *       milliseconds) depending on the limitations of the system
 *
 * @param[out]     time_stamp          current time or up-time (in milliseconds)
 * @param[out]     up_time             set to OS_TRUE, if the system only
 *                                     supports up-time; this is always set
 *                                     correctly independent of this function's
 *                                     return code (optional)
 *
 * @retval OS_STATUS_BAD_PARAMETER     invalid parameter
 * @retval OS_STATUS_FAILURE           system call failed
 * @retval OS_STATUS_SUCCESS           on success
 */
OS_API os_status_t os_time(
	os_timestamp_t *time_stamp,
	os_bool_t *up_time
);

/**
 * @brief Returns the system time in a formatted string
 *
 * @param[out]     buf                 pointer to buffer to place the time
 *                                     string
 * @param[in]      len                 length of the buffer
 * @param[in]      format              format to use for the string following
 *                                     @c strffmt function format style
 * @param[in]      time_stamp          time stamp to format
 * @param[in]      to_local_time       format the time in the local time zone
 *                                     versus UTC time zone
 *
 * @return returns the number of bytes (excluding the null-terminating
 * character) placed into the output buffer.  If the length of the result
 * string (including the null-terminating character) would exceed @c len bytes
 * then this function returns a value of 0 and the contents include only the
 * null-terminating character.
 */
OS_API size_t os_time_format(
	char *buf,
	size_t len,
	const char *format,
	os_timestamp_t time_stamp,
	os_bool_t to_local_time );

/**
 * @brief Calculates the amount of time elapsed from a given time
 *
 * @param[in]      start_time          starting time
 * @param[out]     elapsed_time        elapsed time in milliseconds
 *
 * @retval OS_STATUS_BAD_PARAMETER     invalid parameter
 * @retval OS_STATUS_BAD_REQUEST       starting time is in the future
 * @retval OS_STATUS_SUCCESS           on success
 */
OS_API os_status_t os_time_elapsed(
	const os_timestamp_t *start_time,
	os_millisecond_t *elapsed_time );

/**
 * @brief Calculates the amount of time remaining from a start time and time out
 *
 * @param[in]      start_time          starting time
 * @param[in]      max_time_out        max amount of time (0 means forever)
 * @param[out]     remaining           amount of time remaining
 *
 * @retval OS_STATUS_BAD_PARAMETER     invalid parameter
 * @retval OS_STATUS_BAD_REQUEST       starting time is in the future
 * @retval OS_STATUS_TIMED_OUT         time out exceeded
 * @retval OS_STATUS_SUCCESS           on success
 */
OS_API os_status_t os_time_remaining(
	const os_timestamp_t *start_time,
	os_millisecond_t max_time_out,
	os_millisecond_t *remaining );

/**
 * @brief Sleeps for the specified time in milliseconds
 *
 * @param[in]      ms                  number of milliseconds to sleep for
 * @param[in]      allow_interrupts    allow the sleep to break on an interrupt
 *
 * @retval OS_STATUS_FAILURE           on system failure or system interruption
 * @retval OS_STATUS_SUCCESS           on success
 */
OS_API os_status_t os_time_sleep(
	os_millisecond_t ms,
	os_bool_t allow_interrupts
);

/* uuid support */
/**
 * @brief Generates a universally unique identifier
 *
 * @param[out]     uuid                generated identifier
 *
 * @retval OS_STATUS_BAD_PARAMETER     invalid parameter passed to function
 * @retval OS_STATUS_FAILURE           function failed
 * @retval OS_STATUS_SUCCESS           on success
 */
OS_API os_status_t os_uuid_generate(
	os_uuid_t *uuid
);

/**
 * @brief Converts universally unique identifier to lower-case
 *
 * @param[in]      uuid                universally unique identifier
 * @param[out]     dest                destination buffer
 * @param[in]      len                 size of destination buffer
 *
 * @retval OS_STATUS_BAD_PARAMETER     invalid parameter passed to function
 * @retval OS_STATUS_NO_MEMORY         buffer length is not large enough
 * @retval OS_STATUS_SUCCESS           on success
 */
OS_API os_status_t os_uuid_to_string_lower(
	os_uuid_t *uuid,
	char *dest,
	size_t len
);

#if !defined(_WRS_KERNEL)
extern void deviceCloudConfigDirSet ( const char *str );
extern void deviceCloudRuntimeDirSet ( const char *str );
extern void deviceCloudBinDirSet ( const char *str );
extern void deviceCloudPrioritySet ( const char *str );
extern void deviceCloudStackSizeSet ( const char *str );
#endif /* _WRS_KERNEL */

extern const char *deviceCloudConfigDirGet ( void );
extern const char *deviceCloudRuntimeDirGet ( void );
extern const char *deviceCloudBinDirGet ( void );
extern unsigned int deviceCloudPriorityGet ( void );
extern unsigned int deviceCloudStackSizeGet ( void );

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

#endif /* ifndef OS_H */
