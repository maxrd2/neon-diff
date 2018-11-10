cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(neon-diff)

add_executable(${PROJECT_NAME}
	"src/neonapp.cpp"
	"src/main.cpp")

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
