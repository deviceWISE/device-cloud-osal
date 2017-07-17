function( cat IN_FILE OUT_FILE )
  file( READ ${IN_FILE} CONTENTS )
  file( APPEND ${OUT_FILE} "${CONTENTS}" )
endfunction()

set( HEADER_FILE "os.h")

file( REMOVE "${HEADER_FILE}" )
file( WRITE "${HEADER_FILE}" "" )

cat( "${OSAL_SOURCE_DIR}/header/os_top.h.in" "${HEADER_FILE}" )

if( WIN32 )
	cat( "${OSAL_SOURCE_DIR}/os/os_win.h" "${HEADER_FILE}" )
	if( NOT ${OSAL_WRAP} )
		cat( "${OSAL_SOURCE_DIR}/os/os_win_macros.h" "${HEADER_FILE}" )
	endif()
else()
	string( TOLOWER "${OSAL_TARGET}" OSAL_TARGET )

	if( "${OSAL_TARGET}" STREQUAL "android" )
		cat( "${OSAL_SOURCE_DIR}/os/os_android.h" "${HEADER_FILE}" )
	elseif( "${OSAL_TARGET}" STREQUAL "vxworks" )
		cat( "${OSAL_SOURCE_DIR}/os/os_vxworks.h" "${HEADER_FILE}" )
	endif()

	cat( "${OSAL_SOURCE_DIR}/os/os_posix.h" "${HEADER_FILE}" )
	if( NOT ${OSAL_WRAP} )
		cat( "${OSAL_SOURCE_DIR}/os/os_posix_macros.h" "${HEADER_FILE}" )
	endif()
endif()

if( ${OSAL_WRAP} )
	cat( "${OSAL_SOURCE_DIR}/header/os_wrap.h.in" "${HEADER_FILE}" )
else()
	cat( "${OSAL_SOURCE_DIR}/header/os_bot.h.in" "${HEADER_FILE}" )
endif()
