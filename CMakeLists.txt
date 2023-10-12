cmake_minimum_required(VERSION 3.10)

# set the project name
project(BlackHole)
set(CMAKE_CXX_FLAGS "-O3")
set (CMAKE_CXX_STANDARD 11)


file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})

#include header files
include_directories(include) 
if(WIN32)
    message("WIN32")
    file(GLOB WINDOWSDLL "libs/WindowsDll/*")
    file(COPY ${WINDOWSDLL} DESTINATION ${CMAKE_BINARY_DIR})
    set(CMAKE_CXX_FLAGS "-D_GNU_SOURCE -Wa,-mbig-obj -O1")
    set(CMAKE_C_FLAGS "-D_GNU_SOURCE -Wa,-mbig-obj -O1")
    set(ENABLE_PRECOMPILED_HEADERS OFF)
    add_definitions(-DGCC_WINDRES)
    set(WIN_LINKS "gdi32")
elseif(APPLE)
    message("APPLE")
    set(CMAKE_CXX_FLAGS "-DNS_FORMAT_ARGUMENT(A)= -O1")
    set(CMAKE_C_FLAGS "-DNS_FORMAT_ARGUMENT(A)= -O1")
endif()



#include glm (header only library)
#---------------------------------
include_directories(libs/glm)

#include glfw
#---------------------------------
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Do not build GLFW example programs")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "Do not build GLFW tests")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "Do not build GLFW Docs")
set(GLFW_INSTALL OFF CACHE BOOL "Do not install GLFW")
add_subdirectory(libs/glfw)
include_directories(libs/glfw/include)
link_directories(${PROJECT_BINARY_DIR}/libs/glfw/src)

#include freetype
#---------------------------------
add_subdirectory(libs/freetype)
include_directories(libs/freetype/include)
link_directories(${PROJECT_BINARY_DIR}/libs/freetype/builds)

#set SOURCES to all files in src ending in .cpp
file(GLOB SOURCES "src/*.c*")

# add the executable
add_executable(BlackHole ${SOURCES})

option(DEBUG "Will print Debug information to std out" OFF)
if(${DEBUG})
    message("Compiling with debug info")
    set_target_properties(BlackHole PROPERTIES COMPILE_DEFINITIONS "DEBUG")
    target_compile_options(BlackHole PRIVATE -g)
endif(${DEBUG})

if(APPLE) 
    target_link_libraries(BlackHole 
    glfw ${GLFW_LIBRARIES}
    ${WIN_LINKS} freetype ${FREETYPE_LIBRARIES}
    bz2 png z) #in case freetype goofs
else()
    target_link_libraries(BlackHole 
    glfw ${GLFW_LIBRARIES}
    ${WIN_LINKS} freetype ${FREETYPE_LIBRARIES}) #in case freetype goofs
endif(APPLE)
target_include_directories(BlackHole PRIVATE ${FREETYPE_INCLUDE_DIRS})
#-lBulletSoftBody -lBulletDynamics -lBulletCollision -lLinearMath\
#-lglfw3 -lglad -lassimp -lGL -lX11 -lpthread -lXrandr -lXi -ldl 

#add zipping commands
option(ZIP "Zip the files into the builds folder" OFF)
if(${ZIP} AND WIN32)
    message("Zipping for windows")
    add_custom_command(TARGET BlackHole 
		POST_BUILD
		COMMAND zip 
        ARGS -r ${CMAKE_SOURCE_DIR}/builds/Win64/BlackHole.zip 
		BlackHole.exe *.dll assets)
elseif(${ZIP} AND APPLE)
    message("Zipping for macOS")
    add_custom_command(TARGET BlackHole
		POST_BUILD
		COMMAND zip
		ARGS -r ${CMAKE_SOURCE_DIR}/builds/MacOS/BlackHole.zip
		BlackHole assets)
elseif(${ZIP})
    message("Zipping for linux")
    add_custom_command(TARGET BlackHole 
		POST_BUILD
		COMMAND zip 
		ARGS -r ${CMAKE_SOURCE_DIR}/builds/Linux/BlackHole.zip 
		BlackHole assets)
endif()

