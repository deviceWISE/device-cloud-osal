/**
 * @file
 * @brief source file defining implementations for Windows systems
 *
 * @copyright Copyright (C) 2016-2017 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.  No license to Wind River intellectual property rights is
 * granted herein.  All rights not licensed by Wind River are reserved by Wind
 * River.
 */

#include "../os.h"
#pragma warning( push, 1 )
#include <float.h>  /* for DBL_MAX */
#include <signal.h> /* for SIGINT & SIGTERM */
#include <Shlwapi.h> /* for StrStr */
#if _MSC_VER > 1700
#include <VersionHelpers.h> /* for IsWindowsVersionOrGreater */
#endif
#pragma warning( pop )

/**
 * @brief Time in milliseconds to wait between retrying an operation
 */
#define LOOP_WAIT_TIME 100u

/**
 * @brief Seconds in one minute
 */
#define SECONDS_IN_MINUTE 60u

/**
 * @brief Array used by os_time_stamp_to_datetime
 */
static const unsigned short DAYS[4][12] =
{
	{ 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335 },
	{ 366, 397, 425, 456, 486, 517, 547, 578, 609, 639, 670, 700 },
	{ 731, 762, 790, 821, 851, 882, 912, 943, 974, 1004, 1035, 1065 },
	{ 1096, 1127, 1155, 1186, 1216, 1247, 1277, 1308, 1339, 1369, 1400, 1430 },
};
/** @brief Handle to the open log file */
static os_file_t LOG_HANDLE;
/** @brief Handle to the service controller */
static SERVICE_STATUS_HANDLE SERVICE_CONTROL_HANDLE = 0;
/** @brief Identifier to distinguish service in the service manager */
static char *SERVICE_KEY = NULL;
/** @brief Pointer to the entry-point for the service */
static os_service_main_t SERVICE_MAIN = NULL;
/** @brief Holds the number of arguments to pass to the service process */
static int  SERVICE_MAIN_ARGC = 0u;
/** @brief Holds an array of arguments to pass to the service process */
static char **SERVICE_MAIN_ARGV = NULL;
/** @biref Holds the current status of the service */
static SERVICE_STATUS SERVICE_STATUS_OBJ;
/** @brief Holds the function for handling service signals */
static os_sighandler_t SERVICE_HANDLER = NULL;
/** @brief Signal handler to process the termination signal */
static os_sighandler_t TERMINATE_HANDLER = NULL;
/**
 * @brief Structure that defines the properties of a Windows version
 */
struct win_version_info
{
	WORD major;            /**< @brief Major version number */
	WORD minor;            /**< @brief Minor version number */
	int metric;            /**< @brief System metric to check (0 to ignore) */
	const char *desktop;   /**< @brief Desktop release name */
	const char *server;    /**< @brief Server release name */
};

/**
 * @brief Structure relating version numbers to Windows name
 *
 * @note This must be listed with the latest Windows release first
 */
static struct win_version_info WIN_VERS[] = {
	{ 10,  1, 0,              "Technical Preview",       "Server Technical Preview" },
	{ 10,  0, 0,              "10",                      "Server 2016" },
	{  6,  3, 0,              "8.1",                     "Server 2012 R2" },
	{  6,  2, 0,              "8",                       "Server 2012" },
	{  6,  1, 0,              "7",                       "Server 2008 R2" },
	{  6,  0, 0,              "Vista",                   "Server 2008" },
	{  5,  2, SM_SERVERR2,    NULL,                      "Server 2003 R2" },
	{  5,  2, 0,              "XP Professional x64",     "Server 2003" },
	{  5,  1, SM_MEDIACENTER, "XP Media Center Edition", "Server 2003" },
	{  5,  1, SM_STARTER,     "XP Starter Edition",      "Server 2003" },
	{  5,  1, SM_TABLETPC,    "XP Tablet PC Edition",    "Server 2003" },
	{  5,  1, 0,              "XP",                      "Server 2003" },
	{  5,  0, 0,              NULL,                      "2000" },
	{  4, 90, 0,              "ME",                      NULL },
	{  4, 10, 0,              "98",                      NULL },
	{  4,  0, 0,              "95",                      "NT 4.0" },
	{  3, 51, 0,              NULL,                      "NT 3.51" },
	{  3, 50, 0,              NULL,                      "NT 3.5" },
	{  3,  2, 0,              "3.2",                     NULL },
	{  3, 11, 0,              "Workgroups 3.11",         NULL },
	{  3,  1, 0,              "3.1",                     "NT 3.1" },
	{  3,  0, 0,              "3.0",                     NULL },
	{  2, 11, 0,              "2.11",                    NULL },
	{  2, 10, 0,              "2.10",                    NULL },
	{  2,  3, 0,              "2.03",                    NULL },
	{  1,  4, 0,              "1.04",                    NULL },
	{  1,  3, 0,              "1.03",                    NULL },
	{  1,  2, 0,              "1.02",                    NULL },
	{  1,  1, 0,              "1.01",                    NULL },
	{  1,  0, 0,              "1.00",                    NULL },
	{  0,  0, 0,              "Client",                  "Server" }
};

/**
 * @brief Function to handle signals on Windows
 *
 * @param[in]      ctrl_type           type of signal received
 */
static BOOL WINAPI os_on_terminate( DWORD ctrl_type );

/**
 * @brief Perform a windows service control operation
 *
 * @param[in]      id                  id identifying the service
 * @param[in]      control_code        windows service control code to perform
 * @param[out]     status              a pointer to an SERVICE_STATUS structure
 *                                     for return values
 *
 * @retval OS_STATUS_SUCCESS          on success
 * @retval OS_STATUS_BAD_PARAMETER    on bad mandatory parameter(s)
 * @retval OS_STATUS_FAILURE          on failure
 *
 * @note Forward-slashes (/) and Backslashes (\) are not valid within a
 *       service name.
 */
static os_status_t os_service_control( const char *id,
	DWORD control_code, LPSERVICE_STATUS status );

#ifdef RegisterServiceCtrlHandlerEx
/**
 * @brief Internal function for handling service messages
 *
 * @param[in]      control_code        control code identifying incoming message
 * @param[in]      event_typea         type of event that occurred
 * @param[in]      event_data          additional device information
 * @param[in]      context             user-defined data
 *
 * @retval ERROR_CALL_NOT_IMPLEMENTED  control message was not handled
 * @retval NO_ERROR                    successfully processed message
 */
static DWORD WINAPI os_service_handler( DWORD control_code,
	DWORD UNUSED(event_type), LPVOID UNUSED(event_data),
	LPVOID UNUSED(context) );
#else
/**
 * @brief Internal function for handling service messages
 *
 * @param[in]      control_code        control code identifying incoming message
 */
static VOID WINAPI os_service_handler( DWORD control_code );
#endif

/**
 * @brief Service wrapper function to be called by Windows service control
 * manager
 *
 * @param[in]      argc                the number of arguments in argv
 * @param[in]      argv                Null-terminated argument strings passed
 *                                     to the service by the call to the
 *                                     StartService function that started the
 *                                     service. If there are no arguments, this
 *                                     parameter can be NULL. Otherwise, the
 *                                     first argument(argv[0]) is the name of
 *                                     the service, followed by any additional
 *                                     arguments(argv[1] through argv[argc - 1])
 *
 * If the user starts a manual service using the Services snap - in from the
 * Control Panel, the strings for the argv parameter come from the properties
 * dialog box for the service(from the Services snap - in, right - click the
 * service entry, click Properties, and enter the parameters in Start parameters
 */
static void WINAPI os_service_main( DWORD argc, LPTSTR *argv );

/**
 * @brief Internal helper function to convert a time stamp to a date & time
 *
 * @param[out]     date_time           destination to place result
 * @param[in]      time_stamp          time stamp to convert
 *
 * @retval
 */
static os_status_t os_time_stamp_to_datetime(
	os_date_time_t* date_time, os_timestamp_t time_stamp );


os_status_t os_adapters_address(
	os_adapters_t *adapters,
	int *family,
	int *flags,
	char *address,
	size_t address_len )
{
	os_status_t result = OS_STATUS_FAILURE;
	if ( adapters && adapters->current && address && address_len > 0u )
	{
		void* ptr = NULL;
		struct sockaddr * const addr =
			adapters->current->Address.lpSockaddr;
		if ( family )
			*family = addr->sa_family;

		if ( flags )
		{
			*flags = 0;
			if ( !( adapters->adapter_current->Flags & IP_ADAPTER_NO_MULTICAST ) )
				*flags |= IFF_MULTICAST;
			if ( adapters->adapter_current->IfType & IF_TYPE_SOFTWARE_LOOPBACK )
				*flags |= IFF_LOOPBACK;
		}

		if ( addr->sa_family == AF_INET6 )
			ptr = &(((struct sockaddr_in6 *)addr)->sin6_addr);
		else if ( addr->sa_family == AF_INET )
			ptr = &(((struct sockaddr_in *)addr)->sin_addr);

		if ( ptr && inet_ntop( addr->sa_family, ptr, address, address_len ) )
			result = OS_STATUS_SUCCESS;
	}
	return result;
}

os_status_t os_adapters_index(
	os_adapters_t *adapters,
	unsigned int *index )
{
	os_status_t result = OS_STATUS_FAILURE;
	if ( adapters && adapters->adapter_current && index )
	{
		int family = AF_INET;
		if ( adapters->current )
		{
			family =
			  adapters->current->Address.lpSockaddr->sa_family;
		}
		if ( family == AF_INET6 )
			*index = adapters->adapter_current->Ipv6IfIndex;
		else
			*index = adapters->adapter_current->IfIndex;
		result = OS_STATUS_SUCCESS;
	}
	return result;
}

os_status_t os_adapters_mac(
	os_adapters_t *adapters,
	char *mac,
	size_t mac_len )
{
	os_status_t result = OS_STATUS_FAILURE;
	if ( adapters && adapters->adapter_current &&
		adapters->adapter_current->NoMulticast == FALSE )
	{
		unsigned char *id = adapters->adapter_current->PhysicalAddress;
		const size_t id_len = adapters->adapter_current->PhysicalAddressLength;

		/* loop through to produce mac address */
		os_bool_t good_mac = OS_FALSE;
		size_t i;
		for ( i = 0u; i < id_len && i * 3u <= mac_len; ++i )
		{
			if ( id[ i ] > 0u )
				good_mac = OS_TRUE;
			os_sprintf( &mac[ i * 3u ], "%02.2x:", id[ i ] );
		}

		/* mac contained at least 1 non-zero value */
		if ( good_mac != OS_FALSE )
		{
			/* null-terminate mac */
			if ( i * 3u < mac_len )
				mac_len = i * 3u;
			mac[ mac_len - 1u ] = '\0';
			result = OS_STATUS_SUCCESS;
		}
	}
	return result;
}

os_status_t os_adapters_next(
	os_adapters_t *adapters )
{
	os_status_t result = OS_STATUS_FAILURE;
	if ( adapters && adapters->adapter_current )
	{
		if ( adapters->current )
			adapters->current = adapters->current->Next;
		if ( adapters->current )
			result = OS_STATUS_SUCCESS;
		else
		{
			adapters->adapter_current = adapters->adapter_current->Next;
			while( adapters->adapter_current && !adapters->current )
			{
				adapters->current = adapters->adapter_current->FirstUnicastAddress;
				if ( !adapters->current )
					adapters->adapter_current = adapters->adapter_current->Next;
			}

			if ( adapters->adapter_current && adapters->current )
				result = OS_STATUS_SUCCESS;
		}
	}
	return result;
}

os_status_t os_adapters_obtain(
	os_adapters_t *adapters )
{
	os_status_t result = OS_STATUS_FAILURE;
	if ( adapters )
	{
		ULONG size = 0u;
		ULONG error_code;

		ZeroMemory( adapters, sizeof( os_adapters_t ) );
		do
		{
			adapters->adapter_first = (IP_ADAPTER_ADDRESSES *)
				HeapAlloc( GetProcessHeap(), 0, size );
			error_code = GetAdaptersAddresses( AF_UNSPEC,
				GAA_FLAG_SKIP_MULTICAST,
				NULL, adapters->adapter_first, &size );
			if ( error_code == ERROR_BUFFER_OVERFLOW )
			{
				HeapFree( GetProcessHeap(), 0,
					adapters->adapter_first );
				adapters->adapter_first = NULL;
			}
		} while ( error_code == ERROR_BUFFER_OVERFLOW );

		if ( error_code == ERROR_SUCCESS )
		{
			adapters->adapter_current = adapters->adapter_first;
			while( adapters->adapter_current && !adapters->current )
			{
				adapters->current = adapters->adapter_current->FirstUnicastAddress;
				if ( !adapters->current )
					adapters->adapter_current = adapters->adapter_current->Next;
			}

			if ( adapters->current )
				result = OS_STATUS_SUCCESS;
			else
			{
				HeapFree( GetProcessHeap(), 0,
					adapters->adapter_first );
				ZeroMemory( adapters, sizeof( struct os_adapters ) );
			}
		}
	}
	return result;
}

os_status_t os_adapters_release(
	os_adapters_t *adapters )
{
	os_status_t result = OS_STATUS_FAILURE;
	if ( adapters && adapters->adapter_first )
	{
		HeapFree( GetProcessHeap(), 0, adapters->adapter_first );
		ZeroMemory( adapters, sizeof( struct os_adapters ) );
		result = OS_STATUS_SUCCESS;
	}
	return result;
}

const char *os_directory_get_temp_dir( char * dest, size_t size )
{
	const char * result = NULL;
	if ( dest && size > 0u )
	{
		os_env_get( "TEMP", dest, size );
		if ( dest[0] != '\0' )
			result = dest;
	}
	return result;
}

