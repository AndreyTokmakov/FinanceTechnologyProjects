cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

project(TradingCore)
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
set(PROJECT_VERSION 1.0.0.0)
project(${PROJECT_NAME} VERSION ${PROJECT_VERSION} LANGUAGES CXX)

include(FetchContent)
set(CMAKE_CXX_STANDARD 26)

add_compile_options(-c -Wall -Wextra -O3 -std=c++26)

#[[
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
#]]

set(PROJECT_DIR ${CMAKE_SOURCE_DIR}/${PROJECT_NAME})
set(SOURCES  ${PROJECT_DIR}/src)
set(TESTS    ${PROJECT_DIR}/tests)

set(CORE        ${SOURCES}/core)
set(MARKET_DATA ${SOURCES}/market_data)
set(EXECUTION   ${SOURCES}/execution)
set(EXCHANGES   ${SOURCES}/exchanges)
set(STRATEGY    ${SOURCES}/strategy)
set(BINANCE     ${EXCHANGES}/binance)

include_directories(${CORE})
include_directories(${MARKET_DATA})
include_directories(${EXECUTION})
include_directories(${STRATEGY})

# include all components
add_executable(${PROJECT_NAME}
        main.cpp
        ${CORE}/price.hpp
        ${CORE}/quantity.hpp
        ${CORE}/types.hpp
        ${CORE}/instrument.hpp
        ${CORE}/timestamp.hpp

        ${MARKET_DATA}/book_level.hpp
        ${MARKET_DATA}/book_update.hpp
        ${MARKET_DATA}/market_event.hpp
        ${MARKET_DATA}/market_event_handler.hpp
        ${MARKET_DATA}/market_data_parser.hpp
        ${MARKET_DATA}/market_data_source.hpp
        ${MARKET_DATA}/market_data_message_handler.cpp
        ${MARKET_DATA}/market_data_message_handler.hpp
        ${MARKET_DATA}/order_book.hpp
        ${MARKET_DATA}/order_book.cpp
        ${MARKET_DATA}/book_update_handler.hpp
        ${MARKET_DATA}/book_builder.hpp
        ${MARKET_DATA}/book_builder.cpp

        ${EXECUTION}/order.hpp
        ${EXECUTION}/execution_gateway.hpp
        ${EXECUTION}/execution_report.hpp
        ${EXECUTION}/order_manager.hpp
        ${EXECUTION}/order_manager.cpp

        ${BINANCE}/binance_market_data_source.hpp
        ${BINANCE}/binance_market_data_source.cpp
        ${BINANCE}/binance_market_data_parser.cpp
        ${BINANCE}/binance_market_data_parser.hpp

        ${STRATEGY}/strategy.hpp
        ${STRATEGY}/imbalance_strategy.hpp
        ${STRATEGY}/imbalance_strategy.cpp

        ${TESTS}/market_data/order_book_test.cpp
        ${TESTS}/market_data/book_builder_test.cpp
        ${TESTS}/execution/order_manager_test.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/Utilities")
target_link_directories(${PROJECT_NAME} PUBLIC "${CMAKE_BINARY_DIR}/Utilities")

#[[
# SIMD Json
include_directories(${THIRD_PARTY_DIR}/simdjson/include)
target_link_directories(${PROJECT_NAME} PUBLIC ${THIRD_PARTY_DIR}/simdjson/build)

include_directories(${THIRD_PARTY_DIR}/json/include)
#]]


TARGET_LINK_LIBRARIES(${PROJECT_NAME}
        utils
        pthread
        # simdjson
        # Boost::json
        # Boost::any
        # Boost::url
        # Boost::uuid
        # Boost::thread
        # Boost::asio
        # Boost::beast
        # crypto
        # ssl
        ${EXTRA_LIBS}
)