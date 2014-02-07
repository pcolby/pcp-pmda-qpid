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
    string(REGEX MATCH "PCP_PMDAS_DIR=[^\n]*" PCP_PMDAS_DIR ${PCP_PMCONFIG_OUTPUT})
    if (PCP_PMDAS_DIR)
        string(SUBSTRING ${PCP_PMDAS_DIR} 14 -1 PCP_PMDAS_DIR)
        message(STATUS "Found PMDAs directory: ${PCP_PMDAS_DIR}")
    endif (PCP_PMDAS_DIR)
endif (PCP_PMCONFIG_EXECUTABLE)

if (PCP_PMDAS_DIR)
    # Install the PMDA binary.
    install(
        TARGETS pmda${PROJECT_NAME} DESTINATION ${PCP_PMDAS_DIR}/${PROJECT_NAME}
    )
    # Export the PMDA's support files (domain, help, pmns, etc).
    install(
        CODE "execute_process(
            COMMAND ${PCP_PMDAS_DIR}/${PROJECT_NAME}/pmda${PROJECT_NAME} --export-all
            WORKING_DIRECTORY ${PCP_PMDAS_DIR}/${PROJECT_NAME}
        )"
    )
    # Install a basic PMDA registration script.
    install(
        CODE "file(WRITE ${PCP_PMDAS_DIR}/${PROJECT_NAME}/Install
            \"# Install the ${PROJECT_NAME} PMDA \n\n\"
            \". $PCP_DIR/etc/pcp.env\n\"
            \". $PCP_SHARE_DIR/lib/pmdaproc.sh\n\n\"
            \"iam=${PROJECT_NAME}\n\"
            \"pmns_name=qpid\n\n\"
            \"pmdaSetup\n\"
            \"pmdaInstall\n\"
            \"exit 0\n\")"
    )
    # Install a basic PMDA deregistration script.
    install(
        CODE "file(WRITE ${PCP_PMDAS_DIR}/${PROJECT_NAME}/RemoveNew
            \"# Remove the ${PROJECT_NAME} PMDA \n\n\"
            \". $PCP_DIR/etc/pcp.env\n\"
            \". $PCP_SHARE_DIR/lib/pmdaproc.sh\n\n\"
            \"iam=${PROJECT_NAME}\n\"
            \"pmns_name=qpid\n\n\"
            \"pmdaSetup\n\"
            \"pmdaRemove\n\"
            \"exit 0\n\")"
    )
endif (PCP_PMDAS_DIR)
