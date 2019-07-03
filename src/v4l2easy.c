#include "v4l2easy.h"
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include "circular_list.h"

#define DEVICES_ROOT_PATH "/dev"
#define DEVICES_ROOT_PATH_LENGTH 4
#define VIDEO_DEVICE_NAME_FILTER "video"
#define VIDEO_DEVICE_NAME_FILTER_LENGTH 5 

#define MAX_DEVICE_FORMAT_COUNT 100
#define MAX_DEVICE_FRAME_SIZE_COUNT 100

typedef struct V4L2EASY_DATA_STRUCT
{
    int file_descriptor;
} V4L2EASY_DATA;

static char* get_full_device_path(char* str1)
{
    char* result;
    int str1_length = strlen(str1);
    int result_length = DEVICES_ROOT_PATH_LENGTH + str1_length + 1; // Extra space for the slash

    result = malloc(result_length + 1); // Extra space for \0

    if (result == NULL)
    {

    }
    else
    {
        (void)strncpy(result, DEVICES_ROOT_PATH, DEVICES_ROOT_PATH_LENGTH);
        result[DEVICES_ROOT_PATH_LENGTH] = '/';
        (void)strncpy(result + DEVICES_ROOT_PATH_LENGTH + 1, str1, str1_length);
        result[result_length] = '\0';
    }

    return result;
}

static void destroy_char_list(char** devices, int count)
{
    int i;
    for (i = 0; i < count; i++)
    {
        free(devices[i]);
    }
    free(devices);
}

int v4l2easy_get_device_list(char*** devices, int* count)
{
    int result;

    if (devices == NULL)
    {
        result = __LINE__;
    }
    else
    {
        DIR* dir;

        if ((dir = opendir(DEVICES_ROOT_PATH)) == NULL)
        {
            result = __LINE__;
        }
        else
        {
            struct dirent* ent;

            char** temp_devices = NULL;
            int temp_count = 0;
            result = 0;

            while ((ent = readdir(dir)) != NULL) 
            {
                if (strncmp(ent->d_name, VIDEO_DEVICE_NAME_FILTER, VIDEO_DEVICE_NAME_FILTER_LENGTH) == 0)
                {
                    char* file_path = get_full_device_path(ent->d_name);

                    if (file_path == NULL)
                    {
                        destroy_char_list(temp_devices, temp_count);
                        result = __LINE__;
                        break;                        
                    }
                    else
                    {
                        // Adding the new device to the list.
                        temp_count++;
                        temp_devices = realloc(temp_devices, temp_count);

                        if (temp_devices == NULL)
                        {
                            free(file_path);
                            destroy_char_list(temp_devices, temp_count);
                            result = __LINE__;
                            break;
                        }
                        else
                        {
                            temp_devices[temp_count - 1] = file_path; 
                        }
                    }
                }
            }

            closedir (dir);

            if (result == 0)
            {
                *devices = temp_devices;
                *count = temp_count;
            }
        }
    }

    return result;
}


V4L2EASY_HANDLE v4l2easy_open_device(char* device_path)
{
    V4L2EASY_DATA* result;

    if (device_path == NULL)
    {
        result = NULL;
    }
    else if ((result = malloc(sizeof(V4L2EASY_DATA))) == NULL)
    {

    }
    else
    {
        result->file_descriptor = open(device_path, O_RDWR);

        if (result->file_descriptor < 0)
        {
            free(result);
            result = NULL;
        }
    }

    return result;
}

void v4l2easy_close_device(V4L2EASY_HANDLE v4l2easy_handle)
{
    if (v4l2easy_handle == NULL)
    {

    }
    else
    {
        V4L2EASY_DATA* v4l2easy_data = (V4L2EASY_DATA*)v4l2easy_handle;

        close(v4l2easy_data->file_descriptor);
        free(v4l2easy_data);
    }
}

static char* clone_string(char* src)
{
    char* result;

    if (src == NULL)
    {
        result = NULL;
    }
    else
    { 
        int src_length = strlen(src);

        result = malloc(src_length + 1);
        
        if (result == NULL)
        {

        }
        else
        {
            memcpy(result, src, src_length);
            result[src_length] = '\0';
        }
    }
    
    return result;
}

