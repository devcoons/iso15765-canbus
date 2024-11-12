cmake_minimum_required(VERSION 3.10)
project(iso15765_canbus C)

set(SRC_DIR src)
set(LIB_DIR lib)
set(EXM_DIR exm)

include_directories(${SRC_DIR} ${LIB_DIR} ${EXM_DIR})

# Add the dependency library (libiqueue)
file(GLOB LIB_FILES "${LIB_DIR}/*.c")
add_library(iqueue STATIC ${LIB_FILES})

# Add the main library (libiso15765)
file(GLOB SRC_FILES "${SRC_DIR}/*.c")
add_library(iso15765 STATIC ${SRC_FILES})
target_link_libraries(iso15765 PRIVATE iqueue)

# Add the example executable
file(GLOB EXM_FILES "${EXM_DIR}/*.c")
add_executable(example ${EXM_FILES})
target_link_libraries(example PRIVATE iso15765 iqueue)

set_target_properties(iqueue iso15765 example PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/build"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/build"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/build"
)

if(MSVC)
    target_compile_options(iqueue PRIVATE /W4)
    target_compile_options(iso15765 PRIVATE /W4)
    target_compile_options(example PRIVATE /W4)
else()
    target_compile_options(iqueue PRIVATE -Wall -Wextra)
    target_compile_options(iso15765 PRIVATE -Wall -Wextra)
    target_compile_options(example PRIVATE -Wall -Wextra)
endif()
