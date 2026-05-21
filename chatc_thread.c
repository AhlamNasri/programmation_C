/*
 * chatc_thread.c - Client Chat avec Threads (version simulant un vrai chat)
 * Usage: ./chatc_thread <IP_serveur> <port>
 *
 * Deux threads :
 *   - Thread réception : affiche les messages entrants
 *   - Thread envoi    : lit le clavier et envoie
 * => Pas de tour par tour, communication simultanée
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

/* Structure partagée entre les threads d'une session */
typedef struct {
    int  socket;
    char psl[TAILLE_MSG];
    char psd[TAILLE_MSG];
    int  quitter;
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
            printf("\n[Système] L'autre partie a quitté.\n");
            s->quitter = 1;
            break;
        }

        /* Effacer la ligne courante, afficher le message, ré-afficher le prompt */
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
/*  Chatc avec threads (côté client)                                   */
/* ------------------------------------------------------------------ */
void Chatc_thread(int D)
{
    Session s;
    pthread_t tid_recv, tid_send;

    memset(&s, 0, sizeof(s));
    s.socket  = D;
    s.quitter = 0;

    /* Recevoir le pseudo du serveur, envoyer le sien */
    recv(D, s.psd, sizeof(s.psd), 0);
    printf("Le pseudo name distant est : %s\n", s.psd);

    printf("Donner votre pseudo name : ");
    fgets(s.psl, sizeof(s.psl), stdin);
    s.psl[strcspn(s.psl, "\n")] = '\0';

    send(D, s.psl, strlen(s.psl) + 1, 0);

    printf("------ Chat démarré (tapez 'quitter' pour terminer) ------\n\n");

    /* Lancement des threads */
    pthread_create(&tid_recv, NULL, thread_reception, &s);
    pthread_create(&tid_send, NULL, thread_envoi,     &s);

    pthread_join(tid_recv, NULL);
    pthread_join(tid_send, NULL);

    close(D);
    printf("\nDéconnecté du serveur.\n");
}

/* ------------------------------------------------------------------ */
/*  Programme principal Client                                         */
/* ------------------------------------------------------------------ */
int main(int argc, char *argv[])
{
    int desc;
    struct sockaddr_in addr_distant;
    int err;

    if (argc < 3) {
        fprintf(stderr, "Usage: %s <IP_serveur> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    desc = socket(AF_INET, SOCK_STREAM, 0);
    if (desc < 0) {
        perror("Erreur socket");
        exit(EXIT_FAILURE);
    }

    memset(&addr_distant, 0, sizeof(addr_distant));
    addr_distant.sin_family = AF_INET;
    addr_distant.sin_port   = htons(atoi(argv[2]));

    if (inet_pton(AF_INET, argv[1], &addr_distant.sin_addr) <= 0) {
        fprintf(stderr, "Adresse IP invalide : %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    err = connect(desc, (struct sockaddr *)&addr_distant, sizeof(addr_distant));
    if (err != 0) {
        perror("Erreur connect");
        exit(EXIT_FAILURE);
    }

    printf("Connecté au serveur [THREADS] %s:%s\n", argv[1], argv[2]);

    Chatc_thread(desc);

    return 0;
}
