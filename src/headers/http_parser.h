#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HTTP_HEADER_SIZE 256

typedef struct http_parser http_parser;

struct http_parser {
    char* (*get_query)(http_parser *self, const char *http_request);
    char* (*get_method)(http_parser *self, const char *http_request);
    char* (*get_filename)(http_parser *self, const char *http_request);
    char* (*get_response_header)(http_parser *self, int return_code, const char *status_code, const char *content_type, int file_size);
};

/* Method: get_query_impl
* Argument(self): Pointer to the struct object to access members 
* Argument(http_request): HTTP request from client
* Brief: Searches for query in HTTP request body and returns it
* Return: char*
* Note: User must free memory allocated
*/
char* get_query_impl(http_parser *self, const char *http_request)
{
    const char* body_start = strstr(http_request, "\r\n\r\n");
    if (body_start == NULL)
    {
        perror("strstr() failed");
        return NULL;
    }
    body_start += 4;
    char* http_query = strdup(body_start);

    return http_query;
}

/* Method: get_method_impl
* Argument(self): Pointer to the struct object to access members 
* Argument(http_request): HTTP request from client
* Brief: Searches for method in HTTP request and returns it
* Return: char*
* Note: User must free memory allocated
*/
char* get_method_impl(http_parser *self, const char *http_request)
{
    const char* space_pos = strchr(http_request, ' ');
    if (space_pos == NULL)
    {
        perror("strchr() failed");
        return NULL;
    }

    size_t method_len = space_pos - http_request;
    char* http_method = (char*)calloc(method_len + 1, sizeof(char*));
    if (http_method == NULL)
    {
        perror("calloc() failed");
        return NULL;
    }

    strncpy(http_method, http_request, method_len);
    http_method[method_len] = '\0';

    return http_method;
}

/* Method: get_filename_impl
* Argument(self): Pointer to the struct object to access members 
* Argument(http_request): HTTP request from client
* Brief: Searches for the last filename in HTTP request and returns it
* Return: char*
* Note: User must free memory allocated
*/
char* get_filename_impl(http_parser *self, const char *http_request)
{
    char *start = strchr(http_request, ' ');
    if (start == NULL)
    {
        perror("strchr() failed");
        return NULL;
    }
    start++;

    char *end = strchr(start, ' ');
    if (end == NULL)
    {
        perror("strchr() failed");
        return NULL;
    }

    size_t filename_len = end - start;
    char* filename = (char*)calloc(filename_len + 1, sizeof(char*));
    if (filename == NULL)
    {
        perror("calloc() failed");
        return NULL;
    }

    strncpy(filename, start, filename_len);
    filename[filename_len] = '\0';

    char *slash_pos = strrchr(filename, '/');
    if (slash_pos != NULL) filename = slash_pos + 1;

    char *question_mark_pos = strchr(filename, '?');
    if (question_mark_pos != NULL) *question_mark_pos = '\0';

    return filename;
}

/* Method: get_response_header_impl
* Argument(self): Pointer to the struct object to access members 
* Argument(return_code): Integer describing HTTP return code, i.e. 200, 304, 404...
* Argument(status_code): Chars describing HTTP status code, i.e. OK, Moved, Not Found...
* Argument(content_type): Chars describing HTTP content type, i.e. plain, html...
* Argument(file_size): Integer descirbing size of body in bytes, i.e. "Hello" == 5
* Brief: Creates and returns a HTTP response header
* Return: char*
* Note: User must free memory allocated
*/
char* get_response_header_impl(http_parser *self, int return_code, const char *status_code, const char *content_type, int file_size)
{
    char* http_header = (char*)calloc(HTTP_HEADER_SIZE, sizeof(char*));
    if (http_header == NULL)
    {
        perror("calloc() failed");
        return NULL;
    }

    snprintf(http_header, HTTP_HEADER_SIZE, "HTTP/1.1 %d %s\r\nServer: ServeStuff\r\nContent-Type: text/%s\r\nContent-Length: %d\r\n\r\n", return_code, status_code, content_type, file_size);

    return http_header;
}

/* Method: http_parser_ctor
* Argument(self): Pointer to the struct object to access members 
* Brief: Initializes the struct by mapping functions
* Return: void
*/
void http_parser_ctor(http_parser *self)
{
    self->get_query = get_query_impl;
    self->get_method = get_method_impl;
    self->get_filename = get_filename_impl;
    self->get_response_header = get_response_header_impl;
}

/* Method: http_parser_dtor
* Argument(self): Pointer to the struct object to access members 
* Brief: Nothing to free
* Return: void
*/
void http_parser_dtor(http_parser *self){ /*NOTHING*/ }

#endif