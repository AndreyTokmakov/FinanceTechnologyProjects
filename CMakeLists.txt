cmake_minimum_required(VERSION 3.21)
project(FinanceTechnologyProjects)

set(CMAKE_CXX_STANDARD 26)

# 5.15 - My home Ubuntu-20.04
if (CMAKE_SYSTEM_VERSION MATCHES "5.15")
    set(THIRD_PARTY_DIR /home/andtokm/DiskS/ProjectsUbuntu/third_party)
    set(EXTRA_LIBS tbb)
else()
    set(THIRD_PARTY_DIR /home/andtokm/Projects/third_party)
endif()

add_subdirectory(DaVinchiTest)
add_subdirectory(MatchingEngine)
add_subdirectory(MatchingEngine_Pulsar)
add_subdirectory(Utilities)
