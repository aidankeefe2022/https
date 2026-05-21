//
// Created by aidankeefe on 5/21/26.
//

#include "../include/deserializeResponse.h"


bool https_responseAddStatus(struct https_response* response, u64 status, struct aid_string statusReason) {
    response->status = status;
    response->statusReason = statusReason;
    return true;
}
bool https_responseAddHeader(struct https_response* response, struct aid_string headerText) {
    aid_push(&response->headers, &headerText, STRING);
    return true;
}
bool https_responseAddBody(struct https_response* response, struct aid_string body) {
    response->body = body;
    return true;
}

struct aid_string https_responseDeserialize(struct https_response* response) {
    struct aid_string responseString = (struct aid_string){.options = AID_STR_AUTO_RESIZE};
    aid_str_append_int(&responseString, response->status);
    aid_str_append_char(&responseString, ' ');
    aid_str_append_string(&responseString, &response->statusReason);
    aid_str_append_char(&responseString, '\r');
    aid_str_append_char(&responseString, '\n');
    for (aid_LLNode *node = response->headers.head; node != nullptr; node = node->next) {
        aid_str_append_string(&responseString, node->STRING);
        aid_str_append_char(&responseString, '\r');
        aid_str_append_char(&responseString, '\n');
    }
    aid_str_append_string(&responseString, &response->body);
    return responseString;
}
