//
// Created by aidankeefe on 5/20/26.
//
#include "../include/https.h"
#include "../src/https.c"
#include "../src/serializeRequest.c"
#include <LibAidan/AidanTesting.h>

int testParseMethod(test_arg) {
    struct aid_string parsing_string = STR_LIT("GET /helloworld HTTP/1.1\r\n");
    u64 idx = 0;
    t_assert(parseMethod(&idx, parsing_string) == GET);
    t_assert(idx == 4);

    parsing_string = STR_LIT("POST /helloworld HTTP/1.1\r\n");
    idx = 0;
    t_assert(parseMethod(&idx, parsing_string) == POST);
    t_assert(idx == 5);

    parsing_string = STR_LIT("DELETE /helloworld HTTP/1.1\r\n");
    idx = 0;
    t_assert(parseMethod(&idx, parsing_string) == DELETE);
    t_assert(idx == 7);

    parsing_string = STR_LIT("PUT /helloworld HTTP/1.1\r\n");
    idx = 0;
    t_assert(parseMethod(&idx, parsing_string) == PUT);
    t_assert(idx == 4);

    parsing_string = STR_LIT("PATCH /helloworld HTTP/1.1\r\n");
    idx = 0;
    t_assert(parseMethod(&idx, parsing_string) == PATCH);
    t_assert(idx == 6);
    test_end
}

int testParseRoute(test_arg) {
    struct aid_arena* arena = aid_create_arena(MiB(1));
    struct aid_string parsing_string = STR_LIT("GET /helloworld HTTP/1.1\r\n");
    u64 idx = 0;
    t_assert(parseMethod(&idx, parsing_string) == GET);
    struct aid_string route = parseRoute(arena, &idx, parsing_string);
    t_assert(aid_str_cmp(&route, &STR_LIT("/helloworld")));
    t_assert(idx == 16);
    aid_arena_free(arena);
    test_end
}

int testParseSingleHeader(test_arg) {
    struct aid_arena* arena = aid_create_arena(MiB(1));
    struct aid_string parsing_string = STR_LIT("Host: localhost\r\nContent-Length: 5\r\n");
    u64 idx = 0;

    struct aid_string* header = parseSingleHeader(arena, &idx, parsing_string);
    t_assert(header);
    t_assert(aid_str_cmp(header, &STR_LIT("Host: localhost")));
    t_assert(idx == 17);

    header = parseSingleHeader(arena, &idx, parsing_string);
    t_assert(header);
    t_assert(aid_str_cmp(header, &STR_LIT("Content-Length: 5")));
    aid_arena_free(arena);
    test_end
}

int testParseHeaders(test_arg) {
    struct aid_arena* arena = aid_create_arena(MiB(1));
    struct aid_string parsing_string = STR_LIT(
            "Host: localhost\r\n"
            "Content-Type: text/plain\r\n"
            "\r\n"
    );
    u64 idx = 0;

    struct aid_LinkedList headers = parseHeaders(arena, &idx, parsing_string);
    t_assert(headers.size == 2);
    t_assert(headers.head->type == STRING);
    t_assert(aid_str_cmp(headers.head->STRING, &STR_LIT("Host: localhost")));
    t_assert(headers.head->next->type == STRING);
    t_assert(aid_str_cmp(headers.head->next->STRING, &STR_LIT("Content-Type: text/plain")));
    aid_arena_free(arena);
    aid_free_LL(&headers);
    test_end
}

int testSerializeRequestParsesHeadersAndBody(test_arg) {
    struct aid_arena* arena = aid_create_arena(MiB(1));
    struct aid_string raw_request = STR_LIT(
            "POST /submit HTTP/1.1\r\n"
            "Host: localhost\r\n"
            "Content-Type: text/plain\r\n"
            "\r\n"
            "hello"
    );

    struct https_request* request = https_serializeRequest(arena, raw_request);
    t_assert(request);
    t_assert(request->method == POST);
    t_assert(aid_str_cmp(&request->route, &STR_LIT("/submit")));
    t_assert(aid_str_cmp(&request->body, &STR_LIT("hello")));

    aid_arena_free(arena);
    aid_free_LL(&request->headers);
    test_end
}

int testSerializeRequestHandlesEmptyBody(test_arg) {
    struct aid_arena* arena = aid_create_arena(MiB(1));
    struct aid_string raw_request = STR_LIT(
            "GET / HTTP/1.1\r\n"
            "Host: localhost\r\n"
            "\r\n"
    );

    struct https_request* request = https_serializeRequest(arena, raw_request);
    t_assert(request);
    t_assert(request->method == GET);
    t_assert(aid_str_cmp(&request->route, &STR_LIT("/")));
    t_assert(aid_str_cmp(&request->body, &STR_LIT("")));

    aid_arena_free(arena);
    aid_free_LL(&request->headers);
    test_end
}

int testSerializeRequestRejectsInvalidMethod(test_arg) {
    struct aid_arena* arena = aid_create_arena(MiB(1));
    struct aid_string raw_request = STR_LIT("HEAD / HTTP/1.1\r\n\r\n");

    t_assert(!https_serializeRequest(arena, raw_request));

    aid_arena_free(arena);
    test_end
}

int testSerializeRequestRejectsInvalidVersion(test_arg) {
    struct aid_arena* arena = aid_create_arena(MiB(1));
    struct aid_string raw_request = STR_LIT("GET / HTTP/1.0\r\n\r\n");

    t_assert(!https_serializeRequest(arena, raw_request));

    aid_arena_free(arena);
    test_end
}

int main() {
    Tests_set ts = ts_init();
    reg_test(ts, testParseMethod);
    reg_test(ts, testParseRoute);
    reg_test(ts, testParseSingleHeader);
    reg_test(ts, testParseHeaders);
    reg_test(ts, testSerializeRequestParsesHeadersAndBody);
    reg_test(ts, testSerializeRequestHandlesEmptyBody);
    reg_test(ts, testSerializeRequestRejectsInvalidMethod);
    reg_test(ts, testSerializeRequestRejectsInvalidVersion);
    return run_tests(&ts);
}
