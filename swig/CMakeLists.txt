
if (BUILD_PYTHON OR BUILD_MATLAB)

    FIND_PACKAGE(SWIG REQUIRED)
    INCLUDE(${SWIG_USE_FILE})

    INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
    INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/src/helics/shared_api_library")

endif()

if (BUILD_PYTHON)
    ADD_SUBDIRECTORY(python)
endif(BUILD_PYTHON)

if (BUILD_JAVA)

    FIND_PACKAGE(SWIG REQUIRED)
    INCLUDE(${SWIG_USE_FILE})

    INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
    INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/src/helics/shared_api_library")

    ADD_SUBDIRECTORY(java)

endif (BUILD_JAVA)
if(BUILD_MATLAB)
    FIND_PACKAGE(MATLAB REQUIRED)
    add_subdirectory(matlab)
endif(BUILD_MATLAB)