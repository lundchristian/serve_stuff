#ifndef FILE_GENERATOR_H
#define FILE_GENERATOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

typedef struct file_generator file_generator;

struct file_generator {
    char* ressources_dir;
    
    int (*file_exist)(file_generator *self, const char *file_name);
    int (*get_file_size)(file_generator *self, const char *file_name);

    char* (*get_file_path)(file_generator *self, const char *file_name);
    char* (*get_file_content)(file_generator *self, const char *file_name);
};

/* Method: file_exist_impl
* Argument(self): Pointer to the struct object to access members 
* Argument(file_name): Name of file to find, ex. "index.html"
* Brief: Searches for file in ressources directory, return 1 if file exist, else 0
* Return: int used as bool
*/
int file_exist_impl(file_generator *self, const char *file_name)
{
    struct dirent *dir_ptr;
    DIR *dir = opendir(self->ressources_dir);

    if (dir == NULL)
    {
        perror("opendir() failed");
        return 0;
    }

    while ((dir_ptr = readdir(dir)) != NULL)
    {
        if (dir_ptr->d_type == DT_REG)
        {
            if (strcmp(dir_ptr->d_name, file_name) == 0)
            {
                closedir(dir);
                return 1;
            }
        }
    }

    closedir(dir);
    return 0;
}

/* Method: get_file_size_impl
* Argument(self): Pointer to the struct object to access members 
* Argument(file_name): Name of file to use, ex. "index.html"
* Brief: Returns the size of a file in the ressource directory
* Return: int
*/
int get_file_size_impl(file_generator *self, const char *file_name)
{
    int num_bytes_file = 0;

    FILE *file_handle = fopen(self->get_file_path(self, file_name), "rb");
    if (file_handle == NULL)
    {
        perror("fopen() failed");
        num_bytes_file = -1;
    }
    else
    {
        fseek(file_handle, 0L, SEEK_END);
        num_bytes_file = ftell(file_handle);
        rewind(file_handle);
        fclose(file_handle);
    }

    return num_bytes_file;
}

/* Method: get_file_path_impl
* Argument(self): Pointer to the struct object to access members 
* Argument(file_name): Name of file to use, ex. "index.html"
* Brief: Returns the full path of a file in the ressource directory
* Return: char*
* Note: User must free memory allocated
*/
char* get_file_path_impl(file_generator *self, const char *file_name)
{
    size_t len_dir = strlen(self->ressources_dir);
    size_t len_file = strlen(file_name);
    size_t tot_size = len_dir + len_file + 2;

    char* full_path = (char*)calloc(tot_size, sizeof(char*));
    if (full_path == NULL)
    {
        perror("calloc() failed");
        return NULL;
    }

    snprintf(full_path, tot_size, "%s/%s", self->ressources_dir, file_name);

    return full_path;
}

/* Method: get_file_content_impl
* Argument(self): Pointer to the struct object to access members 
* Argument(file_name): Name of file to use, ex. "index.html"
* Brief: Returns the content of a specified file
* Return: char*
* Note: User must free memory allocated
*/
char* get_file_content_impl(file_generator *self, const char *file_name)
{
    int file_size = self->get_file_size(self, file_name);
    if (file_size == -1)
    {
        perror("get_file_size() failed");
        return NULL;
    }

    char* file_content = (char*)calloc(file_size + 1, sizeof(char*));
    if (file_content == NULL)
    {
        perror("calloc() failed");
        return NULL;
    }

    FILE *file_handle = fopen(self->get_file_path(self, file_name), "rb");
    if (file_handle == NULL)
    {
        perror("fopen() failed");
        free(file_content);
        return NULL;
    }

    if (fread(file_content, 1, file_size, file_handle) != file_size)
    {
        perror("fread() failed");
        fclose(file_handle);
        free(file_content);
        return NULL;
    }

    fclose(file_handle);
    file_content[file_size] = '\0';

    return file_content;
}

/* Method: file_generator_ctor
* Argument(self): Pointer to the struct object to access members 
* Argument(ressources_dir): Name of directory to store files, ex. "templates"
* Brief: Initializes the struct, mapping functions and setting members
* Return: void
*/
void file_generator_ctor(file_generator *self, const char *ressources_dir)
{
    size_t len_res_dir = strlen(ressources_dir);
    self->ressources_dir = (char*)calloc(len_res_dir + 1, sizeof(char*));
    if (self->ressources_dir == NULL)
    {
        perror("calloc() failed");
        return;
    }
    strcpy(self->ressources_dir, ressources_dir);

    self->file_exist = file_exist_impl;
    self->get_file_size = get_file_size_impl;
    self->get_file_path = get_file_path_impl;
    self->get_file_content = get_file_content_impl;
}

/* Method: file_generator_dtor
* Argument(self): Pointer to the struct object to access members 
* Brief: Frees the allocated members, if not freed already
* Return: void
*/
void file_generator_dtor(file_generator *self)
{
    if (self != NULL)
    {
        if (self->ressources_dir != NULL)
        {
            free(self->ressources_dir);
            self->ressources_dir = NULL;
        }
    }
}

#endif