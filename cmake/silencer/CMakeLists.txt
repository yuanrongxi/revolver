CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(silencer)

# definitions
#

# includes
SET(includes
    "../../common"
    )
INCLUDE_DIRECTORIES(${includes})

# sources
FILE(GLOB sources_cpp "../../common/silencer/*.cpp")

# targets
ADD_LIBRARY (silencer STATIC ${sources_cpp})
