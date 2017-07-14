/**
 * @file
 * @brief header file declaring types for Windows systems
 *
 * @copyright Copyright (C) 2016-2017 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.  No license to Wind River intellectual property rights is
 * granted herein.  All rights not licensed by Wind River are reserved by Wind
 * River.
 */
#ifndef OS_WIN32_H
#define OS_WIN32_H

#ifndef OS_H
#error "This file must be included only by os_.h"
#endif /* ifndef OS_H */

/** @brief Don't support advanced Windows kernel calls */
#define WIN32_LEAN_AND_MEAN
#pragma warning( push, 1 )
#include <Windows.h>
#include <Winsock2.h>
#include <wincrypt.h>

#include <rpc.h>        /* for uuid functions */
	/** @brief Universally unique id type */
	typedef struct _GUID os_uuid_t;
#pragma warning( pop )

/* Define missing signals on Windows */
/** @brief Interruption signal */
#define SIGINT  2
/** @brief User defined signal 1 */
#define SIGUSR1 10
/** @brief Termination signal */
#define SIGTERM 15
/** @brief Child process exit signal */
#define SIGCHLD 17
/** @brief Continue signal */
#define SIGCONT 18
/** @brief Stop signal */
#define SIGSTOP 19

/** @brief Maximum length of a host name on Windows systems */
#define OS_HOST_MAX_LEN 255u

/* missing types in Windows */
/** @brief Signed string length type */
#if !defined(ssize_t)
#define ssize_t SSIZE_T
#endif
/** @brief Socket port type */
typedef u_short in_port_t;

/**
 * @brief Character used to split environment variables
 */
#define OS_ENV_SPLIT               ';'

/**
 * @brief Character sequence for a line break
 */
#define OS_FILE_LINE_BREAK         "\r\n"
/**
 * @brief Seek from start of file
 * @see os_file_fseek
 */
#define OS_FILE_SEEK_START         FILE_BEGIN
/**
 * @brief Seek from current position in file
 * @see os_file_fseek
 */
#define OS_FILE_SEEK_CURRENT       FILE_CURRENT
/**
 * @brief Seek from end of file
 * @see os_file_fseek
 */
#define OS_FILE_SEEK_END           FILE_END
/**
 * @brief Null device, used to discard data
 */
#define OS_NULL_DEVICE             "NUL"
/**
 * @brief Invalid socket symbol
 */
#define OS_SOCKET_INVALID INVALID_SOCKET
/**
 * @brief Symbol to use when thread linking
 */
#define OS_THREAD_LINK   __stdcall
/**
 * @brief Return type for a thread main function
 */
#define OS_THREAD_RETURN DWORD

/**
 * @brief Handle to an open file
 */
typedef HANDLE os_file_t;
/**
 * @brief Defines type for invalid file handle
 */
#define OS_FILE_INVALID  INVALID_HANDLE_VALUE
/**
 * @brief Defines the symbol for standard error
 */
#define OS_STDERR        GetStdHandle( STD_ERROR_HANDLE )
/**
 * @brief Defines the symbol for standard in
 */
#define OS_STDIN         GetStdHandle( STD_INPUT_HANDLE )
/**
 * @brief Defines the symbol for standard out
 */
#define OS_STDOUT        GetStdHandle( STD_OUTPUT_HANDLE )

#pragma warning( push, 1 )
#include <iphlpapi.h>
#include <strsafe.h>
#include <ws2tcpip.h>
#pragma warning( pop )
	/**
	 * @def PATH_MAX
	 * @brief Maximum path length
	 */
#	ifndef PATH_MAX
#		define PATH_MAX 4096
#	endif /* ifndef PATH_MAX */

	/**
	 * @brief Structure holding internal adapter list
	 */
	typedef struct os_adapters
	{
		/** @brief Current adapter */
		IP_ADAPTER_ADDRESSES *adapter_current;
		/** @brief First adapter */
		IP_ADAPTER_ADDRESSES *adapter_first;
		/** @brief Current unicast address */
		IP_ADAPTER_UNICAST_ADDRESS *current;
	} os_adapters_t;

	/**
	 * @brief Directory seperator character
	 */
#	define OS_DIR_SEP '\\'
	/**
	 * @brief Structure holding directory list information
	 */
	typedef struct os_dir
	{
		/** @brief Handle to the open directory */
		HANDLE dir;
		/** @brief Last error result */
		os_status_t last_result;
		/** @brief Currently found file within the directory */
		WIN32_FIND_DATA wfd;
		/** @brief Path to the directory */
		char path[PATH_MAX];
	} os_dir_t;

	/**
	 * @brief Handle to an open dynamically-linked library
	 */
	typedef HMODULE os_lib_handle;

	/**
	 * @brief Handle to a thread
	 */
	typedef HANDLE os_thread_t;
	/**
	 * @brief Thread condition lock
	 */
	typedef CONDITION_VARIABLE os_thread_condition_t;
	/**
	 * @brief Thread mutually exclusive (mutex) lock
	 */
	typedef CRITICAL_SECTION os_thread_mutex_t;
	/**
	 * @brief Thread read/write lock
	 */
	typedef SRWLOCK os_thread_rwlock_t;


