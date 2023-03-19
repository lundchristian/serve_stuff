#ifndef TCP_SOCKET_H
#define TCP_SOCKET_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define READ_BUFFER 1024

typedef struct tcp_socket tcp_socket;

struct tcp_socket {
    int new_socket;
    int base_socket;
    int backlog;
    struct sockaddr_in address;
    int address_size;

    void (*bind)(tcp_socket*);
    void (*create)(tcp_socket*);
    void (*listen)(tcp_socket*);
    void (*accept)(tcp_socket*);
    void (*setup)(tcp_socket*);

    char* (*read)(tcp_socket*);
    void (*write)(tcp_socket*, const char*);
};

void create_socket(tcp_socket *self)
{
    self->base_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (self->base_socket < 0)
    {
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }
    else printf("[+] Socket created\n");
}

void bind_socket(tcp_socket *self)
{
    int status = bind(self->base_socket, (struct sockaddr*)&self->address, sizeof(self->address));

    if (status < 0)
    {
        perror("bind() failed");
        exit(EXIT_FAILURE);
    }
    else printf("[+] Socket bound\n");
}

void listen_socket(tcp_socket *self)
{
    int status = listen(self->base_socket, self->backlog);

    if (status < 0)
    {
        perror("listen() failed");
        exit(EXIT_FAILURE);
    }
    else printf("[+] Socket listening\n");
}

void accept_socket(tcp_socket *self)
{
    self->new_socket = accept(self->base_socket, (struct sockaddr*)&self->address, (socklen_t*)&self->address_size);

    if (self->new_socket < 0)
    {
        perror("accept() failed");
        exit(EXIT_FAILURE);
    }
    else printf("[+] Socket accepted\n");
}

void setup_socket(tcp_socket *self)
{
    self->create(self);
    self->bind(self);
    self->listen(self);
}

char* read_tcp(tcp_socket *self)
{
    char* buffer = (char*)malloc(READ_BUFFER * sizeof(char));
    if (buffer == NULL) return NULL;

    int bytes_read = read(self->new_socket, buffer, READ_BUFFER);
    if (bytes_read == -1)
    { 
        if (errno == EAGAIN || errno == EINTR)
        {
            free(buffer);
            return NULL;
        }
    }

    buffer[bytes_read] = '\0';

    return buffer;
}

void write_tcp(tcp_socket *self, const char* message)
{
    size_t message_len = strlen(message);
    int bytes_wrote = write(self->new_socket, message, message_len);
    if (bytes_wrote == -1)
    {
        if (errno == EAGAIN || errno == EINTR)
        {
            perror("write() failed");
            return;
        }
    }

    close(self->new_socket);
}

void tcp_socket_ctor(tcp_socket *self, int port, int backlog)
{
    self->new_socket = 0;
    self->base_socket = 0;
    self->backlog = backlog;

    self->address_size = sizeof(self->address);
    self->address.sin_family = AF_INET;
    self->address.sin_addr.s_addr = INADDR_ANY;
    self->address.sin_port = htons(port);

    self->bind = bind_socket;
    self->create = create_socket;
    self->listen = listen_socket;
    self->accept = accept_socket;
    self->setup = setup_socket;

    self->read = read_tcp;
    self->write = write_tcp;
}

void tcp_socket_dtor(tcp_socket *self)
{
    close(self->base_socket);
}

#endif