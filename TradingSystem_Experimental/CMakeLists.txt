cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

project(TradingSystem_Experimental)
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -pthread")
set(PROJECT_VERSION 1.0.0.0)
project(${PROJECT_NAME} VERSION ${PROJECT_VERSION} LANGUAGES CXX)

add_compile_options(-c -Wall -Wextra -O3 -std=c++2c)

set(PROJECT_DIR ${CMAKE_SOURCE_DIR}/${PROJECT_NAME})

set(COMMON ${PROJECT_DIR}/common)
# set(GATEWAY ${PROJECT_DIR}/gateway)
set(GATEWAY_EXPERIMENTAL ${PROJECT_DIR}/gateway_experimental)
set(ORDER_BOOK ${PROJECT_DIR}/order_book)
set(PARSER ${PROJECT_DIR}/parser)
set(TESTS ${PROJECT_DIR}/tests)
# set(SERVICE ${PROJECT_DIR}/service)
set(SERVICE_EXPERIMENTAL ${PROJECT_DIR}/service_experiments)

# include_directories(${PROJECT_DIR})
include_directories(${COMMON})
include_directories(${GATEWAY})
include_directories(${GATEWAY_EXPERIMENTAL})
include_directories(${PARSER})
include_directories(${ORDER_BOOK})
include_directories(${TESTS})
include_directories(${SERVICE})
include_directories(${SERVICE_EXPERIMENTAL})


# include all components
add_executable(${PROJECT_NAME}
        main.cpp
        ${ORDER_BOOK}/OrderBook.cpp
        ${ORDER_BOOK}/Manager.cpp
        # ${GATEWAY}/UDSServer.cpp
        ${GATEWAY_EXPERIMENTAL}/MarketDataGatewayUDP.cpp
        ${GATEWAY_EXPERIMENTAL}/MarketDataGatewayUDS.cpp
        ${GATEWAY_EXPERIMENTAL}/UDSServer.cpp
        ${COMMON}/Utils.cpp
        ${COMMON}/Event.h
        ${COMMON}/Order.cpp
        ${PARSER}/BinanceEventsParser.cpp
        ${SERVICE_EXPERIMENTAL}/Service_Demo_One.cpp
        ${SERVICE_EXPERIMENTAL}/Service_Demo_Two.cpp
        ${TESTS}/UnitTests.cpp
        ${TESTS}/Module_Impl_Test.cpp
        ${TESTS}/Buffer_Tests.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/Utilities")
target_link_directories(${PROJECT_NAME} PUBLIC "${CMAKE_BINARY_DIR}/Utilities")

include_directories(${THIRD_PARTY_DIR}/json/include)

TARGET_LINK_LIBRARIES(${PROJECT_NAME}
        utils
        pthread
        ${EXTRA_LIBS}
)