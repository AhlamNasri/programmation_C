#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

typedef struct Noeud {
    char *app;
    struct Noeud *suivant;
} Noeud;

Noeud *ajouter(Noeud *tete, const char *app) {
    Noeud *n = malloc(sizeof(Noeud));
    n->app = strdup(app);
    n->suivant = tete;
    return n;
}

void lancer(Noeud *tete) {
    for (Noeud *c = tete; c; c = c->suivant) {
        pid_t pid = fork();
        if (pid == 0) {
            execlp(c->app, c->app, NULL);
            perror(c->app);
            exit(1);
        } else if (pid < 0) {
            perror("fork");
        }
    }
    while (wait(NULL) > 0);
}

void liberer(Noeud *tete) {
    while (tete) {
        Noeud *tmp = tete;
        tete = tete->suivant;
        free(tmp->app);
        free(tmp);
    }
}

int main() {
    Noeud *liste = NULL;
    char buf[256];

    printf("Entrez les applications (ligne vide pour terminer) :\n");
    while (fgets(buf, sizeof(buf), stdin)) {
        buf[strcspn(buf, "\n")] = 0;
        if (buf[0] == '\0') break;
        liste = ajouter(liste, buf);
    }

    lancer(liste);
    liberer(liste);
    return 0;
}
