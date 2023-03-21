#include "tcp_socket.h"
#include "http_parser.h"
#include "serve_stuff.h"
#include "serve_utils.h"
#include "file_generator.h"

int main(int argc, char **argv)
{
    struct serve_stuff app;

    int backlog = 10;
    const char* port = argv[1];
    const char* home_page = "index.html";
    const char* ressource_dir = "templates";
    const char* not_found_page = "notfound.html";

    serve_stuff_ctor(&app, port, backlog, ressource_dir, home_page, not_found_page);

    while (1)
    {
        app.serve_client(&app);
    }
    serve_stuff_dtor(&app);
}