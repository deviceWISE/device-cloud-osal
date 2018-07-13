/**
 * @file
 * @brief Function definitions for Windows operating systems
 *
 * @copyright Copyright (C) 2016-2018 Wind River Systems, Inc. All Rights Reserved.
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

/** @brief Don't support advanced Windows kernel calls */
#pragma warning( push, 1 )
#	define WIN32_LEAN_AND_MEAN
#	include <Windows.h>
#	include <Winsock2.h>       /* for socket functions and structures */
#	include <ws2tcpip.h>       /* for SOCKETADDR_IN6 structure */
#	include <Iphlpapi.h>       /* for GetAdaptersAddresses */
#	include <rpc.h>        /* for uuid functions */
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
typedef USHORT in_port_t;

/** @brief Structure representing a network adapter address */
struct os_adapter_address
{
	IP_ADAPTER_ADDRESSES *adapter; /**< adapter */
	IP_ADAPTER_UNICAST_ADDRESS *cur; /**< current address */
};

/** @brief Structure representing a network adapter */
struct os_adapter
{
	IP_ADAPTER_ADDRESSES *first; /**< first network adapter */
	IP_ADAPTER_ADDRESSES *cur;   /**< current network adapter */
};

/**
 * @brief Directory seperator character
 */
#define OS_DIR_SEP                     '\\'
/**
 * @brief Character used to split environment variables
 */
#define OS_ENV_SPLIT                   ';'
/**
 * @brief Character sequence for a line break
 */
#define OS_FILE_LINE_BREAK             "\r\n"
/**
 * @brief Seek from start of file
 * @see os_file_seek
 */
#define OS_FILE_SEEK_START             FILE_BEGIN
/**
 * @brief Seek from current position in file
 * @see os_file_seek
 */
#define OS_FILE_SEEK_CURRENT           FILE_CURRENT
/**
 * @brief Seek from end of file
 * @see os_file_seek
 */
#define OS_FILE_SEEK_END               FILE_END
/**
 * @brief Null device, used to discard data
 */
#define OS_NULL_DEVICE                 "NUL"
/**
 * @brief Invalid socket symbol
 */
#define OS_SOCKET_INVALID              INVALID_SOCKET

#if OSAL_THREAD_SUPPORT
/**
 * @brief Symbol to use when thread linking
 */
#define OS_THREAD_LINK                 __stdcall
/**
 * @brief Return type for a thread main function
 */
#define OS_THREAD_RETURN               DWORD
/**
 * @brief Symbol to correct declare a thread on all platforms
 */
#define OS_THREAD_DECL OS_THREAD_RETURN OS_THREAD_LINK
/**
 * @brief Type defining the starting point for a thread
 */
typedef OS_THREAD_RETURN
	(OS_THREAD_LINK *os_thread_main_t)( void *arg );
#endif /* if OSAL_THREAD_SUPPORT */

/**
 * @brief Handle to an open file
 */
typedef HANDLE os_file_t;
/**
 * @brief Defines type for invalid file handle
 */
#define OS_FILE_INVALID                INVALID_HANDLE_VALUE
/**
 * @brief Defines the symbol for standard error
 */
#define OS_STDERR                      GetStdHandle( STD_ERROR_HANDLE )
/**
 * @brief Defines the symbol for standard in
 */
#define OS_STDIN                       GetStdHandle( STD_INPUT_HANDLE )
/**
 * @brief Defines the symbol for standard out
 */
#define OS_STDOUT                      GetStdHandle( STD_OUTPUT_HANDLE )

/**
 * @def PATH_MAX
 * @brief Maximum path length
 */
#ifndef PATH_MAX
#	define PATH_MAX 4096
#endif /* ifndef PATH_MAX */

/**
 * @brief Handle to an open dynamically-linked library
 */
typedef HMODULE os_lib_handle;

/**
 * @brief type represent a service entry
 */
