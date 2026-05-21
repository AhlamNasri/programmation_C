/*
 * client_v2.c
 * Version 2 — Chat avec THREADS (envoi et réception simultanés)
 *
 * Usage : ./client_v2 <IP_serveur> <port>
 *
 * Compilation : gcc client_v2.c -o client_v2 -lpthread
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define TAILLE_MSG 1024

/* ------------------------------------------------------------------ */
/*  Structure partagée entre les deux threads                          */
/* ------------------------------------------------------------------ */
typedef struct {
    int    D;                   /* descripteur socket          */
    char   psl[TAILLE_MSG];     /* pseudo local                */
    char   psd[TAILLE_MSG];     /* pseudo distant              */
    int    quitter;             /* flag d'arrêt partagé        */
} t_args;

/* ------------------------------------------------------------------ */
/*  Prototypes                                                         */
/* ------------------------------------------------------------------ */
void  Chats(int D);
void *thread_envoi(void *arg);
void *thread_reception(void *arg);

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
/*  THREAD ENVOI                                                       */
/*  Lit en boucle depuis stdin et envoie au serveur                   */
/* ================================================================== */
void *thread_envoi(void *arg)
{
    t_args *a = (t_args *)arg;
    char me[TAILLE_MSG];

    while (!a->quitter)
    {
        memset(me, 0, TAILLE_MSG);

        printf("[%s] : ", a->psl);
        fflush(stdout);

        if (fgets(me, TAILLE_MSG, stdin) == NULL) break;
        me[strcspn(me, "\n")] = '\0';

        send(a->D, me, strlen(me) + 1, 0);

        if (strcmp(me, "quitter") == 0) {
            a->quitter = 1;
            break;
        }
    }
    return NULL;
}

/* ================================================================== */
/*  THREAD RECEPTION                                                   */
/*  Reçoit en boucle les messages du serveur et les affiche           */
/* ================================================================== */
void *thread_reception(void *arg)
{
    t_args *a = (t_args *)arg;
    char mr[TAILLE_MSG];

    while (!a->quitter)
    {
        memset(mr, 0, TAILLE_MSG);

        int n = recv(a->D, mr, TAILLE_MSG, 0);
        if (n <= 0) {
            /* Serveur déconnecté */
            printf("\nServeur déconnecté.\n");
            a->quitter = 1;
            break;
        }

        printf("\n[%s] : %s\n", a->psd, mr);
        printf("[%s] : ", a->psl);
        fflush(stdout);

        if (strcmp(mr, "quitter") == 0) {
            a->quitter = 1;
            break;
        }
    }
    return NULL;
}

/* ================================================================== */
/*  FONCTION Chats — Version Chat avec Threads                        */
/*                                                                     */
/*  Protocole d'échange des pseudos :                                  */
/*    1. Client saisit son pseudo et l'envoie   (Send)                 */
/*    2. Client reçoit le pseudo du serveur     (recv)                 */
/*  Puis : création de 2 threads                                       */
/*    - thread_envoi    : lit stdin → envoie                          */
/*    - thread_reception: reçoit   → affiche                          */
/* ================================================================== */
void Chats(int D)
{
    t_args  args;
    pthread_t tid_envoi, tid_reception;

    /* ----- Initialisation ----- */
    memset(&args, 0, sizeof(args));
    args.D       = D;
    args.quitter = 0;

    /* ----- Échange des pseudos ----- */
    printf("Saisir votre pseudo : ");
    fgets(args.psl, TAILLE_MSG, stdin);
    args.psl[strcspn(args.psl, "\n")] = '\0';

    /* Envoyer son pseudo au serveur */
    send(D, args.psl, strlen(args.psl) + 1, 0);

    /* Recevoir le pseudo du serveur */
    recv(D, args.psd, TAILLE_MSG, 0);
    printf("Pseudo du serveur distant : %s\n", args.psd);

    /* ----- Tant que (mr != "quitter") et (me != "quitter") ----- */
    /*       → géré par le flag args.quitter dans chaque thread     */

    /* Créer le thread d'envoi */
    pthread_create(&tid_envoi,    NULL, thread_envoi,    (void *)&args);

    /* Créer le thread de réception */
    pthread_create(&tid_reception, NULL, thread_reception, (void *)&args);

    /* Attendre la fin des deux threads */
    pthread_join(tid_envoi,    NULL);
    pthread_join(tid_reception, NULL);

    /* Fin tant que */

    close(D);
}
/* Fin */