/* character testing support */
os_bool_t os_char_isalnum(
	char c )
{
	os_bool_t result = OS_FALSE;
	if ( IsCharAlphaNumeric( c ) )
		result = OS_TRUE;
	return result;
}

os_bool_t os_char_isxdigit(
	char c )
{
	const char ch = c;
	os_bool_t result = OS_FALSE;
	if ( ( ch >= '0' && ch <= '9' ) || ( ch >='A' && ch <= 'F' ) ||
		( ch >= 'a' && ch <= 'f' ) )
		result = OS_TRUE;
	return result;
}

char os_char_tolower(
	char c )
{
	char str[2u];
	str[0u] = c;
	str[1u] = '\0';
	CharLowerA( str );
	return str[0u];
}

char os_char_toupper(
	char c )
{
	char str[2u];
	str[0u] = c;
	str[1u] = '\0';
	CharUpperA( str );
	return str[0u];
}

/* file & directory support */
os_status_t os_directory_create(
		const char *path,
		os_millisecond_t timeout )
{
	os_bool_t create_dir_failed = OS_FALSE;
	os_status_t result;
	os_timestamp_t start_time;
	os_millisecond_t time_elapsed;

	os_time( &start_time, NULL );
	do {
		result = os_directory_create_nowait( path );
		if ( result != OS_STATUS_SUCCESS )
		{
			os_time_elapsed( &start_time, &time_elapsed );
			os_time_sleep( LOOP_WAIT_TIME, OS_FALSE );
		}
	} while ( result != OS_STATUS_SUCCESS &&
		( timeout == 0u || time_elapsed < timeout ) );

	return result;
}

os_status_t os_directory_create_nowait(
	const char *path )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( path )
	{
		size_t path_len = 0;
		char *p = NULL;
		char temp_path[ PATH_MAX + 1u ];
		result = OS_STATUS_FAILURE;

		if ( os_directory_exists( path ) )
			return OS_STATUS_SUCCESS;

		/* Get the parent dir and check if it exists */
		os_strncpy( temp_path, path, PATH_MAX );
		path_len = os_strlen( temp_path );
		for ( p = &temp_path[ path_len - 1u ];
			  p > temp_path && *p != OS_DIR_SEP;
			  p--);
		if ( p > temp_path )
		{
			*p = '\0';
			if ( os_directory_exists( temp_path ) == OS_FALSE )
				os_directory_create_nowait( temp_path );
		}

		if ( CreateDirectory( path, NULL ) != 0 )
			result = OS_STATUS_SUCCESS;
	}
	return result;
}

os_status_t os_directory_current(
	char *buffer,
	size_t size
)
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( buffer )
	{
		DWORD cur_dir_result;

		result = OS_STATUS_FAILURE;
		cur_dir_result = GetCurrentDirectory( size, buffer );
		if ( cur_dir_result > 0 && cur_dir_result <= size )
			result = OS_STATUS_SUCCESS;
	}
	return result;
}

os_status_t os_directory_close(
	os_dir_t *dir )
{
	os_status_t result = OS_STATUS_FAILURE;
	if ( dir && dir->dir && FindClose( dir->dir ) )
	{
		ZeroMemory( dir, sizeof( os_dir_t ) );
		result = OS_STATUS_SUCCESS;
	}
	return result;
}

os_status_t os_directory_change(const char *path)
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( path )
	{
		result = OS_STATUS_SUCCESS;
		if ( !SetCurrentDirectory( path ))
			result = OS_STATUS_FAILURE;
	}
	return result;
}

os_status_t os_directory_delete(
	const char *path, const char *regex, os_bool_t recursive )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( path )
	{
		result = OS_STATUS_SUCCESS;
		const char *regex_str = regex;
		if ( !regex_str || *regex_str == '\0' )
			regex_str = "*";
		else if ( os_strncmp( regex, ".", 2u ) == 0 ||
			os_strncmp( regex, "..", 3u ) == 0 ||
			os_strstr( regex, "\\/" ) != NULL )
			result = OS_STATUS_BAD_REQUEST;

		if ( result == OS_STATUS_SUCCESS )
		{
			HANDLE dir;
			WIN32_FIND_DATA wfd;
			char search_path[MAX_PATH];
			ZeroMemory( &wfd, sizeof( wfd ) );

			/* go through all subdirectories first */
			if ( recursive != OS_FALSE )
			{
				os_snprintf( search_path, MAX_PATH - 1u,
					"%s\\*", path );
				search_path[ MAX_PATH - 1u ] = '\0';
				dir = FindFirstFile( search_path, &wfd );
				result = OS_STATUS_FAILURE;
				if ( dir != INVALID_HANDLE_VALUE )
				{
					do
					{
						result = OS_STATUS_SUCCESS;
						if ( os_strncmp( wfd.cFileName, ".", 2u ) != 0 &&
							os_strncmp( wfd.cFileName, "..", 3u ) != 0 &&
							wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
						{
							size_t buf_len = os_strlen( path ) +
								os_strlen( wfd.cFileName ) + 2u;
							char *buf = (char*)os_heap_malloc( buf_len );

							result = OS_STATUS_NO_MEMORY;
							if ( buf )
							{
								result = OS_STATUS_SUCCESS;
								os_snprintf( buf, buf_len,
									"%s\\%s", path,
									wfd.cFileName );
								result = os_directory_delete(
									buf,
									regex,
									recursive );
								os_heap_free( (void **)&buf );
							}
						}
					} while ( result == OS_STATUS_SUCCESS &&
						FindNextFile( dir, &wfd ) );
					FindClose( dir );
				}
			}

			/* find all matching files and directories */
			ZeroMemory( &wfd, sizeof( wfd ) );
			os_snprintf( search_path, MAX_PATH - 1u, "%s\\%s",
				path, regex_str );
			search_path[ MAX_PATH - 1u ] = '\0';
			dir = FindFirstFile( search_path, &wfd );
			result = OS_STATUS_FAILURE;
			if ( dir != INVALID_HANDLE_VALUE )
			{
				do
				{
					result = OS_STATUS_SUCCESS;
					if ( os_strncmp( wfd.cFileName, ".", 2u ) != 0 &&
						os_strncmp( wfd.cFileName, "..", 3u ) != 0 )
					{
						size_t buf_len = os_strlen( path ) +
							os_strlen( wfd.cFileName ) + 2u;
						char *buf = (char*)os_heap_malloc( buf_len );

						result = OS_STATUS_NO_MEMORY;
						if ( buf )
						{
							result = OS_STATUS_SUCCESS;
							os_snprintf( buf, buf_len,
								"%s\\%s", path,
								wfd.cFileName );
							if ( ( wfd.dwFileAttributes &
								FILE_ATTRIBUTE_DIRECTORY ) )
								RemoveDirectory( buf );
							else if ( !DeleteFile( buf ) )
								result = OS_STATUS_FAILURE;
							os_heap_free( (void **)&buf );
						}
					}
				} while ( result == OS_STATUS_SUCCESS &&
					FindNextFile( dir, &wfd ) );
				FindClose( dir );
			}

			/* clear empty directory */
			if ( result == OS_STATUS_SUCCESS && regex == NULL )
			{
				if ( !RemoveDirectory( path ) )
				{
					if ( GetLastError() == ERROR_DIR_NOT_EMPTY )
						result = OS_STATUS_TRY_AGAIN;
					else
						result = OS_STATUS_FAILURE;
				}
			}
		}
	}
	return result;
}

os_bool_t os_directory_exists(
	const char *dir_path )
{
	DWORD dir_attrib;
	os_bool_t result = OS_FALSE;
	dir_attrib = GetFileAttributes( dir_path );
	if ( dir_attrib != INVALID_FILE_ATTRIBUTES &&
		dir_attrib & FILE_ATTRIBUTE_DIRECTORY )
		result = OS_TRUE;
	return result;
}

os_status_t os_directory_rewind(
	os_dir_t *dir )
{
	os_status_t result = OS_STATUS_FAILURE;
	if( dir && dir->dir )
	{
		char path[PATH_MAX];
		os_strncpy( path, dir->path, PATH_MAX );
		os_directory_close( dir );
		os_directory_open( path, dir );
		result = OS_STATUS_SUCCESS;
	}
	return result;
}

os_status_t os_directory_next(
	os_dir_t *dir,
	os_bool_t files_only,
	char *path,
	size_t path_len )
{
	os_status_t result = OS_STATUS_FAILURE;
	if ( dir && dir->path && dir->last_result == OS_STATUS_SUCCESS )
	{
		BOOL file_found = FALSE;

		/* Search for next acceptable file */
		while ( ( file_found = FindNextFile( dir->dir, &dir->wfd ) ) &&
			( os_strncmp( dir->wfd.cFileName, ".", 1 ) == 0 ||
			os_strncmp( dir->wfd.cFileName, "..", 2 ) == 0 ||
			( files_only != OS_FALSE &&
			dir->wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) ) );

		if ( file_found )
		{
			os_make_path( path, path_len, dir->path,
				dir->wfd.cFileName, NULL );
			result = OS_STATUS_SUCCESS;
		}

		dir->last_result = result;
	}
	return result;
}

os_status_t os_directory_open(
	const char *dir_path,
	os_dir_t* out )
{
	os_status_t result = OS_STATUS_FAILURE;
	if ( dir_path && out )
	{
		char new_dir_path[ PATH_MAX + 1u];
		ZeroMemory( out, sizeof( os_dir_t ) );
		StringCchCopy( out->path, PATH_MAX, dir_path );
		StringCchCopy( new_dir_path, PATH_MAX, dir_path );
		StringCchCat( new_dir_path, PATH_MAX, "\\*" );
		out->dir = FindFirstFile( new_dir_path, &out->wfd );
		if ( out->dir != INVALID_HANDLE_VALUE )
			out->last_result = result = OS_STATUS_SUCCESS;
	}
	return result;
}

os_uint64_t os_directory_free_space(
	const char* path )
{
	DWORD free_bytes = -1;

	if ( path && os_strlen( path ) >= 3 )
	{
		int drive_number = -1;
		drive_number = PathGetDriveNumber( path );
		if ( drive_number >= 0 || drive_number <= 25 )
		{
			const char* alpha = "abcdefghilmnopqrstuvwxyz";
			BOOL  fResult;
			char  drive[4];
			DWORD dwSectPerClust,
				dwBytesPerSect,
				dwFreeClusters,
				dwTotalClusters;

			os_sprintf(drive, "%c:\\", alpha[drive_number]);

			fResult = GetDiskFreeSpace(drive,
				&dwSectPerClust,
				&dwBytesPerSect,
				&dwFreeClusters,
				&dwTotalClusters);

			if (fResult)
				free_bytes = dwFreeClusters * dwSectPerClust *
					dwBytesPerSect;
		}
	}
	return (os_uint64_t)free_bytes;
}

os_status_t os_file_chown(
	const char *UNUSED(path),
	const char *UNUSED(user) )
{
	return OS_STATUS_SUCCESS;
}

os_status_t os_file_close(
	os_file_t handle )
{
	os_status_t result = OS_STATUS_FAILURE;
	if ( handle && CloseHandle( handle ) )
		result = OS_STATUS_SUCCESS;
	return result;
}

os_status_t os_file_copy(
	const char *old_path,
	const char *new_path )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( old_path && new_path )
	{
		result = OS_STATUS_FAILURE;
		if ( CopyFileEx( old_path, new_path, NULL, NULL, NULL, 0 ) )
			result = OS_STATUS_SUCCESS;
	}
	return result;
}

os_status_t os_file_delete(
	const char *path )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( path )
	{
		result = OS_STATUS_FAILURE;
		if ( os_file_exists( path ) )
		{
			if ( DeleteFile( path ) )
				result = OS_STATUS_SUCCESS;
		}
	}
	return result;
}

os_bool_t os_file_exists(
	const char *file_path )
{
	DWORD file_attrib;
	os_bool_t result = OS_FALSE;
	file_attrib = GetFileAttributes( file_path );
	if ( file_attrib != INVALID_FILE_ATTRIBUTES &&
		!( file_attrib & FILE_ATTRIBUTE_DIRECTORY ) )
		result = OS_TRUE;
	return result;
}

char *os_file_fgets(
	char *str,
	size_t size,
	os_file_t stream )
{
	DWORD number_of_bytes_read = 0u;
	char *read = (char *)os_heap_malloc( size );
	if ( read )
	{
		read[size - 1u] = '\0';
		if ( size > 0u )
		{
			if ( ReadFile( stream, read, size - 1u,
				&number_of_bytes_read, NULL ) )
			{
				size_t result = 0u;
				size_t i = 0u;
				char *str_pos = str;
				char *read_pos  = read;
				while ( i < number_of_bytes_read &&
					*read_pos != '\0' )
				{
					*str_pos = *read_pos;
					++read_pos;
					++str_pos;
					++i;
				}
				*str_pos = '\0';
				result = i + 1u;

				/* Set file pointer to next char after
				 * null-terminator */
				if ( result < number_of_bytes_read )
					SetFilePointer( stream,
						result - number_of_bytes_read + 1u,
						NULL, 1 );
			}
		}
		os_heap_free( (void **)&read );
	}
	if ( number_of_bytes_read == 0u )
		str =  NULL;
	return str;
}

size_t os_file_fputs(
	char *str,
	os_file_t stream )
{
	DWORD len;
	DWORD result;
	len = (DWORD)os_strlen( str );
	WriteFile( stream, str, len, &result, NULL );
	return (size_t)result;
}

