cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

project(MatchingEngine)
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -pthread")
set(PROJECT_VERSION 1.0.0.0)
project(${PROJECT_NAME} VERSION ${PROJECT_VERSION} LANGUAGES CXX)

add_compile_options(-c -Wall -Wextra -O3 -std=c++2c)

# include all components
add_executable(${PROJECT_NAME}
        main.cpp
        Order.cpp
        MatchingEngine.cpp
        MatchingEngineEx.cpp
        MatchingEngine_Simple.cpp
        MatchingEngine_PriceLvLPtr.cpp
        MatchingEngine_OrderAsPtr.cpp
        MatchingEngine_OrderAsPtr_Alloc.cpp
)

message("CMAKE_SOURCE_DIR       : ${CMAKE_SOURCE_DIR}")
message("PROJECT_SOURCE_DIR     : ${PROJECT_SOURCE_DIR}")
message("CMAKE_BINARY_DIR       : ${CMAKE_BINARY_DIR}")
message("CMAKE_CURRENT_LIST_DIR : ${CMAKE_CURRENT_LIST_DIR}")

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/Utilities")
target_link_directories(${PROJECT_NAME} PUBLIC "${CMAKE_BINARY_DIR}/Utilities")

TARGET_LINK_LIBRARIES(${PROJECT_NAME}
        utils
        pthread
        ${EXTRA_LIBS}
)