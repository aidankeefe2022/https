//
// Created by aidankeefe on 5/19/26.
//

#ifndef HTTPS_HTTPS_H
#define HTTPS_HTTPS_H

#include <LibAidan/AidanAidAllocator.h>
#include <LibAidan/AidanArena.h>
#include "serializeRequest.h"



typedef struct https_routes https_routes;

typedef struct https_ctx {
    https_request request;
}https_ctx;



void https_listen(u64 port, u64 options);

#endif //HTTPS_HTTPS_H
