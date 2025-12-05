cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

project(TradingSystem_MVP_Ps1)
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -pthread")
set(PROJECT_VERSION 1.0.0.0)
project(${PROJECT_NAME} VERSION ${PROJECT_VERSION} LANGUAGES CXX)

add_compile_options(-c -Wall -Wextra -Wno-interference-size -O3 -std=c++26)

set(PROJECT_DIR ${CMAKE_SOURCE_DIR}/${PROJECT_NAME})
set(COMMON ${PROJECT_DIR}/common)
set(MARKET_DATA ${PROJECT_DIR}/market_data)

include_directories(${COMMON})
include_directories(${MARKET_DATA})

# include all components
add_executable(${PROJECT_NAME}
        ${COMMON}/Buffer.hpp common/Buffer.cpp
        ${COMMON}/FlatMap.hpp
        ${COMMON}/RingBuffer.hpp
        ${COMMON}/Common.hpp
        ${COMMON}/Types.hpp
        ${MARKET_DATA}/MarketData.hpp
        ${MARKET_DATA}/Parser.cpp market_data/Parser.hpp
        price_engine/MarketDepthBook.hpp
        main.cpp
)

# Nlohmann JSON
include_directories(${THIRD_PARTY_DIR}/json/include)

# SIND JSON
include_directories(${THIRD_PARTY_DIR}/simdjson/include)
target_link_directories(${PROJECT_NAME} PUBLIC ${THIRD_PARTY_DIR}/simdjson/build)

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/Utilities")
target_link_directories(${PROJECT_NAME} PUBLIC "${CMAKE_BINARY_DIR}/Utilities")

target_link_libraries(${PROJECT_NAME}
        utils
        pthread
        ${EXTRA_LIBS}
)