typedef struct servent os_service_entry_t;

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
 * @brief Allocates memory for an array of elements
 *
 * The memory returned is NOT initialized. Any allocated memory should be
 * deallocated with the corrosponding @p os_free command
 *
 * @note Specifying either 0 elements or elements with a size of 0 may return a
 *       valid memory pointer (that can be later freed) or NULL
 *
 * @param[in]      num                 number of elements to allocate memory for
 * @param[in]      size                size of each element in bytes
 *
 * @retval NULL    the specified amount of memory is not continously available
 * @retval !NULL   a pointer to the allocated memory
 *
 * @see os_free
 * @see os_free_null
 * @see os_malloc
 * @see os_realloc
 */
#if !OSAL_WRAP
#define os_calloc( num, size )         HeapAlloc( GetProcessHeap(), 0, num * size )
#else
OS_API void *os_calloc(
	size_t nmemb,
	size_t size
);
#endif

/**
 * @brief Frees previously allocated memory
 *
 * @param[in]      ptr                 pointer to the allocated memory to free
 *
 * @warning @c ptr is not checked for NULL prior to deallocation
 *
 * @see os_calloc
 * @see os_free_null
 * @see os_malloc
 * @see os_realloc
 */
#if !OSAL_WRAP
#define os_free( ptr )                 HeapFree( GetProcessHeap(), 0, ptr );
#else
OS_API void os_free(
	void *ptr
);
#endif

/**
 * @brief Frees previously allocated memory specified
 *
 * @param[in]      ptr                 pointer of pointer to the allocated
 *                                     memory to free
 *
 * @see os_calloc
 * @see os_free
 * @see os_malloc
 * @see os_realloc
 */
#if !OSAL_WRAP
#define os_free_null(ptr)              { if ( *ptr ) HeapFree( GetProcessHeap(), 0, *ptr ); *ptr = NULL; }
#else
OS_API void os_free_null(
	void **ptr
);
#endif

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
 * @see os_realloc
 */
#if !OSAL_WRAP
#define os_malloc(size)                HeapAlloc( GetProcessHeap(), 0, size );
#else
OS_API void *os_malloc(
	size_t size
);
#endif

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
	char c );

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
 * @brief Returns the process id of the current process
 *
 * @return the process id of the current process
 */
OS_API os_uint32_t os_system_pid( void );

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
);

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
);

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
);

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
 * @brief Locate a substring
 *
 * @param[in]      str1                string to be searched
 * @param[in]      str2                string to search for
 *
 * @retval         !NULL               pointer to the substring of str2 in str1
 * @retval         NULL                no substring found
 */
#if !OSAL_WRAP
#define os_strstr(str1, str2)                  StrStr(str1, str2)
#else
OS_API char *os_strstr(
	const char *str1,
	const char *str2
);
#endif

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
 * @brief Finds a function within an open runtime library
 *
 * @param[in]      lib                 open library handle
 * @param[in]      function            function to find
 *
 * @retval NULL    no matching function found (or an error occurred)
 * @retval !NULL   a pointer to the matching function
 *
 * @see os_library_close
 * @see os_library_open
 */
#if !OSAL_WRAP
#define os_library_find(lib, function)         GetProcAddress(lib, function)
#else
OS_API void *os_library_find(
	os_lib_handle lib,
	const char *function
);
#endif

/**
 * @brief Opens a runtime library
 *
 * @param[in]      path                library to open
 *
 * @return         os_lib_handle of the opened library
 *
 * @see os_library_close
 * @see os_library_find
 */
#if !OSAL_WRAP
#define os_library_open(path)                  LoadLibrary(path)
#else
OS_API os_lib_handle os_library_open(
	const char *path
);
#endif

/**
 * @brief Copy a block of memory
 *
 * @warning The destination and source memory block must not overlap
 *
 * @param[out]     dest                destination to write to
 * @param[in]      src                 source block of memory
 * @param[in]      len                 amount of data to copy
 */
#if !OSAL_WRAP
#define os_memcpy(dest, src, len)      dest; CopyMemory(dest, src, len)
#else
OS_API void *os_memcpy(
	void *dest,
	const void *src,
	size_t len
);
#endif

/**
 * @brief Moves a block of memory
 *
 * @param[out]     dest                destination to write to
 * @param[in]      src                 source block of memory
 * @param[in]      len                 amount of data to move
 */
