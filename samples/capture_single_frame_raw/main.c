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
                        if (formats[i]->pixelformat == V4L2_PIX_FMT_YUYV)
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

#define FOUR (4)
#define ALIGN_TO_FOUR(VAL) (((VAL) + FOUR - 1) & ~(FOUR - 1))


#define R_DIFF(VV) ((VV) + (103 * (VV) >> 8))
#define G_DIFF(UU, VV) (-((88 * (UU)) >> 8) - ((VV * 183) >> 8))
#define B_DIFF(UU) ((UU) + ((198 * (UU)) >> 8))

#define Y_PLUS_RDIFF(YY, VV) ((YY) + R_DIFF(VV))
#define Y_PLUS_GDIFF(YY, UU, VV) ((YY) + G_DIFF(UU, VV))
#define Y_PLUS_BDIFF(YY, UU) ((YY) + B_DIFF(UU))

#define EVEN_ZERO_ODD_ONE(XX) (((XX) & (0x01)))

/*if X > 255, X = 255; if X< 0, X = 0*/
#define CLIP1(XX) ((unsigned char)((XX & ~255) ? (~XX >> 15) : XX))

static int convert_YUYV_to_RGB24(unsigned char *pYUYV, int width, int height, unsigned char *pRGB24)
{
  unsigned int i, j;

  unsigned char *pMovY, *pMovU, *pMovV;
  unsigned char *pMovRGB;

  unsigned int pitch;
  unsigned int pitchRGB;

  pitch = 2 * width;
  pitchRGB = ALIGN_TO_FOUR(3 * width);

  for (j = 0; j < height; j++)
  {
    pMovY = pYUYV + j * pitch;
    pMovU = pMovY + 1;
    pMovV = pMovY + 3;

    pMovRGB = pRGB24 + pitchRGB * j;

    for (i = 0; i < width; i++)
    {
      int R, G, B;
      int Y, U, V;

      Y = pMovY[0];
      U = *pMovU - 128;
      V = *pMovV - 128;
      R = Y_PLUS_RDIFF(Y, V);
      G = Y_PLUS_GDIFF(Y, U, V);
      B = Y_PLUS_BDIFF(Y, U);

      *pMovRGB = CLIP1(B);
      *(pMovRGB + 1) = CLIP1(G);
      *(pMovRGB + 2) = CLIP1(R);

      pMovY += 2;

      pMovU += 4 * EVEN_ZERO_ODD_ONE(i);
      pMovV += 4 * EVEN_ZERO_ODD_ONE(i);

      pMovRGB += 3;
    } /*for i*/
  }   /*for j*/

  return 0;
}

int BMPwriter(unsigned char *pRGB, int bitNum, int width, int height, char *pFileName)
{
  FILE *fp;
  int fileSize;
  unsigned char *pMovRGB;
  int i;
  int widthStep;

  unsigned char header[54] = {
      0x42,        // identity : B
      0x4d,        // identity : M
      0, 0, 0, 0,  // file size
      0, 0,        // reserved1
      0, 0,        // reserved2
      54, 0, 0, 0, // RGB data offset
      40, 0, 0, 0, // struct BITMAPINFOHEADER size
      0, 0, 0, 0,  // bmp width
      0, 0, 0, 0,  // bmp height
      1, 0,        // planes
      24, 0,       // bit per pixel
      0, 0, 0, 0,  // compression
      0, 0, 0, 0,  // data size
      0, 0, 0, 0,  // h resolution
      0, 0, 0, 0,  // v resolution
      0, 0, 0, 0,  // used colors
      0, 0, 0, 0   // important colors
  };

  widthStep = ALIGN_TO_FOUR(width * bitNum / 8);

  fileSize = ALIGN_TO_FOUR(widthStep * height) + sizeof(header);

  memcpy(&header[2], &fileSize, sizeof(int));
  memcpy(&header[18], &width, sizeof(int));
  memcpy(&header[22], &height, sizeof(int));
  memcpy(&header[28], &bitNum, sizeof(short));

  printf("written on file %s ...", pFileName);
  fp = fopen(pFileName, "wb");

  fwrite(&header[0], 1, sizeof(header), fp);

  pMovRGB = pRGB + (height - 1) * widthStep;

  for (i = 0; i < height; i++)
  {
    fwrite(pMovRGB, 1, widthStep, fp);
    pMovRGB -= widthStep;
  } /*for i*/

  fclose(fp);
  printf("done\n");

  return 0;
} /*BMPwriter*/

int StoreRAWImage(unsigned char *pMappingBuffer, int width, int height,
                  unsigned int fmt, char *pFileName)
{
  char blankStr[MAX_STR_LEN];
  char outfileName[MAX_STR_LEN];

  if (V4L2_PIX_FMT_YUYV != fmt)
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

  char tempStr[MAX_STR_LEN];

  snprintf(&tempStr[0], MAX_STR_LEN, "%d-%d-%d--%d-%d-%d-%d",
           lt.tm_year + 1900, lt.tm_mon + 1, lt.tm_mday,
           lt.tm_hour, lt.tm_min, lt.tm_sec, (int)tv.tv_usec / 1000);

  strncat(&outfileName[0], &tempStr[0], MAX_STR_LEN);
  strncat(&outfileName[0], ".bmp", MAX_STR_LEN);

  unsigned char *pRGB24;
  pRGB24 = (unsigned char *)malloc(ALIGN_TO_FOUR(3 * width) * height);

  convert_YUYV_to_RGB24(pMappingBuffer, width, height, pRGB24);

  BMPwriter(pRGB24, 24, width, height, &outfileName[0]);
  free(pRGB24);
  pRGB24 = NULL;

  return 0;
}
    
int main(void)
{
    V4L2EASY_HANDLE v4l2easy_handle;
    struct v4l2_fmtdesc* format;
    struct v4l2_frmsizeenum* frame_size;
    
    if (pick_a_device_and_params(&v4l2easy_handle, &format, &frame_size) == 0)
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
                if (StoreRAWImage(frame_data, final_format.fmt.pix.width, final_format.fmt.pix.height, final_format.fmt.pix.pixelformat, "capture_file") != 0)
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
