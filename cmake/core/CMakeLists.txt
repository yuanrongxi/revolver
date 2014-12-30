CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(core)

# definitions
#

# includes
SET(includes
    "../../common"
    "../../common/zlib"
    "./"
    )
INCLUDE_DIRECTORIES(${includes})

# sources
FILE(GLOB sources_cpp "../../common/core/*.cpp")

# targets
ADD_LIBRARY (core STATIC ${sources_cpp})