size_t os_file_fread(
	void *ptr,
	size_t size,
	size_t nmemb,
	os_file_t stream )
{
	size_t result = 0;
	if ( size > 0 && nmemb > 0 )
	{
		DWORD number_of_bytes_read = 0;
		ReadFile( stream, ptr, ( size * nmemb ),
			&number_of_bytes_read, NULL );
		result = number_of_bytes_read / size;
	}
	return result;
}

os_status_t os_file_fseek(
	os_file_t stream,
	long offset,
	int whence
)
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( stream != OS_FILE_INVALID )
	{

		result = OS_STATUS_FAILURE;
		if ( SetFilePointer( stream, offset, NULL, whence )
			!= INVALID_SET_FILE_POINTER )
			result = OS_STATUS_SUCCESS;
	}
	return result;
}

os_status_t os_file_fsync( const char *UNUSED(file_path) )
{
	/* buffers will be flushed when a stream is closed */
	return OS_STATUS_SUCCESS;
}

size_t os_file_fwrite(
	const void *ptr,
	size_t size,
	size_t nmemb,
	os_file_t stream )
{
	size_t result = 0;
	if ( size > 0 && nmemb > 0 )
	{
		DWORD number_of_bytes_wrote = 0;
		WriteFile( stream, ptr, ( size * nmemb ),
			&number_of_bytes_wrote, NULL );
		result = number_of_bytes_wrote / size;
	}
	return result;
}

os_uint64_t os_file_get_size(
	const char *file_path )
{

	WIN32_FILE_ATTRIBUTE_DATA attr;
	LARGE_INTEGER size;
	LONGLONG file_size = 0;

	if ( GetFileAttributesEx( file_path, GetFileExInfoStandard, &attr ) )
	{
		size.HighPart = attr.nFileSizeHigh;
		size.LowPart = attr.nFileSizeLow;
		file_size = size.QuadPart;
	}
	return (os_uint64_t)file_size;
}

os_uint64_t os_file_get_size_handle(
	os_file_t file_handle )
{
	DWORD file_size = 0;
	if ( file_handle )
		file_size = GetFileSize( file_handle, NULL );
	return (os_uint64_t)file_size;
}

os_status_t os_file_move(
	const char *old_path,
	const char *new_path
)
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( old_path && new_path )
	{
		result = OS_STATUS_FAILURE;
		if ( MoveFileEx( old_path, new_path,
			MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH |
			MOVEFILE_COPY_ALLOWED ) )
			result = OS_STATUS_SUCCESS;
	}
	return result;
}

os_file_t os_file_open(
	const char *file_path,
	int flags )
{
	os_file_t result = NULL;
	if ( file_path )
	{
		OFSTRUCT ofs;
		DWORD access = 0;
		DWORD share_mode = FILE_SHARE_READ;
		DWORD create_mode = OPEN_EXISTING;

		if ( flags & OS_WRITE )
			access = GENERIC_WRITE;
		if ( flags & OS_READ )
			access |= GENERIC_READ;

		if ( ( flags & OS_CREATE ) && ( flags & OS_EXCLUSIVE ) )
			create_mode = CREATE_NEW;
		else if ( flags & OS_WRITE )
		{
			if ( flags & OS_CREATE )
			{
				if ( ( flags & OS_APPEND ) && !( flags & OS_TRUNCATE ) )
					create_mode = OPEN_ALWAYS;
				else
					create_mode = CREATE_ALWAYS;
			}
			else if ( flags & OS_TRUNCATE )
				create_mode = TRUNCATE_EXISTING;
		}
		else if ( flags & OS_CREATE )
			create_mode = OPEN_ALWAYS;

		if ( flags & OS_EXCLUSIVE )
			share_mode = 0;

		ZeroMemory( &ofs, sizeof( OFSTRUCT ) );
		result = CreateFile( file_path, access, share_mode, NULL,
			create_mode, FILE_ATTRIBUTE_NORMAL, NULL );
	}
	if ( result == INVALID_HANDLE_VALUE )
		result = NULL;
	else if ( ( flags & OS_APPEND ) && ( flags & OS_WRITE ) )
		SetFilePointer( result, 0, NULL, FILE_END );
	return result;
}

os_status_t os_file_temp(
	char *prototype,
	size_t suffix_len )
{
	os_status_t result = OS_STATUS_FAILURE;
	if ( prototype && os_strstr( prototype, "XXXXXX" ) )
	{
		int const max_attempt = 128;
		int i = 0;
		int str_len = os_strlen( prototype );
		char suffix[64];
		char *x_location = prototype + str_len - suffix_len - 6;

		os_strncpy( suffix, prototype + str_len - suffix_len, suffix_len );

		for ( ; i < max_attempt && result == OS_STATUS_FAILURE; i++ )
		{
			unsigned int random_number = (int)os_random( 0, 999999 );
			os_snprintf( x_location, 6u + suffix_len + 1u, "%.6u%s", random_number, suffix );

			if ( os_file_exists( prototype ) == OS_FALSE )
				result = OS_STATUS_SUCCESS;
		}
	}
	return result;
}

char os_key_wait( void )
{
	char result = 0;
	HANDLE handle = GetStdHandle( STD_INPUT_HANDLE );
	/* console found */
	if ( handle != NULL )
	{
		DWORD mode, chars_read;
		GetConsoleMode( handle, &mode );
		SetConsoleMode( handle,
			mode & ~( ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT ) );
		ReadConsoleA( handle, &result, 1, &chars_read, NULL );
		SetConsoleMode( handle, mode );
	}
	return result;
}

os_status_t os_library_close(
	os_lib_handle lib )
{
	os_status_t result = OS_STATUS_FAILURE;
	if ( FreeLibrary( lib ) )
		result = OS_STATUS_SUCCESS;
	return result;
}

int os_atoi( const char *str )
{
	int result = 0;
	int multiplier = 1;

	if ( str )
	{
		if ( *str == '-' )
		{
			multiplier = -1;
			++str;
		}
		while ( *str && *str != '\0' && *str >= '0' && *str <= '9' )
		{
			result = (result * 10) + ( *str - '0' );
			++str;
		}
	}
	return result * multiplier;
}

char *os_strchr(
	const char *s,
	char c )
{
	char *result = NULL;
	while ( *s != '\0' && *s != c )
		++s;
	if ( *s == c )
		result = (char *)s;
	return result;
}

int os_strcmp(
	const char *s1,
	const char *s2
)
{
	while( ( *s1 ) && ( *s1 == *s2 ) )
		s1++, s2++;
	return *( const unsigned char* )s1 - *( const unsigned char* )s2;
}

size_t os_strlen(
	const char *s )
{
	size_t result = 0u;
	while ( *s != '\0' )
	{
		++result;
		++s;
	}
	return result;
}

char *os_strncat(
	char *s1,
	const char *s2,
	size_t count
	)
{
	size_t i, j;
	for (i = 0; s1[i] != '\0'; i++)
		;
	for (j = 0; s2[j] != '\0' && j < count; j++)
		s1[i + j] = s2[j];
	s1[i + j] = '\0';
	return s1;
}

int os_strncmp(
	const char *s1,
	const char *s2,
	size_t len
)
{
	size_t i = 0u;
	while( ( *s1 ) && ( *s1 == *s2 ) && ( i < len ) )
		s1++, s2++, i++;
	return i == len ? 0 : *( const unsigned char* )s1 - *( const unsigned char* )s2;
}

char *os_strncpy(
	char *destination,
	const char *source,
	size_t num )
{
	size_t i = 0;
	for ( ; i < num && *source != '\0'; ++i, ++destination, ++source )
		*destination = *source;
	if ( i < num )
		*destination = '\0';
	return destination;
}

char *os_strpbrk(
	const char *str1,
	const char *str2 )
{
	char *result = NULL;
	while ( *str1 != '\0' && !result )
	{
		const char *c = str2;
		while ( *c != '\0' && !result )
		{
			if ( *(c++) == *str1 )
				result = (char *)str1;
		}
		++str1;
	}
	return result;
}

char *os_strrchr(
	const char *s,
	char c )
{
	char *result = NULL;
	if ( s )
	{
		while ( *s )
		{
			if ( *s == c )
				result = (char *)s;
			s++;
		}
	}
	return result;
}

double os_strtod(
	const char *str,
	char **endptr )
{
	double result = 0.0;
	if ( str )
	{
		int integer = 0;
		int decimal = 0;
		int *current = &integer;
		int decimal_points = 1;
		os_bool_t after = OS_FALSE;
		int negative = 1;
		char *num;
		char nums[] = "0123456789.";
		if ( *str == '-' )
		{
			negative = -1;
			++str;
		}
		num = os_strchr( nums, *str );
		while ( *str && num && !( num-nums == 10 && after == OS_TRUE ) )
		{
			if ( num-nums == 10 && after == OS_FALSE )
			{
				after = OS_TRUE;
				current = &decimal;
			}
			else
			{
				*current = (*current)*10 + num-nums;
				if ( after == OS_TRUE )
					decimal_points *= 10;
			}
			++str;
			if ( *str )
				num = os_strchr( nums, *str );
		}
		result = negative * ( (double)integer + (double)decimal / decimal_points );
		if ( endptr )
			*endptr = (char *)str;
	}
	return result;
}

long os_strtol(
	const char *str,
	char **endptr )
{
	long result = 0L;
	if ( str )
	{
		int negative = 1;
		char *num;
		char nums[] = "0123456789";
		if ( *str == '-' )
		{
			negative = -1;
			++str;
		}
		num = os_strchr( nums, *str );
		while ( *str && num )
		{
			result = result*10 + num-nums;
			++str;
			if ( *str )
				num = os_strchr( nums, *str );
		}
		result *= negative;
		if ( endptr )
			*endptr = (char *)str;
	}
	return result;
}

unsigned long os_strtoul(
	const char *str,
	char **endptr )
{
	unsigned long result = 0L;
	if ( str )
	{
		char *num;
		char nums[] = "0123456789";
		num = os_strchr( nums, *str );
		while ( *str && num )
		{
			result = result*10 + num-nums;
			++str;
			if ( *str )
				num = os_strchr( nums, *str );
		}
		if ( endptr )
			*endptr = (char *)str;
	}
	return result;
}

/* memory functions */
int os_memcmp(
	const void *ptr1,
	const void *ptr2,
	size_t num )
{
	int result = 0;
	if ( ptr1 && ptr2 )
	{
		size_t i = 0u;
		const unsigned char *p1 = (unsigned char*)ptr1;
		const unsigned char *p2 = (unsigned char*)ptr2;
		while ( result == 0 && i < num )
		{
			if ( *p1 < *p2 )
				result = -1;
			else if (  *p1 > *p2 )
				result = 1;
			++p1;
			++p2;
			++i;
		}
	}
	else if ( !ptr1 )
		result = -1;
	else
		result = 1;
	return result;
}

/* print functions */
int os_printf(
	const char *format,
	... )
{
	int result;
	va_list args;
	va_start( args, format );
	result = os_vfprintf( GetStdHandle( STD_OUTPUT_HANDLE ), format,
		args );
	va_end( args );
	return result;
}

size_t os_env_expand(
	char *src,
	size_t len )
{
	size_t result = 0u;
	if ( src )
	{
		result = (size_t)ExpandEnvironmentStrings( src, NULL, 0u );
		if ( result < len )
		{
			char *dest = (char*)HeapAlloc( GetProcessHeap(), 0,
				result + 1u );
			if ( dest )
			{
				ExpandEnvironmentStrings( src, dest, len );
				os_memcpy( src, dest, len );
				HeapFree( GetProcessHeap(), 0, dest );
			}
		}
	}
	return result;
}

size_t os_env_get(
	const char *env,
	char *dest,
	size_t len )
{
	size_t result = 0u;
	if ( dest && len > 0u )
		dest[0] = '\0';
	if ( env && dest && len > 0u )
		result = (size_t)GetEnvironmentVariable( env, dest, len );
	return result;
}

int os_fprintf(
	os_file_t stream,
	const char *format,
	... )
{
	int result;
	va_list args;
	va_start( args, format );
	result = os_vfprintf( stream, format, args );
	va_end( args );
	return result;
}

int os_sprintf(
	char *str,
	const char *format,
	... )
{
	int result;
	va_list args;
	va_start( args, format );
	result = os_vsnprintf( str, STRSAFE_MAX_CCH, format, args );
	va_end( args );
	return result;
}

int os_snprintf(
	char *str,
	size_t size,
	const char *format,
	... )
{
	int result;
	va_list args;
	va_start( args, format );
	result = os_vsnprintf( str, size, format, args );
	va_end( args );
	return result;
}

int os_vfprintf(
	os_file_t stream,
	const char *format,
	va_list args )
{
	char *buffer;
	const size_t max_str_len = 1024u;
	int result = -1;

	buffer = (char *)os_heap_malloc( max_str_len );
	if ( buffer )
	{
		const int str_len =
			os_vsnprintf( buffer, max_str_len, format, args );
		if ( str_len > 0 )
		{
			/* convert unbounded "\n" to "\r\n" */
			int i;
			const char *write_start = buffer;
			const char *pos = buffer;
			os_bool_t have_carriage_return = OS_FALSE;
			DWORD number_written;
			result = 0;
			for ( i = 0; i < str_len && result >= 0; ++i )
			{
				if ( *pos == '\r' )
					have_carriage_return = OS_TRUE;
				else
				{
					if ( have_carriage_return == OS_FALSE
						&& *pos == '\n' )
					{
						const DWORD to_write =
							pos - write_start;
						if ( WriteFile( stream,
							write_start,
							to_write,
							&number_written,
							NULL ) )
						{
							write_start = pos;
							result +=
								(int)number_written;
							if ( WriteFile( stream,
								"\r", 1,
								&number_written,
								NULL ) )
								result +=
									(int)number_written;
							else
								result = -1;
						}
						else
							result  = -1;
					}
					have_carriage_return = OS_FALSE;
				}
				++pos;
			}
			if ( WriteFile( stream, write_start,
				pos - write_start, &number_written, NULL ) )
				result += (int)number_written;
			else
				result = -1;
		}
		os_heap_free( (void **)&buffer );
	}
	return result;
}

