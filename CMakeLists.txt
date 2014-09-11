project( SnoreGrowl )
cmake_minimum_required( VERSION 2.6 )

include(GenerateExportHeader)


set(SNOREGROWL_VERSION_MAJOR 0)
set(SNOREGROWL_VERSION_MINOR 4)
set(SNOREGROWL_VERSION_PATCH 0)


find_package(Threads REQUIRED)

add_subdirectory(src)
