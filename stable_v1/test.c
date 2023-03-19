#include "tcp_socket.h"
#include "http_parser.h"
#include "serve_stuff.h"
#include "file_generator.h"

void test_tcp_socket()
{
    tcp_socket connection;
    tcp_socket_ctor(&connection, 9000, 10);
    connection.setup(&connection);
    connection.accept(&connection);
    printf("test tcp_socket\n%s\n", connection.read(&connection));
    tcp_socket_dtor(&connection);
}

void test_http_parser()
{
    const char* req =
        "GET /more/nyhos.html HTTP/1.1\r\n"
        "Content-type: text/html, text/plain\r\n"
        "Other-stuff: More fillers\r\n";

    http_parser par;
    http_parser_ctor(&par);

    char* method = par.get_method(&par, req);
    printf("test get_method\n%s\n", method);

    char* filename = par.get_filename(&par, req);
    printf("test get_filename\n%s\n", filename);

    char* response = par.get_response_header(&par, 200, "OK", "html", 128);
    printf("test get_response\n%s\n", response);

    http_parser_dtor(&par);
    free(method);
    free(filename);
    free(response);
}

void test_file_generator()
{
    const char* file_name = "index.html";
    file_generator fgen;
    file_generator_ctor(&fgen, "templates");
    printf("test file_exist = %i\n", fgen.file_exist(&fgen, file_name));
    printf("test file_path\n%s\n", fgen.get_file_path(&fgen, file_name));
    printf("test file_size = %i\n", fgen.get_file_size(&fgen, file_name));
    printf("test file_content\n%s\n", fgen.get_file_content(&fgen, file_name));
    file_generator_dtor(&fgen);
}

void test_serve_stuff()
{
    serve_stuff app;
    serve_stuff_ctor(&app, 8080, 10, "templates");
    while (1)
    {
        app.run(&app);
    }
    serve_stuff_dtor(&app);
}

int main()
{
    // test_tcp_socket();
    test_serve_stuff();
    // test_http_parser();
    // test_file_generator();
}