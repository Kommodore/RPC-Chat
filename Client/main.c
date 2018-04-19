#include <stdio.h>
#include <string.h>
#include <rpc/rpc.h>
#include <errno.h>
#include <stdlib.h>
#include "pub_sub.h"
#include "return_codes.h"
#include "sha_hashing.h"

extern int errno;
struct timeval TIMEOUT = {25, 0}; /* used by one_way_clnt.c with clnt_call() timeouts */

void getInput(char* input){
    printf("> ");
    scanf("%s", input);
    fflush(stdin);
}

param auth_data;

int main() {
    CLIENT *cl;
    char option[MESLEN];
    char server[] = "192.168.56.101";
    char username[USERLEN];
    char password[PWDLEN];
    user tempUser;
    sessionid  sessionid;
    short *result;

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
        printf("Please enter your username:\n");
        getInput(username);
        tempUser = strdup(username);
        printf("Please enter your password:\n");
        getInput(password);
        sessionid = *get_session_1(&tempUser, cl);
        if(sessionid > 8 || sessionid == 0){
            char hash[MAX_HASH_DIGEST_LENGTH];
            auth_data.id = sessionid;
            strcpy(auth_data.arg.argument_u.m, "");
            sprintf(hash, "%d;\"\";%s", sessionid, hash_user_pwd(tempUser, password));
            auth_data.hash = hash_sha(hash);
            result = validate_1(&auth_data, cl);
            if(*result == OK){
                break;
            } else {
                sessionid = *result;
            }
        }

        printf("%s\n", PUB_SUB_RET_CODE[sessionid]);
    }


    printf("RPC Client v1.0\n\n Aktion auswählen: \n\t>subscribe\t\t- Nachrichten von Server erhalten"
           "\n\t>unsubscribe\t- Keine Nachrichten mehr von Server erhalten\n\t>set_channel\t- Kanal wechseln"
           "\n\t>publish\t\t- Nachricht in aktuellem Kanal verfassen\n\t>exit\t\t\t- Client beenden\n");

    while(1){
        getInput(option);
        if(strcmp(option, "subscribe") == 0){
            printf("Subscribing to server.\n");
            strcpy(auth_data.arg.argument_u.t, "");
            result = subscribe_1(&auth_data, cl);
            if(*result == OK){
                printf("Successfully subscribed.\n");
            } else {
                printf("Could not subscribe: %s\n", PUB_SUB_RET_CODE[*result]);
            }
            clnt_perror(cl, server); /* ignore the time-out errors */
        } else if(strcmp(option, "unsubscribe") == 0){
            printf("Unsubscribing from server.\n");
            strcpy(auth_data.arg.argument_u.t, "");
            result = unsubscribe_1(&auth_data, cl);
            if(*result == OK){
                printf("Successfully unsubscribed.\n");
            } else {
                printf("Could not unsubscribe: %s\n", PUB_SUB_RET_CODE[*result]);
            }
            clnt_perror(cl, server); /* ignore the time-out errors */
        } else if(strcmp(option, "publish") == 0){
            char tempMessage[MESLEN];
            message message1;

            printf("Send message: ");
            getInput(tempMessage);
            message1 = strdup(tempMessage);
            strcpy(auth_data.arg.argument_u.m, message1);
            strcpy(auth_data.arg.argument_u.t, "");
            result = publish_1(&auth_data, cl);
            if(*result == OK){
                printf("Message sent.\n");
            } else {
                printf("Message not sent: %s\n", PUB_SUB_RET_CODE[*result]);
            }
            clnt_perror(cl, server); /* ignore the time-out errors */
        } else if(strcmp(option, "set_channel") == 0){
            char tempChannel[TOPLEN];
            topic topic1;

            printf("Enter channel name: ");
            getInput(tempChannel);
            topic1 = strdup(tempChannel);
            strcpy(auth_data.arg.argument_u.t, topic1);
            result = set_channel_1(&auth_data, cl);
            if(*result == OK){
                printf("Entered channel.\n");
            } else {
                printf("Channel not entered: %s", PUB_SUB_RET_CODE[*result]);
            }
            clnt_perror(cl, server); /* ignore the time-out errors */
        } else if(strcmp(option, "exit") == 0) {
            printf("Closing client...\n");
            strcpy(auth_data.arg.argument_u.t, "");
            unsubscribe_1(&auth_data, cl);
            invalidate_1(&sessionid, cl);
            clnt_perror(cl, server); /* ignore the time-out errors */
            return 0;
        } else if(strcmp(option, "logout") == 0) {
            printf("Logging out...");
            strcpy(auth_data.arg.argument_u.t, "");
            unsubscribe_1(&auth_data, cl);
            invalidate_1(&sessionid, cl);
            clnt_perror(cl, server); /* ignore the time-out errors */
        } else {
                printf("Aktion nicht gefunden.\n");
        }
    }
}
