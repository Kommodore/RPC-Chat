#include <stdio.h>
#include "pub_sub_deliv.h"

void * deliver_1(postmessage * msg, CLIENT * client){

    printf("%s\n", *msg);

    return NULL;
}

