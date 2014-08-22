project( snore-gntp )
cmake_minimum_required( VERSION 2.6 )

include(GenerateExportHeader)

option(WITH_STATIC    "Build static growl library"           OFF)

find_package(Threads REQUIRED)

include_directories ( ./include ${CMAKE_CURRENT_BINARY_DIR} )

if(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS                  "${CMAKE_CXX_FLAGS}  -Wall -Wextra -Wnon-virtual-dtor -fno-strict-aliasing")
  set(CMAKE_CXX_FLAGS_RELEASE          "-O2 ${CMAKE_CXX_FLAGS}")
  set(CMAKE_CXX_FLAGS_RELWITHDEBUGINFO "-g -O2 ${CMAKE_CXX_FLAGS}")
  set(CMAKE_CXX_FLAGS_DEBUG            "-g -ggdb -fno-reorder-blocks -fno-schedule-insns -fno-inline ${CMAKE_CXX_FLAGS}")
  set(CMAKE_CXX_FLAGS_DEBUGFULL        "-g3 ${CMAKE_CXX_FLAGS_DEBUG}")
endif(CMAKE_COMPILER_IS_GNUCXX)

if( WIN32 )
  link_libraries ( ws2_32 )
endif( WIN32 )


set( GROWL_NTP_C_SRC
        src/growl.c
        src/tcp.c
        src/md5.c )

add_library( growl SHARED  ${GROWL_NTP_C_SRC} )
set_target_properties( growl PROPERTIES COMPILE_FLAGS "-DGROWL_DLL" )
target_link_libraries( growl ${CMAKE_THREAD_LIBS_INIT})

generate_export_header(growl
  EXPORT_FILE_NAME growl_exports.h
  EXPORT_MACRO_NAME GROWL_EXPORT
)

if( WITH_STATIC )
  add_library( growl-static  STATIC ${GROWL_NTP_C_SRC} )
  set_target_properties( growl-static PROPERTIES COMPILE_FLAGS "-DGROWL_DLL" )
  target_link_libraries( growl-static ${CMAKE_THREAD_LIBS_INIT})
endif( WITH_STATIC )

add_library( growl++ SHARED src/growl.cpp )
target_link_libraries ( growl++  growl)
set_target_properties( growl++ PROPERTIES COMPILE_FLAGS "-DGROWL_CPP_DLL" )

generate_export_header(growl++
  EXPORT_FILE_NAME growl_exports.hpp
  EXPORT_MACRO_NAME GROWL_CPP_EXPORT
)

if( WITH_STATIC )
add_library( growl++-static STATIC src/growl.cpp )
target_link_libraries ( growl++-static growl)
set_target_properties( growl++-static PROPERTIES COMPILE_FLAGS "-DGROWL_CPP_DLL" )
endif( WITH_STATIC )

add_executable( gntp-send src/gntp-send.c )
target_link_libraries ( gntp-send growl)

add_executable( gntp-send++ src/gntp-send.cpp )
target_link_libraries ( gntp-send++ growl growl++)

install(TARGETS growl growl++ RUNTIME DESTINATION bin
                              LIBRARY DESTINATION lib
                              ARCHIVE DESTINATION lib)

install(FILES include/growl.h include/growl.hpp include/growl_exports.h include/growl_exports.hpp DESTINATION include/gntp)
