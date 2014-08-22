include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})


if( WIN32 )
    link_libraries( ws2_32 )
endif( WIN32 )


set( GROWL_NTP_C_SRC    growl.c
                        tcp.c
                        md5.c )

add_library( growl SHARED  ${GROWL_NTP_C_SRC} )
target_link_libraries( growl ${CMAKE_THREAD_LIBS_INIT})

generate_export_header(growl EXPORT_FILE_NAME growl_exports.h EXPORT_MACRO_NAME GROWL_EXPORT)

add_library( growl++ SHARED growl.cpp )
target_link_libraries ( growl++  growl)

generate_export_header(growl++ EXPORT_FILE_NAME growl_exports.hpp EXPORT_MACRO_NAME GROWL_CPP_EXPORT)

add_executable( gntp-send gntp-send.c )
target_link_libraries ( gntp-send growl)

add_executable( gntp-send++ gntp-send.cpp )
target_link_libraries ( gntp-send++ growl growl++)

install(TARGETS growl growl++ RUNTIME DESTINATION bin
                              LIBRARY DESTINATION lib
                              ARCHIVE DESTINATION lib)

install(FILES growl.h growl.hpp growl_exports.h growl_exports.hpp DESTINATION include/gntp)

