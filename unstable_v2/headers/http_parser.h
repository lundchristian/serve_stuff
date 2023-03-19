#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct http_parser http_parser;

struct http_parser {
    char* (*get_query)(http_parser *self, const char* request_header);
    char* (*get_method)(http_parser *self, const char* request_header);
    char* (*get_filename)(http_parser *self, const char* request_header);
    char* (*get_response_header)(http_parser *self, int return_code, const char* status_code, const char* content_type, int file_size);
};

char* get_query_impl(http_parser *self, const char* request_header)
{
    const char* body_start = strstr(request_header, "\r\n\r\n");
    if (!body_start) return NULL;
    body_start += 4;
    char* query = strdup(body_start);

    return query;
}

char* get_method_impl(http_parser *self, const char* request_header)
{
    const char* space_pos = strchr(request_header, ' ');
    if (space_pos == NULL) return NULL;

    size_t method_len = space_pos - request_header;

    char* http_method = (char*) malloc(method_len + 1);
    if (http_method == NULL) return NULL;

    strncpy(http_method, request_header, method_len);
    http_method[method_len] = '\0';

    return http_method;
}

char* get_filename_impl(http_parser *self, const char* request_header)
{
    char *start = strchr(request_header, ' ');
    if (start == NULL) return NULL;
    start++;
    char *end = strchr(start, ' ');
    if (end == NULL) return NULL;

    size_t len = end - start;
    char* filename = (char*)malloc(len + 1);
    if (filename == NULL) return NULL;

    strncpy(filename, start, len);
    filename[len] = '\0';

    char *slash_pos = strrchr(filename, '/');
    if (slash_pos != NULL) filename = slash_pos + 1;

    char *question_mark_pos = strchr(filename, '?');
    if (question_mark_pos != NULL) *question_mark_pos = '\0';

    return filename;
}

char* get_response_header_impl(http_parser *self, int return_code, const char* status_code, const char* content_type, int file_size)
{
    char* header = (char*)malloc(128);
    if (header == NULL) return NULL;

    snprintf(header, 128, "HTTP/1.1 %d %s\r\nServer: ServeStuff\r\nContent-Type: text/%s\r\nContent-Length: %d\r\n\r\n", return_code, status_code, content_type, file_size);

    return header;
}

void http_parser_ctor(http_parser *self)
{
    self->get_query = get_query_impl;
    self->get_method = get_method_impl;
    self->get_filename = get_filename_impl;
    self->get_response_header = get_response_header_impl;
}

void http_parser_dtor(http_parser *self){ /*NOTHING*/ }

#endif