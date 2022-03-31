cmake_minimum_required(VERSION 3.16)
project(KIS_2022_spring)

set(CMAKE_CXX_STANDARD 17)

set(SERVER_PROJECT_FILES ${CMAKE_CURRENT_SOURCE_DIR}/Server)
set(CLIENT_PROJECT_FILES ${CMAKE_CURRENT_SOURCE_DIR}/Client)
set(PROTOCOL_PROJECT_FILES ${CMAKE_CURRENT_SOURCE_DIR}/Protocol)
set(SERVER_HEADERS ${SERVER_PROJECT_FILES}/Server.hpp ${SERVER_PROJECT_FILES}/Session.hpp ${PROTOCOL_PROJECT_FILES}/ProtocolPackage.hpp)
set(SERVER_SOURCES ${SERVER_PROJECT_FILES}/Server.cpp ${SERVER_PROJECT_FILES}/Session.cpp)
set(CLIENT_HEADERS ${CLIENT_PROJECT_FILES}/Client.hpp ${PROTOCOL_PROJECT_FILES}/ProtocolPackage.hpp)
set(CLIENT_SOURCES ${CLIENT_PROJECT_FILES}/Client.cpp)

find_package(Boost 1.42.0 REQUIRED COMPONENTS system filesystem serialization)
include_directories(${Boost_INCLUDE_DIRS} ${SERVER_PROJECT_FILES} ${CLIENT_PROJECT_FILES} ${PROTOCOL_PROJECT_FILES})
link_directories(${Boost_LIBRARY_DIRS})


add_executable(Client ${CLIENT_PROJECT_FILES}/main.cpp ${CLIENT_HEADERS} ${CLIENT_SOURCES})
add_executable(Server ${SERVER_PROJECT_FILES}/main.cpp ${SERVER_HEADERS} ${SERVER_SOURCES})
target_link_libraries(Client ${Boost_LIBRARIES})
target_link_libraries(Server ${Boost_LIBRARIES})
