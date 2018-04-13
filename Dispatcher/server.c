#include <stdio.h>
#include <rpc/rpc.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>

#include "pub_sub.h"
#include "return_codes.h"

typedef struct Subscriber {
    char client_addr[INET6_ADDRSTRLEN];
    char topic[TOPLEN];
    struct Subscriber *next;
} Subscriber;

Subscriber *subscriber_list = NULL;

/**
 * Finde einen Subscriber in der Subscriber Liste.
 *
 * @param char *client_addr Die zu suchende IP.
 * @return Subscriber* Pointer auf den Subscriber oder NULL, falls nicht gefunden.
 */
Subscriber* find_subscriber(char* client_addr){
    Subscriber *element = subscriber_list;

    while(element != NULL){
        if(strcmp(element->client_addr, client_addr) == 0){
            return element;
        }

        element = element->next;
    }

    return NULL;
}

/**
 * Den Channel eines Subscribers wechseln.
 *
 * @param topic *topic Zu setzender Channel.
 * @param svc_req *req Enthält Infos zum Client. Wir brauchen die IP.
 * @return Ein Code definiert in return_codes.h.
 */
short * set_channel_1_svc(topic *topic, struct svc_req *req){
    static short return_code = OK; // Static damit nicht abgeräumt
    char client_addr[INET6_ADDRSTRLEN];
    Subscriber *client;

    strcpy(client_addr, inet_ntoa(req->rq_xprt->xp_raddr.sin_addr));

    if((client = find_subscriber(client_addr)) != NULL){
        strcpy(client->topic, *topic);
        printf("Topic set to %s for %s\n", (char*)topic, client_addr);
    } else {
        printf("Can't set topic. Subscriber not found.\n");
        return_code = CANNOT_SET_TOPIC;
    }

    return &return_code;
}

/**
 * Einen neuen Subscriber erstellen und ans Ende der Subscriber Liste packen.
 *
 * @param void *argp Enthält NULL da keine Daten übergeben.
 * @param svc_req *req Enthält Infos zum Client. Wir brauchen die IP.
 * @return short* Ein Code definiert in return_codes.h.
 */
short * subscribe_1_svc(void *argp, struct svc_req *req){
    static short return_code = OK; // Static damit nicht abgeräumt
    char client_addr[INET6_ADDRSTRLEN];
    Subscriber* new_subscriber = malloc(sizeof(Subscriber));

    strcpy(client_addr, inet_ntoa(req->rq_xprt->xp_raddr.sin_addr));

    if(new_subscriber == NULL){
        return_code = CANNOT_REGISTER;
        printf("Couldn't allocate memory for new subscriber.\n");
    } else {
        if(find_subscriber(client_addr) == NULL){ // Wenn Subscriber noch nicht existiert einfügen
            // Struct erstellen
            memset(new_subscriber->client_addr, '\0', INET6_ADDRSTRLEN);
            memset(new_subscriber->topic, '\0', TOPLEN);
            strcpy(new_subscriber->client_addr, client_addr);
            new_subscriber->next = NULL;

            // Ans Ende packen
            if(subscriber_list ==  NULL) {
                subscriber_list = new_subscriber;
                printf("Subscribed %s\n", client_addr);
            } else {
                Subscriber* element = subscriber_list;
                while(element->next != NULL){
                    element = element->next;
                }
                element->next = new_subscriber;
                printf("Subscribed %s\n", client_addr);
            }
        } else {
            return_code = CLIENT_ALREADY_REGISTERED;
            printf("Subscriber already existing\n");
            free(new_subscriber);
        }
    }

    return &return_code;
}

/**
 * Einen Subscriber aus der Liste löschen.
 *
 * @param void *argp Enthält NULL da keine Daten übergeben.
 * @param svc_req *req Enthält Infos zum Client. Wir brauchen die IP.
 * @return short* Ein Code definiert in return_codes.h.
 */
short * unsubscribe_1_svc(void *argp, struct svc_req *req){
    static short return_code = OK; // Static damit nicht abgeräumt
    char client_addr[INET6_ADDRSTRLEN];

    strcpy(client_addr, inet_ntoa(req->rq_xprt->xp_raddr.sin_addr));

    if(subscriber_list == NULL){
        return_code = CANNOT_UNREGISTER;
        printf("Cannot unregister: No Client existing.\n");
    } else {
        Subscriber *element = subscriber_list;

        if(strcmp(element->client_addr, client_addr) == 0){
            subscriber_list = element->next;
            free(element);
            printf("Client unregistered.\n");
        } else {
            while(element->next != NULL && strcmp(element->next->client_addr, client_addr) != 0){
                element = element->next;
            }

            if(element->next == NULL){
                return_code = CANNOT_UNREGISTER;
                printf("Cannot unregister: Client not found.\n");
            } else {
                Subscriber *dummy = element->next;
                element->next = element->next->next;
                free(dummy);
                printf("Client unregistered.\n");
            }
        }
    }

    return &return_code;
}

/**
 *
 *
 * @param message
 * @param req
 * @return
 */
short * publish_1_svc(message *message, struct svc_req *req){
    static short return_code = OK; // Static damit nicht abgeräumt

    return &return_code;
}
