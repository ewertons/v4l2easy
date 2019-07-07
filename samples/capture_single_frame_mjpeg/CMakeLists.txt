project(capture_single_frame_mjpeg_sample)

cmake_minimum_required(VERSION 3.0.2)

set(header_files
)

set(source_files
    ./main.c
)

add_executable(capture_single_frame_mjpeg_sample ${header_files} ${source_files})

target_link_libraries(capture_single_frame_mjpeg_sample
    v4l2easy 
)
