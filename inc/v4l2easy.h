#ifndef V4L2EASY_H
#define V4L2EASY_H

#include <stdlib.h>
#include <linux/videodev2.h>

typedef void* V4L2EASY_HANDLE;

extern int v4l2easy_get_device_list(char*** devices, int* count);

extern V4L2EASY_HANDLE v4l2easy_open_device(char* device_path);
extern void v4l2easy_close_device(V4L2EASY_HANDLE v4l2easy_handle); 

extern int v4l2easy_get_device_capability(V4L2EASY_HANDLE v4l2easy_handle, struct v4l2_capability* capability);
extern int v4l2easy_get_device_supported_formats(V4L2EASY_HANDLE v4l2easy_handle, struct v4l2_fmtdesc*** formats, unsigned int* count);
extern int v4l2easy_get_device_supported_format_frame_sizes(V4L2EASY_HANDLE v4l2easy_handle, struct v4l2_fmtdesc* format, struct v4l2_frmsizeenum*** frame_sizes, unsigned int* count);

extern int v4l2easy_get_format(V4L2EASY_HANDLE v4l2easy_handle, struct v4l2_format* format);
extern int v4l2easy_set_format(V4L2EASY_HANDLE v4l2easy_handle, struct v4l2_format* format);

extern int v4l2easy_start_camera(V4L2EASY_HANDLE v4l2easy_handle);
extern int v4l2easy_capture(V4L2EASY_HANDLE v4l2easy_handle, unsigned char** frame_data, unsigned int* size);
extern int v4l2easy_stop_camera(V4L2EASY_HANDLE v4l2easy_handle);

#endif // V4L2EASY_H
