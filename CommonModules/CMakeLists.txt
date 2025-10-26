cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

project(CommonModules)
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -pthread")
set(PROJECT_VERSION 1.0.0.0)
project(${PROJECT_NAME} VERSION ${PROJECT_VERSION} LANGUAGES CXX)

add_compile_options(-c -Wall -Wextra -O3 -std=c++26)

add_library(common_modules
    queues/LockFreeQueue.cpp
    logging/LowLatencyLogger.cpp
    logging/LowLatencyLoggerDebug.cpp
    ring_buffer/RingBuffer_SPSC.cpp
    ring_buffer_ex/RingBufferEx.cpp
    locking/SpinLock.cpp
    cryptography/Certificates.cpp
    consumers/UnixDomainSocketConsumer.cpp
    buffer/Buffer.cpp buffer/Buffer.hpp
)

target_include_directories(common_modules PUBLIC "${CMAKE_SOURCE_DIR}/Utilities")
target_link_directories(common_modules PUBLIC "${CMAKE_BINARY_DIR}/Utilities")

# include all components
add_executable(${PROJECT_NAME}
        main.cpp
        ring_buffer_ex/RingBufferEx.cpp
        ring_buffer_ex/RingBufferEx.h
)

target_link_libraries(${PROJECT_NAME}
        common_modules
        utils
        pthread
        crypto
        ssl
        ${EXTRA_LIBS}
)