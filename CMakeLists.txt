cmake_minimum_required(VERSION 3.22.1)

project(CobaltAsioExample)
add_executable(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(Boost_USE_STATIC_LIBS ON)

find_package(Boost 1.84.0 COMPONENTS cobalt REQUIRED)

target_sources(${PROJECT_NAME} PUBLIC main.cpp)
target_link_libraries(${PROJECT_NAME} PUBLIC Boost::cobalt)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
 