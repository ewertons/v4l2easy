#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "v4l2easy.h"

#define TAB "  "

#define MAX_STR_LEN (256)

static int get_highest_frame_size(struct v4l2_frmsizeenum** frame_sizes, int frame_sizes_count)
{
    int i;
    unsigned int biggest_frame_size = 0;
    int result = -1;
    
    for (i = 0; i< frame_sizes_count; i++)
    {
        unsigned int frame_size = 0;
        
        if (frame_sizes[i]->type == V4L2_FRMSIZE_TYPE_DISCRETE)
        {
             frame_size = frame_sizes[i]->discrete.width * frame_sizes[i]->discrete.height;
        }
        else if (frame_sizes[i]->type == V4L2_FRMSIZE_TYPE_STEPWISE)
        {
            frame_size = frame_sizes[i]->stepwise.max_width * frame_sizes[i]->stepwise.max_height;
        }

        if (frame_size > biggest_frame_size)
        {
            result = i;
            biggest_frame_size = frame_size;
        }
    }
    
    return result;
}

static int pick_a_device_and_params(V4L2EASY_HANDLE* v4l2easy_handle, struct v4l2_fmtdesc** format, struct v4l2_frmsizeenum** frame_size)
{
    int result;
    int device_count;
    char** devices;

    if (v4l2easy_get_device_list(&devices, &device_count) != 0)
    { 
        result = __LINE__;
    }
    else if (device_count == 0)
    {
        result = __LINE__;
    }
    else
    {
        int h;
        
        *v4l2easy_handle = NULL;
        *format = NULL;
        *frame_size = NULL;
        
        result = __LINE__;

        for (h = 0; h < device_count && *v4l2easy_handle == NULL; h++)
        {
            V4L2EASY_HANDLE dev = v4l2easy_open_device(devices[h]);

            if (dev == NULL)
            {
                printf("Failed opening device %s\r\n", devices[h]);
            }
            else
            {
                int format_count;
                struct v4l2_fmtdesc** formats;

                if (v4l2easy_get_device_supported_formats(dev, &formats, &format_count) != 0)
                {
                    printf("Failed getting device supported formats\r\n");
                    result = __LINE__;
                    break;
                }
                else
                {
                    int i;
                    for (i = 0; i < format_count && *format == NULL; i++)
                    {
                        if (formats[i]->pixelformat == V4L2_PIX_FMT_MJPEG)
                        {
                            int frame_size_count;
                            struct v4l2_frmsizeenum** frame_sizes;

                            if (v4l2easy_get_device_supported_format_frame_sizes(dev, formats[i], &frame_sizes, &frame_size_count) != 0)
                            {
                                printf("Failed getting device supported format frame sizes.\r\n");
                            }
                            else
                            {
                                int j;
                                int biggest_frame_size_index = get_highest_frame_size(frame_sizes, frame_size_count);
                                
                                if (biggest_frame_size_index != -1)
                                {
                                    *v4l2easy_handle = dev;
                                    *format = formats[i];
                                    *frame_size = frame_sizes[biggest_frame_size_index];
                                    result = 0;
                                }

                                for (j = 0; j < frame_size_count; j++)
                                {
                                    if (frame_sizes[j] != *frame_size)
                                    {
                                        free(frame_sizes[j]);
                                    }
                                }
                                
                                free(frame_sizes);
                            }
                        }                        
                    }
                    
                    for (i = 0; i < format_count; i++)
                    {
                        if (formats[i] != *format)
                        {
                            free(formats[i]);
                        }
                    }
                }

                if (*v4l2easy_handle == NULL)
                {
                    v4l2easy_close_device(dev);
                }
            }
        }

        for (h = 0; h < device_count; h++)
        {
            free(devices[h]);
        }
        
        free(devices);
    }
    
    return result;
}

