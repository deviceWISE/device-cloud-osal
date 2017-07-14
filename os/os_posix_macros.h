/**
 * @file
 * @brief Function macro definitions for POSIX
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
 * @brief Frees previously allocated memory specified
 *
 * @param[in]      ptr            pointer of pointer to the allocated memory to free
 *
 * @see os_heap_calloc
 * @see os_heap_malloc
 * @see os_heap_realloc
 */
#define os_heap_free(ptr)                      { if ( *ptr ) free( *ptr ); *ptr = NULL; }

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
#define os_heap_malloc(size)                   malloc(size)

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
#define os_heap_calloc(num, size)              calloc(num, size)

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
#define os_heap_realloc(ptr, size)             realloc(ptr, size)

/**
 * @brief Copy a block of memory
 *
 * @warning The destination and source memory block must not overlap
 *
 * @param[out]     dst                 destination to write to
 * @param[in]      src                 source block of memory
 * @param[in]      len                 amount of data to copy
 */
#define os_memcpy(dst, src, len)               memcpy(dst, src, len)

/**
 * @brief Zeroizes block of memory
 *
 * @param[out]     dst                destination to write to
 * @param[in]      len                amount of data to zeroize
 */
#define os_memzero(dst, len)                   bzero(dst, len)

/**
 * @brief Moves a block of memory
 *
 * @param[out]     dst                 destination to write to
 * @param[in]      src                 source block of memory
 * @param[in]      len                 amount of data to move
 */
#define os_memmove(dst, src, len)              memmove(dst, src, len)

/**
 * @brief Sets a block of memory to a specific byte
 *
 * @param[out]     dst                 destination to write to
 * @param[in]      val                 byte to set
 * @param[in]      len                 amount of data to set
 */
#define os_memset(dst, val, len)               memset(len, val, num)

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
#define os_strcmp(s1, s2)                      strcmp(s1, s2)

/**
 * @brief Get the length of a string
 *
 * @param[in]      s                   string to get length from
 *
 * @return         Number of characters until a terminating null character
 */
#define os_strlen(s)                           strlen(s)

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
#define os_strncmp(s1, s2, len)                strncmp(s1, s2, len)

/**
 * @brief Copy characters from string
 *
 * @param[out]     dst                 string to copy characters to
 * @param[in]      src                 string to copy character from
 * @param[in]      num                 maximum number of characters to copy
 *
 * @retval         dst
 */
#define os_strncpy(dst, src, num)              strncpy(dst, src, num)

/**
 * @brief Locate the last occurance of a character in a string
 *
 * @param[in]      s                   string to be searched
 * @param[in]      c                   character to search for as an int
 *
 * @retval         !NULL               pointer to last occurance of c in s
 * @retval         NULL                c was not found in s
 */
#define os_strrchr(s, c)                       strrchr(s, c)

/**
 * @brief Locate the first occurance of a character in a string
 *
 * @param[in]      s                   string to be searched
 * @param[in]      c                   character to search for as an int
 *
 * @retval         !NULL               pointer to last occurance of c in s
 * @retval         NULL                c was not found in s
 */
#define os_strchr(s, c)                        strchr(s, c)

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
#define os_strpbrk(str1, str2)                 strpbrk(str1, str2)

/**
 * @brief Locate a substring
 *
 * @param[in]      str1                string to be searched
 * @param[in]      str2                string to search for
 *
 * @retval         !NULL               pointer to the substring of str2 in str1
 * @retval         NULL                no substring found
 */
#define os_strstr(str1, str2)                  strstr(str1, str2)

/**
 * @brief Parse a string to retrieve a double
 *
 * @param[in]      str                 string to parse for double
 * @param[out]     endptr              (optional)pointer to character 
 *                                     immediately following double in string
 * @return value of double parsed
 */
#define os_strtod(str, endptr)                 strtod(str, endptr)

/**
 * @brief Parse a string to retrieve a long
 *
 * @note This implementation only supports base-ten
 *
 * @param[in]      str                 string to parse for long
 * @param[out]     endptr              (optional)pointer to character
 *                                     immediately following long in string
 * @return value of long parsed
 */
#define os_strtol(str, endptr)                 strtol(str, endptr, 10)

/**
 * @brief Parse a string to retrieve an unsigned long
 *
 * @note This implementation only supports base-ten
 *
 * @param[in]      str                 string to parse for unsigned long
 * @param[out]     endptr              (optional)pointer to character
 *                                     immediately following unsigned long in
 *                                     string
 * @return value of unsigned long parsed
 */
#define os_strtoul(str, endptr)                strtoul(str, endptr, 10)

/**
 * @brief Returns the process id of the current process
 *
 * @return the process id of the current process
 */
#define os_system_pid()                        (os_uint32_t)getpid()

/**
 * @brief Returns the operating system code for the last system error
 *        encountered
 *
 * @return The operating system code for the last error encountered
 */
#define os_system_error_last()                 errno

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
#define os_fprintf(stream, format, ...)        fprintf(stream, format, ##__VA_ARGS__)

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
#define os_printf(format, ...)                 printf(format, ##__VA_ARGS__)

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
#define os_snprintf(str, size, format, ...)    snprintf(str, size, format, ##__VA_ARGS__)

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
#define os_vfprintf(stream, format, args)      vfprintf(stream, format, args)

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
#define os_file_fgets(str, size, stream)       fgets(str, (int) size, stream)

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
#define os_file_fputs(str, stream)             (size_t)fputs(str, stream)

/**
 * @brief Read bytes from a file into an array
 *
 * @note Stops when encountering max read, EOF, or null terminator
 *
 * @param[in,out]  ptr                 Pointer to array to write to
 * @param[in]      size                Size of each item to read
 * @param[in]      num                 Number of items to read
 * @param[in,out]  stream              Pointer to file to read from
 *
 * @return         Number of items read
 */
#define os_file_fread(ptr, size, num, stream)  fread(ptr, size, num, stream)

/**
 * @brief Write bytes from an array into a file stream
 *
 * @param[in]      ptr                 Pointer to array to write from
 * @param[in]      size                Size of each item to be written
 * @param[in]      num                 Number of items to be written
 * @param[in,out]  stream              Pointer to file to write to
 *
 * @return         Number of items written
 */
#define os_file_fwrite(ptr, size, num, stream) fwrite(ptr, size, num, stream)

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
#define os_library_open(path)                  dlopen(path, 0)

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
#define os_library_find(lib, function)         dlsym(lib, function)

/**
 * @brief Converts a character to lower case
 *
 * @param[in]      c                   character to convert
 *
 * @return the lower-case value of the character, or @p c if not possible
 */
#define os_char_tolower(c)                     (char)tolower(c)

/**
 * @brief Converts a character to upper case
 *
 * @param[in]      c                   character to convert
 *
 * @return the upper-case value of the character, or @p c if not possible
 */
#define os_char_toupper(c)                     (char)toupper(c)

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

