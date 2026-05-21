/*
 * chatc.c - Client Chat (Talkie-Walkie, sans threads)
 * Usage: ./chatc <IP_serveur> <port>
 *
 * Basé sur l'algorithme du tableau (Image 1 & 2)
 * Communication alternée : le client reçoit d'abord, puis répond.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define TAILLE_MSG 512

/* ------------------------------------------------------------------ */
/*  Chatc : dialogue côté client (Image 2)                             */
/* ------------------------------------------------------------------ */
void Chatc(int D)
{
    char psl[TAILLE_MSG];   /* pseudo local   */
    char psd[TAILLE_MSG];   /* pseudo distant */
    char me[TAILLE_MSG];    /* message envoyé */
    char mr[TAILLE_MSG];    /* message reçu   */

    memset(psl, 0, sizeof(psl));
    memset(psd, 0, sizeof(psd));

    /* Recevoir le pseudo du serveur en premier */
    recv(D, psd, sizeof(psd), 0);
    printf("Le pseudo name distant est : %s\n", psd);

    /* Saisir et envoyer son propre pseudo */
    printf("Donner votre pseudo name : ");
    fgets(psl, sizeof(psl), stdin);
    psl[strcspn(psl, "\n")] = '\0';

    send(D, psl, strlen(psl) + 1, 0);

    printf("------ Chat démarré (tapez 'quitter' pour terminer) ------\n\n");

    /* Boucle de dialogue */
    while (1) {
        memset(mr, 0, sizeof(mr));
        memset(me, 0, sizeof(me));

        /* Recevoir le message du serveur */
        recv(D, mr, sizeof(mr), 0);
        printf("[%s] : %s\n", psd, mr);

        if (strcmp(mr, "quitter") == 0) break;

        /* Répondre */
        printf("[%s] : ", psl);
        fgets(me, sizeof(me), stdin);
        me[strcspn(me, "\n")] = '\0';

        send(D, me, strlen(me) + 1, 0);

        if (strcmp(me, "quitter") == 0) break;
    }

    close(D);
    printf("Connexion fermée.\n");
}

/* ------------------------------------------------------------------ */
/*  Programme principal Client (Image 1)                               */
/* ------------------------------------------------------------------ */
int main(int argc, char *argv[])
{
    int desc;
    struct sockaddr_in addr_distant;
    int err;

    /* Vérification des arguments */
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <IP_serveur> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Création du socket */
    desc = socket(AF_INET, SOCK_STREAM, 0);
    if (desc < 0) {
        perror("Erreur socket");
        exit(EXIT_FAILURE);
    }

    /* Configuration de l'adresse distante */
    memset(&addr_distant, 0, sizeof(addr_distant));
    addr_distant.sin_family = AF_INET;
    addr_distant.sin_port   = htons(atoi(argv[2]));

    if (inet_pton(AF_INET, argv[1], &addr_distant.sin_addr) <= 0) {
        fprintf(stderr, "Adresse IP invalide : %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    /* Connexion au serveur */
    err = connect(desc, (struct sockaddr *)&addr_distant, sizeof(addr_distant));
    if (err != 0) {
        perror("Erreur connect");
        exit(EXIT_FAILURE);
    }

    printf("Connecté au serveur %s:%s\n", argv[1], argv[2]);

    Chatc(desc);

    return 0;
}
