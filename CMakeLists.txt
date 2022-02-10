CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
PROJECT(exhy)
include_directories(.)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -pthread -lyaml-cpp -rdynamic -ldl -lm -lz")
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/output)  
include_directories("usr/include")

set(LIB_SRC
    exhy/exhy_config.cpp
    exhy/exhy_fiber.cpp
    exhy/exhy_log.cpp
    exhy/exhy_thread.cpp
    exhy/exhy_util.cpp
    exhy/exhy_scheduler.cpp
    exhy/exhy_iomanager.cpp
    exhy/exhy_timer.cpp
    exhy/exhy_fdmanager.cpp
    exhy/exhy_hook.cpp
    exhy/exhy_address.cpp
    exhy/exhy_socket.cpp
    exhy/exhy_bytearray.cpp
    exhy/exhy_tcpserver.cpp
    exhy/exhy_stream.cpp
    exhy/streams/socket_stream.cpp
    exhy/streams/zlib_stream.cpp
    exhy/http/http.cpp
    exhy/exhy_uri.cpp
    exhy/http/http_servlet.cpp
    exhy/http/http_server.cpp
    exhy/http/http_session.cpp
    exhy/http/http_connection.cpp
    exhy/http/httpclient_parser.cpp
    exhy/http/http11_parser.cpp
    exhy/http/http_parser.cpp
    
)
# ragelmaker(exhy/exhy_uri.rl LIB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/exhy)

LINK_LIBRARIES("/usr/local/lib/libyaml-cpp.so")
LINK_LIBRARIES("/usr/local/lib/libz.so.1.2.11")

# Tests
ADD_EXECUTABLE(test_main exhy/main.cpp ${LIB_SRC}) 
ADD_EXECUTABLE(test_fiber tests/test_fiber.cpp ${LIB_SRC}) 
TARGET_LINK_LIBRARIES(test_fiber ${CMAKE_DL_LIBS})
ADD_EXECUTABLE(test_scheduler test_scheduler.cpp ${LIB_SRC}) 
ADD_EXECUTABLE(test_iomanager tests/test_iomanager.cpp ${LIB_SRC}) 
TARGET_LINK_LIBRARIES(test_iomanager ${CMAKE_DL_LIBS})
ADD_EXECUTABLE(test_hook tests/test_hook.cpp ${LIB_SRC}) 
TARGET_LINK_LIBRARIES(test_hook ${CMAKE_DL_LIBS})
ADD_EXECUTABLE(test_address tests/test_address.cpp ${LIB_SRC}) 
TARGET_LINK_LIBRARIES(test_address ${CMAKE_DL_LIBS})
ADD_EXECUTABLE(test_socket tests/test_socket.cpp ${LIB_SRC}) 
TARGET_LINK_LIBRARIES(test_socket ${CMAKE_DL_LIBS})
ADD_EXECUTABLE(test_bytearray tests/test_bytearray.cpp ${LIB_SRC}) 
TARGET_LINK_LIBRARIES(test_bytearray ${CMAKE_DL_LIBS})
ADD_EXECUTABLE(test_http tests/test_http.cpp ${LIB_SRC}) 
TARGET_LINK_LIBRARIES(test_http ${CMAKE_DL_LIBS})
ADD_EXECUTABLE(test_http_parser tests/test_http_parser.cpp ${LIB_SRC}) 
TARGET_LINK_LIBRARIES(test_http_parser ${CMAKE_DL_LIBS})
ADD_EXECUTABLE(test_tcpserver tests/test_tcpserver.cpp ${LIB_SRC}) 
TARGET_LINK_LIBRARIES(test_tcpserver ${CMAKE_DL_LIBS})
ADD_EXECUTABLE(test_http_server tests/test_http_server.cpp ${LIB_SRC}) 
TARGET_LINK_LIBRARIES(test_http_server ${CMAKE_DL_LIBS})
ADD_EXECUTABLE(test_http_connection tests/test_http_connection.cpp ${LIB_SRC}) 
TARGET_LINK_LIBRARIES(test_http_connection ${CMAKE_DL_LIBS})
ADD_EXECUTABLE(test_uri tests/test_uri.cpp ${LIB_SRC}) 
TARGET_LINK_LIBRARIES(test_uri ${CMAKE_DL_LIBS})

ADD_EXECUTABLE(test_data tests/test_data.cpp ${LIB_SRC}) 
TARGET_LINK_LIBRARIES(test_data ${CMAKE_DL_LIBS})
ADD_EXECUTABLE(test_parser tests/test_parser.cpp ${LIB_SRC}) 
TARGET_LINK_LIBRARIES(test_parser ${CMAKE_DL_LIBS})

# Examples
# ADD_EXECUTABLE(echo_server examples/echo_server.cpp ${LIB_SRC}) 
# TARGET_LINK_LIBRARIES(echo_server ${CMAKE_DL_LIBS})
