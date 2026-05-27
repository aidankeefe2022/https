//
// Created by aidankeefe on 5/20/26.
//

#ifndef HTTPS_PARSER_H
#define HTTPS_PARSER_H
#include <LibAidan/AidanString.h>
#include <LibAidan/AidanLinkedList.h>

typedef enum https_method {
    GET,
    POST,
    DELETE,
    PUT,
    PATCH,
}https_method;

typedef struct https_request {
    i32 client;
    https_method method;
    struct aid_string route;
    struct aid_LinkedList headers;
    struct aid_string body;
}https_request;



struct https_request* https_serializeRequest(struct aid_arena *arena, struct aid_string string);

bool https_freeRequest(struct https_request* request);
#endif //HTTPS_PARSER_H
