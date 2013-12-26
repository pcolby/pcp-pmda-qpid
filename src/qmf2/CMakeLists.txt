cmake_minimum_required (VERSION 2.6)

project(QpidPmda)

add_executable(pmdaqpid-qmf2 QpidPmda.cpp)

target_link_libraries(
    pmdaqpid-qmf2
    boost_program_options
    pcp
    pcp_pmda
)