int v4l2easy_get_device_capability(V4L2EASY_HANDLE v4l2easy_handle, struct v4l2_capability* capability)
{
    int result;

    if (v4l2easy_handle == NULL || capability == NULL)
    {
        result = __LINE__;
    }
    else
    {
        V4L2EASY_DATA* v4l2easy_data = (V4L2EASY_DATA*)v4l2easy_handle;

        if (ioctl(v4l2easy_data->file_descriptor, VIDIOC_QUERYCAP, capability) < 0)
        {
            result = __LINE__;
        }
        else
        {
            result = 0;
        }
    }
}

int v4l2easy_get_device_supported_formats(V4L2EASY_HANDLE v4l2easy_handle, struct v4l2_fmtdesc*** formats, unsigned int* count)
{
    int result;

    if (v4l2easy_handle == NULL || formats == NULL || count == NULL)
    {
        result = __LINE__;
    }
    else
    {
        V4L2EASY_DATA* v4l2easy_data = (V4L2EASY_DATA*)v4l2easy_handle;
        int index;
        struct v4l2_fmtdesc** temp_formats = NULL;
        int temp_count = 0;

        result = 0;

        for (index = 0; index < MAX_DEVICE_FORMAT_COUNT; index++)
        {
            struct v4l2_fmtdesc* supportedFmt = malloc(sizeof(struct v4l2_fmtdesc));

            if (supportedFmt == NULL)
            {
                result = __LINE__;
                break;
            }
            else
            {
                supportedFmt->index = index;
                supportedFmt->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                
                if (ioctl(v4l2easy_data->file_descriptor, VIDIOC_ENUM_FMT, supportedFmt) >= 0)
                {
                    if ((temp_formats = realloc(temp_formats, temp_count + 1)) == NULL)
                    {
                        free(supportedFmt);
                        result = __LINE__;
                        break;                        
                    }
                    else
                    {
                        temp_formats[temp_count] = supportedFmt;
                        temp_count++;
                    }
                }
                else
                {
                    free(supportedFmt);
                    break;
                }   
            }        
        }

        if (result != 0)
        {
            while(temp_count > 0)
            {
                temp_count--;
                free(temp_formats[temp_count]);
            }
            free(temp_formats);
        }
        else
        {
            *formats = temp_formats;
            *count = temp_count;
        }
    }

    return result;
}

static bool destroy_every_item_in_list(LIST_NODE_HANDLE node, void* context, bool* continue_processing)
{
	(void)context;
	struct v4l2_frmsizeenum* frame_size = (struct v4l2_frmsizeenum*)circular_list_node_get_value(node);
	
	free(frame_size);
	
	*continue_processing = true;
	return true;
}

int v4l2easy_get_device_supported_format_frame_sizes(V4L2EASY_HANDLE v4l2easy_handle, struct v4l2_fmtdesc* format, struct v4l2_frmsizeenum*** frame_sizes, unsigned int* count)
{
    int result;

    if (v4l2easy_handle == NULL || format == NULL || frame_sizes == NULL || count == NULL)
    {
        result = __LINE__;
    }
    else
	{
		CIRCULAR_LIST_HANDLE list = circular_list_create();
		
		if (list == NULL)
		{
			result = __LINE__;
		}
		else
		{
			V4L2EASY_DATA* v4l2easy_data = (V4L2EASY_DATA*)v4l2easy_handle;
			int index;

			result = 0;

			for (index = 0; index < MAX_DEVICE_FRAME_SIZE_COUNT; index++)
			{
				struct v4l2_frmsizeenum* frame_size = malloc(sizeof(struct v4l2_frmsizeenum));

				if (frame_size == NULL)
				{
					result = __LINE__;
					break;
				}
				else
				{
					frame_size->index = index;
					frame_size->pixel_format = format->pixelformat;    

					if (ioctl(v4l2easy_data->file_descriptor, VIDIOC_ENUM_FRAMESIZES, frame_size) >= 0)
					{
						if (circular_list_add(list, frame_size) == NULL)
						{
							free(frame_size);
							result = __LINE__;
							break;                        
						}
					}
					else
					{
						free(frame_size);
						break;
					}         
				}
			}

			if (result == 0)	
			{
				void** temp_frame_sizes;
				int temp_count;
				
				if (circular_list_to_array(list, &temp_frame_sizes, &temp_count) != 0)
				{
					result = __LINE__;
				}
				else
				{
					*count = temp_count;
					*frame_sizes = (struct v4l2_frmsizeenum**)temp_frame_sizes;
				}
			}
			
			if (result != 0)
			{
				(void)circular_list_remove_if(list, destroy_every_item_in_list, NULL);
			}
			
			circular_list_destroy(list);
		}
	}

    return result;
}

void v4l2easy_set_capture_preferences();
