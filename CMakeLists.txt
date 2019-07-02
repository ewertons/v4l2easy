project(v4l2easy)

cmake_minimum_required(VERSION 3.0.2)

add_subdirectory(deps)

set(header_files
    ./inc/v4l2easy.h
)

set(source_files
    ./src/v4l2easy.c
)

add_library(v4l2easy ${header_files} ${source_files})

include_directories(./inc)

target_link_libraries(v4l2easy 
)

add_subdirectory(samples)
