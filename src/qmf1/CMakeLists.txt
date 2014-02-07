cmake_minimum_required (VERSION 2.6)

project(qpid-qmf1)

add_executable(pmda${PROJECT_NAME} QpidPmda.cpp ConsoleListener.cpp)

find_package(Boost COMPONENTS program_options regex REQUIRED)

target_link_libraries(
    pmda${PROJECT_NAME}
    ${Boost_LIBRARIES}
    pcp
    pcp_pmda
    qmfconsole
    qpidclient
    qpidcommon
)

find_program(PCP_PMCONFIG_EXECUTABLE NAMES pmconfig)
if (PCP_PMCONFIG_EXECUTABLE)
    message(STATUS "Found pmconfig: ${PCP_PMCONFIG_EXECUTABLE}")
    execute_process(
        COMMAND ${PCP_PMCONFIG_EXECUTABLE}
        OUTPUT_VARIABLE PCP_PMCONFIG_OUTPUT
    )
    string(REGEX MATCH "PCP_PMDAS_DIR=[^\n]*" PCP_PMDA_DIR ${PCP_PMCONFIG_OUTPUT})
    if (PCP_PMDA_DIR)
        string(SUBSTRING ${PCP_PMDA_DIR} 14 -1 PCP_PMDA_DIR)
        message(STATUS "Found PMDAs directory: ${PCP_PMDA_DIR}")
    endif (PCP_PMDA_DIR)
endif (PCP_PMCONFIG_EXECUTABLE)

if (PCP_PMDA_DIR)
    install(
        TARGETS pmda${PROJECT_NAME} DESTINATION ${PCP_PMDA_DIR}/${PROJECT_NAME}
    )
endif (PCP_PMDA_DIR)
