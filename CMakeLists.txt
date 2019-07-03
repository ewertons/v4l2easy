project(v4l2easy)

cmake_minimum_required(VERSION 3.0.2)

add_subdirectory(deps)

set(header_files
    ./inc/v4l2easy.h
)

set(source_files
    ./src/v4l2easy.c
)

include_directories(deps/common-lib-c/inc/)

add_library(v4l2easy ${header_files} ${source_files})

include_directories(./inc)

target_link_libraries(v4l2easy 
    common-lib-c
)

add_subdirectory(samples)
