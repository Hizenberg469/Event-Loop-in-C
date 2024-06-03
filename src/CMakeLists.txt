set(EVENT_LOOP_SRC "event_loop.c")

add_library(${EVENT_LOOP_LIB} STATIC
					${EVENT_LOOP_SRC})

target_include_directories(${EVENT_LOOP_LIB} PUBLIC
								${HEADER_DIR})

#Linking dependent library...

if( NOT (CMAKE_SYSTEM_NAME STREQUAL "Linux") )
	message(FATAL_ERROR "It's not a Unix-based system.\n \
	POSIX Library will not compile in this project.\n")
endif()
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

target_link_libraries(${EVENT_LOOP_LIB} PUBLIC
							Threads::Threads)
