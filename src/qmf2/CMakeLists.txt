cmake_minimum_required (VERSION 2.6 FATAL_ERROR)

project(QpidPmda)

add_executable(pmdaqpid QpidPmda.cpp)

# A quick hack to reference a local pcp-pmda-cpp checkout (https://github.com/pcolby/pcp-pmda-cpp).
include_directories ("${PROJECT_SOURCE_DIR}/../../../cpp/include")

target_link_libraries(
    pmdaqpid
    boost_program_options
    pcp
    pcp_pmda
)