int os_vsnprintf(
	char *str,
	size_t size,
	const char *format,
	va_list args )
{
	int result = -1;
	char *str_end = NULL;
	if ( StringCchVPrintfEx( str, size, &str_end, NULL, 0, format, args ) ==
		S_OK )
		result = str_end - str;
	return result;
}

os_bool_t os_flush( os_file_t stream )
{
	os_bool_t result = OS_FALSE;
	if ( FlushFileBuffers( stream ) )
		result = OS_TRUE;
	return result;
}

/* memory functions */
void os_heap_free( void **ptr )
{
	if ( ptr && *ptr )
	{
		HeapFree( GetProcessHeap(), 0, *ptr );
		*ptr = NULL;
	}
}

void *os_heap_malloc( size_t size )
{
	return HeapAlloc( GetProcessHeap(), 0, size );
}

void *os_heap_realloc( void *ptr, size_t size )
{
	void *result;
	if ( ptr )
		result = HeapReAlloc( GetProcessHeap(), 0, ptr, size );
	else
		result = HeapAlloc( GetProcessHeap(), 0, size );
	return result;
}

BOOL WINAPI os_on_terminate( DWORD ctrl_type )
{
	BOOL result = FALSE;
	int signum = SIGINT;
	if ( ctrl_type == CTRL_CLOSE_EVENT || ctrl_type == CTRL_LOGOFF_EVENT ||
		ctrl_type == CTRL_SHUTDOWN_EVENT )
		signum = SIGTERM;
	if ( TERMINATE_HANDLER )
	{
		(*TERMINATE_HANDLER)( signum );
		result = TRUE;
	}
	return result;
}

os_bool_t os_path_is_absolute( const char *path )
{
	os_bool_t result = OS_FALSE;

	/* test path started with a letter + colon (i.e. C:, D:, a: ... ) */
	if ( path && ( ( path[0] >= 'A' && path[0] <= 'Z' ) ||
		     ( path[0] >= 'a' && path[0] <= 'z' ) ) &&
		path[1] == ':' )
		result = OS_TRUE;
	return result;
}

os_status_t os_path_executable(
	char *path,
	size_t size )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( path )
	{
		result = OS_STATUS_FAILURE;
		if ( GetModuleFileNameA( NULL, path, size ) > 0 )
			result = OS_STATUS_SUCCESS;
	}
	return result;
}

/* process functions */
os_status_t os_process_cleanup( void )
{
	return OS_STATUS_SUCCESS;
}

/* service functions */
os_status_t os_service_control(
	const char *id,
	DWORD control_code,
	LPSERVICE_STATUS status )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( id != NULL && *id != '\0' )
	{
		SC_HANDLE sc_manager;
		result = OS_STATUS_FAILURE;
		/* Get a handle to the SCM database. */
		sc_manager = OpenSCManager( 0, 0, SC_MANAGER_ALL_ACCESS );
		if ( sc_manager )
		{
			const SC_HANDLE sc_service = OpenService( sc_manager,
				id, SERVICE_ALL_ACCESS );
			/* Open the service */
			if ( sc_service )
			{
				BOOL control_result;
				control_result = ControlService( sc_service,
					control_code, status );
				if ( control_result ||
				   ( !control_result && GetLastError() ==
				     ERROR_SERVICE_NOT_ACTIVE ) )
					result = OS_STATUS_SUCCESS;
				CloseServiceHandle( sc_service );
			}
			CloseServiceHandle( sc_manager );
		}
	}
	return result;
}

os_status_t os_service_run(
	const char *id,
	os_service_main_t main,
	int argc,
	char *argv[],
	int remove_argc,
	const char *remove_argv[],
	os_sighandler_t handler,
	const char *logdir )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( logdir )
	{
		struct tm *current_date = NULL;
		char date_stamp[OS_NAME_MAX_LEN];
		os_date_time_t date_time;
		char logfile_name[OS_NAME_MAX_LEN];
		char logfile_full_path[PATH_MAX];
		os_bool_t up_time;
		os_timestamp_t time_stamp;

		ZeroMemory( &date_time, sizeof(os_date_time_t) );
		os_time( &time_stamp, &up_time );

		/* Make sure that os_time did not return the uptime for
		 * the device */
		/* This will not convert properly into a date stamp */
		if ( time_stamp > 0u && up_time == OS_FALSE )
		{

			result = os_time_stamp_to_datetime(
				&date_time, time_stamp );
			if ( result == OS_STATUS_SUCCESS )
			{
				os_snprintf( date_stamp, OS_NAME_MAX_LEN,
					"%04d-%02d-%02d_%02d-%02d-%02d",
					date_time.year,
					date_time.month,
					date_time.day,
					date_time.hour,
					date_time.minute,
					date_time.second
					);
				date_stamp[ OS_NAME_MAX_LEN - 1u ] = '\0';
				os_snprintf( logfile_name,
					OS_NAME_MAX_LEN,
					"%s_%s.log", id, date_stamp );
				logfile_name[ OS_NAME_MAX_LEN - 1u ] = '\0';
			}
		}
		else
		{
			os_snprintf( logfile_name, OS_NAME_MAX_LEN,
				"%s.log", id );
			logfile_name[ OS_NAME_MAX_LEN - 1u ] = '\0';
		}

		os_make_path( logfile_full_path, PATH_MAX,
			logdir, logfile_name, NULL );

		LOG_HANDLE = os_file_open( logfile_full_path,
			OS_WRITE | OS_CREATE );
		if ( LOG_HANDLE )
		{
			SetStdHandle( STD_ERROR_HANDLE, LOG_HANDLE );
			SetStdHandle( STD_OUTPUT_HANDLE, LOG_HANDLE );
		}
	}

	if ( id && main )
	{
		result = OS_STATUS_EXISTS;
		if ( SERVICE_KEY == NULL && SERVICE_MAIN == NULL )
		{
			const size_t service_id_len =
				os_strlen( id );

			result = OS_STATUS_NO_MEMORY;
			SERVICE_KEY = (char *)os_heap_malloc( service_id_len );
			if ( SERVICE_KEY )
			{
				os_strncpy( SERVICE_KEY, id, service_id_len );
				SERVICE_KEY[ service_id_len - 1u ] = '\0';

				SERVICE_MAIN = main;
				SERVICE_MAIN_ARGC = 0;
				SERVICE_MAIN_ARGV = (char**)os_heap_malloc(
					sizeof(char*) * argc );
				if ( SERVICE_MAIN_ARGV )
				{
					os_bool_t no_error = OS_TRUE;
					int i;
					for ( i = 0; i < argc; ++i && no_error != OS_FALSE )
					{
						BOOL is_good_arg = TRUE; /* true */
						if ( remove_argc > 0 && remove_argv && argv[i] )
						{
							int j;
							for ( j = 0;
								j < remove_argc && is_good_arg; ++j )
							{
								if ( remove_argv[j] )
								{
									size_t arg_len =
										os_strlen( remove_argv[j] );
									is_good_arg = os_strncmp( argv[i],
										remove_argv[j], arg_len );
								}
							}
						}

						if ( is_good_arg )
						{
							SERVICE_MAIN_ARGV[SERVICE_MAIN_ARGC] = NULL;
							if ( argv[i] )
							{
								const size_t arg_len =
									os_strlen( argv[i] ) + 1u;
								SERVICE_MAIN_ARGV[SERVICE_MAIN_ARGC] =
									(char*)os_heap_malloc( sizeof( char ) * arg_len );
								if ( SERVICE_MAIN_ARGV[SERVICE_MAIN_ARGC] )
									os_strncpy( SERVICE_MAIN_ARGV[SERVICE_MAIN_ARGC], argv[i], arg_len );
							}
							else
								no_error = OS_TRUE;
							++SERVICE_MAIN_ARGC;
						}
					}

					if ( no_error != OS_FALSE )
					{
						SERVICE_TABLE_ENTRY ste[] = {
							{ SERVICE_KEY,
							  &os_service_main },
							{ NULL, NULL }
						};

						/* bind any provided callbacks */
						if ( handler )
							SERVICE_HANDLER = handler;
						result = OS_STATUS_FAILURE;
						if ( StartServiceCtrlDispatcher( ste ) )
							result = OS_STATUS_SUCCESS;
					}
					else
					{
						while ( SERVICE_MAIN_ARGC > 0 )
							os_heap_free( (void **)&SERVICE_MAIN_ARGV[SERVICE_MAIN_ARGC - 1] );
						os_heap_free( (void **)&SERVICE_MAIN_ARGV );
						os_heap_free( (void **)&SERVICE_KEY );
					}
				}
			}
		}
	}
	return result;
}

