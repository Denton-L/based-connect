project(based-connect)

cmake_minimum_required(VERSION 3.2)
if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

find_path(GETOPT_HEADER NAMES getopt.h REQUIRED)
find_library(GETOPT_LIBRARIES NAMES getopt REQUIRED)
message(WARNING "Found lib ${GETOPT_LIBRARIES}")

include_directories(${GETOPT_HEADER})

add_executable(based-connect
	based.c
	based.h
	bluetooth.c
	bluetooth.h
	main.c
	util.c
	util.h
)
set_property(TARGET based-connect PROPERTY C_STANDARD 99)
target_link_libraries(based-connect ${GETOPT_LIBRARIES})
if (WIN32)
	target_link_libraries(based-connect wsock32 ws2_32)
endif (WIN32)
