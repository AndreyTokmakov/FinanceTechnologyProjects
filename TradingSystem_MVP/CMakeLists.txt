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

include_directories(${COMMON})

# include all components
add_executable(${PROJECT_NAME}
        main.cpp
        ${COMMON}/Utils.cpp
        ${COMMON}/Event.h
)

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/Utilities")
target_link_directories(${PROJECT_NAME} PUBLIC "${CMAKE_BINARY_DIR}/Utilities")

include_directories(${THIRD_PARTY_DIR}/json/include)

TARGET_LINK_LIBRARIES(${PROJECT_NAME}
        utils
        pthread
        Boost::json
        Boost::any
        Boost::url
        Boost::asio
        Boost::thread
        Boost::uuid
        Boost::beast
        crypto
        ssl
        ${EXTRA_LIBS}
)