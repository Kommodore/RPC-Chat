/*
 * pub_sub_deliv.x: Protokoll-Datei fuer Pub-Sub System (Receiver).
 */
const           POSTMESLEN = 192;
typedef string  postmessage <POSTMESLEN>;        /* Nachricht / Message */

/*
 * Funktion zur Auslieferung von Nachrichten.
 */
program PUBSUBCLTPROG {
  version PUBSUBCLTVERS {
    void deliver (postmessage) = 1;     /* Auslieferung von Nachrichten */ 
  } =             1;
} =             0x20000012;
