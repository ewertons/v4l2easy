#include <stdio.h>
#include <stdlib.h>
#include "v4l2easy.h"

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
                    printf("\tdriver: %s\r\n", capability.driver);
                    printf("\tdriver version: %d\r\n", capability.version);
                    printf("\tcard: %s\r\n", capability.card);
                    printf("\tbus info: %s\r\n", capability.bus_info);
                    printf("\r\n");
                }

                if (v4l2easy_get_device_supported_formats(dev, &formats, &count) != 0)
                {
                    printf("Failed getting device supported formats\r\n");
                }
                else
                {
                    printf("\tSupported formats:\r\n");

                    int i;
                    for (i = 0; i < count; i++)
                    {
                        V4L2EASY_FRAME_SIZE_INFO* frame_sizes;

                        printf("\tindex: %d\r\n", formats[i]->index);
                        printf("\tdescription: %s\r\n", formats[i]->description);
                        printf("\ttype: %d\r\n", formats[i]->type);
                        printf("\tflags: %d\r\n", formats[i]->flags);
                        printf("\tpixelformat: %d\r\n", formats[i]->pixelformat);
                        printf("\r\n");

                        if (v4l2easy_get_device_supported_format_frame_sizes2(dev, formats[i], &frame_sizes) != 0)
                        {
                            printf("Failed getting device format frame sizes\r\n");
                        }
                        else
                        {
                            printf("\tFrame sizes:\r\n");

                            V4L2EASY_FRAME_SIZE_INFO* frame_size = frame_sizes;

                            while (frame_size != NULL)
                            {
                                printf("\t\t[%d]: ", frame_size->info.index);

                                if (frame_size->info.type == V4L2_FRMSIZE_TYPE_DISCRETE)
                                {
                                    printf("%dx%d", frame_size->info.discrete.width, frame_size->info.discrete.height);
                                }
                                else if (frame_size->info.type == V4L2_FRMSIZE_TYPE_STEPWISE)
                                {
                                    printf("%dx%d", frame_size->info.stepwise.max_width, frame_size->info.stepwise.max_height);
                                }
                                else
                                {
                                    printf("continuous");
                                }

                                printf("\r\n");
                                frame_size = frame_size->next;
                            }

                            v4l2easy_destroy_format_frame_size_list(frame_sizes);
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