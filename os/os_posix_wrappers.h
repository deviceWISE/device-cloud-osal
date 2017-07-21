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
 * @brief Locate the first occurance of a character in a string
 *
 * @param[in]      s                   string to be searched
 * @param[in]      c                   character to search for as an int
 *
 * @retval         !NULL               pointer to last occurance of c in s
 * @retval         NULL                c was not found in s
 */
OS_API OS_SECTION char *os_strchr(
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
 * @brief Finds the first occurrence of some specified characters in a string
 *
 * @param[in]      str1                String to search
 * @param[in]      str2                Characters to search for
 *
 * @retval         !NULL               Pointer to the first occurrence of
 *                                     one of the characters in str2 in str1
 * @retval         NULL                None of the characters in str2 appear
 */
OS_API OS_SECTION char *os_strpbrk(
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
OS_API OS_SECTION char *os_strrchr(
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
OS_API OS_SECTION char *os_strstr(
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

/**
 * @brief Copy a block of memory
 *
 * @warning The destination and source memory block must not overlap
 *
 * @param[out]     dest                destination to write to
 * @param[in]      src                 source block of memory
 * @param[in]      len                 amount of data to copy
 */
OS_API OS_SECTION void *os_memcpy(
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
OS_API OS_SECTION void *os_memmove(
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
OS_API OS_SECTION void os_memset(
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
OS_API OS_SECTION void os_memzero(
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
OS_API OS_SECTION void *os_calloc(
	size_t nmemb,
	size_t size
) __attribute__((malloc));

/**
 * @brief Frees previously allocated memory specified
 *
 * @param[in]      ptr            pointer of pointer to the allocated memory to free
 *
 * @see os_calloc
 * @see os_malloc
 * @see os_realloc
 */
OS_API OS_SECTION void os_free(
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
 * @see os_realloc
 */
OS_API OS_SECTION void *os_malloc(
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
OS_API OS_SECTION void *os_realloc(
	void *ptr,
	size_t size
) __attribute__((malloc));

/**
 * @brief Returns the operating system code for the last system error
 *        encountered
 *
 * @return The operating system code for the last error encountered
 */
OS_API OS_SECTION int os_system_error_last( void );

/**
 * @brief Returns the process id of the current process
 * @return the process id of the current process
 */
OS_API OS_SECTION os_uint32_t os_system_pid( void );

#ifndef _WRS_KERNEL
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
OS_API OS_SECTION void *os_library_find(
	os_lib_handle lib,
	const char *function
);
/**
 * @brief Opens a runtime library
 *
 * @param[in]      path                library to open
 *
 * @retval OS_STATUS_FAILURE          on failure
 * @retval OS_STATUS_SUCCESS          on success
 *
 * @see os_library_close
 * @see os_library_find
 */
OS_API OS_SECTION os_lib_handle os_library_open(
	const char *path
);
#endif/* ifndef _WRS_KERNEL */

#ifndef NO_THREAD_SUPPORT
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
OS_API OS_SECTION os_status_t os_thread_condition_wait(
	os_thread_condition_t *cond,
	os_thread_mutex_t *lock
);
#endif /* ifndef NO_THREAD_SUPPORT */