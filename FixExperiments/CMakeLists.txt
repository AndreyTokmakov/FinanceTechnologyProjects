cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

project(FixExperiments)
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -pthread")
set(PROJECT_VERSION 1.0.0.0)
project(${PROJECT_NAME} VERSION ${PROJECT_VERSION} LANGUAGES CXX)

add_compile_options(-c -Wall -Wextra -O3 -std=c++26)

set(QUICK_FIX_DIR ${THIRD_PARTY_DIR}/quickfix)
set(PROJECT_DIR ${CMAKE_SOURCE_DIR}/${PROJECT_NAME})
set(SERVER ${PROJECT_DIR}/server)
set(CLIENT ${PROJECT_DIR}/client)

include_directories(${QUICK_FIX_DIR}/include)
add_executable(${PROJECT_NAME}
        main.cpp
        ${SERVER}/Server.cpp ${SERVER}/Server.h
        ${SERVER}/Application.cpp  ${SERVER}/Application.h
        ${CLIENT}/Client.cpp ${CLIENT}/Client.h
        ${CLIENT}/Application.cpp  ${CLIENT}/Application.h
)

target_link_directories(${PROJECT_NAME} PUBLIC ${QUICK_FIX_DIR}/lib)
target_link_directories(${PROJECT_NAME} PUBLIC ${QUICK_FIX_DIR}/src/C++/.libs)

TARGET_LINK_LIBRARIES(${PROJECT_NAME}
        utils
        pthread
        quickfix
        ${EXTRA_LIBS}
)