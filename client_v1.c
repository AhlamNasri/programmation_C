/*
 * client_v1.c
 * Version 1 — Talkie-Walkie (SANS threads, séquentiel)
 *
 * Usage : ./client_v1 <IP_serveur> <port>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define TAILLE_MSG 1024

/* ------------------------------------------------------------------ */
/*  Prototype                                                          */
/* ------------------------------------------------------------------ */
void Chats(int D);

/* ================================================================== */
/*  MAIN                                                               */
/* ================================================================== */
int main(int argc, char *argv[])
{
    /* ----- Vérification des arguments ----- */
    if (argc < 3) {
        fprintf(stderr, "Usage : %s <IP_serveur> <port>\n", argv[0]);
        exit(1);
    }

    /* ----- Initialisation des variables ----- */
    int Desc;
    int err;
    struct sockaddr_in addr_distant;

    /* ----- Création du socket ----- */
    Desc = socket(AF_INET, SOCK_STREAM, 0);
    if (Desc < 0) {
        perror("Erreur socket");
        exit(1);
    }

    /* ----- Préparation de l'adresse distante ----- */
    memset(&addr_distant, 0, sizeof(addr_distant));
    addr_distant.sin_family      = AF_INET;

    /* addr_distant.IP ← inetaddr(argv[1]) */
    addr_distant.sin_addr.s_addr = inet_addr(argv[1]);

    /* addr_distant.port ← atoi(argv[2]) */
    addr_distant.sin_port        = htons(atoi(argv[2]));

    /* ----- Connexion au serveur ----- */
    err = connect(Desc, (struct sockaddr *)&addr_distant, sizeof(addr_distant));
    if (err != 0) {
        perror("Erreur connect");
        close(Desc);
        exit(1);
    }

    printf("Connecté au serveur %s : %d\n", argv[1], atoi(argv[2]));

    /* ----- Appel de la fonction Chats ----- */
    Chats(Desc);

    return 0;
}
/* Fin */

/* ================================================================== */
/*  FONCTION Chats — Version Talkie-Walkie (séquentielle)             */
/*                                                                     */
/*  Protocole d'échange des pseudos :                                  */
/*    1. Client reçoit le pseudo du serveur (recv)                     */
/*    2. Client envoie son pseudo           (Send)                     */
/*  Puis boucle talkie-walkie :                                        */
/*    Client envoie d'abord, puis reçoit                               */
/* ================================================================== */
void Chats(int D)
{
    char psl[TAILLE_MSG];   /* pseudo local  (du client) */
    char psd[TAILLE_MSG];   /* pseudo distant (du serveur) */
    char mr[TAILLE_MSG];    /* message reçu  */
    char me[TAILLE_MSG];    /* message envoyé */

    /* ----- Initialisation ----- */
    memset(psl, 0, TAILLE_MSG);
    memset(psd, 0, TAILLE_MSG);
    memset(mr,  0, TAILLE_MSG);
    memset(me,  0, TAILLE_MSG);

    /* ----- Échange des pseudos ----- */

    /* Recevoir le pseudo du serveur en premier */
    recv(D, psd, TAILLE_MSG, 0);
    printf("Pseudo du serveur distant : %s\n", psd);

    /* Saisir et envoyer son propre pseudo */
    printf("Donner votre pseudo : ");
    fgets(psl, TAILLE_MSG, stdin);
    psl[strcspn(psl, "\n")] = '\0';
    send(D, psl, strlen(psl) + 1, 0);

    /* ----- Boucle tant que (mr != "quitter") et (me != "quitter") ----- */
    while (strcmp(mr, "quitter") != 0 && strcmp(me, "quitter") != 0)
    {
        /* Réinitialiser les buffers */
        memset(mr, 0, TAILLE_MSG);
        memset(me, 0, TAILLE_MSG);

        /* Afficher le pseudo local et saisir un message */
        printf("[%s] : ", psl);
        fgets(me, TAILLE_MSG, stdin);
        me[strcspn(me, "\n")] = '\0';

        /* Envoyer le message au serveur */
        send(D, me, strlen(me) + 1, 0);

        if (strcmp(me, "quitter") == 0) break;

        /* Recevoir la réponse du serveur */
        recv(D, mr, TAILLE_MSG, 0);
        printf("[%s] : %s\n", psd, mr);
    }
    /* Fin tant que */

    close(D);
}
/* Fin */
