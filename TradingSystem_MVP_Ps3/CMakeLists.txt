cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

project(TradingSystem_MVP_Ps3)
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -pthread")
set(PROJECT_VERSION 1.0.0.0)
project(${PROJECT_NAME} VERSION ${PROJECT_VERSION} LANGUAGES CXX)

add_compile_options(-c -Wall -Wextra -Wno-interference-size -O3 -std=c++26)

set(PROJECT_DIR  ${CMAKE_SOURCE_DIR}/${PROJECT_NAME})
set(COMMON       ${PROJECT_DIR}/common)
set(PRICE_ENGINE ${PROJECT_DIR}/price_engine)
set(PARSER       ${PROJECT_DIR}/parser)
set(MARKET_DATA  ${PROJECT_DIR}/market_data)
set(CONNECTORS   ${PROJECT_DIR}/connectors)
set(UTILS        ${PROJECT_DIR}/utils)

include_directories(${COMMON})
include_directories(${CONNECTORS})
include_directories(${MARKET_DATA})
include_directories(${PARSER})
include_directories(${PRICE_ENGINE})
include_directories(${UTILS})

link_directories(${THIRD_PARTY_DIR}/openssl)
include_directories(${THIRD_PARTY_DIR}/openssl/include)

# Nlohmann JSON
include_directories(${THIRD_PARTY_DIR}/json/include)

# IXWebSocket
include_directories(${THIRD_PARTY_DIR}/IXWebSocket)
link_directories(${THIRD_PARTY_DIR}/IXWebSocket/build)

message(${THIRD_PARTY_DIR}/IXWebSocket/build)

# include all components
add_executable(${PROJECT_NAME}
        ${COMMON}/Buffer.hpp common/Buffer.cpp
        ${COMMON}/FlatMap.hpp
        ${COMMON}/RingBuffer.hpp
        ${COMMON}/Common.hpp
        ${COMMON}/Types.hpp
        ${MARKET_DATA}/MarketData.hpp
        ${MARKET_DATA}/Formatting.hpp
        ${MARKET_DATA}/BinanceDataParser.cpp
        ${PRICE_ENGINE}/ExchangeDataProcessor.cpp
        ${PRICE_ENGINE}/MarketStateManager.cpp
        ${PARSER}/Parser.cpp
        ${CONNECTORS}/Connector.cpp ${CONNECTORS}/Connector.hpp
        ${UTILS}/Utils.cpp
        tests/Tests.cpp tests/Tests.hpp
        main.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/Utilities")
target_link_directories(${PROJECT_NAME} PUBLIC "${CMAKE_BINARY_DIR}/Utilities")

target_link_libraries(${PROJECT_NAME}
        utils
        pthread
        crypto
        ssl
        ixwebsocket
        z
        ${EXTRA_LIBS}
)

target_compile_definitions(${PROJECT_NAME} PUBLIC
        TEST_DATA_PATH="${DATA_PATH}"
)