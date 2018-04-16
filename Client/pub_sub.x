/*
 * pub_sub.x: Protokoll-Datei fuer Publish-Subscribe System.
 */

const           TOPLEN = 64;
const           MESLEN = 128;
typedef string  topic <TOPLEN>;          /* Topic Name */
typedef string  message <MESLEN>;        /* Nachricht / Message */

const USERLEN = 12;
const HASHLEN = 96; /* bei SHA 256 */
typedef string hashstring <HASHLEN>; /* Parameter */
typedef string user <USERLEN>; /* User */
typedef int sessionid;
union argument switch (int topic_or_message) {
 case 0: topic t; /* topic wird uebertragen */
 case 1: message m; /* message wird uebertragen */
 default: void; /* kein Parameter */
};

struct param {
 sessionid id;
 argument arg;
 hashstring hash;
};
typedef struct param param;

/*
 * Funktionen des Pub / Sub Systems.
 */
program PUBSUBPROG {
  version PUBSUBVERS {
    short set_channel (topic) = 1;  /* Setzen des Channel Topics     */
    short subscribe () = 2;         /* Receiver registrieren         */
    short unsubscribe () = 3;       /* Receiver deregistrieren       */
    short publish (message) = 4;    /* Nachricht an Channel schicken */
    sessionid get_session (user) = 5;
    short validate (param) = 6;
    short invalidate (sessionid) = 7;
  } =             1;
} =             0x20000011;