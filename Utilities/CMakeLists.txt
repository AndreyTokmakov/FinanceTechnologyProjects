cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 26)

project(Utilities)
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -pthread")
set(PROJECT_VERSION 1.0.0.0)
project(${PROJECT_NAME} VERSION ${PROJECT_VERSION} LANGUAGES CXX)

add_compile_options(-c -Wall -Wextra -O3 -std=c++2c)

# include all components
add_executable(${PROJECT_NAME}
        main.cpp
        Base64.cpp Base64.h
        FileUtilities.cpp FileUtilities.h
        StringUtilities.cpp StringUtilities.h
        HexConverter.cpp HexConverter.h
        PerfUtilities.cpp PerfUtilities.h
)

message(STATUS "EXTRA_LIBS: ${EXTRA_LIBS}")

TARGET_LINK_LIBRARIES(${PROJECT_NAME}
        pthread
        ${EXTRA_LIBS}
)