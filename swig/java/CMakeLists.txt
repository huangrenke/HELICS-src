
FIND_PACKAGE(Java REQUIRED)
FIND_PACKAGE(JNI REQUIRED)
INCLUDE(UseJava)
INCLUDE(UseSWIG)
INCLUDE_DIRECTORIES(${JAVA_INCLUDE_PATH})
INCLUDE_DIRECTORIES(${JAVA_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${JNI_INCLUDE_DIRS})

SET(CMAKE_SWIG_FLAGS "")

SET_PROPERTY(SOURCE ../helics.i PROPERTY SWIG_MODULE_NAME JNIhelics)

SWIG_ADD_LIBRARY(JNIhelics TYPE MODULE LANGUAGE java SOURCES ../helics.i)

SWIG_LINK_LIBRARIES(JNIhelics helicsSharedLib)
SWIG_LINK_LIBRARIES(JNIhelics ${JAVA_LIBRARIES})



if (APPLE)

    # SET_TARGET_PROPERTIES(helicsSharedLib PROPERTIES MACOSX_RPATH TRUE)
    # SET_TARGET_PROPERTIES(helicsSharedLib PROPERTIES CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
    
    # SET_TARGET_PROPERTIES(_helics PROPERTIES MACOSX_RPATH TRUE)
    # SET_TARGET_PROPERTIES(_helics PROPERTIES CMAKE_INSTALL_RPATH_USE_LINK_PATH
    # SET_TARGET_PROPERTIES(helics PROPERTIES INSTALL_RPATH "@loader_path/../../..")
    
else()

    # SET_TARGET_PROPERTIES(_helics PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
    # SET_TARGET_PROPERTIES(_helics PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)

endif()
