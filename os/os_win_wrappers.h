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
 * @brief Returns the operating system code for the last system error
 *        encountered
 *
 * @return The operating system code for the last error encountered
 */
OS_API OS_SECTION int os_system_error_last( void );

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