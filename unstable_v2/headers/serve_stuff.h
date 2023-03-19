#ifndef SERVE_STUFF_H
#define SERVE_STUFF_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tcp_socket.h"
#include "serve_utils.h"
#include "http_parser.h"
#include "file_generator.h"

typedef struct serve_stuff serve_stuff;

struct serve_stuff {
    struct tcp_socket tcp_socket_;
    struct http_parser http_parser_;
    struct file_generator file_generator_;
    void (*run)(serve_stuff*);
};

void run_impl(serve_stuff *self)
{
    self->tcp_socket_.accept(&self->tcp_socket_);

    char* client_request = self->tcp_socket_.read(&self->tcp_socket_);
    printf("request  ============================\n%s\n", client_request);

    char* client_method = self->http_parser_.get_method(&self->http_parser_, client_request);
    printf("method   ============================\n%s\n", client_method);

    char* client_filename = self->http_parser_.get_filename(&self->http_parser_, client_request);
    printf("filename ============================\n%s\n", client_filename);

    int file_exist = self->file_generator_.file_exist(&self->file_generator_, client_filename);
    printf("filebool ============================\n%i\n", file_exist);

    if (file_exist)
    {
        int file_size = self->file_generator_.get_file_size(&self->file_generator_, client_filename);
        printf("filesize ============================\n%i\n", file_size);

        char* body = self->file_generator_.get_file_content(&self->file_generator_, client_filename);
        printf("body     ============================\n%s\n", body);

        char* header = self->http_parser_.get_response_header(&self->http_parser_, 200, "OK", "html", file_size);
        printf("header   ============================\n%s\n", header);

        if (strcmp(client_method, "GET") == 0)
        {
            file_size += 128;
            char* response = (char*)malloc(file_size);
            if (response == NULL) return;
            snprintf(response, file_size, "%s%s", header, body);
            printf("response ============================\n%s\n", response);

            self->tcp_socket_.write(&self->tcp_socket_, response);
        }
        else if (strcmp(client_method, "HEAD") == 0)
        {
            file_size += 128;
            char* response = (char*)malloc(file_size);
            if (response == NULL) return;
            snprintf(response, file_size, "%s", header);
            printf("response ============================\n%s\n", response);

            self->tcp_socket_.write(&self->tcp_socket_, response);
        }
        else if (strcmp(client_method, "POST") == 0)
        {
            char* query = self->http_parser_.get_query(&self->http_parser_, client_request);
            printf("query    ============================\n%s\n", query);

            char* decoded_query = malloc(1024);
            url_decoder(decoded_query, query);
            printf("decoded query ============================\n%s\n", decoded_query);
        }

    }
    else if (strcmp(client_filename, "") == 0)
    {
        const char* default_page = "index.html";

        int file_size = self->file_generator_.get_file_size(&self->file_generator_, default_page);
        printf("filesize ============================\n%i\n", file_size);

        char* body = self->file_generator_.get_file_content(&self->file_generator_, default_page);
        printf("body     ============================\n%s\n", body);

        char* header = self->http_parser_.get_response_header(&self->http_parser_, 200, "OK", "html", file_size);
        printf("header   ============================\n%s\n", header);

        file_size += 128; /*ADD HEADER SIZE*/
        char* response = (char*)malloc(file_size);
        if (response == NULL) return;
        snprintf(response, file_size, "%s%s", header, body);
        printf("response ============================\n%s\n", response);

        self->tcp_socket_.write(&self->tcp_socket_, response);
    }
    else
    {
        int file_size = self->file_generator_.get_file_size(&self->file_generator_, "notfound.html");
        printf("filesize ============================\n%i\n", file_size);

        char* body = self->file_generator_.get_file_content(&self->file_generator_, "notfound.html");
        printf("body     ============================\n%s\n", body);

        char* header = self->http_parser_.get_response_header(&self->http_parser_, 404, "Not Found", "html", file_size);
        printf("header   ============================\n%s\n", header);

        file_size += 128; /*ADD HEADER SIZE*/
        char* response = (char*)malloc(file_size);
        if (response == NULL) return;
        snprintf(response, file_size, "%s%s", header, body);
        printf("response ============================\n%s\n", response);

        self->tcp_socket_.write(&self->tcp_socket_, response);
    }
}

void serve_stuff_ctor(serve_stuff *self, int port, int backlog, const char *resources_dir)
{
    http_parser_ctor(&self->http_parser_);
    tcp_socket_ctor(&self->tcp_socket_, port, backlog);
    file_generator_ctor(&self->file_generator_, resources_dir);

    self->run = run_impl;
    self->tcp_socket_.setup(&self->tcp_socket_);
}

void serve_stuff_dtor(serve_stuff *self)
{
    tcp_socket_dtor(&self->tcp_socket_);
    http_parser_dtor(&self->http_parser_);
    file_generator_dtor(&self->file_generator_);
}

#endif