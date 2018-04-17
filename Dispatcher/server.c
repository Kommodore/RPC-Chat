#include <stdio.h>
#include <rpc/rpc.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <rpc/svc.h>
#include <time.h>

#include "pub_sub.h"
#include "pub_sub_deliv.h"
#include "return_codes.h"

typedef struct Subscriber {
    char client_addr[INET6_ADDRSTRLEN];
    char *topic;
    struct Subscriber *next;
    param auth_data;
} Subscriber;

Subscriber *subscriber_list = NULL;
struct timeval TIMEOUT = {0, 0}; /* used by one_way_clnt.c with clnt_call() timeouts */

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
        client->topic = strdup(*topic);
        printf("Topic set to %s for %s\n", client->topic, client_addr);
    } else {
        printf("Can't set topic. Subscriber not found.\n");
        return_code = CANNOT_SET_TOPIC;
    }

    return &return_code;
}

sessionid * get_session_1_svc(user *argp, struct svc_req * req){
    /*if(GLOB_hash_digest_initialized != TRUE){
        init_hash_digest();
    }*/

    static sessionid session_id;
    session_id = clock();

    return &session_id;
}

short * invalidate_1_svc(sessionid *argp, struct svc_req *req){
    static short return_code = OK;

    return &return_code;
}

short * validate_1_svc(param *argp, struct svc_req * req){
    static short return_code = OK; // Static damit nicht abgeräumt
    char client_addr[INET6_ADDRSTRLEN];
    clock_t nonce = clock();
    Subscriber* new_subscriber = malloc(sizeof(Subscriber));

    strcpy(client_addr, inet_ntoa(req->rq_xprt->xp_raddr.sin_addr));

    if(new_subscriber == NULL){
        return_code = CANNOT_REGISTER;
        printf("Couldn't allocate memory for new subscriber.\n");
    } else {
        if(find_subscriber(client_addr) == NULL){ // Wenn Subscriber noch nicht existiert einfügen
            // Struct erstellen
            memset(new_subscriber->client_addr, '\0', INET6_ADDRSTRLEN);
            strcpy(new_subscriber->client_addr, client_addr);
            new_subscriber->next = NULL;
            new_subscriber->topic = NULL;

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
            strcpy(new_subscriber->client_addr, client_addr);
            new_subscriber->next = NULL;
            new_subscriber->topic = NULL;

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
 * Send a message to all client subscribed to that topic.
 *
 * @param message
 * @param req
 * @return
 */
short * publish_1_svc(message *message, struct svc_req *req){
    static short return_code = OK; // Static damit nicht abgeräumt
    char* tempMessage;
    CLIENT *cl;


    char topic[TOPLEN];
    char* cl_topic = find_subscriber(inet_ntoa(req->rq_xprt->xp_raddr.sin_addr))->topic;
    if(cl_topic != NULL){
        strcpy(topic, cl_topic);
        tempMessage = malloc(sizeof(char)*POSTMESLEN);
        sprintf(tempMessage, "%s#%s", cl_topic, *message);
    } else {
        memset(topic, '\0', TOPLEN);
        tempMessage = malloc(sizeof(char)*(strlen(*message)+7));
        sprintf(tempMessage, "GLOBAL#%s", *message);
    }

    Subscriber *element = subscriber_list;

    while(element != NULL){
        if(element->topic == NULL || strlen(topic) == 0 || strcmp(element->topic, topic) == 0){
            /*
             * Erzeugung eines Client Handles.
             * Fuer asynchrone One-way-Aufrufe wird hier TCP eingestellt,
             * damit der Aufruf in jedem Fall den Server erreicht.
             */
            if ((cl = clnt_create(element->client_addr, PUBSUBCLTPROG, PUBSUBCLTVERS, "tcp")) == NULL) {
                clnt_pcreateerror(element->client_addr);
                exit(1);
            }
            /*
             * Fuer alle Argumente der Kommandozeile wird die Server-Funktion
             * aufgerufen. Der Timeout wird auf 0 gesetzt, auf die Antwort
             * muss (und sollte) nicht gewartet werden.
             */
            TIMEOUT.tv_sec = TIMEOUT.tv_usec = 0;
            if (clnt_control(cl, CLSET_TIMEOUT, (char*) &TIMEOUT) == FALSE) {
                fprintf (stderr, "can't zero timeout\n");
                exit(1);
            }

            deliver_1(&tempMessage, cl);
            clnt_perror(cl, element->client_addr); /* ignore the time-out errors */
        }
        element = element->next;
    }

    return &return_code;
}
