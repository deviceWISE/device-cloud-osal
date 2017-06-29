/**
 * @file
 * @brief declares functions for the operating system abstraction layer
 *
 * @copyright Copyright (C) 2016-2017 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.  No license to Wind River intellectual property rights is
 * granted herein.  All rights not licensed by Wind River are reserved by Wind
 * River.
 */
#ifndef OS_H
#define OS_H

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

/** @brief False */
#define OS_FALSE                                (os_bool_t)(0 == 1)
/** @brief True */
#define OS_TRUE                                 (os_bool_t)(1 == 1)

/**
 * @defgroup os_types List of basic OS types
 * @{
 */
/** @brief OS boolean (true or false) */
typedef int                                      os_bool_t;
/** @brief OS 32-bit floating-point */
typedef float                                    os_float32_t;
/** @brief OS 64-bit floating-point */
typedef double                                   os_float64_t;
/** @brief OS 8-bit signed integer */
typedef int8_t                                   os_int8_t;
/** @brief OS 16-bit signed integer */
typedef int16_t                                  os_int16_t;
/** @brief OS 32-bit signed integer */
typedef int32_t                                  os_int32_t;
/** @brief OS 64-bit signed integer */
typedef int64_t                                  os_int64_t;
/** @brief OS 8-bit unsigned integer */
typedef uint8_t                                  os_uint8_t;
/** @brief OS 16-bit unsigned integer */
typedef uint16_t                                 os_uint16_t;
/** @brief OS 32-bit unsigned integer */
typedef uint32_t                                 os_uint32_t;
/** @brief OS 64-bit unsigned integer */
typedef uint64_t                                 os_uint64_t;
/** @brief OS time interval in milliseconds */
typedef uint32_t                                 os_millisecond_t;
/** @brief OS time stamp in milliseconds */
typedef uint64_t                                 os_timestamp_t;

/**
 * @}
 */

/**** DEFS, may be moved later ****/
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
 * @def OS_SECTION
 * @brief Macro to add compiler specifications for hints where to store library
 * functions in memory
 */
#ifdef ARDUINO
#	define OS_SECTION __attribute__((section(".irom0.text")))
#else
#	define OS_SECTION
#endif /* ifdef ARDUINO */

/**
 * @def OS_API
 * @brief Macro to add compiler specifications for external/internal functions
 */
#ifdef OS_STATIC
#	define OS_API
#else /* ifdef OS_STATIC */
#	if defined _WIN32 || defined __CYGWIN__
#		if osal_EXPORTS
#			ifdef __GNUC__
#				define OS_API __attribute__((dllexport))
#			else
#				define OS_API __declspec(dllexport)
#			endif
#		else /* if osal_EXPORTS */
#			ifdef __GNUC__
#				define OS_API __attribute__((dllimport))
#			else
#				define OS_API __declspec(dllimport)
#			endif
#		endif /* if osal_EXPORTS */
#	else /* if defined _WIN32 || defined __CYGWIN__ */
#		if __GNUC__ >= 4
#			define OS_API __attribute__((visibility("default")))
#		else
#			define OS_API
#		endif
#	endif /* if defined _WIN32 || defined __CYGWIN__ */
#endif /* ifdef OS_STATIC */

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

/** @brief Maximum name length (128 = Matches AWS name) */
#define OS_NAME_MAX_LEN               128u

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

#if defined( _WIN32 )
#include "os/os_win.h"
#elif defined ( _WRS_KERNEL )
#include "os/os_vxworks.h"
#elif defined ( __ANDROID__ )
#include "os/os_android.h"
#elif defined( __unix__ )
#include "os/os_posix.h"
#else
#error "Unsupported platform"
#endif

/* if there is no file system */
/**
 * @def PATH_MAX
 * @brief Maximum path length
 */
#ifndef PATH_MAX
#	define PATH_MAX 0
#endif

/**
 * @def DIRECTORY_CREATE_MAX_TIMEOUT
 * @brief Amount of time in milliseconds to wait for a directory to be
 *        created
 */
#ifdef __ANDROID__
#	define DIRECTORY_CREATE_MAX_TIMEOUT 3000000u
#else
#	define DIRECTORY_CREATE_MAX_TIMEOUT 300u
#endif /* __ANDROID__ */

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

/* flags */
/** @brief Time stamps are relative to uptime */
#define OS_FLAG_RELATIVE_TIME  0x1

/** @brief maximum length for a field in system info structure */
#define OS_SYSTEM_INFO_MAX_LEN 64u

/** @brief Type for a socket */
typedef struct os_socket
{
	/** @brief Contains the address host address */
	struct sockaddr addr;
	/** @brief Contains the host port */
	os_uint16_t port;
	/** @brief File descriptor to the open socket */
	int fd;
	/** @brief Socket type */
	int type;
	/** @brief Socket protocol */
	int protocol;
} os_socket_t;

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
	/** @brief operating system flag */
	os_uint8_t system_flags;
} os_system_info_t;

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
 * @retval OS_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval OS_STATUS_FAILURE          system error
 * @retval OS_STATUS_SUCCESS          on success
 *
 * @see os_adapters_next
 * @see os_adapters_obtain
 * @see os_adapters_release
 */
