CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(revolver)

# definitions
#

# includes
SET(includes
    "./"
    "../../common/"
    )
INCLUDE_DIRECTORIES(${includes})

# sources
FILE(GLOB sources_cpp "../../common/revolver/*.cpp")
FILE(GLOB sources_c "../../common/revolver/*.c")

# targets
ADD_LIBRARY (revolver STATIC ${sources_cpp} ${sources_c})
