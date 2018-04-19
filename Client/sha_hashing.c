/**************************************************************
 * sha_hashing.c
 *
 * Nutzung der SHA256 Funktion.
 * Kompilieren mit: gcc -o -g ... -lssl -lcrypto 
 *
 * Heinz-Josef Eikerling, HS OS
 * 7.04.2017: Fertigstellung
 * 
 **************************************************************/

#include "sha_hashing.h"

int s_getline(char *str, int l, FILE *f) {
    if (l > 256) {
        fprintf(stderr, "s_getline(): Error with string length.");
        str[0] = '\0';
        return FALSE;
    }
    char tmp_str [256];
    if (!fgets(tmp_str, 256 - 1, f)) return FALSE;
    for (int i = 0; i < (l - 1); i++) {
        char c = tmp_str[i];
        if ((c == '\n') || (c == '\0')) {
            str[i] = '\0';
            return TRUE;
        } else str[i] = c;
    }
    return TRUE;
}

void init_hash_digest() {
    /* Initialisierung erfolgt nur einmal */
    if (GLOB_hash_digest_initialized == TRUE) return;
    /* else */
    FILE* f = fopen("hashes.txt", "r");
    if (f == NULL) {
        fprintf(stderr, "Cannot open digest 'hashes.txt'.\n");
        return;
    }
    /* else */
    size_t MAX_LINE_LENGTH = 120;
    char line[MAX_LINE_LENGTH];
    while (s_getline(line, MAX_LINE_LENGTH, f) == TRUE) {
        char user[USERLEN + 1];
        char hash[HASHLEN + 1];
        int n = sscanf(line, "%s %s", user, hash);
        if (n == 2) {
            GLOB_max_idx++;
            GLOB_hash_digest[GLOB_max_idx].user = strdup(user);
            GLOB_hash_digest[GLOB_max_idx].hash = strdup(hash);
            printf("%d: %s %s\n", GLOB_max_idx, user, hash);
        }
        else {
            fprintf(stderr, "Line '%s' malformatted!\n", line);
        }
    }
    fclose(f);
    GLOB_hash_digest_initialized = TRUE;
    printf("Hash digest 'hashes.txt' read.\n");
}

char *hash_sha(char *str) {
    unsigned char result[SHA256_DIGEST_LENGTH];
    SHA256(str, strlen(str), result);
    char hash_val[96];
    hash_val[0] = '\0';
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        char tmp[3];
        sprintf(tmp, "%02x", result[i]);
        strcat(hash_val, tmp);
    }
    // printf ("hashval: %s\n", hash_val);
    return strdup(hash_val);
}

char *hash_user_pwd(char *user, char *pwd) {
    char str [256];
    sprintf(str, "%s;%s", user, pwd);
    return hash_sha(str);
}

///*
// * Main Funktion: Einlesen von Hash-Digest,
// * und Generierung eines Hash-Wertes aus
// * Nutzername und Pwd.
// */
//int main(int argc, char** argv) {
//    /* Initialisierung des Hash-Digest */
//    init_hash_digest();
//    /* Generierung eines Hash-Wertes */
//    char user [USERLEN];
//    user[0] = '\0';
//    char pwd [PWDLEN];
//    pwd[0] = '\0';
//    printf("enter user name (%d)> ", USERLEN);
//    s_getline(user, USERLEN, stdin);
//    printf("enter pwd (%d)> ", PWDLEN);
//    fflush(stdout);
//    s_getline(pwd, PWDLEN, stdin);
//    char* hash_val = hash_user_pwd(user, pwd);
//    printf("%s %s\n", user, hash_val);
//    free(hash_val);
//    /* ... koennte anschließend in Hash-Digest eingefuegt werden ... */
//    return EXIT_SUCCESS;
//}

