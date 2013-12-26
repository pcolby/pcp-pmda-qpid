cmake_minimum_required (VERSION 2.6)

project(QpidPmda)

add_executable(pmdaqpid-qmf1 QpidPmda.cpp ConsoleListener.cpp)

# A quick hack to reference a local pcp-pmda-cpp checkout (https://github.com/pcolby/pcp-pmda-cpp).
include_directories ("${PROJECT_SOURCE_DIR}/../../../cpp/include")

target_link_libraries(
    pmdaqpid-qmf1
    boost_program_options
    pcp
    pcp_pmda
    qmfconsole
    qpidcommon
)
