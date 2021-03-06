##############################################################################
#Copyright (C) 2017-2018, Battelle Memorial Institute
#All rights reserved.

#This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
##############################################################################

#-----------------------------------------------------------------------------
# Common library tests using Boost
#-----------------------------------------------------------------------------

set(common_test_headers

)

set(common_test_sources
common-tests.cpp
MappedVectorTests.cpp
MappedPointerVectorTests.cpp
DualMappedVectorTests.cpp
DualMappedPointerVectorTests.cpp
BlockingQueueTests.cpp
PriorityBlockingQueueTests.cpp
SimpleQueueTests.cpp
TimeTests.cpp
)


add_executable(common-tests ${common_test_sources} ${common_test_headers} )
target_link_libraries(common-tests helics-static)

include_directories(SYSTEM ${Boost_INCLUDE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/ThirdParty)
include_directories(${PROJECT_SOURCE_DIR}/src)


target_link_libraries(common-tests ${Boost_LIBRARIES_test})



add_test(NAME common-tests COMMAND common-tests --log_level=warning --report_level=short)
set_target_properties (common-tests PROPERTIES FOLDER tests)

foreach(keyfile IN LISTS KEY_LIBRARY_FILES)
add_custom_command(TARGET common-tests POST_BUILD        # Adds a post-build event to core tests
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${keyfile}"      # <--this is in-file
        "$<TARGET_FILE_DIR:common-tests>/")                 # <--this is out-file path
endforeach(keyfile)
