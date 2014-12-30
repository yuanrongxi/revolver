CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(test)

# definitions
#

# includes
SET(includes
    "../../common"
    )
INCLUDE_DIRECTORIES(${includes})

SET(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin/test")

# sources
FILE(GLOB sources_cpp "../../test/test/*.cpp")

# targets
ADD_EXECUTABLE(test ${sources_cpp})
TARGET_LINK_LIBRARIES(test silencer core revolver pthread)