#if !OSAL_WRAP
#define os_memmove(dest, src, len)     dest; MoveMemory(dest, src, len)
#else
OS_API void *os_memmove(
	void *dest,
	const void *src,
	size_t len
);
#endif

/**
 * @brief Sets a block of memory to a specific byte
 *
 * @param[out]     dest                destination to write to
 * @param[in]      val                 byte to set
 * @param[in]      len                 amount of data to set
 */
#if !OSAL_WRAP
#define os_memset( dest, val, len )    FillMemory(dest, len, val)
#else
OS_API void os_memset(
	void *dest,
	int c,
	size_t len
);
#endif

/**
 * @brief Zeroizes block of memory
 *
 * @param[out]     dest               destination to write to
 * @param[in]      len                amount of data to zeroize
 */
#if !OSAL_WRAP
#define os_memzero(dest, len)         ZeroMemory(dest, len)
#else
OS_API void os_memzero(
	void *dest,
	size_t len
);
#endif

/* service entry (servent) functions */
/**
 * @brief Finds a service entry by service name
 *
 * @note This is equivilent to the @p getservbyname POSIX function
 *
 * @note This function will open the service entry database if required
 *
 * @param[in]      name                service name to search for
 * @param[in]      proto               protocol to search for (optional)
 *
 * @return a pointer to the service entry if found.  NULL otherwise
 *
 * @see os_service_entry_by_port
 * @see os_service_entry_close
 */
OS_API os_service_entry_t *os_service_entry_by_name(
	const char *name,
	const char *proto );

/**
 * @brief Finds a service entry by port
 *
 * @note This is equivilent to the @p getservbyport POSIX function
 *
 * @note This function will open the service entry database if required
 *
 * @param[in]      port                service port to search for
 * @param[in]      proto               protocol to search for (optional)
 *
 * @return a pointer to the service entry if found.  NULL otherwise
 *
 * @see os_service_entry_by_name
 * @see os_service_entry_close
 */
OS_API os_service_entry_t *os_service_entry_by_port(
	int port,
	const char *proto );

/**
 * @brief Closes the service entry database if open
 *
 * @note This is equivilent to the @p endservent POSIX function
 *
 * @see os_service_entry_get
 * @see os_service_entry_open
 */
OS_API void os_service_entry_close( void );

/**
 * @brief Retrieves the next service entry from the database
 *
 * @note This is equivilent to the @p getservent POSIX function
 *
 * @return the next entry in the service database
 *
 * @see os_service_entry_open
 * @see os_service_entry_close
 */
OS_API os_service_entry_t *os_service_entry_get( void );

/**
 * @brief Opens the service entry database
 *
 * @note This is equivilent to the @p setservent POSIX function
 *
 * @param[in]      stayopen            whether to keep the database open
 *
 * @see os_service_entry_get
 * @see os_service_entry_close
 */
OS_API void os_service_entry_open(
	int stayopen );

/**
 * @brief Returns the operating system code for the last system error
 *        encountered
 *
 * @return The operating system code for the last error encountered
 */
#if !OSAL_WRAP
#define os_system_error_last()                 (int)GetLastError()
#else
OS_API int os_system_error_last( void );
#endif

#if OSAL_THREAD_SUPPORT
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
 * @param[in]      stack_size          size of stack to use (0 = default)
 *
 * @retval OS_STATUS_BAD_PARAMETER     invalid parameter passed to function
 * @retval OS_STATUS_FAILURE           function failed
 * @retval OS_STATUS_SUCCESS           on success
 */
OS_API os_status_t os_thread_create(
	os_thread_t *thread,
	os_thread_main_t main,
	void *arg,
	size_t stack_size
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
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval IOT_STATUS_FAILURE          function failed
 * @retval IOT_STATUS_SUCCESS          on success
 */
#if !OSAL_WRAP
#define os_thread_condition_wait(cond, lock)   os_thread_condition_timed_wait( cond, lock, 0 )
#else
OS_API os_status_t os_thread_condition_wait(
	os_thread_condition_t *cond,
	os_thread_mutex_t *lock
);
#endif
#endif /* if OSAL_THREAD_SUPPORT */

