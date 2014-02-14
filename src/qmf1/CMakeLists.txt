cmake_minimum_required (VERSION 2.6)

project(qpid-qmf1)

add_executable(
    pmda${PROJECT_NAME}
    QpidPmda.cpp
    ConsoleListener.cpp
    ConsoleLogger.cpp
    ConsoleUtils.cpp
)

find_package(Boost COMPONENTS program_options REQUIRED)

target_link_libraries(
    pmda${PROJECT_NAME}
    ${Boost_LIBRARIES}
    pcp
    pcp_pmda
    qmfconsole
    qpidclient
    qpidcommon
)

# Generate a basic PMDA registration script.
file(WRITE Install
    "# Install the ${PROJECT_NAME} PMDA\n\n"
    ". $PCP_DIR/etc/pcp.env\n"
    ". $PCP_SHARE_DIR/lib/pmdaproc.sh\n\n"
    "iam=${PROJECT_NAME}\n"
    "pmns_name=qpid\n\n"
    "pmdaSetup\n"
    "pmdaInstall\n"
    "exit 0\n"
)

# Generate a basic PMDA deregistration script.
file(WRITE Remove
    "# Remove the ${PROJECT_NAME} PMDA\n\n"
    ". $PCP_DIR/etc/pcp.env\n"
    ". $PCP_SHARE_DIR/lib/pmdaproc.sh\n\n"
    "iam=${PROJECT_NAME}\n"
    "pmns_name=qpid\n\n"
    "pmdaSetup\n"
    "pmdaRemove\n"
    "exit 0\n"
)

find_program(PCP_PMCONFIG_EXECUTABLE NAMES pmconfig)
if (PCP_PMCONFIG_EXECUTABLE)
    message(STATUS "Found pmconfig: ${PCP_PMCONFIG_EXECUTABLE}")
    execute_process(
        COMMAND ${PCP_PMCONFIG_EXECUTABLE} PCP_PMDAS_DIR
        OUTPUT_VARIABLE PCP_PMDAS_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if (PCP_PMDAS_DIR)
        string(REGEX REPLACE "^PCP_PMDAS_DIR=" "" PCP_PMDAS_DIR ${PCP_PMDAS_DIR})
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
    # Install the PMDA registration scripts.
    install(
        FILES Install Remove DESTINATION ${PCP_PMDAS_DIR}/${PROJECT_NAME}
        PERMISSIONS
            OWNER_READ OWNER_WRITE OWNER_EXECUTE
            GROUP_READ GROUP_EXECUTE
            WORLD_READ WORLD_EXECUTE
    )
endif (PCP_PMDAS_DIR)

# Enable (and stop on) compiler warnings.
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag(-Wall   HAVE_WALL)
check_cxx_compiler_flag(-Werror HAVE_WERROR)
check_cxx_compiler_flag(-Wextra HAVE_WEXTRA)
if (HAVE_WALL)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
endif()
if (HAVE_WERROR)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
endif()
if (HAVE_WEXTRA)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
endif()
