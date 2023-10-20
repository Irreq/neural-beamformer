cmake_minimum_required(VERSION 3.10.1)
project(test)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

IF(NOT PYTHON)
    find_program(PYTHON "python")
ENDIF()

set(CMAKE_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/build")
set(SETUP_PY_IN "${CMAKE_CURRENT_SOURCE_DIR}/python/setup.py.in")
set(SETUP_PY    "${CMAKE_CURRENT_BINARY_DIR}/python/setup.py")

set(PY_OUTPUT      "${CMAKE_CURRENT_BINARY_DIR}/build/pytimestamp")

project(First VERSION 0.0.1)

# For matrix/vector
find_package(Eigen3 REQUIRED)

configure_file(
    ${SETUP_PY_IN}
    ${SETUP_PY}
)

#execute_process(COMMAND cp -r )

add_custom_target(config ALL DEPENDS
    # Generate C/C++ config.h
    COMMAND ${PYTHON} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/config_maker.py ${CMAKE_CURRENT_SOURCE_DIR}/config.json ${CMAKE_CURRENT_SOURCE_DIR}/src --c
    
    # Generate Cython config.h
    COMMAND ${PYTHON} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/config_maker.py ${CMAKE_CURRENT_SOURCE_DIR}/config.json ${CMAKE_CURRENT_SOURCE_DIR}/python/module --cython
    
    # Python 
    COMMAND ${PYTHON} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/config_maker.py ${CMAKE_CURRENT_SOURCE_DIR}/config.json ${CMAKE_CURRENT_BINARY_DIR} --python
    COMMAND ${PYTHON} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/config_maker.py ${CMAKE_CURRENT_SOURCE_DIR}/config.json ${CMAKE_CURRENT_SOURCE_DIR}/python --python
)

add_custom_command(OUTPUT "${PY_OUTPUT}"
                   COMMAND ${PYTHON} ${SETUP_PY} build_ext --build-lib=lib --build-temp=build
                   COMMAND ${CMAKE_COMMAND} -E touch ${PY_OUTPUT}
               )
#execte_process(COMMAND cp )

add_custom_target(cython_binds ALL DEPENDS ${PY_OUTPUT})

# add_library(bf ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

# Link each target with other targets or add options, etc.

# # Adding something we can run - Output name matches target name
# add_executable(antenna 
#     ${CMAKE_CURRENT_SOURCE_DIR}/src/antenna.cpp 
#     #${CMAKE_CURRENT_SOURCE_DIR}/src/.c
# )
# target_link_libraries(antennaa_py
#     Eigen3::Eigen
# )

# add_executable(receiver
#     ${CMAKE_CURRENT_SOURCE_DIR}/src/receiver.c
#     ${CMAKE_CURRENT_SOURCE_DIR}/src/receiver.h
#     ${CMAKE_CURRENT_SOURCE_DIR}/src/utils.c
#     ${CMAKE_CURRENT_SOURCE_DIR}/src/utils.h
#     ${CMAKE_CURRENT_SOURCE_DIR}/src/ring_buffer.h
#     ${CMAKE_CURRENT_SOURCE_DIR}/src/ring_buffer.c
# )

add_executable(beamformer
    ${CMAKE_CURRENT_SOURCE_DIR}/src/beamformer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/receiver.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/receiver.h
    # ${CMAKE_CURRENT_SOURCE_DIR}/src/beamformer.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/src/delay.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ring_buffer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ring_buffer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/delay.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/delay.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hmi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hmi.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/antenna.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/antenna.h
    
)

target_link_libraries(beamformer
    Eigen3::Eigen
)

add_executable(main
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ring_buffer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ring_buffer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils.c
)


# Adding something we can run - Output name matches target name
# add_executable(run 
#     ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c 
#     ${CMAKE_CURRENT_SOURCE_DIR}/src/ring_buffer.c
# )

# add_executable(delay
#     ${CMAKE_CURRENT_SOURCE_DIR}/src/delay.c 
#     ${CMAKE_CURRENT_SOURCE_DIR}/src/ring_buffer.c
# )

# Make sure you link your targets with this command. It can also link libraries and
# even flags, so linking a target that does not exist will not give a configure-time error.
# target_link_libraries(run PRIVATE bf)


add_custom_target(test
    # COMMAND ${PYTHON} -m unittest discover -s ..
    # COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/tests/test.sh
    COMMAND ${PYTHON} -m unittest discover -s ${CMAKE_CURRENT_SOURCE_DIR} -vv #--failfast

)

install(CODE "execute_process(COMMAND ${PYTHON} ${SETUP_PY} install)"
)

