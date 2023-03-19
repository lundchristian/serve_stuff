#include "tcp_socket.h"

int main()
{
    tcp_socket t;
    tcp_socket_ctor(&t, "9000", 5);
    t.create(&t);
    t.listen(&t);
    t.accept(&t);
}