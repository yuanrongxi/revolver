CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(daemon)

# definitions
#

# includes
SET(includes
    "../../common"
    "../common"
    )
INCLUDE_DIRECTORIES(${includes})

SET(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin/daemon")

# sources
FILE(GLOB sources_cpp "../../daemon/*.cpp")

# targets
ADD_EXECUTABLE(daemond ${sources_cpp})
TARGET_LINK_LIBRARIES(daemond silencer core revolver pthread)
