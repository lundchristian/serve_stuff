#ifndef FILE_GENERATOR_H
#define FILE_GENERATOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

typedef struct file_generator file_generator;

struct file_generator {
    char* ressources_dir;
    
    int (*file_exist)(file_generator *self, const char* file_name);
    int (*get_file_size)(file_generator *self, const char* file_name);

    char* (*get_file_path)(file_generator *self, const char* file_name);
    char* (*get_file_content)(file_generator *self, const char* file_name);
};

int file_exist_impl(file_generator *self, const char* file_name)
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

int get_file_size_impl(file_generator *self, const char* file_name)
{
    int num_bytes_file = 0;

    FILE *file_handle = fopen(self->get_file_path(self, file_name), "rb");
    if (file_handle == NULL) num_bytes_file = -1;
    else
    {
        fseek(file_handle, 0L, SEEK_END);
        num_bytes_file = ftell(file_handle);
        rewind(file_handle);
        fclose(file_handle);
    }

    return num_bytes_file;
}

char* get_file_path_impl(file_generator *self, const char* file_name)
{
    size_t len_dir = strlen(self->ressources_dir);
    size_t len_file = strlen(file_name);

    char* path = (char*)malloc(len_dir + len_file + 1);
    if (path == NULL) return NULL;

    snprintf(path, (len_dir + len_file + 2), "%s/%s", self->ressources_dir, file_name);

    return path;
}

char* get_file_content_impl(file_generator *self, const char* file_name)
{
    int file_size = self->get_file_size(self, file_name);
    if (file_size == -1) return NULL;

    char* content = (char*)malloc(file_size * sizeof(char) + 1);
    if (content == NULL) return NULL;

    FILE *file_handle = fopen(self->get_file_path(self, file_name), "rb");
    if (file_handle == NULL)
    {
        free(content);
        return NULL;
    }

    if (fread(content, 1, file_size, file_handle) != file_size)
    {
        fclose(file_handle);
        free(content);
        return NULL;
    }

    fclose(file_handle);
    content[file_size] = '\0';

    return content;
}

void file_generator_ctor(file_generator *self, const char* ressources_dir)
{
    size_t len_res_dir = strlen(ressources_dir);
    self->ressources_dir = (char*)malloc((len_res_dir + 1) * sizeof(char));
    if (self->ressources_dir == NULL) return;
    strcpy(self->ressources_dir, ressources_dir);

    self->file_exist = file_exist_impl;
    self->get_file_size = get_file_size_impl;
    self->get_file_path = get_file_path_impl;
    self->get_file_content = get_file_content_impl;
}

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