/* Service control callback */
#ifdef RegisterServiceCtrlHandlerEx
DWORD WINAPI os_service_handler( DWORD control_code,
	DWORD UNUSED(event_type), LPVOID UNUSED(event_data),
	LPVOID UNUSED(context) )
{
	DWORD result = ERROR_CALL_NOT_IMPLEMENTED;
#else
VOID WINAPI os_service_handler( DWORD control_code )
{
#endif
	int signal_id = 0;
	switch ( control_code )
	{
		case SERVICE_CONTROL_CONTINUE:
			/* Continue from Paused state. */
			signal_id = SIGCONT;
			break;
		case SERVICE_CONTROL_PAUSE:
			/* Pause service. */
			signal_id = SIGSTOP; /* can't be caught on linux */
			break;
		case SERVICE_CONTROL_INTERROGATE:
			/* Interrogate service */
#ifdef RegisterServiceCtrlHandlerEx
			result = NO_ERROR;
#endif
			break;
		case SERVICE_CONTROL_STOP:
			/* Service should stop. */
			signal_id = SIGTERM;
			SERVICE_STATUS_OBJ.dwCurrentState = SERVICE_STOP_PENDING;
			SERVICE_STATUS_OBJ.dwWin32ExitCode = NO_ERROR;
			SERVICE_STATUS_OBJ.dwWaitHint = 0;
			++SERVICE_STATUS_OBJ.dwCheckPoint;
			if ( SERVICE_CONTROL_HANDLE )
				SetServiceStatus( SERVICE_CONTROL_HANDLE,
					&SERVICE_STATUS_OBJ );
			break;
	}
	if ( signal_id != 0 )
	{
		if ( SERVICE_HANDLER )
				SERVICE_HANDLER( signal_id );
		++SERVICE_STATUS_OBJ.dwCheckPoint;
		SetServiceStatus( SERVICE_CONTROL_HANDLE, &SERVICE_STATUS_OBJ );

#ifdef RegisterServiceCtrlHandlerEx
		result = NO_ERROR;
#endif
	}
#ifdef RegisterServiceCtrlHandlerEx
	return result;
#endif
}

void WINAPI os_service_main( DWORD argc, LPTSTR *argv )
{
#ifdef RegisterServiceCtrlHandlerEx
	SERVICE_CONTROL_HANDLE = RegisterServiceCtrlHandlerEx( SERVICE_KEY,
		os_service_handler, NULL );
#else
	SERVICE_CONTROL_HANDLE = RegisterServiceCtrlHandler( SERVICE_KEY,
		os_service_handler );
#endif
	if ( SERVICE_CONTROL_HANDLE != 0 )
	{
		ZeroMemory( &SERVICE_STATUS_OBJ,
			sizeof( SERVICE_STATUS_OBJ ) );
		SERVICE_STATUS_OBJ.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
		SERVICE_STATUS_OBJ.dwCurrentState = SERVICE_RUNNING;
		SERVICE_STATUS_OBJ.dwControlsAccepted =
			SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_STOP;
		SERVICE_STATUS_OBJ.dwWin32ExitCode = NO_ERROR;
		SERVICE_STATUS_OBJ.dwCheckPoint = 0;
		SetServiceStatus( SERVICE_CONTROL_HANDLE, &SERVICE_STATUS_OBJ );

		if ( SERVICE_MAIN )
		{
			const int main_result = SERVICE_MAIN(
				SERVICE_MAIN_ARGC, SERVICE_MAIN_ARGV );
			if ( main_result != EXIT_SUCCESS )
			{
				SERVICE_STATUS_OBJ.dwWin32ExitCode =
					ERROR_SERVICE_SPECIFIC_ERROR;
				SERVICE_STATUS_OBJ.dwServiceSpecificExitCode =
					(DWORD)main_result;
			}
		}

		SERVICE_STATUS_OBJ.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus( SERVICE_CONTROL_HANDLE, &SERVICE_STATUS_OBJ );

		/* clean up */
		if ( SERVICE_MAIN_ARGV )
		{
			int i;
			for ( i = 0; i < SERVICE_MAIN_ARGC; ++i )
				os_heap_free( (void **)&SERVICE_MAIN_ARGV[i] );
			os_heap_free( (void **)&SERVICE_MAIN_ARGV );
		}

		if ( LOG_HANDLE)
			os_file_close( LOG_HANDLE );

		os_heap_free( (void **)&SERVICE_KEY );
		SERVICE_MAIN = NULL;
	}
}

os_status_t os_service_install(
	const char *id,
	const char *executable,
	const char *args,
	const char *name,
	const char *description,
	const char *dependencies,
	os_millisecond_t timeout
)
{
/** @brief Maximum number of retries when service encounters a failure */
#define SERVICE_RETRY_COUNT_MAX  3u
/** @brief Number of seconds between each retry */
#define SERVICE_RETRY_DELAY_SECONDS 120u
/** @brief Number of hours before resetting the fail counter of the service */
#define SERVICE_RESET_FAIL_COUNTER_HOURS 6u

	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( executable && id && *id != '\0' && name && *name != '\0' )
	{
		char *cmd_line;
		size_t cmd_line_len =
			os_strlen( executable );

		if ( args && *args != '\0' )
			/* +1 for space character between arguments */
			cmd_line_len += os_strlen( args ) + 1u;
		result = OS_STATUS_NO_MEMORY;
		cmd_line = (char *)os_heap_malloc( sizeof(char) *
			( cmd_line_len + 1u ) );
		if ( cmd_line )
		{
			SC_HANDLE sc_manager;

			if ( args && *args != '\0' )
				os_snprintf( cmd_line,
					cmd_line_len + 1u,
					"%s %s", executable, args );
			else
				os_strncpy( cmd_line,
					executable,
					cmd_line_len + 1u );
			cmd_line[cmd_line_len] = '\0';

			result = OS_STATUS_FAILURE;
			if ( ( sc_manager = OpenSCManager( 0, 0,
				SC_MANAGER_ALL_ACCESS ) ) != 0 )
			{
				SC_HANDLE sc_service;
				char *depends = NULL;
				if ( dependencies )
				{
					/* +1 for null-terminator */
					const size_t dep_len =
						os_strlen( dependencies ) + 1u;
					/* extra +1 because must be double
					 * null-terminated */
					depends = (char*)os_heap_malloc(
						 dep_len + 1u );
					if ( depends )
					{
						char *d = depends;
						os_strncpy( depends,
							dependencies, dep_len );

						/* add double null-terminator */
						depends[dep_len - 1u] = '\0';
						depends[dep_len] = '\0';

						/* replace all ';' with '\0' */
						while ( *d != '\0' )
						{
							if ( *d == ';' )
								*d ='\0';
							++d;
						}
					}
				}

				if ( ( sc_service = CreateServiceA( sc_manager,
					id, name, SERVICE_ALL_ACCESS,
					SERVICE_WIN32_OWN_PROCESS,
					SERVICE_AUTO_START,
					SERVICE_ERROR_NORMAL,
					cmd_line,
					0, /* LoadOrderGroup */
					0, /* TagId (out) */
					depends, /* Dependencies */
					0, /* System User */
					0 /* Password */ ) ) != 0 )
				{
					result = OS_STATUS_SUCCESS;
					if ( description )
					{
						LPTSTR desc_heap;
#ifdef UNICODE
						size_t desc_len = MultiByteToWideChar(
								CP_UTF8, 0,
								description, -1,
								NULL, 0 );
#else
						size_t desc_len =
							os_strlen( description ) + 1u;
#endif
						desc_heap = (LPTSTR)os_heap_malloc( desc_len );
						if ( desc_heap )
						{
							SERVICE_DESCRIPTION sd;
#ifdef UNICODE
							MultiByteToWideChar(
								CP_UTF8, 0,
								description,
								-1,
								(LPWSTR)desc_heap,
								desc_len );
#else
							os_strncpy( (char*)desc_heap,
									description, desc_len );
							desc_heap[ desc_len - 1u ] = '\0';
#endif
							sd.lpDescription = desc_heap;
							if ( !ChangeServiceConfig2( sc_service,
								SERVICE_CONFIG_DESCRIPTION, &sd ) )
								result = OS_STATUS_FAILURE;
							os_heap_free( (void**)&desc_heap );
						}
						else
							result = OS_STATUS_NO_MEMORY;
					}

					/* reboot service on failure */
					if ( result == OS_STATUS_SUCCESS )
					{
						SERVICE_FAILURE_ACTIONS sfa;
						SERVICE_FAILURE_ACTIONS_FLAG sfaf;

						ZeroMemory( &sfa, sizeof( sfa ) );
						sfa.dwResetPeriod = (DWORD)(
							SERVICE_RESET_FAIL_COUNTER_HOURS
							* OS_MINUTES_IN_HOUR
							* OS_SECONDS_IN_MINUTE );
						sfa.lpRebootMsg = NULL;
						sfa.lpCommand = NULL;

						sfa.lpsaActions = (SC_ACTION*)
							os_heap_malloc(
								sizeof( SC_ACTION ) *
								SERVICE_RETRY_COUNT_MAX );
						if ( sfa.lpsaActions )
						{
							unsigned int i;
							SC_ACTION *lpAction =
								sfa.lpsaActions;
							for ( i = 0u; i <
								SERVICE_RETRY_COUNT_MAX; ++i )
							{
								lpAction->Type =
									SC_ACTION_RESTART;
								lpAction->Delay =
									SERVICE_RETRY_DELAY_SECONDS *
									OS_MILLISECONDS_IN_SECOND;
								++lpAction;
							}
							sfa.cActions = SERVICE_RETRY_COUNT_MAX;
						}
						else
							result = OS_STATUS_NO_MEMORY;

						if ( result == OS_STATUS_SUCCESS &&
							!ChangeServiceConfig2(
								sc_service,
								SERVICE_CONFIG_FAILURE_ACTIONS,
								&sfa ) )
							result = OS_STATUS_FAILURE;

						ZeroMemory( &sfaf, sizeof( sfaf ) );
						sfaf.fFailureActionsOnNonCrashFailures = TRUE;
						if ( result == OS_STATUS_SUCCESS &&
							!ChangeServiceConfig2(
								sc_service,
								SERVICE_CONFIG_FAILURE_ACTIONS_FLAG,
								&sfaf ) )
							result = OS_STATUS_FAILURE;

						os_heap_free( (void**)&sfa.lpsaActions );
					}

					/* on failure, delete the service */
					if ( result != OS_STATUS_SUCCESS )
						DeleteService( sc_service );

					CloseServiceHandle( sc_service );
				}
				else if ( GetLastError() == ERROR_DUPLICATE_SERVICE_NAME )
					result = OS_STATUS_EXISTS;
				CloseServiceHandle( sc_manager );
			}
		}
	}
	return result;
}

os_status_t os_service_uninstall(
	const char *id,
	os_millisecond_t timeout )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	SC_HANDLE sc_manager;
	SC_HANDLE sc_service;

	if ( id != NULL && *id != '\0' )
	{
		/* Get a handle to the SCM database. */
		result = OS_STATUS_FAILURE;
		if ((sc_manager = OpenSCManager(0, 0, SC_MANAGER_ALL_ACCESS)) != 0)
		{
			/* Open the service */
			if ((sc_service = OpenServiceA(sc_manager, id, DELETE)) != 0)
			{
				/* Delete the service. */
				if (DeleteService(sc_service))
					result = OS_STATUS_SUCCESS;
				CloseServiceHandle(sc_service);
			}
			else if ( GetLastError() == ERROR_INVALID_NAME ||
				GetLastError() == ERROR_SERVICE_DOES_NOT_EXIST )
			{
				result = OS_STATUS_NOT_FOUND;
			}
			CloseServiceHandle(sc_manager);
		}
	}
	return result;
}

os_status_t os_service_start(
	const char *id,
	os_millisecond_t timeout
)
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	SC_HANDLE sc_manager;
	SC_HANDLE sc_service;

	if ( id != NULL && *id != '\0' )
	{
		result = OS_STATUS_FAILURE;
		/* Get a handle to the SCM database. */
		if ((sc_manager = OpenSCManager(0, 0, SC_MANAGER_ALL_ACCESS)) != 0)
		{
			/* Open the service */
			if ((sc_service = OpenServiceA(sc_manager, id, SERVICE_START)) != 0)
			{
				/* start the service */
				if ( StartService( sc_service, 0, NULL ) ||
					GetLastError() ==
						ERROR_SERVICE_ALREADY_RUNNING )
					result = OS_STATUS_SUCCESS;
				CloseServiceHandle(sc_service);
			}
			CloseServiceHandle(sc_manager);
		}
	}
	return result;
}

os_status_t os_service_stop(
	const char *id,
	const char *UNUSED(exe),
	os_millisecond_t timeout
)
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	const DWORD start_time = GetTickCount();

	if ( id && *id != '\0' )
	{
		SC_HANDLE sc_manager;
		result = OS_STATUS_FAILURE;
		sc_manager = OpenSCManager(
			NULL,                    /* local computer */
			NULL,                    /* ServicesActive database */
			SC_MANAGER_ALL_ACCESS ); /* full access rights */

		if ( sc_manager )
		{
			/* Get a handle to the service. */
			const SC_HANDLE sc_service = OpenServiceA(
				sc_manager, id,
				SERVICE_STOP |
				SERVICE_QUERY_STATUS |
				SERVICE_ENUMERATE_DEPENDENTS );

			if ( sc_service )
			{
				SERVICE_STATUS_PROCESS ssp;
				DWORD bytes_needed;
				BOOL query_succeeded;
				os_bool_t timeout_expired = OS_FALSE;

				ZeroMemory( &ssp,
					sizeof( SERVICE_STATUS_PROCESS ) );

				/* check the current state of the service */
				query_succeeded = QueryServiceStatusEx(
					sc_service,
					SC_STATUS_PROCESS_INFO,
					(LPBYTE)&ssp,
					sizeof(SERVICE_STATUS_PROCESS),
					&bytes_needed );
				result = OS_STATUS_SUCCESS;
				if ( query_succeeded &&
					ssp.dwCurrentState != SERVICE_STOP_PENDING &&
					ssp.dwCurrentState != SERVICE_STOPPED )
				{
					SERVICE_STATUS status = {0};

					/* stop the service */
					result = os_service_control(
						id, SERVICE_CONTROL_STOP,
						&status );

					/* assume in pending state for loop */
					ssp.dwCurrentState =
						SERVICE_STOP_PENDING;
				}

				while ( result == OS_STATUS_SUCCESS &&
					query_succeeded &&
					ssp.dwCurrentState == SERVICE_STOP_PENDING &&
					timeout_expired == OS_FALSE )
				{
					DWORD wait_time;
					/* Do not wait longer than the wait hint. A good interval is */
					/* one-tenth of the wait hint but not less than 1 second     */
					/* and not more than 10 seconds. (MSDN recommendation).      */
					wait_time = ssp.dwWaitHint / 10;
					if ( wait_time < 1000 )
						wait_time = 1000;
					else if ( wait_time > 10000 )
						wait_time = 10000;
					else if ( wait_time > timeout )
						wait_time = timeout;

					Sleep( wait_time );

					query_succeeded = QueryServiceStatusEx(
						sc_service,
						SC_STATUS_PROCESS_INFO,
						(LPBYTE)&ssp,
						sizeof(SERVICE_STATUS_PROCESS),
						&bytes_needed );

					if ( timeout != 0u &&
						( ( GetTickCount() - start_time ) > timeout ) )
						timeout_expired = OS_TRUE;
				}

				if ( ssp.dwCurrentState != SERVICE_STOPPED )
					result = OS_STATUS_FAILURE;
				else if ( timeout_expired != OS_FALSE )
					result = OS_STATUS_TIMED_OUT;
			}
			else if ( GetLastError() == ERROR_INVALID_NAME ||
				GetLastError() == ERROR_SERVICE_DOES_NOT_EXIST )
			{
				result = OS_STATUS_NOT_FOUND;
			}
		}
	}
	return result;
}

os_status_t os_service_query(
	const char *id,
	os_millisecond_t timeout
)
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	const DWORD start_time = GetTickCount();

	if ( id && *id != '\0' )
	{
		const SC_HANDLE sc_manager = OpenSCManager(
			NULL,                    /* local computer */
			NULL,                    /* ServicesActive database */
			SC_MANAGER_CONNECT );    /* connect to SCM */

		result = OS_STATUS_FAILURE;
		if ( sc_manager )
		{
			/* Get a handle to the service. */
			const SC_HANDLE sc_service = OpenServiceA(
				sc_manager, id,
				SERVICE_QUERY_STATUS );

			if ( sc_service )
			{
				SERVICE_STATUS_PROCESS ssp;
				DWORD bytes_needed;
				BOOL query_succeeded;
				os_bool_t timeout_expired = OS_FALSE;

				ZeroMemory( &ssp,
					sizeof( SERVICE_STATUS_PROCESS ) );

				/* check the current state of the service */
				query_succeeded = QueryServiceStatusEx(
					sc_service,
					SC_STATUS_PROCESS_INFO,
					(LPBYTE)&ssp,
					sizeof( SERVICE_STATUS_PROCESS ),
					&bytes_needed );

				if ( query_succeeded )
				{
					result = OS_STATUS_NOT_INITIALIZED;
					if ( ssp.dwCurrentState == SERVICE_RUNNING )
						result = OS_STATUS_SUCCESS;
				}

				if ( timeout != 0u &&
					( ( GetTickCount() - start_time ) > timeout ) )
				{
					result = OS_STATUS_TIMED_OUT;
				}
			}
			else if ( GetLastError() == ERROR_INVALID_NAME ||
				GetLastError() == ERROR_SERVICE_DOES_NOT_EXIST )
			{
				result = OS_STATUS_NOT_FOUND;
			}
		}
	}
	return result;
}

