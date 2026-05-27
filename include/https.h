//
// Created by aidankeefe on 5/19/26.
//

#ifndef HTTPS_HTTPS_H
#define HTTPS_HTTPS_H

#include <wolfssl/options.h>
#include <wolfssl/ssl.h>
#include <LibAidan/AidanAidAllocator.h>
#include <LibAidan/AidanArena.h>
#include "serializeRequest.h"
#include "deserializeResponse.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>


typedef struct https_routes https_routes;
typedef struct https_ctx {
    struct https_request* request;
}https_ctx;
typedef void(*https_callback)(WOLFSSL*, https_ctx*) ;


typedef struct https_certs {
    char* ca_cert_path;
    char* cert_path;
    char* private_key;
}https_certs;

void https_listen(u64 port, https_certs certs, u64 options);
void https_reg_route(https_method method, struct aid_string route, https_callback callback);

#endif //HTTPS_HTTPS_H
