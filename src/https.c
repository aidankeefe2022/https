//
// Created by aidankeefe on 5/19/26.
//
#include "../include/https.h"


struct https_route {
    struct aid_string routeName;
    https_method method;
    void(*callback)(https_ctx);
    struct https_route* next;
};

struct https_routes {
    struct https_route* routes;
};




void https_listen(u64 port, u64 options) {

}
