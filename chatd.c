/*
 * chatd.c - Serveur Chat (Talkie-Walkie, sans threads)
 * Usage: ./chatd <port>
 *
 * Basé sur l'algorithme du tableau (Image 3 & 4)
 * Communication alternée : le serveur parle, puis le client, etc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define TAILLE_MSG  512
#define NBR_MAX     5

/* ------------------------------------------------------------------ */
/*  Chats : dialogue côté serveur (Image 4)                            */
/* ------------------------------------------------------------------ */
void Chats(int D)
{
    char psl[TAILLE_MSG];   /* pseudo local  */
    char psd[TAILLE_MSG];   /* pseudo distant */
    char me[TAILLE_MSG];    /* message envoyé */
    char mr[TAILLE_MSG];    /* message reçu   */

    memset(psl, 0, sizeof(psl));
    memset(psd, 0, sizeof(psd));

    /* Le serveur saisit son pseudo et l'envoie en premier */
    printf("Saisir votre pseudo name : ");
    fgets(psl, sizeof(psl), stdin);
    psl[strcspn(psl, "\n")] = '\0';   /* supprimer le \n */

    send(D, psl, strlen(psl) + 1, 0);

    /* Recevoir le pseudo du client */
    recv(D, psd, sizeof(psd), 0);
    printf("Pseudo name distant est : %s\n", psd);
    printf("------ Chat démarré (tapez 'quitter' pour terminer) ------\n\n");

    /* Boucle de dialogue */
    while (1) {
        memset(me, 0, sizeof(me));
        memset(mr, 0, sizeof(mr));

        /* Serveur parle en premier */
        printf("[%s] : ", psl);
        fgets(me, sizeof(me), stdin);
        me[strcspn(me, "\n")] = '\0';

        send(D, me, strlen(me) + 1, 0);

        if (strcmp(me, "quitter") == 0) break;

        /* Recevoir la réponse du client */
        recv(D, mr, sizeof(mr), 0);
        printf("[%s] : %s\n", psd, mr);

        if (strcmp(mr, "quitter") == 0) break;
    }

    close(D);
    printf("Connexion fermée.\n");
}

/* ------------------------------------------------------------------ */
/*  Programme principal Serveur (Image 3)                              */
/* ------------------------------------------------------------------ */
int main(int argc, char *argv[])
{
    int desc, descc;
    struct sockaddr_in addr_l, addr_distant;
    socklen_t taille_addr;
    int err;

    /* Vérification des arguments */
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <mon-port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Création du socket */
    desc = socket(AF_INET, SOCK_STREAM, 0);
    if (desc < 0) {
        perror("Erreur socket");
        exit(EXIT_FAILURE);
    }

    /* Option pour réutiliser l'adresse */
    int opt = 1;
    setsockopt(desc, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    /* Configuration de l'adresse locale */
    memset(&addr_l, 0, sizeof(addr_l));
    addr_l.sin_family      = AF_INET;
    addr_l.sin_addr.s_addr = INADDR_ANY;
    addr_l.sin_port        = htons(atoi(argv[1]));

    /* Bind */
    err = bind(desc, (struct sockaddr *)&addr_l, sizeof(addr_l));
    if (err != 0) {
        perror("Erreur bind");
        exit(EXIT_FAILURE);
    }

    /* Listen */
    err = listen(desc, NBR_MAX);
    if (err != 0) {
        perror("Erreur listen");
        exit(EXIT_FAILURE);
    }

    printf("Serveur en écoute sur le port %s...\n", argv[1]);

    /* Boucle d'acceptation (une connexion à la fois) */
    while (1) {
        taille_addr = sizeof(addr_distant);
        descc = accept(desc, (struct sockaddr *)&addr_distant, &taille_addr);

        if (descc > 0) {
            printf("Client connecté : %s:%d\n",
                   inet_ntoa(addr_distant.sin_addr),
                   ntohs(addr_distant.sin_port));

            Chats(descc);   /* traitement du client */
        }
    }

    close(desc);
    return 0;
}