/* memory functions */
/**
 * @brief Change the size of an allocated memory block
 *
 * The contents of the previously allocated memory will be unchanged in the
 * range from the start up to the minimum of the old and new sizes.  If hte new
 * size is larger than the old size the added memory will not be initialized.
 *
 * @param[in]      ptr                 pointer to the previously allocated
 *                                     block, if ptr is NULL the call is
 *                                     equivilant to os_heap_malloc(size).
 *                                     This value must be a value returned by an
 *                                     earlier call to os_heap_calloc,
 *                                     os_heap_malloc or os_heap_realloc
 * @param[in]      size                new size for the allocated block
 *
 * @retval NULL    the specified amount of memory is not continously available
 * @retval !NULL   a pointer to the allocated memory
 *
 * @see os_heap_calloc
 * @see os_heap_free
 * @see os_heap_malloc
 */
OS_API OS_SECTION void *os_heap_realloc(
	void *ptr,
	size_t size
) __attribute__((malloc));

/**
 * @brief Frees previously allocated memory specified
 *
 * @param[in]      ptr            pointer of pointer to the allocated memory to free
 *
 * @see os_heap_calloc
 * @see os_heap_malloc
 * @see os_heap_realloc
 */
OS_API OS_SECTION void os_heap_free(
	void **ptr
);

/**
 * @brief Allocates the specified amount of bytes
 *
 * The memory returned is NOT initialized. Any allocated memory should be
 * deallocated with the corrosponding @p os_heap_free command
 *
 * @param[in]      size                amount of memory to allocate
 *
 * @retval NULL    the specified amount of memory is not continously available
 * @retval !NULL   a pointer to the allocated memory
 *
 * @see os_heap_calloc
 * @see os_heap_free
 * @see os_heap_realloc
 */
OS_API OS_SECTION void *os_heap_malloc(
	size_t size
) __attribute__((malloc));

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
OS_API OS_SECTION int os_strcmp(
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
OS_API OS_SECTION size_t os_strlen(
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
OS_API OS_SECTION int os_strncmp(
	const char *s1,
	const char *s2,
	size_t len
);

/**
 * @brief Copy characters from string
 *
 * @param[out]     destination         string to copy characters to
 * @param[in]      source              string to copy character from
 * @param[in]      num                 maximum number of characters to copy
 *
 * @retval         destination
 */
OS_API OS_SECTION char *os_strncpy(
	char *destination,
	const char *source,
	size_t num
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
OS_API OS_SECTION char *os_strrchr(
	const char *s,
	char c
);

/**
 * @brief Returns the process id of the current process
 *
 * @return the process id of the current process
 */
OS_API OS_SECTION os_uint32_t os_system_pid( void );

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
OS_API OS_SECTION int os_fprintf(
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
OS_API OS_SECTION int os_printf(
	const char *format,
	...
) __attribute__((format(printf,1,2)));
/**
 * @brief Writes output to a string with a maximum size
 *
 * @param[out]     str                 string to output to
 * @param[in]      size                maximum size of buffer
 * @param[in]      format              string format
 * @param[in]      ...                 items to replace based on @p format
 *
 * @return the number of characters printed including the null-terminator,
 *         if the output is truncated then the return value -1
 *
 * @see os_sprintf
 * @see os_vsnprintf
 */
OS_API OS_SECTION int os_snprintf(
	char *str,
	size_t size,
	const char *format,
	...
) __attribute__((format(printf,3,4)));
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
OS_API OS_SECTION int os_vfprintf(
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
OS_API OS_SECTION int os_vsnprintf(
	char *str,
	size_t size,
	const char *format,
	va_list args
) __attribute__((format(printf,3,0)));

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
OS_API OS_SECTION char *os_file_fgets(
	char *str,
	size_t size,
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
OS_API OS_SECTION size_t os_file_fputs(
	char *str,
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
OS_API OS_SECTION size_t os_file_fread(
	void *ptr,
	size_t size,
	size_t nmemb,
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
OS_API OS_SECTION size_t os_file_fwrite(
	const void *ptr,
	size_t size,
	size_t nmemb,
	os_file_t stream
);

/**
 * @brief Converts a character to lower case
 *
 * @param[in]      c                   character to convert
 *
 * @return the lower-case value of the character, or @p c if not possible
 */
OS_API OS_SECTION char os_char_tolower(
	char c
);

/**
 * @brief Converts a character to upper case
 *
 * @param[in]      c                   character to convert
 *
 * @return the upper-case value of the character, or @p c if not possible
 */
OS_API OS_SECTION char os_char_toupper(
	char c
);

/**
 * @brief Parse a string to retrieve a double
 *
 * @param[in]      str                 string to parse for double
 * @param[out]     endptr              (optional)pointer to character 
 *                                     immediately following double in string
 * @return value of double parsed
 */
OS_API OS_SECTION double os_strtod(
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
OS_API OS_SECTION long os_strtol(
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
OS_API OS_SECTION unsigned long os_strtoul(
	const char *str,
	char **endptr
);

#endif /* ifndef OS_WIN32_H */

