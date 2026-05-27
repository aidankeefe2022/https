//
// Created by aidankeefe on 5/21/26.
//

#include "../include/deserializeResponse.h"
#include "../src/https.c"
#include "../src/serializeRequest.c"
#include "../src/deserializeResponse.c"
#include <LibAidan/AidanTesting.h>

int testAddStatus(test_arg) {
    https_response resp = {0};
    https_responseAddStatus(&resp, 200, STR_LIT("OK"));
    t_assert(resp.status == 200);
    t_assert(aid_str_cmp(&(resp.statusReason), &STR_LIT("OK")));
    test_end
}

int testAddHeader(test_arg) {
    https_response resp = {0};
    https_responseAddHeader(&resp, &STR_LIT("doctype:json"));
    https_responseAddHeader(&resp, &STR_LIT("auth:ctx1234"));
    t_assert(resp.headers.size == 2);
    t_assert(resp.headers.head->type == STRING);
    t_assert(aid_str_cmp(resp.headers.head->STRING, &STR_LIT("doctype:json")));
    aid_free_LL(&resp.headers);
    test_end
}

int testAddBody(test_arg) {
    https_response resp = {0};
    https_responseAddBody(&resp, STR_LIT("My name is aidan"));
    t_assert(aid_str_cmp(&resp.body, &STR_LIT("My name is aidan")));
    test_end
}


int testDeserializeResponse(test_arg) {
    https_response resp = {0};

    https_responseAddStatus(&resp, 200, STR_LIT("OK"));
    https_responseAddHeader(&resp, &STR_LIT("doctype:json"));
    https_responseAddHeader(&resp, &STR_LIT("auth:ctx1234"));
    https_responseAddBody(&resp, STR_LIT("My name is aidan"));

    struct aid_string responseString = https_responseDeserialize(&resp);
    auto testString = &STR_LIT("HTTP/1.1 200 OK\r\nContent-Length: 16\r\ndoctype:json\r\nauth:ctx1234\r\n\r\nMy name is aidan");
    t_assert(aid_str_cmp(&responseString, testString));
    free(responseString.s);
    test_end
}

int testDeserializeResponseWithOnlyStatus(test_arg) {
    https_response resp = {0};

    https_responseAddStatus(&resp, 404, STR_LIT("Not Found"));

    struct aid_string responseString = https_responseDeserialize(&resp);
    t_assert(aid_str_cmp(&responseString, &STR_LIT("HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n")));
    free(responseString.s);
    test_end
}

int testDeserializeResponsePreservesHeaderOrder(test_arg) {
    https_response resp = {0};

    https_responseAddStatus(&resp, 201, STR_LIT("Created"));
    https_responseAddHeader(&resp, &STR_LIT("Content-Type: application/json"));
    https_responseAddHeader(&resp, &STR_LIT("X-Trace-Id: abc123"));

    struct aid_string responseString = https_responseDeserialize(&resp);
    t_assert(aid_str_cmp(&responseString, &STR_LIT(
            "HTTP/1.1 201 Created\r\n"
            "Content-Length: 0\r\n"
            "Content-Type: application/json\r\n"
            "X-Trace-Id: abc123\r\n\r\n"
    )));
    free(responseString.s);
    test_end
}

int testDeserializeResponsePreservesBodyContent(test_arg) {
    https_response resp = {0};

    https_responseAddStatus(&resp, 500, STR_LIT("Internal Server Error"));
    https_responseAddHeader(&resp, &STR_LIT("Content-Type: text/plain"));
    https_responseAddBody(&resp, STR_LIT("line one\r\nline two\r\nline three"));

    struct aid_string responseString = https_responseDeserialize(&resp);
    auto testString = &STR_LIT(
            "HTTP/1.1 500 Internal Server Error\r\n"
            "Content-Length: 30\r\n"
            "Content-Type: text/plain\r\n\r\n"
            "line one\r\nline two\r\nline three"
    );
    t_assert(aid_str_cmp(&responseString, testString));
    free(responseString.s);
    test_end
}



int main() {
    Tests_set ts = ts_init();
    reg_test(ts, testAddStatus);
    reg_test(ts, testAddHeader);
    reg_test(ts, testAddBody);
    reg_test(ts, testDeserializeResponse);
    reg_test(ts, testDeserializeResponseWithOnlyStatus);
    reg_test(ts, testDeserializeResponsePreservesHeaderOrder);
    reg_test(ts, testDeserializeResponsePreservesBodyContent);
    return run_tests(&ts);
}
