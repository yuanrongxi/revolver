CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(rudp)

# definitions
#

# includes
SET(includes
    "../../common"
    "./"
    )
INCLUDE_DIRECTORIES(${includes})

# sources
FILE(GLOB sources_cpp "../../common/rudp/*.cpp")

# targets
ADD_LIBRARY (rudp STATIC ${sources_cpp})
