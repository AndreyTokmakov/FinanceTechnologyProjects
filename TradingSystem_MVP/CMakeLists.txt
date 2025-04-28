cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

project(TradingSystem_MVP)
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
set(PROJECT_VERSION 1.0.0.0)
project(${PROJECT_NAME} VERSION ${PROJECT_VERSION} LANGUAGES CXX)

include(FetchContent)
set(CMAKE_CXX_STANDARD 26)

add_compile_options(-c -Wall -Wextra -O3 -std=c++26)

message(STATUS "[dependency] fetching Boost")
set(BOOST_ENABLE_PYTHON OFF)
set(BOOST_IOSTREAMS_ENABLE_ZLIB ON)
FetchContent_Declare(boost
        GIT_REPOSITORY https://github.com/boostorg/boost.git
        GIT_TAG boost-1.85.0
        GIT_SHALLOW ON
        GIT_PROGRESS ON
        GIT_SUBMODULES ${BOOST_REQD_SUBMODULES}
        CMAKE_ARGS  -DBOOST_IOSTREAMS_ENABLE_ZLIB=ON
        OVERRIDE_FIND_PACKAGE
)
FetchContent_MakeAvailable(boost)


set(PROJECT_DIR ${CMAKE_SOURCE_DIR}/${PROJECT_NAME})
set(COMMON ${PROJECT_DIR}/common)
set(ENGINE ${PROJECT_DIR}/engine)
set(CONNECTORS ${PROJECT_DIR}/connectors)
set(MARKET_DATA ${PROJECT_DIR}/market_data)

include_directories(${COMMON})
include_directories(${CONNECTORS})
include_directories(${MARKET_DATA})
include_directories(${ENGINE})

# include all components
add_executable(${PROJECT_NAME}
        main.cpp
        ${ENGINE}/Engine.cpp ${ENGINE}/Engine.h
        ${CONNECTORS}/BinanceConnector.cpp
        ${COMMON}/Utilities.cpp
        ${MARKET_DATA}/Parser.cpp ${MARKET_DATA}/Parser.h
        ${MARKET_DATA}/MarketData.cpp ${MARKET_DATA}/MarketData.h
)

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/Utilities")
target_link_directories(${PROJECT_NAME} PUBLIC "${CMAKE_BINARY_DIR}/Utilities")

# SIMD Json
include_directories(${THIRD_PARTY_DIR}/simdjson/include)
target_link_directories(${PROJECT_NAME} PUBLIC ${THIRD_PARTY_DIR}/simdjson/build)

include_directories(${THIRD_PARTY_DIR}/json/include)

TARGET_LINK_LIBRARIES(${PROJECT_NAME}
        utils
        pthread
        simdjson
        # Boost::json
        # Boost::any
        # Boost::url
        # Boost::uuid
        # Boost::thread
        Boost::asio
        Boost::beast
        crypto
        ssl
        ${EXTRA_LIBS}
)