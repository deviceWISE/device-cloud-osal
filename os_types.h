#ifndef OS_TYPES_H
#define OS_TYPES_H

#include <stdint.h> /* for uint8_t, uint16_t, uint32_t, etc. */

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

#endif /* ifndef OS_TYPES_H */