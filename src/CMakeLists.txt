set(EVENT_LOOP_SRC "event_loop.c")

add_library(${EVENT_LOOP_LIB} STATIC
					${EVENT_LOOP_SRC})

target_include_directories(${EVENT_LOOP_LIB} PUBLIC
								${HEADER_DIR})