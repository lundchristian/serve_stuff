#ifndef TCP_SOCKET_H
#define TCP_SOCKET_H

#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define READ_BUFFER 1024

typedef struct tcp_socket tcp_socket;

struct tcp_socket {
    char* port;
    int backlog;
    int new_socket;
    int base_socket;
    socklen_t address_size;
	struct addrinfo address_info;
    struct addrinfo *server_info;
    struct addrinfo *server_list;
	struct sockaddr_storage client_addr;

    void (*create)(tcp_socket*);
    void (*listen)(tcp_socket*);
    void (*accept)(tcp_socket*);

    char* (*read)(tcp_socket*);
    void (*write)(tcp_socket*, const char*);
};

void create_socket(tcp_socket *self)
{
    if (getaddrinfo(NULL, self->port, &self->address_info, &self->server_info) != 0)
    {
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }

    for(self->server_list = self->server_info; self->server_list != NULL; self->server_list = self->server_list->ai_next)
    {
		if ((self->base_socket = socket(self->server_list->ai_family, self->server_list->ai_socktype, self->server_list->ai_protocol)) == -1)
        {
			perror("socket() failed");
			continue;
		}

        int yes = 1;
		if (setsockopt(self->base_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
        {
			perror("setsockopt() failed");
			exit(1);
		}

		if (bind(self->base_socket, self->server_list->ai_addr, self->server_list->ai_addrlen) == -1)
        {
			close(self->base_socket);
			perror("one bind() failed");
			continue;
		}

		break;
	}
    freeaddrinfo(self->server_info);
    
    if (self->server_list == NULL)
    {
		perror("bind() failed");
		exit(1);
	}
}

void listen_socket(tcp_socket *self)
{
    int status = listen(self->base_socket, self->backlog);

    if (status == -1)
    {
        perror("listen() failed");
        exit(1);
    }
    else printf("[+] Socket listening\n");
}

void accept_socket(tcp_socket *self)
{
    self->new_socket = accept(self->base_socket, (struct sockaddr*)&self->client_addr, &self->address_size);

    if (self->new_socket == -1)
    {
        perror("accept() failed");
        exit(1);
    }
    else printf("[+] Socket accepted\n");
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

void tcp_socket_ctor(tcp_socket *self, char* port, int backlog)
{
    self->port = port;
    self->new_socket = 0;
    self->base_socket = 0;
    self->backlog = backlog;
    self->address_size = sizeof(self->client_addr);

	memset(&self->address_info, 0, sizeof(self->address_info));
	self->address_info.ai_family = AF_UNSPEC;
	self->address_info.ai_socktype = SOCK_STREAM;
	self->address_info.ai_flags = AI_PASSIVE;

    self->create = create_socket;
    self->listen = listen_socket;
    self->accept = accept_socket;

    self->read = read_tcp;
    self->write = write_tcp;
}

void tcp_socket_dtor(tcp_socket *self)
{
    close(self->base_socket);
}

void *get_conn_address(struct sockaddr *sock_addr)
{
	if (sock_addr->sa_family == AF_INET) return &(((struct sockaddr_in*)sock_addr)->sin_addr);

	return &(((struct sockaddr_in6*)sock_addr)->sin6_addr);
}

#endif