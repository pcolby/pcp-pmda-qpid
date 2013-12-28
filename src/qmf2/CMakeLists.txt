cmake_minimum_required (VERSION 2.6)

project(QpidPmda)

add_executable(pmdaqpid-qmf2 QpidPmda.cpp)

find_package(Boost COMPONENTS program_options REQUIRED)

target_link_libraries(
    pmdaqpid-qmf2
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
    pcp
    pcp_pmda
)
