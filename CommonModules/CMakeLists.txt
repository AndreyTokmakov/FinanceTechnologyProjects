cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

project(CommonModules)
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -pthread")
set(PROJECT_VERSION 1.0.0.0)
project(${PROJECT_NAME} VERSION ${PROJECT_VERSION} LANGUAGES CXX)

add_compile_options(-c -Wall -Wextra -O3 -std=c++2c)

add_library(common_modules
    queues/LockFreeQueue.cpp
)

# include all components
add_executable(${PROJECT_NAME}
        main.cpp
)

target_link_libraries(${PROJECT_NAME}
        common_modules
        pthread
        ${EXTRA_LIBS}
)