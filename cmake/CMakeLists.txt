CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(all)

#### library paths
SET(LIBRARY_PATH "${CMAKE_SOURCE_DIR}/lib")

#### output paths
SET(LIBRARY_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/lib")
SET(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin")

#### settings
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wno-deprecated")
# SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
SET(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -L${LIBRARY_PATH} -lpthread")

#### projects
ADD_SUBDIRECTORY(revolver)
ADD_SUBDIRECTORY(rudp)
ADD_SUBDIRECTORY(core)
ADD_SUBDIRECTORY(silencer)
ADD_SUBDIRECTORY(daemon)
ADD_SUBDIRECTORY(test) 
ADD_SUBDIRECTORY(rudprecv)
ADD_SUBDIRECTORY(rudpsend)
ADD_SUBDIRECTORY(sample)



if(UNIX)
    add_custom_target (distclean @echo cleaning for source distribution)
    
    add_custom_command(
        COMMENT "distribution clean"
        COMMAND make
        ARGS -C ${CMAKE_CURRENT_BINARY_DIR} clean
        COMMAND find
        ARGS ${CMAKE_CURRENT_BINARY_DIR} -name "CMakeCache.txt" | xargs rm -rf
        COMMAND find
        ARGS ${CMAKE_CURRENT_BINARY_DIR} -name "CMakeFiles" | xargs rm -rf
        COMMAND find
        ARGS ${CMAKE_CURRENT_BINARY_DIR} -name "Makefile" | xargs rm -rf
        COMMAND find
        ARGS ${CMAKE_CURRENT_BINARY_DIR} -name "*.cmake" | xargs rm -rf
        COMMAND find
        ARGS ${CMAKE_CURRENT_SOURCE_DIR} -name "*.qm" | xargs rm -rf
        COMMAND rm
        ARGS -rf ${CMAKE_CURRENT_BINARY_DIR}/install_manifest.txt
        TARGET  distclean
    )
endif(UNIX) 
