#
# Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.
#

if( NOT WIN32 )
# autoconf project
include( CheckIncludeFiles )
function( add_definition_if_include_file VAR_NAME )
	set( FILES "${ARGN}" )
	check_include_files( ${FILES} INCLUDE_FOUND )
	if ( INCLUDE_FOUND )
		add_definitions( "-D${VAR_NAME}=1" )
	endif( INCLUDE_FOUND )
endfunction( add_definition_if_include_file )

include( CheckFunctionExists )
function( add_definition_if_function_exists VAR_NAME )
	set( FUNCS "${ARGN}" )
	check_function_exists( ${FUNCS} FUNC_FOUND )
	if ( FUNC_FOUND )
		add_definitions( "-D${VAR_NAME}=1" )
	endif( FUNC_FOUND )
endfunction( add_definition_if_function_exists )

add_definition_if_function_exists( HAVE_FTRUNCATE "ftruncate" )
add_definition_if_function_exists( HAVE_GETTIMEOFDAY "gettimeofday" )
add_definition_if_function_exists( HAVE_MEMSET "memset" )
add_definition_if_function_exists( HAVE_SOCKET "socket" )
add_definition_if_function_exists( HAVE_SRANDOM "srandom" )
add_definition_if_function_exists( HAVE_USLEEP "usleep" )
add_definition_if_function_exists( HAVE_STRTOUL "strtoul" )

add_definition_if_include_file( HAVE_DLFCN_H "dlfcn.h" )
add_definition_if_include_file( HAVE_ERR_H "err.h" )
add_definition_if_include_file( HAVE_FCNTL_H "fcntl.h" )
add_definition_if_include_file( HAVE_LIMITS_H "limits.h" )
add_definition_if_include_file( HAVE_INTTYPES_H "inttypes.h" )
add_definition_if_include_file( HAVE_MEMORY_H "memory.h" )
add_definition_if_include_file( HAVE_NETINET_IN_H "netinet/in.h" )
add_definition_if_include_file( HAVE_STDINT_H "stdint.h" )
add_definition_if_include_file( HAVE_STDLIB_H "stdlib.h" )
add_definition_if_include_file( HAVE_STRING_H "string.h" )
add_definition_if_include_file( HAVE_STRINGS_H "strings.h" )
add_definition_if_include_file( HAVE_SYS_FILE_H "sys/file.h" )
add_definition_if_include_file( HAVE_SYS_IOCTL_H "sys/ioctl.h" )
add_definition_if_include_file( HAVE_SYS_SOCKET_H "sys/socket.h" )
add_definition_if_include_file( HAVE_SYS_STAT_H "sys/stat.h" )
add_definition_if_include_file( HAVE_SYS_TIME_H "sys/time.h" )
add_definition_if_include_file( HAVE_SYS_TYPES_H "sys/types.h" )
add_definition_if_include_file( HAVE_UNISTD_H "unistd.h" )
add_definition_if_include_file( STDC_HEADERS
	"dlfcn.h"
	"float.h"
	"inttypes.h"
	"stddef.h"
	"stdint.h"
	"stdlib.h"
	"strings.h"
	"string.h" )

include( CheckSymbolExists )
check_symbol_exists( "_SC_HOST_NAME_MAX" "unistd.h" HAVE_SC_HOST_NAME_MAX )
if( HAVE_SC_HOST_NAME_MAX )
	add_definitions( "-DHAVE_DECL__SC_HOST_NAME_MAX=1" )
endif( HAVE_SC_HOST_NAME_MAX )
add_definitions( "-D_DEFAULT_SOURCE=1" )

# Remove some compiler warnings for clang
set_compiler_flag_if_supported(
	"-Wno-conversion"
	"-Wno-reserved-id-macro"
	"-Wno-shorten-64-to-32"
	"-Wno-sign-conversion"
	"-Wno-undef"
	"-Wno-unused-macros" )

set( UUID_C_SRCS
	"clear.c"
	"compare.c"
	"copy.c"
	"gen_uuid.c"
	"isnull.c"
	"pack.c"
	"parse.c"
	"randutils.c"
	"unpack.c"
	"unparse.c"
	"uuid_time.c"
)

set( UUID_C_HDRS
	"all-io.h"
	"c.h"
	"randutils.h"
	"uuidd.h"
	"uuid.h"
	"uuidP.h"
)

add_library( "uuid" OBJECT ${UUID_C_HDRS} ${UUID_C_SRCS} )
add_library( "uuid-unit-test" STATIC ${UUID_C_HDRS} ${UUID_C_SRCS} )

endif() # NOT WIN32
