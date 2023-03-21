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

    void (*create)(tcp_socket *self);
    void (*listen)(tcp_socket *self);
    void (*accept)(tcp_socket *self);

    char* (*read)(tcp_socket *self);
    void (*write)(tcp_socket *self, const char *message);
};

/* Method: create_socket
* Argument(self): Pointer to the struct object to access members 
* Brief: Creates and binds a new base socket from a list of alternatives
* Return: void
*/
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
        else printf("[+] Socket created\n");


        int yes = 1;
		if (setsockopt(self->base_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
        {
			perror("setsockopt() failed");
			exit(EXIT_FAILURE);
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
		exit(EXIT_FAILURE);
	}
    else printf("[+] Socket bound\n");
}

/* Method: listen_socket
* Argument(self): Pointer to the struct object to access members 
* Brief: Listens for new socket connections
* Return: void
*/
void listen_socket(tcp_socket *self)
{
    int status = listen(self->base_socket, self->backlog);

    if (status == -1)
    {
        perror("listen() failed");
        exit(EXIT_FAILURE);
    }
    else printf("[+] Socket listening\n");
}

/* Method: accept_socket
* Argument(self): Pointer to the struct object to access members 
* Brief: Accepts new socket connections
* Return: void
*/
void accept_socket(tcp_socket *self)
{
    self->new_socket = accept(self->base_socket, (struct sockaddr*)&self->client_addr, &self->address_size);

    if (self->new_socket == -1)
    {
        perror("accept() failed");
        exit(EXIT_FAILURE);
    }
    else printf("[+] Socket accepted\n");
}

/* Method: read_tcp
* Argument(self): Pointer to the struct object to access members 
* Brief: Reads from the newly accepted socket connection, and returns the message
* Return: char*
* Note: User must free memory allocated
*/
char* read_tcp(tcp_socket *self)
{
    char* buffer = (char*)calloc(READ_BUFFER, sizeof(char*));
    if (buffer == NULL)
    {
        perror("calloc() failed");
        return NULL;
    }

    int bytes_read = recv(self->new_socket, buffer, READ_BUFFER, 0);
    if (bytes_read == -1)
    { 
        if (errno == EAGAIN || errno == EINTR)
        {
            perror("recv() failed");
            free(buffer);
            close(self->new_socket);
            return NULL;
        }
    }

    buffer[bytes_read] = '\0';

    return buffer;
}

/* Method: write_tcp
* Argument(self): Pointer to the struct object to access members 
* Argument(message): Chars to send to the accepted socket 
* Brief: Writes to the newly accepted socket connection, and closes the connection
* Return: void
* Note: User must free memory allocated
*/
void write_tcp(tcp_socket *self, const char *message)
{
    size_t message_len = strlen(message);
    int bytes_wrote = write(self->new_socket, message, message_len);
    if (bytes_wrote == -1)
    {
        if (errno == EAGAIN || errno == EINTR)
        {
            perror("write() failed");
            close(self->new_socket);
            return;
        }
    }

    close(self->new_socket);
    printf("[+] Socket removed\n");
}

/* Method: tcp_socket_ctor
* Argument(self): Pointer to the struct object to access members 
* Argument(port): Name of the port to use, ex. "8080"
* Argument(backlog): The backlog of the socket, ex. 10
* Brief: Inits the TCP server by setting port, backlog and IP
* Return: void
*/
void tcp_socket_ctor(tcp_socket *self, const char *port, int backlog)
{
    size_t port_len = strlen(port);
    self->port = (char*)calloc(port_len + 1, sizeof(char*));
    if (self->port == NULL)
    {
        perror("calloc() failed");
        return;
    }
    strcpy(self->port, port);

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

/* Method: tcp_socket_dtor
* Argument(self): Pointer to the struct object to access members
* Brief: Frees the possible memory and closes the base socket
* Return: void
*/
void tcp_socket_dtor(tcp_socket *self)
{
    free(self->port);
    close(self->base_socket);
}

/* Method: get_conn_address
* Argument(sock_addr): Pointer to the struct object to access members
* Brief: Wrapper to get address of connecting socket
* Return: void*
*/
void *get_conn_address(struct sockaddr *sock_addr)
{
	if (sock_addr->sa_family == AF_INET) return &(((struct sockaddr_in*)sock_addr)->sin_addr);

	return &(((struct sockaddr_in6*)sock_addr)->sin6_addr);
}

#endif