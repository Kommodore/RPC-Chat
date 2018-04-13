#include <stdio.h>
#include <string.h>
#include "pub_sub_deliv.h"

void * deliver_1_svc(postmessage * message, struct svc_req * req){
    char* tempMessage;

    tempMessage = strdup(*message);

    printf("%s\n", tempMessage);

    return NULL;
}