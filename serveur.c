/*
 * serveur.c
 * Algorithme Programme Serveur ChatD
 * Commun Version 1 (Talkie-Walkie) et Version 2 (Chat + Threads)
 *
 * Usage : ./serveur <port>
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
#define NBR_MAX    5

/* ------------------------------------------------------------------ */
/*  Prototype de la fonction Chats (définie plus bas)                  */
/* ------------------------------------------------------------------ */
void Chats(int D);

/* ================================================================== */
/*  MAIN                                                               */
/* ================================================================== */
int main(int argc, char *argv[])
{
    /* ----- Vérification des arguments ----- */
    if (argc < 2) {
        fprintf(stderr, "Usage : %s <port>\n", argv[0]);
        exit(1);
    }

    /* ----- Initialisation des variables ----- */
    int Desc, Descc;
    int err;
    struct sockaddr_in addr_local, addr_distant;
    socklen_t taille = sizeof(addr_distant);

    /* ----- Création du point de communication (socket) ----- */
    Desc = socket(AF_INET /* INET */, SOCK_STREAM /* Connecté, TCP */, 0);
    if (Desc < 0) {
        perror("Erreur socket");
        exit(1);
    }

    /* ----- Préparation de @local ----- */
    memset(&addr_local, 0, sizeof(addr_local));
    addr_local.sin_family      = AF_INET;
    addr_local.sin_addr.s_addr = INADDR_ANY;

    /* ----- Préparation du port ----- */
    addr_local.sin_port = htons(atoi(argv[1]));

    /* ----- Bind ----- */
    err = bind(Desc, (struct sockaddr *)&addr_local, sizeof(addr_local));
    if (err != 0) {
        perror("Erreur bind");
        close(Desc);
        exit(1);
    }

    /* ----- Listen ----- */
    err = listen(Desc, NBR_MAX);
    if (err != 0) {
        perror("Erreur listen");
        close(Desc);
        exit(1);
    }

    printf("Serveur en attente sur le port %s...\n", argv[1]);

    /* ----- Boucle principale : tant que vrai ----- */
    while (1) {
        Descc = accept(Desc, (struct sockaddr *)&addr_distant, &taille);
        if (Descc > 0) {
            printf("Client connecté : %s : %d\n",
                   inet_ntoa(addr_distant.sin_addr),
                   ntohs(addr_distant.sin_port));

            /* Appel de la fonction Chats */
            Chats(Descc);
        }
    }
    /* Fin tant que */

    close(Desc);
    return 0;
}

/* ================================================================== */
/*  FONCTION Chats (côté serveur)                                      */
/*                                                                     */
/*  Protocole d'échange des pseudos :                                  */
/*    1. Serveur envoie son pseudo en premier (Send)                   */
/*    2. Serveur reçoit le pseudo du client   (recv)                   */
/*  Puis boucle talkie-walkie :                                        */
/*    Serveur reçoit un message, puis en envoie un                     */
/* ================================================================== */
void Chats(int D)
{
    char psl[TAILLE_MSG];   /* pseudo local  (du serveur) */
    char psd[TAILLE_MSG];   /* pseudo distant (du client) */
    char mr[TAILLE_MSG];    /* message reçu  */
    char me[TAILLE_MSG];    /* message envoyé */

    /* ----- Initialisation ----- */
    memset(psl, 0, TAILLE_MSG);
    memset(psd, 0, TAILLE_MSG);
    memset(mr,  0, TAILLE_MSG);
    memset(me,  0, TAILLE_MSG);

    /* ----- Échange des pseudos ----- */
    printf("Saisir votre pseudo : ");
    fgets(psl, TAILLE_MSG, stdin);
    psl[strcspn(psl, "\n")] = '\0';   /* supprimer le \n */

    /* Le serveur envoie son pseudo en premier */
    send(D, psl, strlen(psl) + 1, 0);

    /* Le serveur reçoit le pseudo du client */
    recv(D, psd, TAILLE_MSG, 0);
    printf("Pseudo du client distant : %s\n", psd);

    /* ----- Boucle tant que (mr != "quitter") et (me != "quitter") ----- */
    while (strcmp(mr, "quitter") != 0 && strcmp(me, "quitter") != 0)
    {
        /* Réinitialiser les buffers */
        memset(mr, 0, TAILLE_MSG);
        memset(me, 0, TAILLE_MSG);

        /* Recevoir le message du client */
        recv(D, mr, TAILLE_MSG, 0);
        printf("[%s] : %s\n", psd, mr);

        if (strcmp(mr, "quitter") == 0) break;

        /* Envoyer un message au client */
        printf("[%s] : ", psl);
        fgets(me, TAILLE_MSG, stdin);
        me[strcspn(me, "\n")] = '\0';
        send(D, me, strlen(me) + 1, 0);
    }
    /* Fin tant que */

    close(D);
}
/* Fin */
