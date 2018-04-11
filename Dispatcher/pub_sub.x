/*
 * pub_sub.x: Protokoll-Datei fuer Publish-Subscribe System.
 */

const           TOPLEN = 64;
const           MESLEN = 128;
typedef string  topic <TOPLEN>;          /* Topic Name */
typedef string  message <MESLEN>;        /* Nachricht / Message */

/*
 * Funktionen des Pub / Sub Systems.
 */
program PUBSUBPROG {
  version PUBSUBVERS {
    short set_channel (topic) = 1;  /* Setzen des Channel Topics     */
    short subscribe () = 2;         /* Receiver registrieren         */
    short unsubscribe () = 3;       /* Receiver deregistrieren       */
    short publish (message) = 4;    /* Nachricht an Channel schicken */
  } =             1;
} =             0x20000011;