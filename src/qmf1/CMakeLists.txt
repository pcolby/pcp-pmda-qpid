cmake_minimum_required (VERSION 2.6)

project(QpidPmda)

add_executable(pmdaqpid-qmf1 QpidPmda.cpp ConsoleListener.cpp)

find_package(Boost COMPONENTS program_options regex REQUIRED)

target_link_libraries(
    pmdaqpid-qmf1
    ${Boost_LIBRARIES}
    pcp
    pcp_pmda
    qmfconsole
    qpidclient
    qpidcommon
)
