#include <stdio.h>
#include "pub_sub_deliv.h"

void * deliver_1(postmessage * msg, CLIENT * client){
    return NULL;
}
extern  void * deliver_1_svc(postmessage * msg, struct svc_req * request){
    return NULL;
}