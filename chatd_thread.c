/*
 * chatd_thread.c - Serveur Chat avec Threads (version simulant un vrai chat)
 * Usage: ./chatd_thread <port>
 *
 * Deux threads par connexion :
 *   - Thread réception : lit les messages entrants et les affiche
 *   - Thread envoi    : lit le clavier et envoie les messages
 * => Communication simultanée (pas de tour par tour)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define TAILLE_MSG 512
#define NBR_MAX    5

/* Structure partagée entre les deux threads d'une session */
typedef struct {
    int    socket;
    char   psl[TAILLE_MSG];   /* pseudo local   */
    char   psd[TAILLE_MSG];   /* pseudo distant */
    int    quitter;           /* drapeau fin de session */
} Session;

/* ------------------------------------------------------------------ */
/*  Thread de RÉCEPTION                                                */
/* ------------------------------------------------------------------ */
void *thread_reception(void *arg)
{
    Session *s = (Session *)arg;
    char mr[TAILLE_MSG];

    while (!s->quitter) {
        memset(mr, 0, sizeof(mr));
        int n = recv(s->socket, mr, sizeof(mr) - 1, 0);

        if (n <= 0) {
            /* Connexion fermée par l'autre côté */
            printf("\n[Système] L'autre partie a quitté.\n");
            s->quitter = 1;
            break;
        }

        /* Effacer la ligne de saisie courante et afficher le message reçu */
        printf("\r\033[K[%s] : %s\n", s->psd, mr);
        printf("[%s] : ", s->psl);
        fflush(stdout);

        if (strcmp(mr, "quitter") == 0) {
            s->quitter = 1;
            break;
        }
    }

    pthread_exit(NULL);
}

/* ------------------------------------------------------------------ */
/*  Thread d'ENVOI                                                     */
/* ------------------------------------------------------------------ */
void *thread_envoi(void *arg)
{
    Session *s = (Session *)arg;
    char me[TAILLE_MSG];

    while (!s->quitter) {
        printf("[%s] : ", s->psl);
        fflush(stdout);

        memset(me, 0, sizeof(me));
        if (fgets(me, sizeof(me), stdin) == NULL) break;
        me[strcspn(me, "\n")] = '\0';

        send(s->socket, me, strlen(me) + 1, 0);

        if (strcmp(me, "quitter") == 0) {
            s->quitter = 1;
            break;
        }
    }

    pthread_exit(NULL);
}

/* ------------------------------------------------------------------ */
/*  Chats avec threads (côté serveur)                                  */
/* ------------------------------------------------------------------ */
void Chats_thread(int D, struct sockaddr_in addr_dist)
{
    Session s;
    pthread_t tid_recv, tid_send;

    memset(&s, 0, sizeof(s));
    s.socket  = D;
    s.quitter = 0;

    /* Échange des pseudos */
    printf("Saisir votre pseudo name : ");
    fgets(s.psl, sizeof(s.psl), stdin);
    s.psl[strcspn(s.psl, "\n")] = '\0';

    send(D, s.psl, strlen(s.psl) + 1, 0);
    recv(D, s.psd, sizeof(s.psd), 0);

    printf("Pseudo name distant : %s\n", s.psd);
    printf("------ Chat démarré (tapez 'quitter' pour terminer) ------\n\n");

    /* Lancement des threads */
    pthread_create(&tid_recv, NULL, thread_reception, &s);
    pthread_create(&tid_send, NULL, thread_envoi,     &s);

    /* Attente de la fin des deux threads */
    pthread_join(tid_recv, NULL);
    pthread_join(tid_send, NULL);

    close(D);
    printf("\nConnexion fermée avec %s:%d\n",
           inet_ntoa(addr_dist.sin_addr),
           ntohs(addr_dist.sin_port));
}

/* ------------------------------------------------------------------ */
/*  Programme principal Serveur                                        */
/* ------------------------------------------------------------------ */
int main(int argc, char *argv[])
{
    int desc, descc;
    struct sockaddr_in addr_l, addr_distant;
    socklen_t taille_addr;
    int err;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <mon-port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    desc = socket(AF_INET, SOCK_STREAM, 0);
    if (desc < 0) {
        perror("Erreur socket");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(desc, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&addr_l, 0, sizeof(addr_l));
    addr_l.sin_family      = AF_INET;
    addr_l.sin_addr.s_addr = INADDR_ANY;
    addr_l.sin_port        = htons(atoi(argv[1]));

    err = bind(desc, (struct sockaddr *)&addr_l, sizeof(addr_l));
    if (err != 0) { perror("Erreur bind"); exit(EXIT_FAILURE); }

    err = listen(desc, NBR_MAX);
    if (err != 0) { perror("Erreur listen"); exit(EXIT_FAILURE); }

    printf("Serveur [THREADS] en écoute sur le port %s...\n", argv[1]);

    while (1) {
        taille_addr = sizeof(addr_distant);
        descc = accept(desc, (struct sockaddr *)&addr_distant, &taille_addr);

        if (descc > 0) {
            printf("Client connecté : %s:%d\n",
                   inet_ntoa(addr_distant.sin_addr),
                   ntohs(addr_distant.sin_port));

            Chats_thread(descc, addr_distant);
        }
    }

    close(desc);
    return 0;
}
