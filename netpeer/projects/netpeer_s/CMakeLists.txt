cmake_minimum_required(VERSION 2.6)
project(netpeer_s)
add_compile_options(-std=c++11)
ADD_DEFINITIONS(-DNP_STATIC)
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ../../../lib/linux/)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ../../../lib/linux/)
FILE(GLOB ALL_HEADER_SRCS ../../include/*.h)
FILE(GLOB ALL_CPP_SRCS ../../src/*.cpp)

include_directories(
../../../3rd/RakNet-master/Source/
../../include
)


add_library(netpeer_s STATIC ${ALL_CPP_SRCS} ${ALL_HEADER_SRCS})

#FIND_LIBRARY(RAKNET_S_LIB RakNetLibStatic ../../../3rd/RakNet-master/Lib/linux/)

#message(STATUS "raknet lib path:" ${RAKNET_S_LIB})

#if(NOT RAKNET_S_LIB)
#message(STATUS "not find the rakent lib" )
#endif(NOT RAKNET_S_LIB)

#target_link_libraries(netpeer_s ${RAKNET_S_LIB}
