cmake_minimum_required(VERSION 3.10.1)
project(test)

IF(NOT PYTHON)
    find_program(PYTHON "python")
ENDIF()

set(CMAKE_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/build")
set(SETUP_PY_IN "${CMAKE_CURRENT_SOURCE_DIR}/python/setup.py.in")
set(SETUP_PY    "${CMAKE_CURRENT_BINARY_DIR}/python/setup.py")

set(PY_OUTPUT      "${CMAKE_CURRENT_BINARY_DIR}/build/pytimestamp")

project(First VERSION 0.0.1)

configure_file(
    ${SETUP_PY_IN}
    ${SETUP_PY}
)

execute_process(COMMAND echo ${CMAKE_BUILD_DIR})

add_custom_target(config ALL DEPENDS
    # Generate C/C++ config.h
    COMMAND ${PYTHON} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/config_maker.py ${CMAKE_CURRENT_SOURCE_DIR}/config.json ${CMAKE_CURRENT_SOURCE_DIR}/src/config.h
    
    # Generate Cython config.h
    COMMAND ${PYTHON} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/config_maker.py ${CMAKE_CURRENT_SOURCE_DIR}/config.json ${CMAKE_CURRENT_SOURCE_DIR}/python/module/config.h
               )

add_custom_command(OUTPUT "${PY_OUTPUT}"
                   COMMAND ${PYTHON} ${SETUP_PY} build_ext --build-lib=lib --build-temp=build
                   COMMAND ${CMAKE_COMMAND} -E touch ${PY_OUTPUT}
               )

add_custom_target(a_py ALL DEPENDS ${PY_OUTPUT})

add_custom_target(test
    # COMMAND ${PYTHON} -m unittest discover -s ..
    COMMAND ${PYTHON} -m unittest discover -s ${CMAKE_CURRENT_SOURCE_DIR}
)

install(CODE "execute_process(COMMAND ${PYTHON} ${SETUP_PY} install)")

