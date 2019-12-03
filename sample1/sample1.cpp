// sample1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"

#pragma comment(lib, "libuv.lib")
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "psapi.lib") 
#pragma comment(lib, "userenv.lib") 


#define PORT 8000

typedef struct {
    uv_write_t req;
    uv_buf_t buf;
} write_req_t;

static uint64_t data_cntr = 0;


static void on_close(uv_handle_t* handle) {
    free(handle);
}


static void after_write(uv_write_t* req, int status) {
    write_req_t* wr = (write_req_t*)req;

    //fprintf(stdout, "after_write %s\n", wr->buf.base);

    if (wr->buf.base != NULL)
        free(wr->buf.base);
    free(wr);

    if (status == 0)
        return;

    fprintf(stderr, "uv_write error: %s\n", uv_strerror(status));

    if (status == UV_ECANCELED)
        return;

    assert(status == UV_EPIPE);
    uv_close((uv_handle_t*)req->handle, on_close);
}


static void after_shutdown(uv_shutdown_t* req, int status) {
    /*assert(status == 0);*/
    if (status < 0)
        fprintf(stderr, "err: %s\n", uv_strerror(status));
    data_cntr = 0;
    uv_close((uv_handle_t*)req->handle, on_close);
    free(req);
}


static void after_read(uv_stream_t* handle,
    ssize_t nread,
    const uv_buf_t* buf) {
    int r;
    write_req_t* wr;
    uv_shutdown_t* req;

    if (nread <= 0 && buf->base != NULL)
        free(buf->base);

    if (nread == 0)
        return;

    if (nread < 0) {
        /*assert(nread == UV_EOF);*/
        fprintf(stderr, "err: %s\n", uv_strerror(nread));

        req = (uv_shutdown_t*)malloc(sizeof(*req));
        assert(req != NULL);

        r = uv_shutdown(req, handle, after_shutdown);
        assert(r == 0);

        return;
    }

    fprintf(stdout, "data read: %llu bytes\n", nread); // / 1024 / 1024);

    data_cntr += nread;

    wr = (write_req_t*)malloc(sizeof(*wr));
    assert(wr != NULL);

    wr->buf = uv_buf_init(buf->base, nread);

    fprintf(stdout, "uv_writing\n");
    r = uv_write(&wr->req, handle, &wr->buf, 1, after_write);
    fprintf(stdout, "uv_write %d\n", r);
    assert(r == 0);
}


static void alloc_cb(uv_handle_t* handle,
    size_t suggested_size,
    uv_buf_t* buf) {
    buf->base = (char*)malloc(suggested_size);
    assert(buf->base != NULL);
    buf->len = suggested_size;
}


static void on_connection(uv_stream_t* server, int status) {
    uv_tcp_t* stream;
    int r;

    assert(status == 0);

    stream = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
    assert(stream != NULL);

    r = uv_tcp_init(uv_default_loop(), stream);
    fprintf(stdout, "uv_tcp_init %d\n", r);
    assert(r == 0);

    stream->data = server;

    r = uv_accept(server, (uv_stream_t*)stream);
    fprintf(stdout, "uv_accept %d\n", r);
    assert(r == 0);

    r = uv_read_start((uv_stream_t*)stream, alloc_cb, after_read);
    fprintf(stdout, "uv_read_start %d\n", r);
    assert(r == 0);
}


static int tcp_echo_server() {
    uv_tcp_t* tcp_server;
    struct sockaddr_in addr;
    int r;

    r = uv_ip4_addr("0.0.0.0", PORT, &addr);
    fprintf(stdout, "uv_ip4_addr %d\n", r);
    assert(r == 0);

    tcp_server = (uv_tcp_t*)malloc(sizeof(*tcp_server));
    assert(tcp_server != NULL);

    r = uv_tcp_init(uv_default_loop(), tcp_server);
    fprintf(stdout, "uv_tcp_init %d\n", r);
    assert(r == 0);

    r = uv_tcp_bind(tcp_server, (const struct sockaddr*)&addr, 0);
    fprintf(stdout, "uv_tcp_bind %d\n", r);
    assert(r == 0);

    r = uv_listen((uv_stream_t*)tcp_server, SOMAXCONN, on_connection);
    fprintf(stdout, "uv_listen %d\n", r);
    assert(r == 0);

    return 0;
}


int main() {
    int r;

    r = tcp_echo_server();
    fprintf(stdout, "tcp_echo_server %d\n", r);
    assert(r == 0);

    r = uv_run(uv_default_loop(), UV_RUN_DEFAULT);
    fprintf(stdout, "uv_run %d\n", r);
    assert(r == 0);

    return 0;
}

/*
int64_t counter = 0;

void wait_for_a_while(uv_idle_t* handle) {
    counter++;

//    printf("Idling...%lld\n", counter);

//    if (counter >= 10e6)
//        uv_idle_stop(handle);
    if (counter == 10e5)
    {
        printf("Signalling...\n");
        uv_idle_stop(handle);
    }
}

int main() {
    uv_idle_t idler;

    uv_idle_init(uv_default_loop(), &idler);
    uv_idle_start(&idler, wait_for_a_while);

    printf("Idling...\n");
    uv_run(uv_default_loop(), UV_RUN_DEFAULT);
    printf("Signaled...\n");

    uv_loop_close(uv_default_loop());
    return 0;
}
*/
/*
int main()
{
    printf("Starting.\n");

    uv_loop_t *loop = (uv_loop_t *)malloc(sizeof(uv_loop_t));
    uv_loop_init(loop);

    printf("Now quitting.\n");
    uv_run(loop, UV_RUN_DEFAULT);

    uv_loop_close(loop);
    free(loop);

    return 0;
}
*/
// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
