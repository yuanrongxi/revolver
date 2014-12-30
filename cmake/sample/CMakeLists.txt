CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(sampled)

# definitions
#

# includes
SET(includes
    "../../common"
    )
INCLUDE_DIRECTORIES(${includes})

SET(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin/sample")

# sources
FILE(GLOB sources_cpp "../../test/sample_server/*.cpp")

# targets
ADD_EXECUTABLE(sampled ${sources_cpp})
TARGET_LINK_LIBRARIES(sampled silencer core revolver pthread)
