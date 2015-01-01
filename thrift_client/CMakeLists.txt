cmake_minimum_required (VERSION 2.8)
project( thrift_cllient )
#aux_source_directory( ./gun_cpp GUNCPP )
AUX_SOURCE_DIRECTORY(./ DIR_SRCS)

message( ${DIR_SRCS}) 
AUX_SOURCE_DIRECTORY(./gen-cpp  DIR_gencpp_SRCS)
message( ${DIR_gencpp_SRCS} ) 

#add_subdirectory( gen-cpp )
set( SRCS ${DIR_SRCS}  ${DIR_gencpp_SRCS} )

INCLUDE_DIRECTORIES( /home/wangxin/wangxin_test/thrift_wx/include  )
LINK_DIRECTORIES(   /usr/local/lib/  ${PROJECT_SOURCE_DIR}/bin  /home/wangxin/wangxin_test/thrift_wx/lib   )

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)


ADD_DEFINITIONS(-g)

add_executable(  thrift_client ${SRCS} )
TARGET_LINK_LIBRARIES( thrift_client  pthread dl boost_system   thrift  thriftnb thriftasio event boost_thread  )
