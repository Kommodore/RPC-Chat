#include <stdio.h>
#include <string.h>
#include <rpc/rpc.h>
#include <errno.h>
#include <stdlib.h>
#include "pub_sub.h"
#include "return_codes.h"
#include "sha_hashing.h"

#define SET_TOPIC 1
#define SET_MESSAGE 0

extern int errno;
struct timeval TIMEOUT = {25, 0}; /* used by one_way_clnt.c with clnt_call() timeouts */

void get_input(char *input){
    printf("> ");
    scanf("%s", input);
    fflush(stdin);
}

void set_argument(param *auth_data, int is_topic, char* arg){
    if(is_topic == SET_TOPIC){
        auth_data->arg.argument_u.t = strdup(arg);
        auth_data->arg.topic_or_message = 0;
    } else {
        auth_data->arg.argument_u.m = strdup(arg);
        auth_data->arg.topic_or_message = 1;
    }
}

void create_hash(param *auth_data, user username, char *password){
    char hash[MAX_HASH_DIGEST_LENGTH];

    if(auth_data->arg.topic_or_message == 0){
        sprintf(hash, "%d;%s;%s", auth_data->id, auth_data->arg.argument_u.t, hash_user_pwd(username, password));
    } else {
        sprintf(hash, "%d;%s;%s", auth_data->id, auth_data->arg.argument_u.m, hash_user_pwd(username, password));
    }

    auth_data->hash = hash_sha(hash);
    printf("Created hash %s from %s\n", auth_data->hash, hash);
}

param auth_data;

int main() {
    CLIENT *cl;
    char option[MESLEN];
    char server[] = "192.168.56.101";
    char username[USERLEN];
    char password[PWDLEN];
    user tempUser = NULL;
    char* tempPass = NULL;
    sessionid  sessionid;
    short *result;
    int login_required = 1;

    /*
     * Erzeugung eines Client Handles.
     * Fuer asynchrone One-way-Aufrufe wird hier TCP eingestellt,
     * damit der Aufruf in jedem Fall den Server erreicht.
     */
    if ((cl = clnt_create(server, PUBSUBPROG, PUBSUBVERS, "tcp")) == NULL) {
        clnt_pcreateerror(server);
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

    while(1){
        while(login_required == 1){
            printf("Please enter your username:\n");
            get_input(username);
            tempUser = strdup(username);
            printf("Please enter your password:\n");
            get_input(password);
            tempPass = strdup(password);
            sessionid = *get_session_1(&tempUser, cl);
            if(sessionid > 8 || sessionid == 0){
                auth_data.id = sessionid;
                set_argument(&auth_data, SET_TOPIC, "");
                create_hash(&auth_data, tempUser, tempPass);
                result = validate_1(&auth_data, cl);
                if(*result == OK){
                    login_required = 0;
                    printf("RPC Client v1.0\n\n Aktion auswählen: \n\t>subscribe\t\t- Nachrichten von Server erhalten"
                           "\n\t>unsubscribe\t- Keine Nachrichten mehr von Server erhalten\n\t>set_channel\t- Kanal wechseln"
                           "\n\t>publish\t\t- Nachricht in aktuellem Kanal verfassen\n\t>exit\t\t\t- Client beenden\n\t>logout\t\t\t- Abmelden\n"
                           "");
                    break;
                } else {
                    sessionid = *result;
                }
            }

            printf("%s\n", PUB_SUB_RET_CODE[sessionid]);
        }


        get_input(option);
        if(strcmp(option, "subscribe") == 0){
            printf("Subscribing to server.\n");
            set_argument(&auth_data, SET_TOPIC, "");
            create_hash(&auth_data, tempUser, tempPass);
            result = subscribe_1(&auth_data, cl);
            if(*result == OK){
                printf("Successfully subscribed.\n");
            } else {
                printf("Could not subscribe: %s\n", PUB_SUB_RET_CODE[*result]);
            }
            clnt_perror(cl, server); /* ignore the time-out errors */
        } else if(strcmp(option, "unsubscribe") == 0){
            printf("Unsubscribing from server.\n");
            set_argument(&auth_data, SET_TOPIC, "");
            create_hash(&auth_data, tempUser, tempPass);
            result = unsubscribe_1(&auth_data, cl);
            if(*result == OK){
                printf("Successfully unsubscribed.\n");
            } else {
                printf("Could not unsubscribe: %s\n", PUB_SUB_RET_CODE[*result]);
            }
            clnt_perror(cl, server); /* ignore the time-out errors */
        } else if(strcmp(option, "publish") == 0){
            char tempMessage[MESLEN];

            printf("Send message: ");
            get_input(tempMessage);
            set_argument(&auth_data, SET_MESSAGE, tempMessage);
            create_hash(&auth_data, tempUser, tempPass);
            result = publish_1(&auth_data, cl);
            if(*result == OK){
                printf("Message sent.\n");
            } else {
                printf("Message not sent: %s\n", PUB_SUB_RET_CODE[*result]);
            }
            clnt_perror(cl, server); /* ignore the time-out errors */
        } else if(strcmp(option, "set_channel") == 0){
            char tempChannel[TOPLEN];

            printf("Enter channel name: ");
            get_input(tempChannel);
            set_argument(&auth_data, SET_TOPIC, tempChannel);
            create_hash(&auth_data, tempUser, tempPass);
            result = set_channel_1(&auth_data, cl);
            if(*result == OK){
                printf("Entered channel.\n");
            } else {
                printf("Channel not entered: %s", PUB_SUB_RET_CODE[*result]);
            }
            clnt_perror(cl, server); /* ignore the time-out errors */
        } else if(strcmp(option, "exit") == 0) {
            printf("Closing client...\n");
            set_argument(&auth_data, SET_TOPIC, "");
            create_hash(&auth_data, tempUser, tempPass);
            unsubscribe_1(&auth_data, cl);
            invalidate_1(&sessionid, cl);
            clnt_perror(cl, server); /* ignore the time-out errors */
            return 0;
        } else if(strcmp(option, "logout") == 0) {
            printf("Logging out...");
            set_argument(&auth_data, SET_TOPIC, "");
            create_hash(&auth_data, tempUser, tempPass);
            unsubscribe_1(&auth_data, cl);
            invalidate_1(&sessionid, cl);
            clnt_perror(cl, server); /* ignore the time-out errors */
            login_required = 1;
        } else {
            printf("Aktion nicht gefunden.\n");
        }
    }
}
