#ifndef CLIENT_SHA_HASHING_H
#define CLIENT_SHA_HASHING_H

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <openssl/sha.h>

#define TRUE 1
#define FALSE 0

#define USERLEN 12
#define PWDLEN 12
#define HASHLEN 100

/* Datenstruktur zur Repraesentation des Hash-Digest als Feld */
#define MAX_HASH_DIGEST_LENGTH 100

typedef struct {
    char* user;
    char* hash;
} hash_entry;

static int GLOB_max_idx = -1;
hash_entry GLOB_hash_digest [MAX_HASH_DIGEST_LENGTH];

static int GLOB_hash_digest_initialized = FALSE;

/*
 * Sicheres Lesen von f (z.B. stdin).
 * Die Eingabezeile wird vollständig gelesen, aber nur l
 * Zeichen werden in str geschrieben. Speicher fuer str muss allokiert
 * worden sein.
 */
int s_getline(char* str, int l, FILE* f);

/*
 * Lesen des Hash-Digest 'hashes.txt'.
 * Eintraege wird in einer globalen Variablen gespeichert.
 */
void init_hash_digest();

/* Hashfunktionen mit verschiedenen Parametern */
char* hash_sha(char* str);

/*
 * Hash aus user und pwd bilden -> dieser muss im Server-Digest gespeichert
 * werden.
 */
char* hash_user_pwd(char* user, char* pwd);



#endif //CLIENT_SHA_HASHING_H
