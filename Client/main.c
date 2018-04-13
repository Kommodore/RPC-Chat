#include <stdio.h>
#include <string.h>
#include <rpc/rpc.h>
#include <errno.h>
#include <stdlib.h>
#include "pub_sub.h"

extern int errno;
struct timeval TIMEOUT = {0, 0}; /* used by one_way_clnt.c with clnt_call() timeouts */

void getInput(char* input){
    printf("> ");
    scanf("%s", input);
}

int main() {
    CLIENT *cl;
    char option[MESLEN];
    char server[] = "192.168.56.101";
    void* input_arguments = NULL;

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
    TIMEOUT.tv_sec = TIMEOUT.tv_usec = 0;
    if (clnt_control(cl, CLSET_TIMEOUT, (char*) &TIMEOUT) == FALSE) {
        fprintf (stderr, "can't zero timeout\n");
        exit(1);
    }

    printf("RPC Client v1.0\n\n Aktion auswählen: \n\t>subscribe\t\t- Nachrichten von Server erhalten"
           "\n\t>unsubscribe\t- Keine Nachrichten mehr von Server erhalten\n\t>set_channel\t- Kanal wechseln"
           "\n\t>publish\t\t- Nachricht in aktuellem Kanal verfassen\n\t>exit\t\t\t- Client beenden\n");

    while(1){
        getInput(option);
        if(strcmp(option, "subscribe") == 0){
            printf("Subscribing to server.\n");
            subscribe_1(&input_arguments, cl);
            clnt_perror(cl, server); /* ignore the time-out errors */
        } else if(strcmp(option, "unsubscribe") == 0){
            printf("Unsubscribing from server.\n");
            unsubscribe_1(&input_arguments, cl);
            clnt_perror(cl, server); /* ignore the time-out errors */
        } else if(strcmp(option, "publish") == 0){
            char tempMessage[MESLEN];
            message message1;

            printf("Send message: ");
            getInput(tempMessage);
            message1 = strdup(tempMessage);
            publish_1(&message1, cl);
            clnt_perror(cl, server); /* ignore the time-out errors */
        } else if(strcmp(option, "set_channel") == 0){
            char tempChannel[TOPLEN];
            topic topic1;

            printf("Enter channel name: ");
            getInput(tempChannel);
            topic1 = strdup(tempChannel);
            set_channel_1(&topic1, cl);
            clnt_perror(cl, server); /* ignore the time-out errors */
        } else if(strcmp(option, "exit") == 0) {
            printf("Closing client...\n");
            unsubscribe_1(&input_arguments, cl);
            clnt_perror(cl, server); /* ignore the time-out errors */
            return 0;
        } else {
            printf("Aktion nicht gefunden.\n");
        }
    }
}
