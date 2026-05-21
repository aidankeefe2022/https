//
// Created by aidankeefe on 5/20/26.
//
#include "../include/serializeRequest.h"


static https_method parseMethod(u64* idx, struct aid_string string) {
    switch (string.s[0]) {
        case 'D':
            *idx = 7;
            return DELETE;
        case 'G':
            *idx = 4;
            return GET;
    }
    switch (string.s[1]) {
        case 'O':
            *idx = 5;
            return POST;
        case 'U':
            *idx = 4;
            return PUT;
        case 'A':
            *idx = 6;
            return PATCH;
        default:
            return -1;
    }
}

static bool isCorrectHttpVersion(u64* idx, struct aid_string string) {
    const static struct aid_string http_version = STR_LIT("HTTP/1.1\r\n");
    char* request = &string.s[*idx];
    for (int i = 0; i < http_version.length-1; i++) {
        (*idx)++;
        if (http_version.s[i] != *request++) {
            return false;
        }
    }
    return true;
}

static struct aid_string parseRoute(struct aid_arena* arena, u64 *idx, struct aid_string string) {
    u64 route_len = 0; //account for the \0 we are gonna add later
    char* start = &string.s[*idx];
    while (string.s[*idx] != ' ' && *idx < string.length) {
        route_len++;
        (*idx)++;
    }
    struct aid_string* new = aid_str_new(arena, route_len, start, route_len);
    if (!new) {
        return (struct aid_string){0};
    }
    (*idx)++;//skip white space
    return *new;
}

bool checkHttpEndLine(u64* idx, struct aid_string string, bool lookAhead) {
    int bump1, bump2;
    if (lookAhead) {
        bump1 = 2;
        bump2 = 3;
    }else {
        bump1 = 0;
        bump2 = 1;
    }
    return string.s[*idx + bump1] == '\r' && string.s[*idx + bump2] == '\n';
}

static struct aid_string* parseSingleHeader(struct aid_arena* arena, u64* idx, struct aid_string string) {
    char* cur = &string.s[*idx];
    u64 line_len = 1; // set to start at one because we want to include the \r char that will be replaced with \0 char by str_new
    char* end = &string.s[string.length - 1];
    while (*cur != '\r' && *cur != '\n' && cur != end) {
        line_len++;
        cur++;
    }
    struct aid_string* new = aid_str_new(arena, line_len, &string.s[*idx], line_len);
    if (!new) {
        return nullptr;
    }
    *idx += line_len+1;
    return new;
}

static struct aid_LinkedList parseHeaders(struct aid_arena* arena, u64* idx, struct aid_string string) {
    aid_LinkedList* result = aid_arena_alloc(arena, sizeof(*result), alignof(aid_LinkedList));
    if (!result) {
        return (aid_LinkedList){0};
    }
    *result = (aid_LinkedList){0};
    while (*idx < string.length) {
        //check that there are not two newline characters in a row
        if (checkHttpEndLine(idx, string, false) || checkHttpEndLine(idx, string, true)) {
            break;
        }
        if (!aid_push(result, parseSingleHeader(arena, idx, string), STRING)) {
            return (aid_LinkedList){0};
        }
    }
    return *result;
}


struct https_request* https_serializeRequest(struct aid_arena* arena, struct aid_string raw_request) {
    https_request* request = aid_arena_alloc(arena, sizeof(https_request), alignof(https_request));
    if (!request) {
        return nullptr;
    }
    u64 idx = 0;
    request->method = parseMethod(&idx, raw_request);
    if (request->method == -1) {
        goto Error;
    }
    request->route = parseRoute(arena, &idx, raw_request);
    if (!isCorrectHttpVersion(&idx, raw_request)) {
        goto Error;
    }
    request->headers = parseHeaders(arena, &idx, raw_request);

    if (raw_request.length <= idx) {
        request->body = *Q_STR(arena,"");
    }else {
        if (!checkHttpEndLine(&idx, raw_request,false)) {
            goto Error;
        }
        idx+=2;//consume blank line
        u64 body_len = raw_request.length - idx;
        struct aid_string* body = aid_str_new(arena, body_len, &raw_request.s[idx], body_len);
        if (!body) {
            goto Error;
        }
        request->body = *body;
    }

    return request;
Error:
    return nullptr;
}