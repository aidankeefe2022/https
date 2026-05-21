//
// Created by aidankeefe on 5/21/26.
//

#ifndef HTTPS_DESERIALIZERESPONSE_H
#define HTTPS_DESERIALIZERESPONSE_H
#include <LibAidan/AidanString.h>
#include <LibAidan/AidanLinkedList.h>

typedef struct https_response {
    u64 status;
    struct aid_string statusReason;
    struct aid_LinkedList headers;
    struct aid_string body;
}https_response;

bool https_responseAddStatus(struct https_response* response, u64 status, struct aid_string statusReason);
bool https_responseAddHeader(struct https_response* response, struct aid_string header);
bool https_responseAddBody(struct https_response* response, struct aid_string body);
struct aid_string https_responseDeserialize(struct https_response* response);

#endif //HTTPS_DESERIALIZERESPONSE_H
