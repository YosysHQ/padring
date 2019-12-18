# 
#     PADRING -- a padring generator for ASICs.
#
#     Copyright (c) 2019, Niels Moseley <niels@symbioticeda.com>
#
#     Permission to use, copy, modify, and/or distribute this software for any
#     purpose with or without fee is hereby granted, provided that the above
#     copyright notice and this permission notice appear in all copies.
#
#     THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
#     WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
#     MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
#     ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
#     WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
#     ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
#     OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#   
#

cmake_minimum_required(VERSION 3.10)
project(padring)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_BUILD_TYPE Debug)

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

##################################################
## DOXYGEN 
##################################################

option(BUILD_DOC "Build documentation" OFF)

# check if Doxygen is installed
find_package(Doxygen)
if (DOXYGEN_FOUND AND BUILD_DOC)
    # set input and output files
    set(DOXYGEN_IN  ${PROJECT_SOURCE_DIR}/doc/Doxyfile.in)
    set(DOXYGEN_OUT ${PROJECT_SOURCE_DIR}/doc/Doxyfile)

    # request to configure the file
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    message("Doxygen build started")

    # note the option ALL which allows to build the docs together with the application
    add_custom_target(doc_doxygen ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        COMMENT "Generating API documentation with Doxygen"
        DEPENDS glnetlist
        VERBATIM )
else (DOXYGEN_FOUND AND BUILD_DOC)
  message("Doxygen need to be installed to generate the doxygen documentation")
endif (DOXYGEN_FOUND AND BUILD_DOC)

##################################################
## PADRING
##################################################s

include_directories(${PROJECT_SOURCE_DIR}/contrib)
set(PADRINGSRC 
    ${PROJECT_SOURCE_DIR}/src/main.cpp    
    ${PROJECT_SOURCE_DIR}/src/logging.cpp
    ${PROJECT_SOURCE_DIR}/src/layout.cpp
    ${PROJECT_SOURCE_DIR}/src/svgwriter.cpp
    ${PROJECT_SOURCE_DIR}/src/defwriter.cpp
    ${PROJECT_SOURCE_DIR}/src/prlefreader.cpp
    ${PROJECT_SOURCE_DIR}/src/configreader.cpp
    ${PROJECT_SOURCE_DIR}/src/lef/lefreader.cpp
    ${PROJECT_SOURCE_DIR}/src/gds2/gds2writer.cpp
    ${PROJECT_SOURCE_DIR}/src/debugutils.cpp
)

add_executable(padring ${PADRINGSRC})
