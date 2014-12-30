CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(rudprecv)

# definitions
#

# includes
SET(includes
    "../../common"
    )
INCLUDE_DIRECTORIES(${includes})

SET(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin/rudprecv")

# sources
FILE(GLOB sources_cpp "../../test/rudp_reciver/rudp_reciver/*.cpp")

# targets
ADD_EXECUTABLE(rudprecv ${sources_cpp})
TARGET_LINK_LIBRARIES(rudprecv silencer core rudp revolver pthread)
