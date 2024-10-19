set(NETWORK_UTILS	"network_utils.c")
set(RT				"rt.c")
set(STP				"stp.c")
set(UDP_CLIENT		"udp_client.c")
set(UTILS			"utils.c")
set(STP_EL			"stp_el.c")
set(TIMER_LIB		"timerlib.c")

add_executable(stp_exe
						${STP}
						${RT}
						${NETWORK_UTILS}
						${UTILS}
						${STP_EL}
						${TIMER_LIB})

add_executable(udp_client
						${UDP_CLIENT}
						${NETWORK_UTILS}
						${UTILS})


#Linking dependent library...

if( NOT (CMAKE_SYSTEM_NAME STREQUAL "Linux") )
	message(FATAL_ERROR "It's not a Unix-based system.\n \
	POSIX Library will not compile in this project.\n")
endif()
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Find the rt library
find_library(RT_LIB rt)

#Below snippet is important for POSIX timer library in WSL2...
target_compile_definitions(stp_exe PRIVATE _POSIX_C_SOURCE=199309L)

# Check if the rt library is found
if (RT_LIB)
    message(STATUS "librt found: ${RT_LIB}")
else()
    message(FATAL_ERROR "librt not found")
endif()


target_link_libraries(stp_exe PUBLIC
										Threads::Threads
										${RT_LIB}
										${EVENT_LOOP_LIB})

target_link_libraries(udp_client PUBLIC
										Threads::Threads
										${RT_LIB})

target_include_directories(stp_exe PUBLIC
										${HEADER_DIR})