os_status_t os_service_restart(
	const char *id,
	const char *UNUSED(exe),
	os_millisecond_t timeout
)
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	const DWORD start_time = GetTickCount();
	if ( id != NULL && *id != '\0' )
	{
		/* check to see if the service is already running */
		result = os_service_stop( id, NULL,
			timeout - (GetTickCount() - start_time) );
		os_time_sleep(1000, OS_TRUE);

		if (result == OS_STATUS_SUCCESS)
		{
			SERVICE_STATUS service_status = {0};
			os_bool_t service_stopped = OS_FALSE;
			int i = 0;
			int max_retries = 10;

			/* wait for the service to stop */
			while (i < max_retries && !service_stopped)
			{
				result = os_service_control( id,
					SERVICE_CONTROL_INTERROGATE,
					&service_status );
				if (result == OS_STATUS_SUCCESS)
				{
					if (service_status.dwCurrentState == SERVICE_STOPPED)
						service_stopped = OS_TRUE;
					else
						os_time_sleep(1000, OS_TRUE);
				}
				else
					result = OS_STATUS_FAILURE;
				i++;
			}
			result = OS_STATUS_FAILURE;
			if (service_stopped)
				result = os_service_start( id,
					timeout - ( GetTickCount() - start_time ) );
		}
	}
	return result;
}

/* socket functions */
int os_get_host_address(
	const char *host,
	const char *service,
	char *address,
	int address_len,
	int family
	)
{
	int result;
	struct addrinfo *address_list = NULL;
	struct addrinfo hints;

	os_memzero( &hints, sizeof( hints ) );

	hints.ai_family = family;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	result = getaddrinfo( host, service, &hints, &address_list );

	if ( result == 0 && address_list )
	{
		if ( address_list->ai_family == AF_INET )
		{
			struct sockaddr_in *address_struct = (struct sockaddr_in *) address_list->ai_addr;
			inet_ntop( AF_INET, &(address_struct->sin_addr), address, address_len );
		}
		else if ( address_list->ai_family == AF_INET6 )
		{
			struct sockaddr_in6 *address_struct = (struct sockaddr_in6 *) address_list->ai_addr;
			inet_ntop( AF_INET6, &(address_struct->sin6_addr), address, address_len );
		}
	}
	return result;
}

os_status_t os_socket_accept(
	const os_socket_t *socket,
	os_socket_t *out,
	os_millisecond_t max_time_out )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( socket && out )
	{
		result = OS_STATUS_FAILURE;
		if ( socket->fd != OS_SOCKET_INVALID )
		{
			int select_result = 1;
			if ( max_time_out > 0u )
			{
				struct timeval ts;
				fd_set rfds;

				ts.tv_sec = max_time_out / OS_MILLISECONDS_IN_SECOND;
				ts.tv_usec = ( max_time_out % OS_MILLISECONDS_IN_SECOND ) *
					OS_MICROSECONDS_IN_MILLISECOND;

				FD_ZERO( &rfds );
				FD_SET( socket->fd, &rfds );
				select_result = select( socket->fd + 1,
					&rfds, NULL, NULL, &ts );
				if ( select_result == 0 )
					result = OS_STATUS_TIMED_OUT;
			}
			if ( select_result > 0 )
			{
				socklen_t sock_len = sizeof( struct sockaddr );
				os_memcpy( out, socket, sizeof( os_socket_t ) );
				out->fd = accept( socket->fd,
					&out->addr, &sock_len );
				if ( out->fd != OS_SOCKET_INVALID )
					result = OS_STATUS_SUCCESS;
			}
		}
	}
	return result;
}

os_status_t os_socket_bind(
	const os_socket_t *socket,
	int queue_size
)
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( socket )
	{
		result = OS_STATUS_FAILURE;
		if ( socket->fd != OS_SOCKET_INVALID && bind( socket->fd,
			&socket->addr, sizeof( struct sockaddr ) ) == 0 )
		{
			if ( listen( socket->fd, queue_size ) == 0 )
				result = OS_STATUS_SUCCESS;
		}
	}
	return result;
}

os_status_t os_socket_broadcast(
	const os_socket_t *socket,
	const void *buf,
	size_t len,
	int ttl,
	size_t *bytes_written,
	os_millisecond_t max_time_out )
{
	ssize_t result = -1;
	const int broadcast_enable = 1;
	if ( socket && socket->fd != OS_SOCKET_INVALID )
	{
		result = setsockopt( socket->fd, SOL_SOCKET, SO_BROADCAST,
			(const char *)&broadcast_enable,
			sizeof( broadcast_enable ) );
		if ( ttl > 1 )
			setsockopt( socket->fd, IPPROTO_IP, IP_MULTICAST_TTL,
				(const char*)&ttl, sizeof( ttl ) );
		if ( result == 0 )
			result = os_socket_write( socket, buf, len,
				bytes_written, max_time_out );
	}
	return result;
}

os_status_t os_socket_close(
	os_socket_t *socket )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( socket )
	{
		result = OS_STATUS_FAILURE;
		if ( socket->fd != OS_SOCKET_INVALID )
		{
			/* disable send/receive on socket */
			if ( socket->type == SOCK_STREAM )
				shutdown( socket->fd, SD_BOTH );
			if ( closesocket( socket->fd ) == 0 )
			{
				ZeroMemory( socket, sizeof( os_socket_t ) );
				result = OS_STATUS_SUCCESS;
			}
		}
	}
	return result;
}

os_status_t os_socket_connect(
	const os_socket_t *socket )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( socket )
	{
		result = OS_STATUS_FAILURE;
		if ( socket->fd != OS_SOCKET_INVALID &&
			WSAConnect( socket->fd, &socket->addr,
				sizeof( struct sockaddr ),
				NULL, NULL, NULL, NULL ) == 0 )
			result = OS_STATUS_SUCCESS;
	}
	return result;
}

os_status_t os_socket_initialize( void )
{
	os_status_t result = OS_STATUS_FAILURE;
	WSADATA wsa_data;
	const int err = WSAStartup( MAKEWORD( 2, 2 ), &wsa_data );
	if ( err == 0 )
	{
		if ( LOBYTE( wsa_data.wVersion ) != 2 ||
			HIBYTE( wsa_data.wVersion ) != 2 )
			WSACleanup();
		else
			result = OS_STATUS_SUCCESS;
	}
	return result;
}

os_status_t os_socket_open(
	os_socket_t *out,
	const char *address,
	os_uint16_t port,
	int type,
	int protocol,
	os_millisecond_t max_time_out )
{
	os_millisecond_t time_elapsed = 0u;
	os_status_t result = OS_STATUS_BAD_PARAMETER;

	if ( out && address && port > 0u )
	{
		void *const addr_ptr = &out->addr;
		struct sockaddr_in  *const addr4 =
			(struct sockaddr_in *)addr_ptr;
		struct sockaddr_in6 *const addr6 =
			(struct sockaddr_in6 *)addr_ptr;
		result = OS_STATUS_FAILURE;
		memset( out, 0, sizeof( os_socket_t ) );
		if ( inet_pton( AF_INET, address,
			&(addr4->sin_addr) ) == 1 )
		{
			addr4->sin_family = AF_INET;
			addr4->sin_port = (in_port_t)htons( port );
			result = OS_STATUS_SUCCESS;
		}
		else if ( inet_pton( AF_INET6, address,
			&(addr6->sin6_addr) ) == 1 )
		{
			addr6->sin6_family = AF_INET6;
			addr6->sin6_port = (in_port_t)htons( port );
			result = OS_STATUS_SUCCESS;
		}

		if ( result == OS_STATUS_SUCCESS )
		{
			out->type = type;
			out->protocol = protocol;
			out->fd = socket( out->addr.sa_family, type,
				protocol );
			while ( out->fd == OS_SOCKET_INVALID &&
				( max_time_out == 0u || time_elapsed < max_time_out ) )
			{
				struct timeval ts;

				os_millisecond_t wait_time = 2000u;
				if ( max_time_out > 0u && max_time_out - time_elapsed <  wait_time )
					wait_time = max_time_out - time_elapsed;
				ts.tv_sec = wait_time / OS_MILLISECONDS_IN_SECOND;
				ts.tv_usec = ( wait_time % OS_MILLISECONDS_IN_SECOND ) *
					OS_MICROSECONDS_IN_MILLISECOND;

				/* keep trying to obtain a socket until one if available,
				 * this condition may be hit when running in a service, and
				 * the client application is started before network services
				 * are available */
				select( 0, NULL, NULL, NULL, &ts );
				out->fd = socket( out->addr.sa_family,
					type, protocol );
			}

			if ( out->fd == OS_SOCKET_INVALID )
				result = OS_STATUS_TIMED_OUT;
		}
	}
	return result;
}

os_status_t os_socket_option(
	const os_socket_t *socket,
	int level,
	int optname,
	const void *optval,
	size_t optlen )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( socket && socket->fd != OS_SOCKET_INVALID && optval )
	{
		result = OS_STATUS_FAILURE;
		if ( setsockopt( socket->fd, level, optname,
			(const char *)optval, optlen ) == 0 )
			result = OS_STATUS_SUCCESS;
	}
	return result;
}

os_status_t os_socket_read(
	const os_socket_t *socket,
	void *buf,
	size_t len,
	size_t* bytes_read,
	os_millisecond_t max_time_out )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( bytes_read )
		*bytes_read = 0u;
	if ( socket && socket->fd != OS_SOCKET_INVALID )
	{
		DWORD bytes_in = 0;
		result = OS_STATUS_FAILURE;
		if ( max_time_out > 0u )
		{
			fd_set read_fds;
			struct timeval timeout;

			ZeroMemory( &timeout, sizeof( struct timeval ) );
			timeout.tv_sec = max_time_out / OS_MILLISECONDS_IN_SECOND;
			timeout.tv_usec = (max_time_out % OS_MILLISECONDS_IN_SECOND) *
				OS_MICROSECONDS_IN_MILLISECOND;
			FD_ZERO( &read_fds );
			FD_SET( socket->fd, &read_fds );
			bytes_in = select( socket->fd + 1,
				&read_fds, NULL, NULL, &timeout );
		}

		if ( bytes_in == 0 )
			result = OS_STATUS_TIMED_OUT;
		else if ( bytes_in != SOCKET_ERROR )
		{
			bytes_in = recv( socket->fd, buf, len, 0 );
			if ( bytes_in != SOCKET_ERROR )
			{
				if ( bytes_read )
				{
					if ( WSAGetLastError() == WSA_IO_PENDING )
						*bytes_read = len;
					else
						*bytes_read = (size_t)bytes_in;
				}
				result = OS_STATUS_SUCCESS;
			}
			else if ( WSAGetLastError() == WSAECONNABORTED )
				result = OS_STATUS_TIMED_OUT;
			else if ( WSAGetLastError() == WSAECONNRESET )
				result = OS_STATUS_TRY_AGAIN;
		}
	}
	return result;
}

ssize_t os_socket_receive(
	const os_socket_t *socket,
	void *buf,
	size_t len,
	char *src_addr,
	size_t src_addr_len,
	os_uint16_t *port,
	os_millisecond_t max_time_out )
{
	ssize_t result = 0;
	if ( socket && socket->fd != OS_SOCKET_INVALID && max_time_out > 0u )
	{
		struct timeval ts;
		fd_set rfds;

		ts.tv_sec = max_time_out / OS_MILLISECONDS_IN_SECOND;
		ts.tv_usec = ( max_time_out % OS_MILLISECONDS_IN_SECOND ) *
			OS_MICROSECONDS_IN_MILLISECOND;

		FD_ZERO( &rfds );
		FD_SET( socket->fd, &rfds );
		result = (ssize_t)select( socket->fd + 1,
			&rfds, NULL, NULL, &ts );
		if ( result == (ssize_t)SOCKET_ERROR )
			result = 0;
	}
	if ( result >= 0 )
	{
		struct sockaddr peer_addr;
		socklen_t peer_addr_len = sizeof( struct sockaddr );
		result = recvfrom( socket->fd, (char *)buf, len, 0, &peer_addr,
			&peer_addr_len );
		if ( result >= 0 && ( src_addr || port ) )
		{
			if ( peer_addr.sa_family == AF_INET )
			{
				struct sockaddr_in *sa =
					(struct sockaddr_in *)&peer_addr;
				if ( src_addr )
					inet_ntop( AF_INET, &(sa->sin_addr),
						src_addr, src_addr_len );
				if ( port )
					*port = ntohs( sa->sin_port );
			}
			else if ( peer_addr.sa_family == AF_INET6 )
			{
				struct sockaddr_in6 *sa =
					(struct sockaddr_in6 *)&peer_addr;
				if ( src_addr )
					inet_ntop( AF_INET6, &(sa->sin6_addr),
						src_addr, src_addr_len );
				if ( port )
					*port = ntohs( sa->sin6_port );
			}
		}
	}
	return result;
}

ssize_t os_socket_send(
	const os_socket_t *socket,
	const void *buf,
	size_t len,
	const char *dest_addr,
	os_uint16_t port,
	os_millisecond_t max_time_out )
{
	ssize_t result = -1;
	if( socket && socket->fd != OS_SOCKET_INVALID && dest_addr )
	{
		result = 0;
		if ( max_time_out > 0u )
		{
			DWORD tv = (DWORD)( max_time_out / 2u );
			result = setsockopt( socket->fd, SOL_SOCKET, SO_SNDTIMEO,
				(const char *)&tv, sizeof( struct timeval ) );
		}
		if ( result >= 0 )
		{
			struct sockaddr addr;
			struct sockaddr_in  *const addr4 =
				(struct sockaddr_in *)&addr;
			struct sockaddr_in6 *const addr6 =
				(struct sockaddr_in6 *)&addr;
			ZeroMemory( &addr, sizeof( struct sockaddr ) );
			result = -1;
			if ( inet_pton( AF_INET, dest_addr,
				&(addr4->sin_addr) ) == 1 )
			{
				addr4->sin_family = AF_INET;
				addr4->sin_port = (in_port_t)htons( port );
				result = 0;
			}
			else if ( inet_pton( AF_INET6, dest_addr,
				&(addr6->sin6_addr) ) == 1 )
			{
				addr6->sin6_family = AF_INET6;
				addr6->sin6_port = (in_port_t)htons( port );
				result = 0;
			}
			if ( result >= 0 )
			{
				result = sendto( socket->fd, (const char *)buf,
					len, 0,
					(struct sockaddr*)&addr,
					sizeof( struct sockaddr ) );
			}
		}
	}
	return result;
}

