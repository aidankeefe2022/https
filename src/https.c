//
// Created by aidankeefe on 5/19/26.
//
#include "../include/https.h"

#include <strings.h>

#include "../include/deserializeResponse.h"

struct https_route {
    struct aid_string routeName;
    https_method method;
    https_callback callback;
};

static struct aid_LinkedList routes = {0};
static struct aid_AidAllocator allocator = {0};


void https_reg_route(https_method method, struct aid_string routeName, https_callback callback) {
    struct https_route* r = aid_AidAlloc(&allocator, sizeof(struct https_route));
    *r = (struct https_route){routeName, method, callback};
    aid_push(&routes, (void*)r, VOIDPTR);
}

static int set_nonblocking(int sockfd) {
    // 1. Get the current flags
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl(F_GETFL)");
        return -1;
    }

    // 2. Add the O_NONBLOCK flag to the current flags using bitwise OR
    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl(F_SETFL)");
        return -1;
    }

    return 0;
}

void(*getRouteCallback(struct https_ctx* req, WOLFSSL* ssl))(WOLFSSL*, https_ctx*) {

    i32 idx = 0;
    bool hasQueryParams = false;
    if (req->request->method == GET) {
        for (; idx < req->request->route.length ; idx++) {
            if (req->request->route.s[idx] == '?') {
                hasQueryParams = true;
                break;
            }
        }

    }

    struct aid_string routeName;

    if (hasQueryParams) {
        routeName = req->request->route;
        routeName.length = idx;
    }else {
        routeName = req->request->route;
    }

    for (struct aid_LLNode* node = routes.head; node != nullptr; node = node->next ) {
        struct https_route* route = node->VOIDPTR;
        if (aid_str_cmp(&routeName, &route->routeName)) {
            return route->callback;
        }
    }

    return nullptr;
}


void handelRequest(int client, WOLFSSL_CTX* wctx) {

    u8 arena_buffer[MiB(1)] = "";
    aid_Arena arena = {arena_buffer, MiB(1), arena_buffer};
    WOLFSSL* ssl = wolfSSL_new(wctx);
    if (!ssl) {
        fprintf(stderr, "wolfSSL_new failed\n");
        goto CLIENT_ERROR;
    }

    wolfSSL_set_fd(ssl, client);

    i32 ret = wolfSSL_accept(ssl);
    char errorBuffer[80];
    if (ret != SSL_SUCCESS) {
        i32 err = wolfSSL_get_error(ssl, ret);
        printf("error = %d, %s\n", err, wolfSSL_ERR_error_string(err, errorBuffer));
        goto ACCEPT_ERROR;
    }

    char buffer[KiB(5)] = "";

    u64 size = wolfSSL_read(ssl, buffer, KiB(5));

    if (size <= 0) {
        int err = wolfSSL_get_error(ssl, size);
        fprintf(stderr, "wolfSSL_read failed, error: %d\n", err);
    }

    auto req = https_serializeRequest(&arena, (struct aid_string){KiB(5),size-1, 0, buffer });



    https_ctx ctx = {.request = req};

    auto func = getRouteCallback(&ctx, ssl);

    if (func == nullptr) {
        fprintf(stderr, "Invalid Route Hit\n");
    }else {
        func(ssl, &ctx);
    }

    https_freeRequest(req);
    wolfSSL_shutdown(ssl);
    ACCEPT_ERROR:
    wolfSSL_free(ssl);
    CLIENT_ERROR:
    close(client);
}



void https_listen(u64 port, https_certs certs, u64 options) {
    i32 initCode = wolfSSL_Init();
    auto method = wolfSSLv23_server_method();
    auto wctx = wolfSSL_CTX_new(method);
    /* Load server certificates into WOLFSSL_CTX */
    if (wolfSSL_CTX_use_certificate_file(wctx, certs.cert_path,
            SSL_FILETYPE_PEM) != SSL_SUCCESS) {
        fprintf(stderr, "Error loading ../certs/server-cert.pem, please"
            "check the file.\n");
        exit(EXIT_FAILURE);
            }

    /* Load keys */
    if (wolfSSL_CTX_use_PrivateKey_file(wctx, certs.private_key,
            SSL_FILETYPE_PEM) != SSL_SUCCESS) {
        fprintf(stderr, "Error loading ../certs/server-key.pem, please check"
            "the file.\n");
        exit(EXIT_FAILURE);
            }
    SOCKADDR_IN addr;
    int socket_fd;
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return;
    }
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int optval = 1;
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    set_nonblocking(socket_fd);

    if ((bind(socket_fd, (struct sockaddr*)&addr, sizeof(addr))) < 0) {
        return;
    }

    listen(socket_fd, 1000);

    int client;
    while (true) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);

        client = accept(
            socket_fd,
            (struct sockaddr*)&client_addr,
            &client_addr_len
        );

        if (client < 0) {
            continue;
        }

        handelRequest(client, wctx);

    }
}
