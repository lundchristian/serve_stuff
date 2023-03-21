#ifndef SERVE_STUFF_H
#define SERVE_STUFF_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tcp_socket.h"
#include "serve_utils.h"
#include "http_parser.h"
#include "file_generator.h"

#define STD_SIZE 128
#define EXT_SIZE 1024

typedef struct serve_stuff serve_stuff;

struct serve_stuff {
    char* default_page;
    char* not_found_page;
    struct tcp_socket tcp_socket_;
    struct http_parser http_parser_;
    struct file_generator file_generator_;
    void (*serve_client)(serve_stuff *self);
    void (*default_request)(serve_stuff *self);
    void (*not_found_request)(serve_stuff *self);
    void (*post_request)(serve_stuff *self, const char *request);
    void (*get_request)(serve_stuff *self, const char *filename);
    void (*head_request)(serve_stuff *self, const char *filename);
};

/* Method: default_request_impl
* Argument(self): Pointer to the struct object to access members 
* Brief: Serves the client a user set default page if user search for "" or "/"
* Return: void
*/
void default_request_impl(serve_stuff *self)
{
    int file_size = self->file_generator_.get_file_size(&self->file_generator_, self->default_page);
    char* body = self->file_generator_.get_file_content(&self->file_generator_, self->default_page);
    char* header = self->http_parser_.get_response_header(&self->http_parser_, 200, "OK", "html", file_size);

    file_size += STD_SIZE;
    char* response = (char*)calloc(file_size, sizeof(char*));
    if (response == NULL)
    {
        perror("calloc() failed");
        return;
    }
    snprintf(response, file_size, "%s%s", header, body);

    self->tcp_socket_.write(&self->tcp_socket_, response);
}

/* Method: not_found_request_impl
* Argument(self): Pointer to the struct object to access members 
* Brief: Serves the client a user set not found page if user search for anything not specified
* Return: void
*/
void not_found_request_impl(serve_stuff *self)
{
    int file_size = self->file_generator_.get_file_size(&self->file_generator_, self->not_found_page);
    char* body = self->file_generator_.get_file_content(&self->file_generator_, self->not_found_page);
    char* header = self->http_parser_.get_response_header(&self->http_parser_, 404, "Not Found", "html", file_size);

    file_size += STD_SIZE;
    char* response = (char*)calloc(file_size, sizeof(char*));
    if (response == NULL)
    {
        perror("calloc() failed");
        return;
    }
    snprintf(response, file_size, "%s%s", header, body);

    self->tcp_socket_.write(&self->tcp_socket_, response);
}

/* Method: head_request_impl
* Argument(self): Pointer to the struct object to access members
* Argument(filename): Chars of a filename, example is "index.html"
* Brief: Serves the client a HTTP HEAD response of said file
* Return: void
*/
void head_request_impl(serve_stuff *self, const char* filename)
{
    int file_size = self->file_generator_.get_file_size(&self->file_generator_, filename);
    char* body = self->file_generator_.get_file_content(&self->file_generator_, filename);
    char* header = self->http_parser_.get_response_header(&self->http_parser_, 200, "OK", "html", file_size);

    file_size += STD_SIZE;
    char* response = (char*)calloc(file_size, sizeof(char*));
    if (response == NULL)
    {
        perror("calloc() failed");
        return;
    }
    snprintf(response, file_size, "%s", header);

    self->tcp_socket_.write(&self->tcp_socket_, response);
}

/* Method: get_request_impl
* Argument(self): Pointer to the struct object to access members
* Argument(filename): Chars of a filename, example is "index.html"
* Brief: Serves the client a HTTP GET response of said file
* Return: void
*/
void get_request_impl(serve_stuff *self, const char* filename)
{
    int file_size = self->file_generator_.get_file_size(&self->file_generator_, filename);
    char* body = self->file_generator_.get_file_content(&self->file_generator_, filename);
    char* header = self->http_parser_.get_response_header(&self->http_parser_, 200, "OK", "html", file_size);

    file_size += STD_SIZE;
    char* response = (char*)calloc(file_size, sizeof(char*));
    if (response == NULL)
    {
        perror("calloc() failed");
        return;
    }
    snprintf(response, file_size, "%s%s", header, body);

    self->tcp_socket_.write(&self->tcp_socket_, response);
}


