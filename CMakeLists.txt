project( SnoreGrowl )
cmake_minimum_required( VERSION 2.6 )

include(GenerateExportHeader)

find_package(Threads REQUIRED)

if(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS                  "${CMAKE_CXX_FLAGS}  -Wall -Wextra -Wnon-virtual-dtor -fno-strict-aliasing")
  set(CMAKE_CXX_FLAGS_RELEASE          "-O2 ${CMAKE_CXX_FLAGS}")
  set(CMAKE_CXX_FLAGS_RELWITHDEBUGINFO "-g -O2 ${CMAKE_CXX_FLAGS}")
  set(CMAKE_CXX_FLAGS_DEBUG            "-g -ggdb -fno-reorder-blocks -fno-schedule-insns -fno-inline ${CMAKE_CXX_FLAGS}")
  set(CMAKE_CXX_FLAGS_DEBUGFULL        "-g3 ${CMAKE_CXX_FLAGS_DEBUG}")
endif(CMAKE_COMPILER_IS_GNUCXX)


add_subdirectory(src)
