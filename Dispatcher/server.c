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
#include "sha_hashing.h"

typedef struct Subscriber {
    char client_addr[INET6_ADDRSTRLEN];
    char *topic;
    struct Subscriber *next;
    hashstring credentials;
    sessionid session_id;
    short authenticated;
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

short validate_hash(param *auth_data, char *ip_addr){
    char hash[MAX_HASH_DIGEST_LENGTH];
    Subscriber* curr_subscriber = find_subscriber(ip_addr);

    if(curr_subscriber == NULL){
        return AUTH_FAILURE;
    } else {
        if(auth_data->arg.topic_or_message == 0){
            sprintf(hash, "%d;%s;%s", curr_subscriber->session_id, auth_data->arg.argument_u.t, curr_subscriber->credentials);
        } else {
            sprintf(hash, "%d;%s;%s", curr_subscriber->session_id, auth_data->arg.argument_u.m, curr_subscriber->credentials);
        }

        printf("Created hash %s from %s\n", auth_data->hash, hash);
        if(strcmp(hash_sha(hash), auth_data->hash) != 0){
            return AUTH_FAILURE;
        } else {
            return OK;
        }
    }
}

/**
 * Den Channel eines Subscribers wechseln.
 *
 * @param topic *topic Zu setzender Channel.
 * @param svc_req *req Enthält Infos zum Client. Wir brauchen die IP.
 * @return Ein Code definiert in return_codes.h.
 */
short * set_channel_1_svc(param *param, struct svc_req *req){
    static short return_code = OK; // Static damit nicht abgeräumt
    char client_addr[INET6_ADDRSTRLEN];
    Subscriber *client;

    strcpy(client_addr, inet_ntoa(req->rq_xprt->xp_raddr.sin_addr));

    if((client = find_subscriber(client_addr)) != NULL){
        if(client->authenticated == 1 && validate_hash(param, client_addr) == OK){
            client->topic = strdup(param->arg.argument_u.t);
            printf("Topic set to %s for %s\n", client->topic, client_addr);
        } else {
            printf("User not authenticated");
            return_code = AUTH_FAILURE;
        }
    } else {
        printf("Can't set topic. Subscriber not found.\n");
        return_code = CANNOT_SET_TOPIC;
    }

    return &return_code;
}

sessionid * get_session_1_svc(user *argp, struct svc_req * req){
    static sessionid session_id;
    if(GLOB_hash_digest_initialized == FALSE){
        init_hash_digest();
    }

    for(int i = 0; i < MAX_HASH_DIGEST_LENGTH; i++){
        if(GLOB_hash_digest[i].user == NULL){
            session_id = UNKNOWN_USER;
            break;
        } else if(strcmp(GLOB_hash_digest[i].user, *argp) == 0){
            char client_addr[INET6_ADDRSTRLEN];
            strcpy(client_addr, inet_ntoa(req->rq_xprt->xp_raddr.sin_addr));
            Subscriber* new_subscriber = find_subscriber(client_addr);

            if(new_subscriber == NULL){
                new_subscriber = malloc(sizeof(Subscriber));

                if(new_subscriber == NULL){
                    session_id = CANNOT_REGISTER;
                    printf("Couldn't allocate memory for new subscriber.\n");
                    return &session_id;
                }

                // Ans Ende packen
                if(subscriber_list ==  NULL) {
                    subscriber_list = new_subscriber;
                } else {
                    Subscriber* element = subscriber_list;
                    while(element->next != NULL){
                        element = element->next;
                    }
                    element->next = new_subscriber;
                }
                new_subscriber->next = NULL;
            }

            // Struct erstellen
            memset(new_subscriber->client_addr, '\0', INET6_ADDRSTRLEN);
            strcpy(new_subscriber->client_addr, client_addr);
            new_subscriber->topic = NULL;
            new_subscriber->authenticated = 0;
            new_subscriber->session_id = clock();
            new_subscriber->credentials = strdup(GLOB_hash_digest[i].hash);

            printf("Session created for %s with id %d\n", client_addr, new_subscriber->session_id);

            session_id = new_subscriber->session_id;
            break;
        }
    }

    return &session_id;
}

short * invalidate_1_svc(sessionid *argp, struct svc_req *req){
    static short return_code = OK;
    char client_addr[INET6_ADDRSTRLEN];

    strcpy(client_addr, inet_ntoa(req->rq_xprt->xp_raddr.sin_addr));

    if(subscriber_list == NULL){
        return_code = CANNOT_UNREGISTER;
        printf("Cannot unregister: No Client existing.\n");
    } else {
        Subscriber *element = subscriber_list;

        if(strcmp(element->client_addr, client_addr) == 0 && element->session_id == *argp){
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

short * validate_1_svc(param *argp, struct svc_req * req){
    static short return_code; // Static damit nicht abgeräumt

    return_code = validate_hash(argp, inet_ntoa(req->rq_xprt->xp_raddr.sin_addr));

    return &return_code;
}

/**
 * Einen neuen Subscriber erstellen und ans Ende der Subscriber Liste packen.
 *
 * @param void *argp Enthält NULL da keine Daten übergeben.
 * @param svc_req *req Enthält Infos zum Client. Wir brauchen die IP.
 * @return short* Ein Code definiert in return_codes.h.
 */
short * subscribe_1_svc(param *param, struct svc_req *req){
    static short return_code = OK; // Static damit nicht abgeräumt

    if(validate_hash(param, inet_ntoa(req->rq_xprt->xp_raddr.sin_addr)) == OK){
        find_subscriber(inet_ntoa(req->rq_xprt->xp_raddr.sin_addr))->authenticated = 1;
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
short * unsubscribe_1_svc(param *param, struct svc_req *req){
    static short return_code = OK; // Static damit nicht abgeräumt

    if(validate_hash(param, inet_ntoa(req->rq_xprt->xp_raddr.sin_addr)) == OK){
        find_subscriber(inet_ntoa(req->rq_xprt->xp_raddr.sin_addr))->authenticated = 0;
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
short * publish_1_svc(param *param, struct svc_req *req){
    static short return_code = OK; // Static damit nicht abgeräumt
    char* tempMessage;
    CLIENT *cl;
    Subscriber* caller = find_subscriber(inet_ntoa(req->rq_xprt->xp_raddr.sin_addr));

    if(caller->authenticated == 1 && validate_hash(param, inet_ntoa(req->rq_xprt->xp_raddr.sin_addr)) == OK){
        char topic[TOPLEN];
        char* cl_topic = caller->topic;
        if(cl_topic != NULL){
            strcpy(topic, cl_topic);
            tempMessage = malloc(sizeof(char)*POSTMESLEN);
            sprintf(tempMessage, "%s#%s", cl_topic, param->arg.argument_u.m);
        } else {
            memset(topic, '\0', TOPLEN);
            tempMessage = malloc(sizeof(char)*(strlen(param->arg.argument_u.m)+7));
            sprintf(tempMessage, "GLOBAL#%s", param->arg.argument_u.m);
        }

        Subscriber *element = subscriber_list;

        while(element != NULL){
            if((element->authenticated == 1) && ((element->topic == NULL && strlen(topic) == 0) || strlen(topic) == 0 || (element->next != NULL && strcmp(element->topic, topic) == 0))){
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
                if (clnt_control(cl, CLSET_TIMEOUT, (char*) &TIMEOUT) == FALSE) {
                    fprintf (stderr, "can't zero timeout\n");
                    exit(1);
                }

                deliver_1(&tempMessage, cl);
                clnt_perror(cl, element->client_addr); /* ignore the time-out errors */
            }
            element = element->next;
        }
    } else {
        printf("User not authenticated\n");
        return_code = AUTH_FAILURE;
    }

    return &return_code;
}
