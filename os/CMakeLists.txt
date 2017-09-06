#
# Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.
#

# Definitions for build
#######################
if ( OSAL_WRAP )
	add_definitions( "-DOSAL_WRAP=1" ) # true (use defines)
endif( OSAL_WRAP )

# Setup correct header files
############################
if( WIN32 )
	set( C_HDRS "os_win.h" )
	set( C_SRCS "os.c" "os_win.c" "os_win_private.h"  )
	set( C_LIBS version Iphlpapi Rpcrt4 Shlwapi Ws2_32 )
elseif( ANDROID )
	set( C_HDRS "os_posix.h" )
	set( C_SRCS "os.c" "os_android.c" "os_posix.c" "os_posix_private.h" )
elseif( VXWORKS )
	set( C_HDRS "os_posix.h" )
	set( C_SRCS "os.c" "os_vxworks.c" "os_posix.c" "os_posix_private.h" )
else()
	set( C_HDRS "os_posix.h" )
	set( C_SRCS "os.c" "os_linux.c" "os_posix.c" "os_posix_private.h" )
	set( C_LIBS dl )
endif()

# Thread support
################
if ( OSAL_THREAD_SUPPORT )
	find_package( Threads )
	if ( THREADS_FOUND )
		add_definitions( "-DOSAL_THREAD_SUPPORT=1" ) # true (thread support)
		set( C_LIBS ${C_LIBS} ${CMAKE_THREAD_LIBS_INIT} )
	endif()
endif()

# Generate header file
set( IN_FILE "os.h.in" )
set( OUT_FILE "${CMAKE_INCLUDE_OUTPUT_DIRECTORY}/os.h" )
get_directory_property( DEFS
	DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" COMPILE_DEFINITIONS )
set( INTERNAL_VARS "PROJECT_VERSION" "PROJECT_VENDOR" )
foreach( _def ${INTERNAL_VARS} )
	set( ADDITIONAL_DEFS ${ADDITIONAL_DEFS} "-D${_def}:STRING=\"${${_def}}\"" )
endforeach( _def )
add_custom_command( OUTPUT "${OUT_FILE}"
	COMMAND "${CMAKE_COMMAND}" -DHDR_FILES:STRING="${C_HDRS}"
		-DIN_FILE:PATH="${IN_FILE}" -DOUT_FILE:PATH="${OUT_FILE}"
		-DDEFS:STRING="${DEFS}"
		${ADDITIONAL_DEFS}
		 "-P" "GenerateHeader.cmake"
	MAIN_DEPENDENCY "${IN_FILE}"
	DEPENDS ${C_HDRS} "GenerateHeader.cmake"
	WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
	COMMENT "Generating ${OUT_FILE}"
)
set_source_files_properties( "${OUT_FILE}" PROPERTIES GENERATED TRUE )

# Set location to find generated "os.h" file in source files
include_directories( "${CMAKE_INCLUDE_OUTPUT_DIRECTORY}" )

# UUID Support
##############
add_subdirectory( "uuid" )
if ( NOT WIN32 )
	set( UUID_OBJECTS "$<TARGET_OBJECTS:uuid>" )
endif()

# Static Library
################
# On Windows, .lib files are for both Shared and Static libraries. For clarity,
# no renaming of this target is done; the library will exist as libosalStatic
# and osalStatic.lib on POSIX and Windows, respectively, so as to be consistent.
add_library( "${TARGET}${TARGET_STATIC_SUFFIX}" STATIC
	${C_HDRS} ${CXX_HDRS} "${OUT_FILE}"
	${C_SRCS} ${CXX_SRCS}
	${UUID_OBJECTS}
)
target_link_libraries( "${TARGET}${TARGET_STATIC_SUFFIX}" ${C_LIBS} )
target_compile_definitions( "${TARGET}${TARGET_STATIC_SUFFIX}" PRIVATE OSAL_STATIC=1 )

# Shared Library
################
add_library( "${TARGET}${TARGET_SHARED_SUFFIX}" SHARED
	${C_HDRS} ${CXX_HDRS} "${OUT_FILE}"
	${C_SRCS} ${CXX_SRCS}
	${UUID_OBJECTS}
)
set_target_properties(
	"${TARGET}${TARGET_STATIC_SUFFIX}" "${TARGET}${TARGET_SHARED_SUFFIX}"
	PROPERTIES
		OUTPUT_NAME ${TARGET}
		SOVERSION ${PACKAGE_VERSION}
		VERSION ${PACKAGE_VERSION}
)
target_link_libraries( "${TARGET}${TARGET_SHARED_SUFFIX}" ${C_LIBS} )

# Installation Rules
####################
set( LIB_COMPONENT "api" )
if ( WIN32 )
	set( LIB_COMPONENT "libs" )
endif( WIN32 )

install( TARGETS "${TARGET}${TARGET_STATIC_SUFFIX}" "${TARGET}${TARGET_SHARED_SUFFIX}"
	RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT api
	ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT libs
	LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT ${LIB_COMPONENT}
	PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}" COMPONENT headers
)

install( FILES "${OUT_FILE}"
	DESTINATION "${INSTALL_INCLUDE_DIR}" COMPONENT headers
)
