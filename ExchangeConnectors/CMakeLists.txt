cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

project(ExchangeConnectors)

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

link_directories(${THIRD_PARTY_DIR}/openssl)
include_directories(${THIRD_PARTY_DIR}/openssl/include)

# Nlohmann JSON
include_directories(${THIRD_PARTY_DIR}/json/include)

include_directories("binance")
include_directories("experiments")

# include all components
add_executable(${PROJECT_NAME}
        main.cpp
        binance/WebSockerConnector.cpp
        binance/WebSockerConnectorAsynch.cpp
        binance/WSConnectorAsynch.cpp
        binance/ApiClient.cpp
        experiments/WSConnectorAsynchExperimental.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/Utilities")
target_link_directories(${PROJECT_NAME} PUBLIC "${CMAKE_BINARY_DIR}/Utilities")

TARGET_LINK_LIBRARIES(${PROJECT_NAME}
        utils
        pthread
        Boost::json
        Boost::asio
        Boost::thread
        Boost::beast
        crypto
        ssl
        ${EXTRA_LIBS}
)