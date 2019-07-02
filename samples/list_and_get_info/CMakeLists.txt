project(v4l2easy_list_and_get_info_sample)

cmake_minimum_required(VERSION 3.0.2)

set(header_files
)

set(source_files
    ./main.c
)

add_executable(v4l2easy_list_and_get_info_sample ${header_files} ${source_files})

target_link_libraries(v4l2easy_list_and_get_info_sample
    v4l2easy 
)