OS_API OS_SECTION os_status_t os_adapters_address(
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
 * @retval OS_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval OS_STATUS_FAILURE          system error
 * @retval OS_STATUS_SUCCESS          on success
 *
 * @see os_adapters_next
 * @see os_adapters_obtain
 * @see os_adapters_release
 */
OS_API OS_SECTION os_status_t os_adapters_index(
	os_adapters_t *adapters,
	unsigned int *index
);

/**
 * @brief Get the mac address of the current adapter in the list
 *
 * @param[in,out]  adapters            adapter list to retrieve information from
 * @param[out]     mac                 buffer to fill with the mac address
 * @param[in]      max_len             size of the buffer
 *
 * @retval OS_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval OS_STATUS_FAILURE          system error
 * @retval OS_STATUS_SUCCESS          on success
 *
 * @see os_adapters_next
 * @see os_adapters_obtain
 * @see os_adapters_release
 */
OS_API OS_SECTION os_status_t os_adapters_mac(
	os_adapters_t *adapters,
	char *mac,
	size_t mac_len
);

/**
 * @brief Moves to the next adapter in the list
 *
 * @param[in,out]  adapters            adapter list to iterate
 *
 * @retval OS_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval OS_STATUS_FAILURE          system error
 * @retval OS_STATUS_SUCCESS          on success
 *
 * @see os_adapters_obtain
 * @see os_adapters_release
 */
OS_API OS_SECTION os_status_t os_adapters_next(
	os_adapters_t *adapters
);

/**
 * @brief Obtains the list of adapters connected to the system
 *
 * @param[out]     adapters            list of adapters
 *
 * @retval OS_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval OS_STATUS_FAILURE          system error
 * @retval OS_STATUS_NO_MEMORY        out of memory
 * @retval OS_STATUS_SUCCESS          on success
 *
 * @see os_adapters_next
 * @see os_adapters_release
 */
OS_API OS_SECTION os_status_t os_adapters_obtain(
	os_adapters_t *adapters
);

/**
 * @brief Release memory allocated for the adapters
 *
 * @param[in,out]  adapters            adapter list to release
 *
 * @retval OS_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval OS_STATUS_FAILURE          system error
 * @retval OS_STATUS_SUCCESS          on success
 *
 * @see os_adapters_next
 * @see os_adapters_obtain
 */
OS_API OS_SECTION os_status_t os_adapters_release(
	os_adapters_t *adapters
);

/* character testing support */
/**
 * @brief Checks whether @p c is an alphabetical or numerical character
 *
 * @retval OS_FALSE                   character is not a alphabetical or
 *                                     numerical character
 * @retval OS_TRUE                    character is a alphabetical or  numerical
 *                                     character
 */
OS_API OS_SECTION os_bool_t os_char_isalnum(
	char c
);

/**
 * @brief Checks whether @p c is a hexadecimal digit
 *
 * @retval OS_FALSE                   character is not a hexadecimal digit
 * @retval OS_TRUE                    character is hexadecimal digit
 */
OS_API OS_SECTION os_bool_t os_char_isxdigit(
	char c
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
 * @brief Converts a string to lower cases only
 *
 * @param[out]     out                 output string
 * @param[in]      in                  string to convert
 * @param[in]      len                 string's length
 *
 * @return the string with all the characters in lower case
 */
OS_API OS_SECTION char *os_string_tolower(
	char *out,
	const char *in,
	size_t len
);

/**
 * @brief Converts a string to upper cases only
 *
 * @param[out]     out                 output string
 * @param[in]      in                  string to convert
 * @param[in]      len                 string's length
 *
 * @return the string with all the characters in upper case
 */
OS_API OS_SECTION char *os_string_toupper(
	char *out,
	const char *in,
	size_t len
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
 * @retval OS_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval OS_STATUS_FAILURE          system error
 * @retval OS_STATUS_SUCCESS          on success
 */
OS_API OS_SECTION os_status_t os_directory_create(
	const char *path,
	os_millisecond_t timeout
);
/**
 * @brief Create a directory at the path specified
 *
 * @param[in]      path                path to directory to be created
 *
 * @retval OS_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval OS_STATUS_FAILURE          system error
 * @retval OS_STATUS_SUCCESS          on success
 */
OS_API OS_SECTION os_status_t os_directory_create_nowait(
	const char *path
);

/**
 * @brief Stores the current directory into the buffer specified
 *
 * @param[out]     buffer              output destination
 * @param[in]      size                maximum size of destination buffer
 *
 * @retval OS_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval OS_STATUS_FAILURE          system error or the buffer is too small
 * @retval OS_STATUS_SUCCESS          on success
 */
OS_API OS_SECTION os_status_t os_directory_current(
	char *buffer,
	size_t size
);
/**
 * @brief Change the specified directory as current working directory
 *
 * @param[in]      path                path to directory to be changed
 *
 * @retval OS_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval OS_STATUS_FAILURE          failed to change current directory
 * @retval OS_STATUS_SUCCESS          directory was successfully changed
 */
OS_API OS_SECTION os_status_t os_directory_change(const char *path);
/**
 * @brief Opens the specified directory for exploration
 *
 * @param[in]      dir_path            path to directory to open
 * @param[out]     out                 pointer to directory stream
 *
 * @retval OS_STATUS_FAILURE          failed to open directory
 * @retval OS_STATUS_SUCCESS          directory was successfully opened
 */
OS_API OS_SECTION os_status_t os_directory_open(
	const char *dir_path,
	os_dir_t* out
);
/**
 * @brief Closes a previously opened directory stream
 *
 * @param[in]      dir                 directory to close
 *
 * @retval OS_STATUS_FAILURE          failed to close directory
 * @retval OS_STATUS_SUCCESS          successfully closed directory
 */
OS_API OS_SECTION os_status_t os_directory_close(
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
 * @retval OS_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval OS_STATUS_BAD_REQUEST      regular expression contains invalid
 *                                     character(s)
 * @retval OS_STATUS_FAILURE          on failure
 * @retval OS_STATUS_NO_MEMORY        out of memory
 * @retval OS_STATUS_SUCCESS          on success
 * @retval OS_STATUS_TRY_AGAIN        directory is not empty
 *
 * @see os_directory_delete
 * @see os_directory_exists
 * @see os_file_delete
 */
OS_API OS_SECTION os_status_t os_directory_delete(
	const char *path,
	const char *regex,
	os_bool_t recursive
);
/**
 * @brief Returns whether or not a directory exists
 *
 * @param[in]      dir_path            path to directory to check existence
 *
 * @retval OS_TRUE                    directory exists
 * @retval OS_FALSE                   directory does not exist
 */
OS_API OS_SECTION os_bool_t os_directory_exists(
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
OS_API OS_SECTION os_uint64_t os_directory_free_space(
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
OS_API OS_SECTION const char *os_directory_get_temp_dir(
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
 * @retval OS_STATUS_FAILURE          failed to retrieve next file, might have
 *                                     reached the end of the directory
 * @retval OS_STATUS_SUCCESS          successfully retrieved the next file
 */
OS_API OS_SECTION os_status_t os_directory_next(
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
 * @retval OS_STATUS_FAILURE          failed to rewind directory
 * @retval OS_STATUS_SUCCESS          successfully rewound directory
 *
 *
 */
OS_API OS_SECTION os_status_t os_directory_rewind(
	os_dir_t *dir
);

/**
 * @brief changes the owner and group of a file
 *
 * @param[in]      path                path to the specified file
 * @param[in]      user                new owner for the file
 *
 * @retval OS_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval OS_STATUS_FAILURE          Failed to change ownership of the file
 * @retval OS_STATUS_SUCCESS          Successfully changed ownership of the
 *                                     file
 */
OS_API OS_SECTION os_status_t os_file_chown(
	const char *path,
	const char *user
);

/**
 * @brief Close file stream
 *
 * @param[in,out]  handle              Pointer to file stream to close
 *
 * @retval         OS_STATUS_SUCCESS  Successfully closed file stream
 * @retval         OS_STATUS_FAILURE  Failed to close file stream
 *
 * @see os_file_open
 */
OS_API OS_SECTION os_status_t os_file_close(
	os_file_t handle
);

/**
 * @brief Copies a file in the file system
 *
 * @param[in]      old_path            path of the file to copy
 * @param[in]      new_path            location to copy file to
 *
 * @retval OS_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval OS_STATUS_FAILURE          on failure
 * @retval OS_STATUS_SUCCESS          on success
 *
 * @see os_file_move
 */
OS_API OS_SECTION os_status_t os_file_copy(
	const char *old_path,
	const char *new_path
);

/**
 * @brief Deletes a file from the system
 *
 * @param[in]      path                path to file to delete
 *
 * @retval OS_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval OS_STATUS_FAILURE          on failure
 * @retval OS_STATUS_SUCCESS          on success
 *
 * @see os_directory_delete
 * @see os_file_exists
 */
OS_API OS_SECTION os_status_t os_file_delete(
	const char *path
);
/**
 * @brief Determines whether a specified file exists
 *
 * @param[in]      file_path           Path to specified file
 *
 * @retval         OS_TRUE            File exists
 * @retval         OS_FALSE           File does not exist
 *
 * @see os_file_remove
 */
OS_API OS_SECTION os_bool_t os_file_exists(
	const char *file_path
);

/**
 * @brief Move the current position in an file stream
 *
 * @param[in,out]  stream              stream to pointer of
 * @param[in]      offset              amount to remove file pointer
 * @param[in]      whence              where to apply the offset at
 *
 * @retval OS_STATUS_BAD_PARAMETER    bad parameter passed to the function
 * @retval OS_STATUS_FAILURE          failed to move file pointer
 * @retval OS_STATUS_SUCCESS          on success
 */
OS_API OS_SECTION os_status_t os_file_fseek(
	os_file_t stream,
	long offset,
	int whence
);

/**
 * @brief Fsync file
 *
 * @param[in]      file_path           path to file to fsync
 *                                     do sync if file_path is NULL (linux)
 *
 * @retval OS_STATUS_BAD_PARAMETER    bad parameter passed to the function
 * @retval OS_STATUS_SUCCESS          Successfully sync the file
 * @retval OS_STATUS_FAILURE          Failed to sync file stream
 *
 */
OS_API OS_SECTION os_status_t os_file_fsync(
	const char *file_path
);

/**
 * @brief Get size of file in bytes
 *
 * @param[in]      file_path           path to file
 *
 * @return         File size in bytes
 */
OS_API OS_SECTION os_uint64_t os_file_get_size(
	const char *file_path
);

/**
 * @brief Get size of file in bytes
 *
 * @param[in]      file_handle         file handle
 *
 * @return         File size in bytes
 */
OS_API OS_SECTION os_uint64_t os_file_get_size_handle(
	os_file_t file_handle
);

/**
 * @brief Moves a file in the file system
 *
 * @param[in]      old_path            path of the file to move
 * @param[in]      new_path            location to move file to
 *
 * @retval OS_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval OS_STATUS_FAILURE          on failure
 * @retval OS_STATUS_SUCCESS          on success
 *
 * @see os_file_copy
 */
OS_API OS_SECTION os_status_t os_file_move(
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
OS_API OS_SECTION os_file_t os_file_open(
	const char *file_path,
	int flags
);

/**
 * @brief Generates a temporary file based on the specified prototype
 *
 * @param[in,out]  prototype           prototype to use / temporary file name
 * @param[in]      suffix_len          suffix length
 *
 * @retval OS_STATUS_FAILURE          on failure
 * @retval OS_STATUS_SUCCESS          on success
 */
OS_API OS_SECTION os_status_t os_file_temp(
	char *prototype,
	size_t suffix_len
);

/**
 * @brief Waits for the user to press a key on a console window
 *
 * @return the character that was received
 */
OS_API OS_SECTION char os_key_wait( void );

#ifndef _WRS_KERNEL
/**
 * @brief Closes an open runtime library
 *
 * @param[in]      lib                 open library handle to close
 *
 * @retval OS_STATUS_FAILURE          on failure
 * @retval OS_STATUS_SUCCESS          on success
 *
 * @see os_library_find
 * @see os_library_open
 */
OS_API OS_SECTION os_status_t os_library_close(
	os_lib_handle lib
);
#endif /* ifndef _WRS_KERNEL */

/**
 * @brief Builds the path to a file for the operating system
 *
 * @param[in]      path                destination to store result
 * @param[in]      len                 size of destination buffer
 * @param[in]      ...                 directories and file name passed in as
 *                                     strings, until NULL is passed.
 *
 * @retval OS_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval OS_STATUS_FULL             the buffer specified is too small to hold
 *                                     the path
 * @retval OS_STATUS_SUCCESS          on success
 */
OS_API OS_SECTION os_status_t os_make_path(
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
OS_API OS_SECTION int os_atoi( const char *str );

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
OS_API OS_SECTION int os_strcasecmp(
	const char *s1,
	const char *s2
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
OS_API OS_SECTION int os_strncasecmp(
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
OS_API OS_SECTION char *os_strncat(
	char *s1,
	const char *s2,
	size_t count
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
 * @brief Finds the length of the initial portion of str1 which consists only
 *        of characters that are part of str2
 *
 * @param[in]      str1                String to search
 * @param[in]      str2                String containing characters to match
 *
 * @return The length of the initial portion of str1 containing only characters
 *         that appear in str2
 */
OS_API OS_SECTION size_t os_strspn(
	const char *str1,
	const char *str2
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
 * @brief Get the next token from a string
 *
 * @param[in,out]  str                 string to truncate, if it is set to NULL,
 *                                     it will use the last str value
 * @param[in]      delimiters          string containing delimiter characters
 *
 * @retval         !NULL               token is found
 * @retval         NULL                no token found
 */
OS_API OS_SECTION char *os_strtok(
	char *str,
	const char *delimiters
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
OS_API OS_SECTION int os_memcmp(
	const void *ptr1,
	const void *ptr2,
	size_t num
);

/* print functions */
/**
 * @brief Extracts any environment variables in the source string
 *
 * @note Enviroment variables in the source string are in the format as defined
 *       by the operating system (i.e. for Windows: %VAR%; for POSIX: $VAR )
 *
 * @param[in]      src                 source string (containing environment
 *                                     variable as defined by the operating
 *                                     system)
 * @param[in]      len                 buffer size
 *
 * @return the size of the output string (0 if either src is NULL) or
 * required size if too small
 *
 * @see os_env_get
 */
OS_API OS_SECTION size_t os_env_expand(
	char *src,
	size_t len
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
OS_API OS_SECTION size_t os_env_get(
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
 * @return the number of characters printed including the null-terminator
 *
 * @see os_snprintf
 */
OS_API OS_SECTION int os_sprintf(
	char *str,
	const char *format,
	...
) __attribute__((format(printf,2,3)));

/**
 * @brief Flushes a stream to ensure a buffer is transmitted
 *
 * @param[in]      stream              stream to flush
 *
 * @retval OS_FALSE                   on failure
 * @retval OS_TRUE                    on success
 */
OS_API OS_SECTION os_bool_t os_flush(
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
OS_API OS_SECTION os_bool_t os_path_is_absolute(
	const char *path
);

/**
 * @brief Get the full path to executable
 *
 * @param[out]     path                buffer to put full path to executable
 * @param[in]      size                size of the buffer
 *
 * @retval OS_STATUS_BAD_PARAMETER    invalid parameter
 * @retval OS_STATUS_FAILURE          on failure
 * @retval OS_STATUS_SUCCESS          on success
 */
OS_API OS_SECTION os_status_t os_path_executable(
	char *path,
	size_t size
);

/* process functions */
/**
 * @brief Release resources when a child process exists
 *
 * @retval OS_STATUS_FAILURE          on failure
 * @retval OS_STATUS_SUCCESS          on success
 */
OS_API OS_SECTION os_status_t os_process_cleanup( void );

/* service functions */
/**
 * @brief Function to call if running as a service
 *
 * @param[in]      id                  id identifying the service
 * @param[in]      service_function    function pointer for service controller to run
 * @param[in]      argc                parameter count for service_function
 * @param[in,out]  argv                array of parameters for service_function
 * @param[in]      remove_argc         size of remove argument array
 * @param[in]      remove_argv         array of arguments to remove before
 *                                     passing to the @c service_function
 * @param[in]      handler             service signal handler
 * @param[in]      logdir              directory where log files will be placed
 *
 * @retval OS_STATUS_SUCCESS          on success
 * @retval OS_STATUS_BAD_PARAMETER    on bad parameters
 * @retval OS_STATUS_FAILURE          on failure to start service
 */
OS_API OS_SECTION os_status_t os_service_run(
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
 * @param[in]      name                name (or short description) of the service
 * @param[in]      description         (optional) brief description of the service
 * @param[in]      dependencies        (optional) list of dependencies,
 *                                     seperated by semi-colons
 * @param[in]      max_time_out        maximum time out for operation to complete
 *
 * @retval OS_STATUS_SUCCESS          on success
 * @retval OS_STATUS_BAD_PARAMETER    on bad mandatory parameter(s)
 * @retval OS_STATUS_EXISTS           service already exists
 * @retval OS_STATUS_FAILURE          on failure
 * @retval OS_STATUS_NOT_SUPPORTED    if not supported on current OS
 *
 * @note Forward-slashes (/) and Backslashes (\) are not valid within a
 *       service id
 */
OS_API OS_SECTION os_status_t os_service_install(
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
 * @retval OS_STATUS_SUCCESS          on success
 * @retval OS_STATUS_BAD_PARAMETER    on bad mandatory parameter(s)
 * @retval OS_STATUS_FAILURE          on failure
 * @retval OS_STATUS_NOT_SUPPORTED    if not supported on current OS
 *
 * @note Forward-slashes (/) and Backslashes (\) are not valid within a
 *       service id
 */
OS_API OS_SECTION os_status_t os_service_uninstall(
	const char *id,
	os_millisecond_t max_time_out
);

/**
 * @brief Start a service
 *
 * @param[in]      id                  id identifying the service
 * @param[in]      max_time_out        maximum time out for operation to complete
 *
 * @retval OS_STATUS_SUCCESS          on success
 * @retval OS_STATUS_BAD_PARAMETER    on bad mandatory parameter(s)
 * @retval OS_STATUS_FAILURE          on failure
 * @retval OS_STATUS_NOT_SUPPORTED    if not supported on current OS
 *
 * @note Forward-slashes (/) and Backslashes (\) are not valid within a
 *       service id.
 */
OS_API OS_SECTION os_status_t os_service_start(
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
 * @retval OS_STATUS_SUCCESS          on success
 * @retval OS_STATUS_BAD_PARAMETER    on bad mandatory parameter(s)
 * @retval OS_STATUS_FAILURE          on failure
 * @retval OS_STATUS_NOT_SUPPORTED    if not supported on current OS
 *
 * @note Forward-slashes (/) and Backslashes (\) are not valid within a
 *       service id.
 */
OS_API OS_SECTION os_status_t os_service_stop(
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
 * @retval OS_STATUS_SUCCESS          on success
 * @retval OS_STATUS_BAD_PARAMETER    on bad mandatory parameter(s)
 * @retval OS_STATUS_FAILURE          on failure
 * @retval OS_STATUS_NOT_SUPPORTED    if not supported on current OS
 *
 * @note Forward-slashes (/) and Backslashes (\) are not valid within a
 *       service id.
 */
OS_API OS_SECTION os_status_t os_service_restart(
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
 * @retval OS_STATUS_BAD_PARAMETER    invalid parameter(s) passed to function
 * @retval OS_STATUS_FAILURE          failed to query service or in a failed state
 * @retval OS_STATUS_NOT_FOUND        service is not found
 * @retval OS_STATUS_NOT_INITIALIZED  service is not active (or connecting)
 * @retval OS_STATUS_NOT_SUPPORTED    if not supported on current OS
 * @retval OS_STATUS_SUCCESS          service is active & running
 * @retval OS_STATUS_TIMED_OUT        the query timed out before completion
 *
 * @note Forward-slashes (/) and Backslashes (\) are not valid within a
 *       service id.
 */
OS_API OS_SECTION os_status_t os_service_query(
	const char *id,
	os_millisecond_t max_time_out
);

/* socket functions */
/**
 * @brief Get the IP address that a host resolves to
 *
 * @param[in]      host                hostname to resolve
 * @param[in]      service             service or port to use for resolution
 * @param[out]     address             the resulting IP address that the host resolves to
 * @param[in]      address_len         length of the IP address char array
 * @param[in] family         address family to use for resolution
 *
 * @retval 0       on success
 * @retval !0      on failure (see getaddrinfo() return codes)
 */
 OS_API OS_SECTION int os_get_host_address(
	const char *host,
	const char *service,
	char *address,
	int address_len,
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
 * @retval OS_STATUS_BAD_PARAMETER    invalid parameter(s) passed to function
 * @retval OS_STATUS_FAILURE          failed to accept connection
 * @retval OS_STATUS_SUCCESS          successfully bound to socket
 * @retval OS_STATUS_TIMED_OUT        time out exceeded
 *
 * @see os_socket_bind
 */
OS_API OS_SECTION os_status_t os_socket_accept(
	const os_socket_t *socket,
	os_socket_t *out,
	os_millisecond_t max_time_out
);

/**
 * @brief Binds to a socket to listen and accept connections on
 *
 * @param[in]      socket              socket to bind to
 * @param[in]      queue_size          size of queue to hold incoming
 *
 * @retval OS_STATUS_BAD_PARAMETER    invalid parameter(s) passed to function
 * @retval OS_STATUS_FAILURE          failed to bind to socket
 * @retval OS_STATUS_SUCCESS          successfully bound to socket
 *
 * @see os_socket_accept
 * @see os_socket_open
 * @see os_socket_close
 */
OS_API OS_SECTION os_status_t os_socket_bind(
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
 * @retval OS_STATUS_BAD_PARAMETER    invalid parameter(s) passed to function
 * @retval OS_STATUS_FAILURE          failed to accept connection
 * @retval OS_STATUS_SUCCESS          successfully bound to socket
 * @retval OS_STATUS_TIMED_OUT        time out exceeded
 */
OS_API OS_SECTION os_status_t os_socket_broadcast(
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
 * @retval OS_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval OS_STATUS_FAILURE          on failure
 * @retval OS_STATUS_SUCCESS          on success
 *
 * @see os_socket_open
 */
OS_API OS_SECTION os_status_t os_socket_close(
	os_socket_t *socket
);
/**
 * @brief Connects to a socket to send and receive data
 *
 * @param[in]      socket              socket to connect to
 *
 * @retval OS_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval OS_STATUS_FAILURE          on failure
 * @retval OS_STATUS_SUCCESS          on success
 */
OS_API OS_SECTION os_status_t os_socket_connect(
	const os_socket_t *socket
);
/**
 * @brief Initializes resources for raw socket communicationa within a process
 *
 * @retval OS_STATUS_FAILURE          on failure
 * @retval OS_STATUS_SUCCESS          on success
 *
 * @see os_socket_terminate
 */
OS_API OS_SECTION os_status_t os_socket_initialize( void );
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
 * @retval OS_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval OS_STATUS_FAILURE          on failure
 * @retval OS_STATUS_SUCCESS          on success
 * @retval OS_STATUS_TIMED_OUT        time out exceeded
 *
 * @see os_socket_close
 */
OS_API OS_SECTION os_status_t os_socket_open(
	os_socket_t *out,
	const char *address,
	os_uint16_t port,
	int type,
	int protocol,
	os_millisecond_t max_time_out
);
OS_API OS_SECTION os_status_t os_socket_option(
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
 * @param[in]      max_time_out        maximum time out for operation to complete
 *
 * @retval OS_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval OS_STATUS_FAILURE          on failure
 * @retval OS_STATUS_SUCCESS          on success
 * @retval OS_STATUS_TIMED_OUT        time out exceeded
 * @retval OS_STATUS_TRY_AGAIN        socket not connected
 *
 * @see os_socket_write
 */
OS_API OS_SECTION os_status_t os_socket_read(
	const os_socket_t *socket,
	void *buf,
	size_t len,
	size_t* bytes_read,
	os_millisecond_t max_time_out
);
OS_API OS_SECTION ssize_t os_socket_receive(
	const os_socket_t *socket,
	void *buf,
	size_t len,
	char *src_addr,
	size_t src_addr_len,
	os_uint16_t *port,
	os_millisecond_t max_time_out
);
OS_API OS_SECTION ssize_t os_socket_send(
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
 * @retval OS_STATUS_FAILURE          on failure
 * @retval OS_STATUS_SUCCESS          on success
 *
 * @see os_socket_initialize
 */
OS_API OS_SECTION os_status_t os_socket_terminate( void );
/**
 * @brief Writes data to an open socket
 *
 * @param[in]      socket              socket to write to
 * @param[in]      buf                 destination buffer
 * @param[in]      len                 size of buffer
 * @param[out]     bytes_written       number of bytes written in bytes
 * @param[in]      max_time_out        maximum amount of time to wait
 *
 * @retval OS_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval OS_STATUS_FAILURE          on failure
 * @retval OS_STATUS_SUCCESS          on success
 * @retval OS_STATUS_TIMED_OUT        time out exceeded
 *
 * @see os_socket_read
 */
OS_API OS_SECTION os_status_t os_socket_write(
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
 * @retval OS_STATUS_BAD_PARAMETER    invalid parameter
 * @retval OS_STATUS_FAILURE          on failure
 * @retval OS_STATUS_SUCCESS          on success
 */
OS_API OS_SECTION os_status_t os_stream_echo_set(
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
OS_API OS_SECTION const char *os_system_error_string(
	int error_number
);

/**
 * @brief Extracts information about the the operating system
 *
 * @param[in,out]  sys_info            structure to fill with operating system
 *                                     information
 *
 * @retval OS_STATUS_FAILURE          on failure
 * @retval OS_STATUS_SUCCESS          on success
 */
OS_API OS_SECTION os_status_t os_system_info(
	os_system_info_t *sys_info
);

/**
 * @brief run an executable on the operating system
 *
 * @param[in]      command             command string to be run
 * @param[out]     exit_status         return code of the executable (optional)
 * @param[out]     out_buf             pointer to buffers to write
 *                                     'standard out' & 'standard error' to
 * @param[in,out]  out_len             size of buffers for 'standard out' and
 *                                     'standard error'
 * @param[in]      max_time_out        maximum amount of time to wait
 *
 * @note Setting @p out_buf[0] & @p out_buf[1] both to @p NULL, will result
 * in running the command in a background process and this call returning
 * immediately
 *
 * @retval OS_STATUS_INVOKED          command triggered as a background process
 * @retval OS_STATUS_IO_ERROR         failed to setup error & output capture io
 * @retval OS_STATUS_NOT_EXECUTABLE   the path provided can not be executed
 * @retval OS_STATUS_SUCCESS          on success
 * @retval OS_STATUS_TIMED_OUT        time out exceeded
 */
OS_API OS_SECTION os_status_t os_system_run(
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
 * @retval OS_STATUS_INVOKED          shutdown triggered
 * @retval OS_STATUS_NOT_EXECUTABLE   shutdown failed to be triggered
 */
OS_API OS_SECTION os_status_t os_system_shutdown(
	os_bool_t reboot, unsigned int delay
);

/**
 * @brief tests the provided steram to determine if it supports vt-100
 *        terminal codes
 *
 * @param[in]      stream              stream to test
 *
 * @retval OS_TRUE                    terminal supports vt-100 codes
 * @retval OS_FALSE                   terminal does not support vt-100 codes
 */
OS_API OS_SECTION os_bool_t os_terminal_vt100_support(
	os_file_t stream
);

/**
 * @brief Function to setup a signal handler
 *
 * @param[in]      signal_handler      callback function to be called when a
 *                                     signal is encountered, NULL to remove a
 *                                     previously registered signal handler
 *
 * @retval OS_STATUS_FAILURE          on failure
 * @retval OS_STATUS_SUCCESS          on success
 */
OS_API OS_SECTION os_status_t os_terminate_handler(
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
OS_API OS_SECTION double os_random(
	double min,
	double max
);

/**
 * @brief Returns the system time
 *
 * @note This function will return either an absolute time or up time depending
 *       on the system implementation and support level
 *
 * @param[out]     time_stamp          current time
 * @param[out]     up_time             set to OS_TRUE, if the system only
 *                                     supports the up time; this is alway set
 *                                     correctly independent of this function's
 *                                     return code (optional)
 *
 * @retval OS_STATUS_BAD_PARAMETER    invalid parameter
 * @retval OS_STATUS_FAILURE          system call failed
 * @retval OS_STATUS_SUCCESS          on success
 */
OS_API OS_SECTION os_status_t os_time(
	os_timestamp_t *time_stamp,
	os_bool_t *up_time
);

/**
 * @brief Returns the system time in a formatted string
 *
 * @param[out]     buf                 pointer to buffer to place the time string
 * @param[in]      len                 length of the buffer
 *
 * @retval OS_STATUS_BAD_PARAMETER    invalid parameter
 * @retval OS_STATUS_FAILURE          system call failed
 * @retval OS_STATUS_SUCCESS          on success
 */
OS_API OS_SECTION os_status_t os_time_format(
	char *buf,
	size_t len );

/**
 * @brief Calculates the amount of time elapsed from a given time
 *
 * @param[in]      start_time          starting time
 * @param[out]     elapsed_time        elapsed time in milliseconds
 *
 * @retval OS_STATUS_BAD_PARAMETER    invalid parameter
 * @retval OS_STATUS_BAD_REQUEST      starting time is in the future
 * @retval OS_STATUS_SUCCESS          on success
 */
OS_API OS_SECTION os_status_t os_time_elapsed(
	os_timestamp_t *start_time,
	os_millisecond_t *elapsed_time );

/**
 * @brief Calculates the amount of time remaining from a start time and time out
 *
 * @param[in]      start_time          starting time
 * @param[in]      max_time_out        max amount of time (0 means forever)
 * @param[out]     remaining           amount of time remaining
 *
 * @retval OS_STATUS_BAD_PARAMETER    invalid parameter
 * @retval OS_STATUS_BAD_REQUEST      starting time is in the future
 * @retval OS_STATUS_TIMED_OUT        time out exceeded
 * @retval OS_STATUS_SUCCESS          on success
 */
OS_API OS_SECTION os_status_t os_time_remaining(
	os_timestamp_t *start_time,
	os_millisecond_t max_time_out,
	os_millisecond_t *remaining );

/**
 * @brief Sleeps for the specified time in milliseconds
 *
 * @param[in]      ms                  number of milliseconds to sleep for
 * @param[in]      allow_interrupts    allow the sleep to break on an interrupt
 *
 * @retval OS_STATUS_FAILURE          on system failure or system interruption
 * @retval OS_STATUS_SUCCESS          on success
 */
OS_API OS_SECTION os_status_t os_time_sleep(
	os_millisecond_t ms,
	os_bool_t allow_interrupts
);

/* threading */
#ifndef NO_THREAD_SUPPORT
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
 * @brief Wakes up all threads waiting on a condition variable
 *
 * @param[in,out]  cond                condition variable to wake up
 *
 * @retval OS_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval OS_STATUS_FAILURE          function failed
 * @retval OS_STATUS_SUCCESS          on success
 */
OS_API OS_SECTION os_status_t os_thread_condition_broadcast(
	os_thread_condition_t *cond
);

/**
 * @brief Creates a new condition variable
 *
 * @param[in,out]  cond                newly created condition variable
 *
 * @retval OS_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval OS_STATUS_FAILURE          function failed
 * @retval OS_STATUS_SUCCESS          on success
 */
OS_API OS_SECTION os_status_t os_thread_condition_create(
	os_thread_condition_t *cond
);

/**
 * @brief Destroys a previously created condition variable
 *
 * @param[in,out]  cond                previously created condition variable
 *
 * @retval OS_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval OS_STATUS_FAILURE          function failed
 * @retval OS_STATUS_SUCCESS          on success
 */
OS_API OS_SECTION os_status_t os_thread_condition_destroy(
	os_thread_condition_t *cond
);

/**
 * @brief Signals a thread waiting on a condition variable to wake up
 *
 * @param[in,out]  cond                condition variable to signal
 * @param[in,out]  lock                lock prototecting condition variable
 *
 * @retval OS_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval OS_STATUS_FAILURE          function failed
 * @retval OS_STATUS_SUCCESS          on success
 */
OS_API OS_SECTION os_status_t os_thread_condition_signal(
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
 * @retval OS_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval OS_STATUS_FAILURE          function failed
 * @retval OS_STATUS_SUCCESS          on success
 * @retval OS_STATUS_TIMED_OUT        maximum wait time reached
 */
OS_API OS_SECTION os_status_t os_thread_condition_timed_wait(
	os_thread_condition_t *cond,
	os_thread_mutex_t *lock,
	os_millisecond_t max_time_out
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
OS_API OS_SECTION os_status_t os_thread_condition_wait(
	os_thread_condition_t *cond,
	os_thread_mutex_t *lock
);

/**
 * @brief Creates a new thread
 *
 * @param[in,out]  thread              newly created thread object
 * @param[in]      main                main method to call for the thread
 * @param[in]      arg                 user specific data
 *
 * @retval OS_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval OS_STATUS_FAILURE          function failed
 * @retval OS_STATUS_SUCCESS          on success
 */
OS_API OS_SECTION os_status_t os_thread_create(
	os_thread_t *thread,
	os_thread_main_t main,
	void *arg
);

/**
 * @brief Destroys a previously created thread
 *
 * @param[in,out]  thread              thread object to destroy
 *
 * @retval OS_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval OS_STATUS_FAILURE          function failed
 * @retval OS_STATUS_SUCCESS          on success
 */
OS_API OS_SECTION os_status_t os_thread_destroy(
	os_thread_t *thread
);

/**
 * @brief Creates a new mutally exclusive lock
 *
 * @param[in,out]  lock                newly created lock
 *
 * @retval OS_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval OS_STATUS_FAILURE          function failed
 * @retval OS_STATUS_SUCCESS          on success
 */
OS_API OS_SECTION os_status_t os_thread_mutex_create(
	os_thread_mutex_t *lock
);

/**
 * @brief Destroys a mutally exclusive lock
 *
 * @param[in,out]  lock                previously created lock
 *
 * @retval OS_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval OS_STATUS_FAILURE          function failed
 * @retval OS_STATUS_SUCCESS          on success
 */
OS_API OS_SECTION os_status_t os_thread_mutex_destroy(
	os_thread_mutex_t *lock
);

/**
 * @brief Obtains a mutally exclusive lock (waits until lock is available)
 *
 * @param[in,out]  lock                previously created lock
 *
 * @retval OS_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval OS_STATUS_FAILURE          function failed
 * @retval OS_STATUS_SUCCESS          on success
 */
OS_API OS_SECTION os_status_t os_thread_mutex_lock(
	os_thread_mutex_t *lock
);

/**
 * @brief Releases a mutally exclusive lock
 *
 * @param[in,out]  lock                previously created lock
 *
 * @retval OS_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval OS_STATUS_FAILURE          function failed
 * @retval OS_STATUS_SUCCESS          on success
 */
OS_API OS_SECTION os_status_t os_thread_mutex_unlock(
	os_thread_mutex_t *lock
);

/**
 * @brief Creates a new read/write lock
 *
 * @param[in,out]  lock                newly created lock
 *
 * @retval OS_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval OS_STATUS_FAILURE          function failed
 * @retval OS_STATUS_SUCCESS          on success
 */
OS_API OS_SECTION os_status_t os_thread_rwlock_create(
	os_thread_rwlock_t *lock
);

/**
 * @brief Destroys a previously created read/write lock
 *
 * @param[in,out]  lock                previously created lock
 *
 * @retval OS_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval OS_STATUS_FAILURE          function failed
 * @retval OS_STATUS_SUCCESS          on success
 */
OS_API OS_SECTION os_status_t os_thread_rwlock_destroy(
	os_thread_rwlock_t *lock
);

/**
 * @brief Obtains a read/write lock for reading
 *
 * @param[in,out]  lock                previously created lock
 *
 * @retval OS_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval OS_STATUS_FAILURE          function failed
 * @retval OS_STATUS_SUCCESS          on success
 */
OS_API OS_SECTION os_status_t os_thread_rwlock_read_lock(
	os_thread_rwlock_t *lock
);

/**
 * @brief Releases read/write lock from reading
 *
 * @param[in,out]  lock                previously created lock
 *
 * @retval OS_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval OS_STATUS_FAILURE          function failed
 * @retval OS_STATUS_SUCCESS          on success
 */
OS_API OS_SECTION os_status_t os_thread_rwlock_read_unlock(
	os_thread_rwlock_t *lock
);

/**
 * @brief Obtains a read/write lock for writing
 *
 * @param[in,out]  lock                previously created lock
 *
 * @retval OS_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval OS_STATUS_FAILURE          function failed
 * @retval OS_STATUS_SUCCESS          on success
 */
OS_API OS_SECTION os_status_t os_thread_rwlock_write_lock(
	os_thread_rwlock_t *lock
);

/**
 * @brief Releases read/write lock from writing
 *
 * @param[in,out]  lock                previously created lock
 *
 * @retval OS_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval OS_STATUS_FAILURE          function failed
 * @retval OS_STATUS_SUCCESS          on success
 */
OS_API OS_SECTION os_status_t os_thread_rwlock_write_unlock(
	os_thread_rwlock_t *lock
);

/**
 * @brief Waits for a thread to complete
 *
 * @param[in,out]  thread              thread object to wait on
 *
 * @retval OS_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval OS_STATUS_FAILURE          function failed
 * @retval OS_STATUS_SUCCESS          on success
 */
OS_API OS_SECTION os_status_t os_thread_wait(
	os_thread_t *thread
);
#endif /* ifndef NO_THREAD_SUPPORT */

/* uuid support */
/**
 * @brief Generates a universally unique identifier
 *
 * @param[out]     uuid                generated identifier
 *
 * @retval OS_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval OS_STATUS_FAILURE          function failed
 * @retval OS_STATUS_SUCCESS          on success
 */
OS_API OS_SECTION os_status_t os_uuid_generate(
	os_uuid_t *uuid
);

/**
 * @brief Converts universally unique identifier to lower-case
 *
 * @param[in]      uuid                universally unique identifier
 * @param[out]     dest                destination buffer
 * @param[in]      len                 size of destination buffer
 *
 * @retval OS_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval OS_STATUS_NO_MEMORY        buffer length is not large enough
 * @retval OS_STATUS_SUCCESS          on success
 */
OS_API OS_SECTION os_status_t os_uuid_to_string_lower(
	os_uuid_t *uuid,
	char *dest,
	size_t len
);


#ifdef _WIN32 /* windows-specific definitions */

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

/* Use built-ins where possible */
#define os_library_find(lib, func)             GetProcAddress(module, name)
#define os_library_open(name)                  LoadLibrary(name)
#define os_memcpy(dst, src, num)               CopyMemory(dst, src, num); dst
#define os_memmove(dst, src, num)               MoveMemory(dst, src, num); dst
#define os_memset( ptr, val, num )             FillMemory( ptr, num, val )
#define os_memzero(ptr, size)                  ZeroMemory(ptr, size)
#define os_heap_calloc( num, size )            HeapAlloc( GetProcessHeap(), 0, num * size )
#define os_system_error_last()                 (int)GetLastError()

#else /* posix */

/* includes for #defined functions */
#include <dlfcn.h>
#include <errno.h>
#include <limits.h> /* for PATH_MAX */
#include <pthread.h>
#include <stdio.h>  /* for printf */
#include <stdlib.h> /* for malloc */
#include <string.h> /* for bzero, strncmp */
#include <sys/time.h> /* for gettimeofday */
#include <unistd.h> /* for sleep */
#include <time.h>   /* for nanosleep */

/* Use built-ins where possible */
#define os_heap_free(ptr)                      free(*ptr)
#define os_heap_free_null(x)                   { if ( *x ) free( *x ); *x = NULL; }
#define os_heap_malloc(size)                   malloc(size)
#define os_memcpy(dst, src, num)               memcpy(dst, src, num)
#define os_memzero(ptr, num)                   bzero(ptr, num)
#define os_strcmp(str1, str2)                  strcmp(str1, str2)
#define os_strlen(str)                         strlen(str)
#define os_strncmp(str1, str2, num)            strncmp(str1, str2, num)
#define os_strncpy(dst, src, num)              strncpy(dst, src, num)
#define os_strrchr(str, chr)                   strrchr(str, chr)
     
#define os_system_pid()                        (os_uint32_t)getpid()
#define os_system_error_last()                 errno
     
#define os_fprintf(file, fmt, ...)             fprintf(file, fmt, ##__VA_ARGS__)
#define os_printf(fmt, ...)                    printf(fmt, ##__VA_ARGS__)
#define os_snprintf(str, num, fmt, ...)        snprintf(str, num, fmt, ##__VA_ARGS__)
#define os_vfprintf(file, fmt, varg)           vfprintf(file, fmt, varg)
     
#define os_file_fgets(str, num, file)          fgets(str, (int) num, file)
#define os_file_fputs(str, file)               (size_t)fputs(str, file)
#define os_file_fread(str, size, num, file)    fread(str, size, num, file)
#define os_file_fwrite(str, size, num, file)   fwrite(str, size, num, file)
#define os_strchr(str, chr)                    strchr(str, chr)
#define os_strpbrk(str1, str2)                 strpbrk(str1, str2)
#define os_strstr(str1, str2)                  strstr(str1, str2)
#define os_strtod(str, end)                    strtod(str, end)
#define os_strtol(str, end)                    strtol(str, end, 10)
#define os_strtoul(str, end)                   strtoul(str, end, 10)
#define os_memmove(dst, src, num)              memmove(dst, src, num)
#define os_memset(ptr, val, num)               memset(ptr, val, num)
#define os_heap_calloc(num, size)              calloc(num, size)
#define os_heap_realloc(ptr, size)             realloc(ptr, size)

#define os_library_open(name)                  dlopen(name, 0)
#define os_library_find(lib, func)             dlsym(lib, func)

#endif /* ifdef _WIN32 */

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

#endif /* ifndef OS_H */

