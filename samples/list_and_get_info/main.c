#include <stdio.h>
#include <stdlib.h>
#include "v4l2easy.h"

#define TAB "  "

int main(void)
{
    char** devices;
    int count;
    
    if (v4l2easy_get_device_list(&devices, &count) != 0)
    {
        printf("failed getting devices\r\n");
    }
    else
    {
        int i;

        for (i = 0; i < count; i++)
        {
            printf("%d: %s\r\n", i, devices[i]);

            V4L2EASY_HANDLE dev = v4l2easy_open_device(devices[i]);

            if (dev == NULL)
            {
                printf("Failed opening device %s\r\n", devices[i]);
            }
            else
            {
                struct v4l2_capability capability;

                int count;
                struct v4l2_fmtdesc** formats;

                if (v4l2easy_get_device_capability(dev, &capability) != 0)
                {
                    printf("Failed getting device capability\r\n");
                }
                else
                {
                    printf(TAB "driver: %s\r\n", capability.driver);
                    printf(TAB "driver version: %d\r\n", capability.version);
                    printf(TAB "card: %s\r\n", capability.card);
                    printf(TAB "bus info: %s\r\n", capability.bus_info);
                    printf("\r\n");
                }

                if (v4l2easy_get_device_supported_formats(dev, &formats, &count) != 0)
                {
                    printf("Failed getting device supported formats\r\n");
                }
                else
                {
                    printf(TAB "Supported formats:\r\n");

                    int i;
                    for (i = 0; i < count; i++)
                    {
                        int frame_size_count;
                        struct v4l2_frmsizeenum** frame_sizes;

                        printf(TAB "index: %d\r\n", formats[i]->index);
                        printf(TAB "description: %s\r\n", formats[i]->description);
                        printf(TAB "type: %d\r\n", formats[i]->type);
                        printf(TAB "flags: %d\r\n", formats[i]->flags);
                        printf(TAB "pixelformat: %d\r\n", formats[i]->pixelformat);
                        printf("\r\n");

                        if (v4l2easy_get_device_supported_format_frame_sizes(dev, formats[i], &frame_sizes, &frame_size_count) != 0)
                        {
                            printf("Failed getting device format frame sizes\r\n");
                        }
                        else
                        {
                            printf(TAB "Frame sizes:\r\n");

                            int j;
                            for (j = 0; j < frame_size_count; j++)
                            {
                                printf(TAB TAB "[%d]: ", frame_sizes[j]->index);

                                if (frame_sizes[j]->type == V4L2_FRMSIZE_TYPE_DISCRETE)
                                {
                                    printf("%dx%d", frame_sizes[j]->discrete.width, frame_sizes[j]->discrete.height);
                                }
                                else if (frame_sizes[j]->type == V4L2_FRMSIZE_TYPE_STEPWISE)
                                {
                                    printf("%dx%d", frame_sizes[j]->stepwise.max_width, frame_sizes[j]->stepwise.max_height);
                                }
                                else
                                {
                                    printf("continuous");
                                }

                                printf("\r\n");
                            }
                        }
                    }
                }
                

                v4l2easy_close_device(dev);
            }

            free(devices[i]);
        }

        free(devices);
    }
    

    return 0;
}