/* Method: post_request_impl
* Argument(self): Pointer to the struct object to access members
* Argument(query): Chars of a query, example is "key=val&..."
* Brief: Serves the client a HTTP POST response of said query
* Return: void
* Note: Under construction, open for ideas
*/
void post_request_impl(serve_stuff *self, const char *request)
{
    char* query = self->http_parser_.get_query(&self->http_parser_, request);
    char* decoded_query = (char*)calloc(EXT_SIZE, sizeof(char*));
    if (decoded_query == NULL)
    {
        perror("calloc() failed");
        return;
    }
    url_decoder(decoded_query, query);

    int query_len = strlen(decoded_query);
    char* response = (char*)calloc(1024, sizeof(char*));
    if (response == NULL)
    {
        perror("calloc() failed");
        return;
    }
    snprintf(response, 1024, "HTTP/1.1 201 Created\r\nServer: ServeStuff\r\nContent-Type: text/plain\r\nContent-Length: %d\r\n\r\n%s", query_len, decoded_query);
    self->tcp_socket_.write(&self->tcp_socket_, response);
}

/* Method: serve_client_impl
* Argument(self): Pointer to the struct object to access members
* Brief: Functions as the infinity loop runtime of the server
* Return: void
*/
void serve_client_impl(serve_stuff *self)
{
    self->tcp_socket_.accept(&self->tcp_socket_);

    char* client_request = self->tcp_socket_.read(&self->tcp_socket_);
    char* client_method = self->http_parser_.get_method(&self->http_parser_, client_request);
    char* client_filename = self->http_parser_.get_filename(&self->http_parser_, client_request);
    int file_exist = self->file_generator_.file_exist(&self->file_generator_, client_filename);

    if (strcmp(client_method, "POST") == 0) self->post_request(self, client_request);
    else if (file_exist)
    {
        if (strcmp(client_method, "GET") == 0) self->get_request(self, client_filename);
        else if (strcmp(client_method, "HEAD") == 0) self->head_request(self, client_filename);
    }
    else if (strcmp(client_filename, "") == 0) self->default_request(self);
    else self->not_found_request(self);
}

/* Method: serve_stuff_ctor
* Argument(self): Pointer to the struct object to access members
* Argument(port): Chars describing TCP port, ex. "8080"
* Argument(backlog): Integer for setting backlog of clients
* Argument(resources_dir): Chars describing directory containing ressources, ex. "templates"
* Argument(default_page): Chars describing home page, ex. "index.html"
* Argument(not_found_page): Chars describing not found page, ex. "notfound.html"
* Brief: Inits
* Return: void
*/
void serve_stuff_ctor(serve_stuff *self, const char *port, int backlog, const char *resources_dir, const char *default_page, const char *not_found_page)
{
    http_parser_ctor(&self->http_parser_);
    tcp_socket_ctor(&self->tcp_socket_, port, backlog);
    file_generator_ctor(&self->file_generator_, resources_dir);

    size_t len_default = strlen(default_page);
    self->default_page = (char*)calloc(len_default + 1, sizeof(char*));
    if (self->default_page == NULL)
    {
        perror("calloc() failed");
        return;
    }
    strcpy(self->default_page, default_page);

    size_t len_not_found = strlen(not_found_page);
    self->not_found_page = (char*)calloc(len_not_found + 1, sizeof(char*));
    if (self->not_found_page == NULL)
    {
        perror("calloc() failed");
        return;
    }
    strcpy(self->not_found_page, not_found_page);

    self->get_request = get_request_impl;
    self->serve_client = serve_client_impl;
    self->head_request = head_request_impl;
    self->post_request = post_request_impl;
    self->default_request = default_request_impl;
    self->not_found_request = not_found_request_impl;

    self->tcp_socket_.create(&self->tcp_socket_);
    self->tcp_socket_.listen(&self->tcp_socket_);
}

void serve_stuff_dtor(serve_stuff *self)
{
    free(&self->default_page);
    free(&self->not_found_page);
    tcp_socket_dtor(&self->tcp_socket_);
    http_parser_dtor(&self->http_parser_);
    file_generator_dtor(&self->file_generator_);
}

#endif