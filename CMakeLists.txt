cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(neon-diff)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/src/cmake")

include_directories(src)

include(CTest)
include(Catch)
add_subdirectory(src/test)

add_executable(${PROJECT_NAME}
	"src/colors.cpp"
	"src/diffparser.cpp"
	"src/neonapp.cpp"
	"src/main.cpp")

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
