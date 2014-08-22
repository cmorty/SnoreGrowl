include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

set( GROWL_NTP_C_SRC    growl.c
                        tcp.c
                        md5.c )

add_library( snoregrowl SHARED  ${GROWL_NTP_C_SRC} )
target_link_libraries( snoregrowl PUBLIC ${CMAKE_THREAD_LIBS_INIT})

set_target_properties( snoregrowl PROPERTIES EXPORT_NAME SnoreGrowl)


if(WIN32)
    target_link_libraries( snoregrowl PUBLIC ws2_32)
endif()

generate_export_header(snoregrowl EXPORT_FILE_NAME growl_exports.h EXPORT_MACRO_NAME GROWL_EXPORT)

target_include_directories(snoregrowl PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<INSTALL_INTERFACE:include>
    $<INSTALL_INTERFACE:include/snoregrowl/>
)



add_library( snoregrowl++ SHARED growl.cpp )
target_link_libraries ( snoregrowl++  snoregrowl)
set_target_properties( snoregrowl++ PROPERTIES EXPORT_NAME SnoreGrowl++)

generate_export_header(snoregrowl++ EXPORT_FILE_NAME growl_exports.hpp EXPORT_MACRO_NAME GROWL_CPP_EXPORT)

add_executable( gntp-send gntp-send.c )
target_link_libraries ( gntp-send snoregrowl)

add_executable( gntp-send++ gntp-send.cpp )
target_link_libraries ( gntp-send++ snoregrowl++)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/SnoreGrowlConfig.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/SnoreGrowlConfig.cmake" @ONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/SnoreGrowl++Config.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/SnoreGrowl++Config.cmake" @ONLY)

install(TARGETS snoregrowl  EXPORT SnoreGrowlTarget
                            RUNTIME DESTINATION bin
                            LIBRARY DESTINATION lib
                            ARCHIVE DESTINATION lib)


install(TARGETS snoregrowl++  EXPORT SnoreGrowl++Target
                              RUNTIME DESTINATION bin
                              LIBRARY DESTINATION lib
                              ARCHIVE DESTINATION lib)

install(FILES growl.h growl.hpp ${CMAKE_CURRENT_BINARY_DIR}/growl_exports.h ${CMAKE_CURRENT_BINARY_DIR}/growl_exports.hpp DESTINATION include/snoregrowl)

install(EXPORT SnoreGrowlTarget DESTINATION lib/cmake/SnoreGrowl NAMESPACE Snore::)
install(EXPORT SnoreGrowl++Target DESTINATION lib/cmake/SnoreGrowl++ NAMESPACE Snore::)


install(FILES "${CMAKE_CURRENT_BINARY_DIR}/SnoreGrowlConfig.cmake" DESTINATION lib/cmake/SnoreGrowl)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/SnoreGrowl++Config.cmake" DESTINATION lib/cmake/SnoreGrowl++)