os_status_t os_socket_terminate( void )
{
	WSACleanup();
	return OS_STATUS_SUCCESS;
}

os_status_t os_socket_write(
	const os_socket_t *socket,
	const void *buf,
	size_t len,
	size_t *bytes_written,
	os_millisecond_t max_time_out )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( bytes_written )
		*bytes_written = 0u;
	if ( socket && socket->fd != OS_SOCKET_INVALID )
	{
		ssize_t retval = 0;
		result = OS_STATUS_FAILURE;
		if ( max_time_out > 0u )
		{
			DWORD tv = (DWORD)( max_time_out / 2u );
			result = setsockopt( socket->fd, SOL_SOCKET, SO_SNDTIMEO,
				(const char *)&tv, sizeof( struct timeval ) );
		}
		if ( retval >= 0 )
		{
			DWORD bytes_out;
			bytes_out = send( socket->fd, buf, len, 0 );
			if ( bytes_out != SOCKET_ERROR && bytes_out <= len )
			{
				if ( bytes_written )
					*bytes_written = (size_t)bytes_out;
				result = OS_STATUS_SUCCESS;
			}
			else
			{
				if ( WSAGetLastError() == WSA_IO_PENDING )
				{
					if ( bytes_written )
						*bytes_written = len;
					result = OS_STATUS_SUCCESS;
				}
				else if ( WSAGetLastError() == WSAECONNABORTED ||
					WSAGetLastError() == WSAETIMEDOUT )
					result = OS_STATUS_TIMED_OUT;
			}
		}
	}
	return result;
}

os_status_t os_stream_echo_set(
	os_file_t stream, os_bool_t enable )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( stream )
	{
		int mode;
		result = OS_STATUS_FAILURE;
		if ( GetConsoleMode( stream, &mode ) != 0 )
		{
			if ( enable )
				mode |= ENABLE_ECHO_INPUT;
			else
				mode &= ~ENABLE_ECHO_INPUT;

			if ( SetConsoleMode( stream, mode ) != 0 )
				result = OS_STATUS_SUCCESS;
		}
	}
	return result;
}

/* operating system specific */

const char *os_system_error_string(
	int error_number )
{
	static char ERROR_BUF[ 256u ];
	char *result = NULL;

	if ( error_number == -1 )
		error_number = (int)GetLastError();

	if ( FormatMessageA( FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS, NULL, error_number, 0,
		ERROR_BUF, 256u, NULL ) != 0 )
	{
		char *p = ERROR_BUF;
		ERROR_BUF[ 255u ] = '\0';

		/* remove new line added to end of system message */
		while ( *p != '\0' && *p != '\n' && *p != '\r' )
			++p;
		*p = '\0';

		result = ERROR_BUF;
	}
	return result;
}

os_status_t os_system_info(
	os_system_info_t *sys_info )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( sys_info )
	{
		BOOL found_version = FALSE;
		BOOL is_server = FALSE;
		SYSTEM_INFO system_info;
		const char *const system_name = "windows";
		char *system_platform = NULL;
		const char *const vendor_name = "Microsoft";
		struct win_version_info *cur_version;
		char service_pack[ 20 ];
		WORD os_major = 0, os_minor = 0, os_build = 0;

#if _MSC_VER <= 1700
		OSVERSIONINFOEXA os_version;
		ZeroMemory( &os_version, sizeof( os_version ) );
		os_version.dwOSVersionInfoSize = sizeof( os_version );
		GetVersionExA( (LPOSVERSIONINFO)&os_version );
		os_major = (WORD)os_version.dwMajorVersion;
		os_minor = (WORD)os_version.dwMinorVersion;
		os_build = (WORD)os_version.dwBuildNumber;
		is_server = os_version.wProductType != VER_NT_WORKSTATION;
		os_strncpy( service_pack, os_version.szCSDVersion, 20 );
#else
		DWORD version_size = GetFileVersionInfoSize( "kernel32", NULL );
		if ( version_size > 0u )
		{
			BYTE *version_info = (BYTE*)HeapAlloc( GetProcessHeap(),
				0, sizeof( BYTE ) * version_size );
			if ( version_info && GetFileVersionInfo( "kernel32", 0,
				version_size, version_info ) )
			{
				VS_FIXEDFILEINFO *file_info = NULL;
				UINT file_info_len = 0u;
				if ( VerQueryValue( version_info, TEXT("\\"),
					(LPVOID*)&file_info, &file_info_len ) )
				{
					os_major = HIWORD(file_info->dwFileVersionMS);
					os_minor = LOWORD(file_info->dwFileVersionMS);
					os_build = HIWORD(file_info->dwFileVersionLS);
				}
			}
			if ( version_info )
				HeapFree( GetProcessHeap(), 0, version_info );
		}
		is_server = IsWindowsServer();
		*service_pack = '\0';
#endif

		/* Determine system version */
		cur_version = &WIN_VERS[0];
		while ( !found_version && cur_version->major > 0 )
		{
			found_version = ( cur_version->major < os_major || (
				cur_version->major == os_major &&
				cur_version->minor <= os_minor ) );
#if _MSC_VER > 1700
			/* find service pack version */
			if ( found_version )
			{
				WORD i = 1u;
				while ( IsWindowsVersionOrGreater(
					cur_version->major, cur_version->minor,
					i ) ) ++i;
				--i;
				if ( i > 0u )
					os_snprintf( service_pack, 128,
						"Service Pack %hu", i );
			}
#endif
			if ( found_version && cur_version->metric > 0 )
			{
				found_version =
					GetSystemMetrics( cur_version->metric ) != 0;
			}

			if ( !found_version )
				++cur_version;
		}

		/* Get System Information */
		ZeroMemory( sys_info, sizeof( os_system_info_t ) );
		ZeroMemory( &system_info, sizeof( system_info ) );
		GetSystemInfo( &system_info );

		if ( system_info.wProcessorArchitecture ==
			PROCESSOR_ARCHITECTURE_AMD64 )
			system_platform = "x64 (AMD or Intel)";
		else if ( system_info.wProcessorArchitecture ==
			PROCESSOR_ARCHITECTURE_ARM )
			system_platform = "arm";
		else if ( system_info.wProcessorArchitecture ==
			PROCESSOR_ARCHITECTURE_IA64 )
			system_platform = "ia64 (Intel Itanium-based)";
		else if ( system_info.wProcessorArchitecture ==
			PROCESSOR_ARCHITECTURE_INTEL )
			system_platform = "x86";

		if ( service_pack != NULL && *service_pack != '\0' )
			os_strncpy( sys_info->system_release,
				service_pack,
				OS_SYSTEM_INFO_MAX_LEN );

		os_strncpy( sys_info->vendor_name, vendor_name,
			OS_SYSTEM_INFO_MAX_LEN );
		os_strncpy( sys_info->system_name, system_name,
			OS_SYSTEM_INFO_MAX_LEN );
		os_strncpy( sys_info->system_platform, system_platform,
			OS_SYSTEM_INFO_MAX_LEN );
		if ( is_server && cur_version->server )
			os_strncpy( sys_info->system_version,
				cur_version->server,
				OS_SYSTEM_INFO_MAX_LEN );
		else
			os_strncpy( sys_info->system_version,
				cur_version->desktop,
				OS_SYSTEM_INFO_MAX_LEN );

		result = OS_STATUS_SUCCESS;
	}
	return result;
}

os_uint32_t os_system_pid( void )
{
	return (os_uint32_t)GetCurrentProcessId();
}

os_status_t os_system_run(
	const char *command,
	int *exit_status,
	os_file_t pipe_files[2u] )
{
	os_status_t result = OS_STATUS_NOT_EXECUTABLE;
	SECURITY_ATTRIBUTES secure_attr;
	os_timestamp_t start_time;
	DWORD cmd_result = -1;
	PROCESS_INFORMATION proc_info;
	STARTUPINFO start_info;
	/* do not inheritHandles, so that the new process can run separately */
	BOOL inheritHandles = FALSE;
	char comspec_path[PATH_MAX + 1u];
	char command_with_comspec[PATH_MAX + 1u];

	os_time( &start_time, NULL );

	ZeroMemory( &secure_attr, sizeof( SECURITY_ATTRIBUTES ) );
	secure_attr.nLength = sizeof( SECURITY_ATTRIBUTES );
	secure_attr.bInheritHandle = TRUE;
	secure_attr.lpSecurityDescriptor = NULL;

	if ( exit_status )
		*exit_status = -1;

	/* create process */
	ZeroMemory( &proc_info, sizeof( PROCESS_INFORMATION ) );
	ZeroMemory( &start_info, sizeof( STARTUPINFO ) );
	start_info.cb = sizeof( STARTUPINFO );

	/* in order to pipe the stdout/stderr from the new
	 * process back to the parent process, the
	 * CreateProcess's bInheritHandles need to be TRUE.
	 * If the parent is killed, the child process will be
	 * terminated prematurely.
	 *
	 * We test this by checking if either output buffer
	 * contains a destination to write too
	 */
	inheritHandles = TRUE;
	start_info.hStdError = pipe_files[1];
	start_info.hStdOutput = pipe_files[0];
	start_info.hStdInput = NULL;
	start_info.dwFlags |= STARTF_USESTDHANDLES;

	/* add script prefix.  if the command to
	  *be run is dos internal
	 * it needs to be loaded from cmd.exe */
	os_env_get( "COMSPEC", comspec_path, PATH_MAX );
	if (comspec_path[0] != '\0')
		os_snprintf( command_with_comspec,
			PATH_MAX,
			"\"%s\" /C \"%s\"",
			comspec_path,
			command
			);
	else
		os_strncpy( command_with_comspec,
			command,
			PATH_MAX );

	if (CreateProcess(NULL, (LPTSTR)command_with_comspec, NULL, NULL,
		inheritHandles, DETACHED_PROCESS, NULL, NULL, &start_info, &proc_info))
	{
		result = OS_STATUS_INVOKED;

		CloseHandle( proc_info.hProcess );
		CloseHandle( proc_info.hThread );
	}
	return result;
}

os_status_t os_system_run_wait(
	const char *command,
	int *exit_status,
	char *out_buf[2u],
	size_t out_len[2u],
	os_millisecond_t max_time_out )
{
	HANDLE child_read[2u];
	HANDLE child_write[2u];
	size_t i;
	os_status_t result = OS_STATUS_SUCCESS;
	SECURITY_ATTRIBUTES secure_attr;
	os_timestamp_t start_time;

	os_time( &start_time, NULL );

	ZeroMemory( &secure_attr, sizeof( SECURITY_ATTRIBUTES ) );
	secure_attr.nLength = sizeof( SECURITY_ATTRIBUTES );
	secure_attr.bInheritHandle = TRUE;
	secure_attr.lpSecurityDescriptor = NULL;

	if ( exit_status )
		*exit_status = -1;

	for ( i = 0u; i < 2u && result == OS_STATUS_SUCCESS; ++i )
		if ( ! CreatePipe( &child_read[i], &child_write[i], &secure_attr, 0 ) ||
			! SetHandleInformation( child_read[i], HANDLE_FLAG_INHERIT, 0 ) )
			result = OS_STATUS_IO_ERROR;

	if ( result == OS_STATUS_SUCCESS )
	{
		DWORD cmd_result = -1;
		PROCESS_INFORMATION proc_info;
		STARTUPINFO start_info;
		/* do not inheritHandles, so that the new process can run separately */
		BOOL inheritHandles = FALSE;
		char comspec_path[ PATH_MAX + 1u ];
		char command_with_comspec[ PATH_MAX + 1u ];

		/* create process */
		ZeroMemory( &proc_info, sizeof( PROCESS_INFORMATION ) );
		ZeroMemory( &start_info, sizeof( STARTUPINFO ) );
		start_info.cb = sizeof( STARTUPINFO );

		/* in order to pipe the stdout/stderr from the new
		 * process back to the parent process, the
		 * CreateProcess's bInheritHandles need to be TRUE.
		 * If the parent is killed, the child process will be
		 * terminated prematurely.
		 *
		 * We test this by checking if either output buffer
		 * contains a destination to write too
		 */
		inheritHandles = TRUE;
		start_info.hStdError = child_write[1];
		start_info.hStdOutput = child_write[0];
		start_info.hStdInput = NULL;
		start_info.dwFlags |= STARTF_USESTDHANDLES;

		result = OS_STATUS_NOT_EXECUTABLE;

		/* add script prefix.  if the command to
		  *be run is dos internal
		 * it needs to be loaded from cmd.exe */
		os_env_get( "COMSPEC", comspec_path, PATH_MAX );
		if (comspec_path[0] != '\0')
			os_snprintf( command_with_comspec,
				PATH_MAX,
				"\"%s\" /C \"%s\"",
				comspec_path,
				command
				);
		else
			os_strncpy( command_with_comspec,
				command,
				PATH_MAX );

		if (CreateProcess(NULL, (LPTSTR)command_with_comspec, NULL, NULL,
			inheritHandles, DETACHED_PROCESS, NULL, NULL, &start_info, &proc_info))
		{
			os_millisecond_t time_elapsed;
			cmd_result = STILL_ACTIVE;
			do {
				GetExitCodeProcess( proc_info.hProcess,
					&cmd_result );
				os_time_elapsed( &start_time, &time_elapsed );
				os_time_sleep( LOOP_WAIT_TIME, OS_FALSE );
			} while ( cmd_result == STILL_ACTIVE &&
				( max_time_out == 0 || time_elapsed < max_time_out ) );

			if ( cmd_result == STILL_ACTIVE )
			{
				TerminateProcess( proc_info.hProcess, -1 );
				result = OS_STATUS_TIMED_OUT;
			}
			else
				result = OS_STATUS_SUCCESS;
			GetExitCodeProcess( proc_info.hProcess,
				&cmd_result );

			CloseHandle( proc_info.hProcess );
			CloseHandle( proc_info.hThread );
		}

		CloseHandle( child_write[0] );
		CloseHandle( child_write[1] );
		/* read stdout and stderr output */
		for ( i = 0u; i < 2u; ++i )
		{
			if ( out_buf[i] && out_len[i] > 1u )
			{
				DWORD amount_read = 0;
				out_buf[i][0] = '\0';
				ReadFile( child_read[i], out_buf[i],
					out_len[i] - 1u, &amount_read, NULL );
				if ( amount_read >= 0 )
					out_buf[i][amount_read] = '\0';
			}
		}
		if ( exit_status )
			*exit_status = cmd_result;
	}
	return result;
}

