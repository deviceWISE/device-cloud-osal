
if( NOT WIN32 )
set( TARGET "uuid" )
set( C_SRCS
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

set( C_HDRS
	"all-io.h"
	"c.h"
	"randutils.h"
	"uuidd.h"
	"uuid.h"
	"uuidP.h"
)
get_full_path( C_HDRS ${C_HDRS} )
get_full_path( C_SRCS ${C_SRCS} )

#add_library( ${TARGET} STATIC
#       ${C_SRCS}
#       ${C_HDRS}
#)

# FIXME
# override the cflags, since libuuid is opensource and not optimized
#set(UUID_CMAKE_C_FLAGS "-DHAVE_ERR_H=1 -DHAVE_USLEEP=1 -DHAVE_SYS_FILE_H -fPIC")
#set_source_files_properties(gen_uuid.c PROPERTIES COMPILE_FLAGS "" )
#set_source_files_properties(gen_uuid.c PROPERTIES COMPILE_DEFINES "${UUID_CMAKE_FLAGS}" )

set( API_HDRS_C ${API_HDRS_C} ${C_HDRS} PARENT_SCOPE )
set( API_HDRS_CXX ${API_HDRS_CXX} ${CXX_HDRS} PARENT_SCOPE )
set( API_SRCS_C ${API_SRCS_C} ${C_SRCS} PARENT_SCOPE )
set( API_SRCS_CXX ${API_SRCS_CXX} ${CXX_SRCS} PARENT_SCOPE )
set( API_INCS ${API_INCS} PARENT_SCOPE )
endif()
