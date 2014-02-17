cmake_minimum_required(VERSION 2.6)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

project(pmdaqpid)

# Add a pmdaqpid target.
add_executable(
    ${PROJECT_NAME}
    ${PROJECT_NAME}.cpp
    QpidLogger.cpp
)

find_package(Boost COMPONENTS program_options REQUIRED)

target_link_libraries(
    ${PROJECT_NAME}
    ${Boost_LIBRARIES}
    pcp
    pcp_pmda
)

# Detect which QMF libraries are available.
find_library(HAVE_QMF1 qmfconsole)
find_library(HAVE_QMF2 qmf2)
if (HAVE_QMF1 OR HAVE_QMF2)
    message(STATUS "Found the following QMF libraries:")
    if (HAVE_QMF1)
        message(STATUS "  QMFv1 (${HAVE_QMF1})")
    endif (HAVE_QMF1)
    if (HAVE_QMF2)
        message(STATUS "  QMFv2 (${HAVE_QMF2})")
    endif (HAVE_QMF2)
else ()
    message(FATAL_ERROR "Unable to find any QMF libraries.")
endif ()

# If QMF1, include the QMF1 source.
if (HAVE_QMF1)
    # add #define
    add_library(
        ${PROJECT_NAME}-qmf1 STATIC
        qmf1/QpidPmda.cpp
        qmf1/ConsoleListener.cpp
        qmf1/ConsoleLogger.cpp
        qmf1/ConsoleUtils.cpp
    )
    target_link_libraries(
        ${PROJECT_NAME}
        ${PROJECT_NAME}-qmf1
        qmfconsole
        qpidclient
        qpidcommon
    )
endif ()

# If QMF2, include the QMF2 source.
if (HAVE_QMF2)
    message(STATUS "QMF2 support not implemented yet.")
endif ()