os_status_t os_system_shutdown(
	os_bool_t reboot, unsigned int delay )
{
	char cmd[ PATH_MAX ];
	os_file_t pipes[2] = { NULL, NULL };
	if ( reboot == OS_FALSE )
		os_snprintf( cmd, PATH_MAX, "%s %d", "shutdown /s /t ",
			delay * SECONDS_IN_MINUTE );
	else
		os_snprintf( cmd, PATH_MAX, "%s %d", "shutdown /r /t ",
			delay * SECONDS_IN_MINUTE );

	return os_system_run( cmd, NULL, pipes );
}

os_bool_t os_terminal_vt100_support(
	os_file_t stream
)
{
	os_bool_t result = OS_FALSE;
#ifdef ENABLE_VIRTUAL_TERMINAL_PROCESSING
	DWORD mode = 0;
	if ( GetConsoleMode( stream, &mode ) == TRUE &&
		( mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING ) )
		result = OS_TRUE;
#endif /* ifdef ENABLE_VIRTUAL_TERMINAL_PROCESSING */
	return result;
}

os_status_t os_terminate_handler(
	os_sighandler_t signal_handler )
{
	const BOOL add_handler = (signal_handler != NULL);
	os_status_t result = OS_STATUS_FAILURE;

	TERMINATE_HANDLER = signal_handler;
	if ( SetConsoleCtrlHandler( os_on_terminate, add_handler ) )
		result = OS_STATUS_SUCCESS;
	return result;
}

double os_random(
	double min,
	double max
)
{
	double result = min;
	HCRYPTPROV prov;
	BOOL crypto_ok = FALSE;

	if ( CryptAcquireContext( &prov, NULL, NULL, PROV_RSA_FULL, 0 ) )
		crypto_ok = TRUE;
	else
		/* Note: CryptAcquireContext can fail if there is no/bad keyset.
		 * Check the error code and retry */
		if ( GetLastError() == NTE_BAD_KEYSET )
			if ( CryptAcquireContext( &prov, NULL, NULL,
				PROV_RSA_FULL, CRYPT_NEWKEYSET ) )
				crypto_ok = TRUE;

	/* if success */
	if ( crypto_ok != FALSE )
	{
		uint64_t random_num = 0u;
		if ( CryptGenRandom( prov, sizeof( uint64_t ),
			(BYTE *)&random_num ) )
		{
			/* random double value between 0.0 - 1.0 */
			result = (double)random_num / ULLONG_MAX;
			result = min + result * ( max - min );
		}
		CryptReleaseContext( prov, 0 );
	}
	return result;
}

/* time functions */
os_status_t os_time(
	os_timestamp_t *time_stamp,
	os_bool_t *up_time )
{
/**
 * @brief Number of seconds between windows epoch (1601-01-01T00:00:00Z and
 * UNIX/Linux epoch 1970-01-01T00:00:00Z).
 */
#define SEC_TO_UNIX_EPOCH 11644473600LL
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( time_stamp )
	{
		SYSTEMTIME st;
		/* contains the number of 100 ns intervals since
		 * 1601-01-01T00:00:00Z */
		FILETIME ft;
		ULONGLONG time_calc;

		GetSystemTime( &st );
		result = OS_STATUS_FAILURE;
		if ( SystemTimeToFileTime( &st, &ft ) )
		{
			time_calc = (((ULONGLONG)ft.dwHighDateTime) << 32) +
				ft.dwLowDateTime;
			/* convert from 100 ns intervals to milliseconds */
			time_calc /= 10000;
			time_calc -= SEC_TO_UNIX_EPOCH *
				OS_MILLISECONDS_IN_SECOND;
			*time_stamp = time_calc;
			result = OS_STATUS_SUCCESS;
		}
	}
	if ( up_time )
		*up_time = OS_FALSE;
	return result;
}

os_status_t os_time_format(
	char *buf,
	size_t len )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( buf )
	{
		SYSTEMTIME st;
		result = OS_STATUS_FAILURE;
		ZeroMemory( buf, len );
		GetSystemTime( &st );
		if ( GetDateFormatA( LOCALE_SYSTEM_DEFAULT,
			0, &st, "MMM dd ", buf, len ) != 0 )
		{
			if ( GetTimeFormatA( LOCALE_SYSTEM_DEFAULT,
				0, &st, "HH':'mm':'ss",
				buf + os_strlen( buf ),
				len - os_strlen( buf ) ) != 0 )
				result = OS_STATUS_SUCCESS;
		}
	}
	return result;
}

os_status_t os_time_sleep(
	os_millisecond_t ms,
	os_bool_t allow_interrupts )
{
	os_status_t result = OS_STATUS_FAILURE;
	if ( SleepEx( ms, ( allow_interrupts != OS_FALSE ) ) == 0 )
		result = OS_STATUS_SUCCESS;
	return result;
}

os_status_t os_time_stamp_to_datetime( os_date_time_t* date_time,
	os_timestamp_t time_stamp )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if (date_time != NULL)
	{
		os_timestamp_t time_stamp_copy = time_stamp;
		os_uint32_t years;
		os_uint32_t year;
		os_uint32_t month;
		date_time->millisecond = time_stamp_copy % OS_MILLISECONDS_IN_SECOND;
		time_stamp_copy /= 1000;
		date_time->second = time_stamp_copy % OS_SECONDS_IN_MINUTE;
		time_stamp_copy /= OS_SECONDS_IN_MINUTE;
		date_time->minute = time_stamp_copy % OS_MINUTES_IN_HOUR;
		time_stamp_copy /= OS_MINUTES_IN_HOUR;
		date_time->hour = time_stamp_copy % OS_HOURS_IN_DAY;
		time_stamp_copy /= OS_HOURS_IN_DAY;

		years = (os_uint32_t)(time_stamp_copy / ((365 * 4) + 1) * 4);
		time_stamp_copy %= (365 * 4) + 1;

		year = 3;
		while (time_stamp_copy < DAYS[year][0])
			year--;

		month = 11;
		while (time_stamp_copy < DAYS[year][month])
			month--;

		/* the beginning year for epoch time is 1970 */
		date_time->year = years + year + 1970;
		date_time->month = month + 1;
		date_time->day = (os_uint8_t)( time_stamp_copy -
			DAYS[year][month] + 1 );

		result = OS_STATUS_SUCCESS;
	}
	return result;
}

/* threads & lock support */
#ifndef NO_THREAD_SUPPORT
os_status_t os_thread_condition_broadcast(
	os_thread_condition_t *cond )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( cond )
	{
		WakeAllConditionVariable( cond );
		result = OS_STATUS_SUCCESS;
	}
	return result;
}

os_status_t os_thread_condition_create(
	os_thread_condition_t *cond )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( cond )
	{
		InitializeConditionVariable( cond );
		result = OS_STATUS_SUCCESS;
	}
	return result;
}

os_status_t os_thread_condition_destroy(
	os_thread_condition_t *cond )
{
	return OS_STATUS_SUCCESS;
}

os_status_t os_thread_condition_signal(
	os_thread_condition_t *cond,
	os_thread_mutex_t *lock )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( cond && lock )
	{
		WakeConditionVariable( cond );
		result = OS_STATUS_SUCCESS;
	}
	return result;
}

os_status_t os_thread_condition_timed_wait(
	os_thread_condition_t *cond,
	os_thread_mutex_t *lock,
	os_millisecond_t max_time_out )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( cond && lock )
	{
		result = OS_STATUS_FAILURE;
		if ( max_time_out > 0u )
		{
			if ( SleepConditionVariableCS( cond, lock, max_time_out ) > 0 )
				result = OS_STATUS_SUCCESS;
			else if ( GetLastError() == ERROR_TIMEOUT )
				result = OS_STATUS_TIMED_OUT;
		}
		else
			if ( SleepConditionVariableCS( cond, lock, INFINITE ) > 0 )
				result = OS_STATUS_SUCCESS;
	}
	return result;
}

os_status_t os_thread_create(
	os_thread_t *thread,
	os_thread_main_t main,
	void *arg )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( thread && main )
	{
		result = OS_STATUS_FAILURE;
		if ( ( *thread = CreateThread( NULL, 0, main, arg, 0, NULL ) )
			!= NULL )
			result = OS_STATUS_SUCCESS;
	}
	return result;
}

os_status_t os_thread_destroy(
	os_thread_t *thread )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( thread )
	{
		result = OS_STATUS_FAILURE;
		if ( TerminateThread( *thread, 0 ) )
			result = OS_STATUS_SUCCESS;
	}
	return result;
}

os_status_t os_thread_wait(
	os_thread_t *thread )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( thread )
	{
		WaitForSingleObject( *thread, INFINITE );
		result = OS_STATUS_SUCCESS;
	}
	return result;
}

os_status_t os_thread_mutex_create(
	os_thread_mutex_t *lock )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( lock )
	{
		InitializeCriticalSection( lock );
		result = OS_STATUS_SUCCESS;
	}
	return result;
}

os_status_t os_thread_mutex_lock(
	os_thread_mutex_t *lock )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( lock )
	{
		EnterCriticalSection( lock );
		result = OS_STATUS_SUCCESS;
	}
	return result;
}

os_status_t os_thread_mutex_unlock(
	os_thread_mutex_t *lock )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( lock )
	{
		LeaveCriticalSection( lock );
		result = OS_STATUS_SUCCESS;
	}
	return result;
}

os_status_t os_thread_mutex_destroy(
	os_thread_mutex_t *lock )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( lock )
	{
		DeleteCriticalSection( lock );
		result = OS_STATUS_SUCCESS;
	}
	return OS_STATUS_SUCCESS;
}

os_status_t os_thread_rwlock_create(
	os_thread_rwlock_t *lock )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( lock )
	{
		InitializeSRWLock( lock );
		result = OS_STATUS_SUCCESS;
	}
	return result;
}

os_status_t os_thread_rwlock_read_lock(
	os_thread_rwlock_t *lock )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( lock )
	{
		AcquireSRWLockShared( lock );
		result = OS_STATUS_SUCCESS;
	}
	return result;
}

os_status_t os_thread_rwlock_read_unlock(
	os_thread_rwlock_t *lock )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( lock )
	{
		ReleaseSRWLockShared( lock );
		result = OS_STATUS_SUCCESS;
	}
	return result;
}

os_status_t os_thread_rwlock_write_lock(
	os_thread_rwlock_t *lock )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( lock )
	{
		AcquireSRWLockExclusive( lock );
		result = OS_STATUS_SUCCESS;
	}
	return result;
}

os_status_t os_thread_rwlock_write_unlock(
	os_thread_rwlock_t *lock )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( lock )
	{
		ReleaseSRWLockExclusive( lock );
		result = OS_STATUS_SUCCESS;
	}
	return result;
}

os_status_t os_thread_rwlock_destroy(
	os_thread_rwlock_t *lock )
{
	return OS_STATUS_SUCCESS;
}

#endif /* ifndef NO_THREAD_SUPPORT */

/* uuid support */
os_status_t os_uuid_generate(
	os_uuid_t *uuid )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( uuid )
	{
		result = OS_STATUS_FAILURE;
		if ( UuidCreate( uuid ) == RPC_S_OK )
			result = OS_STATUS_SUCCESS;
	}
	return result;
}
os_status_t os_uuid_to_string_lower(
	os_uuid_t *uuid,
	char *dest,
	size_t len )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( uuid && dest )
	{
		RPC_CSTR rpc_str = NULL;
		result = OS_STATUS_NO_MEMORY;
		if ( len >= 37u &&
			UuidToString( uuid, &rpc_str ) == RPC_S_OK && rpc_str )
		{
			size_t i;
			for ( i = 0u; i < len - 1u &&
				rpc_str[ i ] != '\0'; ++i )
				dest[ i ] = rpc_str[ i ];
			dest[ i ] = '\0';
			RpcStringFree( &rpc_str );
			result = OS_STATUS_SUCCESS;
		}
	}
	return result;
}

char *os_string_tolower(
	char *out,
	const char *in,
	size_t len )
{
	if ( in && out )
	{
		size_t i;
		for ( i = 0; i < len && in[i] != '\0'; i++ )
			out[i] = os_char_tolower( in[i] );
	}
	return out;
}

char *os_string_toupper(
	char *out,
	const char *in,
	size_t len )
{
	if ( in && out )
	{
		size_t i;
		for ( i = 0; i < len && in[i] != '\0'; i++ )
			out[i] = os_char_toupper( in[i] );
	}
	return out;
}
