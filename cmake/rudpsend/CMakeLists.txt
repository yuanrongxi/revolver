CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(rudpsend)

# definitions
#

# includes
SET(includes
    "../../common"
    )
INCLUDE_DIRECTORIES(${includes})

SET(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin/rudpsend")

# sources
FILE(GLOB sources_cpp "../../test/rudp_sender/rudp_sender/*.cpp")

# targets
ADD_EXECUTABLE(rudpsend ${sources_cpp})
TARGET_LINK_LIBRARIES(rudpsend silencer core rudp revolver pthread)
