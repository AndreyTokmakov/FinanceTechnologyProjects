cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

project(MatchingEngineService)
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -pthread")
set(PROJECT_VERSION 1.0.0.0)
project(${PROJECT_NAME} VERSION ${PROJECT_VERSION} LANGUAGES CXX)

add_compile_options(-c -Wall -Wextra -O3 -std=c++2c)

set(PROJECT_DIR ${CMAKE_SOURCE_DIR}/MatchingEngineService)

set(COMMON ${PROJECT_DIR}/common)
set(CONSUMERS ${PROJECT_DIR}/consumers)
set(ORDER_BOOK ${PROJECT_DIR}/order_book)
set(TESTS ${PROJECT_DIR}/tests)

include_directories(${COMMON})
# include_directories(${PROJECT_DIR})
# include_directories(${CONSUMERS})
include_directories(${ORDER_BOOK})


# include all components
add_executable(${PROJECT_NAME}
        main.cpp
        ${ORDER_BOOK}/OrderBook.cpp
        ${CONSUMERS}/EventConsumer.cpp
        ${COMMON}/Utils.cpp
        ${COMMON}/Event.h
        ${COMMON}/Order.cpp
        ${TESTS}/UnitTests.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/Utilities")
target_link_directories(${PROJECT_NAME} PUBLIC "${CMAKE_BINARY_DIR}/Utilities")

include_directories(${THIRD_PARTY_DIR}/json/include)

TARGET_LINK_LIBRARIES(${PROJECT_NAME}
        utils
        pthread
        ${EXTRA_LIBS}
)