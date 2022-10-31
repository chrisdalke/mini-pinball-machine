cmake_minimum_required(VERSION 3.16)
cmake_policy(SET CMP0074 NEW)
cmake_policy(SET CMP0083 NEW)
cmake_policy(SET CMP0077 NEW)

project(pinball VERSION 0.1.0
                        DESCRIPTION "Mini Pinball Machine"
                        LANGUAGES C)
set(CMAKE_CXX_STANDARD 17)

### Require out-of-source builds
file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)
if(EXISTS "${LOC_PATH}")
    message(FATAL_ERROR "You cannot build in a source directory (or any directory with a CMakeLists.txt file). Please make a build subdirectory. Feel free to remove CMakeCache.txt and CMakeFiles.")
endif()

# Configuration if this is the main project
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(CMAKE_CXX_EXTENSIONS OFF)
    if (WIN32)
    else()
        set (CMAKE_CXX_FLAGS "-Wall -Wno-ignored-attributes")
    endif()
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

#--------------- SOURCE & HEADER FILES --------------------

set(SOURCES
    src/constants.c 
    src/gameStruct.c 
    src/inputManagerMac.c 
    src/main.c 
    src/physicsDebugDraw.c 
    src/scores.c 
    src/soundManager.c 
    src/sqlite3.c 
)

set(HEADERS_PRIVATE
    src/constants.h 
    src/gameStruct.h
    src/inputManager.h
    src/physicsDebugDraw.h
    src/scores.h
    src/soundManager.h
    src/sqlite3.h
)

#------------------- BUILD TARGETS ------------------------

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS_PRIVATE})
set_target_properties(${PROJECT_NAME}  PROPERTIES LINKER_LANGUAGE C)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "pinball")
set_target_properties(${PROJECT_NAME} PROPERTIES SKIP_BUILD_RPATH TRUE)
#target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)

# Add the build directory to the search path so version header can be found
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_BINARY_DIR})

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/resources/assets $<TARGET_FILE_DIR:${PROJECT_NAME}>/Resources)

#--------------- EXTERNAL DEPENDENCIES --------------------
# set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
# set(BUILD_SHARED_LIBS OFF)
# set(CMAKE_EXE_LINKER_FLAGS "-static")

target_include_directories(${PROJECT_NAME} PRIVATE "/usr/local/include")
target_include_directories(${PROJECT_NAME} PRIVATE "/usr/local/include/chipmunk")
target_link_directories(${PROJECT_NAME} PRIVATE "/usr/local/lib")
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)
target_link_libraries(${PROJECT_NAME} PRIVATE m pthread dl chipmunk)

#--------------- PLATFORM-SPECIFIC DEPENDENCIES & FLAGS --------------------

# Dependencies and build flags for individual platforms
if (WIN32)
    message(STATUS "Adding Windows-specific libraries")
    add_definitions(-DPLATFORM_WIN32)
else()
    if (APPLE)
        message(STATUS "Adding Apple-specific libraries")
        add_definitions(-DPLATFORM_APPLE)

        # General-purpose header location for mac installed libraries
        target_include_directories(${PROJECT_NAME} PUBLIC /usr/local/include)
    else()
        if (UNIX)
            message(STATUS "Adding Linux-specific libraries")
            add_definitions(-DPLATFORM_LINUX)

            # General-purpose header location for linux installed libraries
            target_include_directories(${PROJECT_NAME} PUBLIC /usr/local/include)

        else()
            message(FATAL_ERROR "Unknown platform!")
        endif()
    endif()
endif()

#--------------- VARIOUS PROJECT ORGANIZATION --------------------
# IDEs should put the headers in a nice place
source_group(TREE "${PROJECT_SOURCE_DIR}/include" PREFIX "Header Files" FILES ${HEADERS})
