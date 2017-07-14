/**
 * @file
 * @brief Function macro definitions for Windows
 *
 * @copyright Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.  No license to Wind River intellectual property rights is
 * granted herein.  All rights not licensed by Wind River are reserved by Wind
 * River.
 */

/**
 * @brief Finds a function within an open runtime library
 *
 * @param[in]      lib                 open library handle
 * @param[in]      function            function to find
 *
 * @retval NULL    no matching function found (or an error occurred)
 * @retval !NULL   a pointer to the matching function
 *
 * @see iot_os_library_close
 * @see iot_os_library_open
 */
#define os_library_find(lib, function)         GetProcAddress(lib, function)

/**
 * @brief Opens a runtime library
 *
 * @param[in]      path                library to open
 *
 * @return         os_lib_handle of the opened library
 *
 * @see iot_os_library_close
 * @see iot_os_library_find
 */
#define os_library_open(path)                  LoadLibrary(path)

/**
 * @brief Copy a block of memory
 *
 * @warning The destination and source memory block must not overlap
 *
 * @param[out]     dst                 destination to write to
 * @param[in]      src                 source block of memory
 * @param[in]      len                 amount of data to copy
 */
#define os_memcpy(dst, src, len)               CopyMemory(dst, src, len); dst

/**
 * @brief Moves a block of memory
 *
 * @param[out]     dst                 destination to write to
 * @param[in]      src                 source block of memory
 * @param[in]      len                 amount of data to move
 */
#define os_memmove(dst, src, len)              MoveMemory(dst, src, len); dst

/**
 * @brief Sets a block of memory to a specific byte
 *
 * @param[out]     dst                 destination to write to
 * @param[in]      val                 byte to set
 * @param[in]      len                 amount of data to set
 */
#define os_memset( dst, val, len )             FillMemory( ptr, len, val )

/**
 * @brief Zeroizes block of memory
 *
 * @param[out]     dst                destination to write to
 * @param[in]      len                amount of data to zeroize
 */
#define os_memzero(dst, len)                  ZeroMemory(dst, len)

/**
 * @brief Allocates memory for an array of elements
 *
 * The memory returned is NOT initialized. Any allocated memory should be
 * deallocated with the corrosponding @p iot_os_heap_free command
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
 * @see iot_os_heap_free
 * @see iot_os_heap_malloc
 * @see iot_os_heap_realloc
 */
#define os_heap_calloc( num, size )            HeapAlloc( GetProcessHeap(), 0, num * size )

/**
 * @brief Returns the operating system code for the last system error
 *        encountered
 *
 * @return The operating system code for the last error encountered
 */
#define os_system_error_last()                 (int)GetLastError()

/**
 * @brief Locate a substring
 *
 * @param[in]      str1                string to be searched
 * @param[in]      str2                string to search for
 *
 * @retval         !NULL               pointer to the substring of str2 in str1
 * @retval         NULL                no substring found
 */
#define os_strstr(str1, str2)                  StrStr(str1, str2)

#ifndef NO_THREAD_SUPPORT
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
#define os_thread_condition_wait(cond, lock)   os_thread_condition_timed_wait( cond, lock, 0 )
#endif /* ifndef NO_THREAD_SUPPORT */