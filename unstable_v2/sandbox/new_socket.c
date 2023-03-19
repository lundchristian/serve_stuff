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

#define PORT "9000"
#define BACKLOG 10

void sigchld_handler(int s)
{
	(void)s;
	int saved_errno = errno;
	while(waitpid(-1, NULL, WNOHANG) > 0);
	errno = saved_errno;
}

void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) return &(((struct sockaddr_in*)sa)->sin_addr);

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
	int server_socket, client_socket, yes = 1;
	struct addrinfo hints, *server_info, *server_list;
	struct sockaddr_storage client_addr;
	socklen_t sin_size;
	struct sigaction sa;
	char s[INET6_ADDRSTRLEN];
	int status_code;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if ((status_code = getaddrinfo(NULL, PORT, &hints, &server_info)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status_code));
        return 1;
    }

	for(server_list = server_info; server_list != NULL; server_list = server_list->ai_next)
    {
		if ((server_socket = socket(server_list->ai_family, server_list->ai_socktype, server_list->ai_protocol)) == -1)
        {
			perror("server: socket");
			continue;
		}

		if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
        {
			perror("setsockopt");
			exit(1);
		}

		if (bind(server_socket, server_list->ai_addr, server_list->ai_addrlen) == -1)
        {
			close(server_socket);
			perror("server: bind");
			continue;
		}

		break;
	}

	freeaddrinfo(server_info);

	if (server_list == NULL)
    {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	if (listen(server_socket, BACKLOG) == -1)
    {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1)
    {
		perror("sigaction");
		exit(1);
	}

	printf("server: waiting for connections...\n");

	while(1)
    {
		sin_size = sizeof client_addr;
		client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &sin_size);
		if (client_socket == -1)
        {
			perror("accept");
			continue;
		}

		inet_ntop(client_addr.ss_family, get_in_addr((struct sockaddr *)&client_addr), s, sizeof s);
		printf("server: got connection from %s\n", s);

		if (!fork())
        {
			close(server_socket);
			if (send(client_socket, "Hello, world!", 13, 0) == -1) perror("send");
			close(client_socket);
			exit(0);
		}
		close(client_socket);
	}

	return 0;
}