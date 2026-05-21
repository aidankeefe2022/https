//
// Created by aidankeefe on 5/20/26.
//
#include "../include/https.h"
#include "../src/https.c"
#include "../src/serializeRequest.c"
#include <LibAidan/AidanTesting.h>

int testParseMethod(test_arg) {
    struct aid_arena* arena = aid_create_arena(MiB(1));
    struct aid_string parsing_string = STR_LIT("GET /helloworld HTTP/1.1\r\n");
    u64 idx = 0;
    t_assert(parseMethod(&idx, parsing_string) == GET);
    aid_arena_free(arena);
    test_end
}


int testParseRoute(test_arg) {
    struct aid_arena* arena = aid_create_arena(MiB(1));
    struct aid_string parsing_string = STR_LIT("GET /helloworld HTTP/1.1\r\n");
    u64 idx = 0;
    t_assert(parseMethod(&idx, parsing_string) == GET);
    auto route = parseRoute(arena, &idx, parsing_string);
    t_assert(aid_str_cmp(&route, &STR_LIT("/helloworld")));
    aid_arena_free(arena);
    test_end
}

int testParseHeader(test_arg) {


    test_end
}


int main() {
    Tests_set ts = ts_init();
    reg_test(ts, testParseMethod);
    reg_test(ts, testParseRoute);
    return run_tests(&ts);
}
