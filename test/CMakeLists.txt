set(EVENT_LOOP_APP evloop_app)
set(EV_LOOP_APP_SRC "evloop_app.c")

add_executable(${EVENT_LOOP_APP} 
					${EV_LOOP_APP_SRC})

target_link_libraries(${EVENT_LOOP_APP} PUBLIC
							${EVENT_LOOP_LIB})

target_link_directories(${EVENT_LOOP_APP} PUBLIC
							${HEADER_DIR})