int StoreCompressedImage(unsigned char *pVideoBuffer, unsigned int len, int *pFrameSize, int nFrame, unsigned int fmt, char *pFileName)
{
  char blankStr[MAX_STR_LEN];
  char outfileName[MAX_STR_LEN];

  if (!(V4L2_PIX_FMT_MJPEG == fmt || V4L2_PIX_FMT_H264 == fmt))
    return -1;

  memset(&blankStr[0], 0, MAX_STR_LEN);

  memset(&outfileName[0], 0, MAX_STR_LEN);

  if (0 != memcmp(pFileName, &blankStr[0], MAX_STR_LEN))
    snprintf(&outfileName[0], MAX_STR_LEN, "%s", pFileName);

  time_t t;
  struct tm lt;
  struct timeval tv;

  t = time(NULL);
  lt = *localtime(&t);
  gettimeofday(&tv, NULL);

  char timeStr[MAX_STR_LEN];

  snprintf(&timeStr[0], MAX_STR_LEN, "%d-%d-%d--%d-%d-%d-%d",
           lt.tm_year + 1900, lt.tm_mon + 1, lt.tm_mday,
           lt.tm_hour, lt.tm_min, lt.tm_sec, (int)tv.tv_usec / 1000);

  strncat(&outfileName[0], &timeStr[0], MAX_STR_LEN);

  if (V4L2_PIX_FMT_MJPEG == fmt)
    strncat(&outfileName[0], ".jpg", MAX_STR_LEN);
  else
    strncat(&outfileName[0], ".h264", MAX_STR_LEN);

  FILE *fp;
  fp = fopen(&outfileName[0], "wb");
  if (fp < 0)
  {
    printf("open frame data file failed\n");
    return -1;
  } /*if */

  fwrite(pVideoBuffer, 1, len, fp);
  fclose(fp);

  printf("Capture data saved in %s\n", &outfileName[0]);

  if (NULL == pFrameSize || V4L2_PIX_FMT_MJPEG == fmt)
    return 0;

  /*save each frame size as a file*/

  memset(&outfileName[0], 0, MAX_STR_LEN);

  if (0 != memcmp(pFileName, &blankStr[0], MAX_STR_LEN))
    snprintf(&outfileName[0], MAX_STR_LEN, "%s", pFileName);

  strncat(&outfileName[0], &timeStr[0], MAX_STR_LEN);
  strncat(&outfileName[0], "_h264size.txt", MAX_STR_LEN);

  fp = fopen(&outfileName[0], "w");
  if (fp < 0)
  {
    printf("open frame size file failed\n");
    return -1;
  } /*if */

  int i;
  for (i = 0; i < nFrame; i++)
    fprintf(fp, "%d\n", pFrameSize[i]);

  fclose(fp);
  printf("frame size saved in %s\n", &outfileName[0]);

  return 0;
} /*StoreCompressedImage*/
    
int main(void)
{
    V4L2EASY_HANDLE v4l2easy_handle;
    struct v4l2_fmtdesc* format;
    struct v4l2_frmsizeenum* frame_size;
    
    if (pick_a_device_and_params(&v4l2easy_handle, &format, &frame_size) != 0)
    {
		printf("Failed getting a device and format for this sample.");
	}
	else
	{
        struct v4l2_format final_format;
        
        memset(&final_format, 0, sizeof(struct v4l2_format));
        final_format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        final_format.fmt.pix.pixelformat = format->pixelformat;
        final_format.fmt.pix.field = V4L2_FIELD_INTERLACED;

        
        if (frame_size->type == V4L2_FRMSIZE_TYPE_DISCRETE)
        {
            final_format.fmt.pix.width = frame_size->discrete.width;
            final_format.fmt.pix.height = frame_size->discrete.height;
        }
        else if (frame_size->type == V4L2_FRMSIZE_TYPE_STEPWISE)
        {
            final_format.fmt.pix.width = frame_size->stepwise.max_width;
            final_format.fmt.pix.height = frame_size->stepwise.max_height;
        }
        
        if (v4l2easy_set_format(v4l2easy_handle, &final_format) != 0)
        {
            printf("Failed setting the current format set on the device\r\n");
        }
        
        if (v4l2easy_start_camera(v4l2easy_handle) != 0)
        {
            printf("Failed starting the camera\r\n");
        }
        else
        {
            unsigned char* frame_data;
            unsigned int size;

            if (v4l2easy_capture(v4l2easy_handle, &frame_data, &size) != 0)
            {
                printf("Failed capturing a frame\r\n");
            }
            else
            {
                if (StoreCompressedImage(frame_data, size, NULL, 0, (unsigned int)final_format.fmt.pix.pixelformat, "capture_file") != 0)
                {
                    printf("Failed saving image\r\n");
                }
                
                free(frame_data);
            }
            
            if (v4l2easy_stop_camera(v4l2easy_handle) != 0)
            {
                printf("Failed stopping the camera\r\n");
            }
        }
        
        free(format);
        free(frame_size);
        v4l2easy_close_device(v4l2easy_handle);
    }

    return 0;
}
