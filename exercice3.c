/*
 * Exercice 3 - Programmation Système en C sous Unix
 * Transmission bidirectionnelle père ↔ fils via deux tubes
 *
 * Université Hassan 1er - FIGI
 *
 * Schéma de communication :
 *
 *   PÈRE  ──[p1: écriture]──►  FILS  (père envoie 5 entiers)
 *   PÈRE  ◄──[p2: lecture]──   FILS  (fils renvoie les doubles)
 *
 *   - p1[1] : père écrit   → p1[0] : fils lit
 *   - p2[1] : fils écrit   → p2[0] : père lit
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define NB_ENTIERS 5

int main(void) {
    int p1[2]; /* tube père → fils */
    int p2[2]; /* tube fils → père */
    pid_t pid;

    /* --- Création des deux tubes --- */
    if (pipe(p1) == -1) {
        perror("pipe p1");
        exit(EXIT_FAILURE);
    }
    if (pipe(p2) == -1) {
        perror("pipe p2");
        exit(EXIT_FAILURE);
    }

    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    /* ============================================================
     *  PROCESSUS FILS
     * ============================================================ */
    if (pid == 0) {
        /* Fermer les extrémités inutilisées par le fils */
        close(p1[1]); /* le fils ne écrit pas dans p1 */
        close(p2[0]); /* le fils ne lit pas dans p2  */

        int valeur, double_val;

        for (int i = 0; i < NB_ENTIERS; i++) {
            /* Lire un entier envoyé par le père via p1 */
            if (read(p1[0], &valeur, sizeof(int)) == -1) {
                perror("fils: read p1");
                exit(EXIT_FAILURE);
            }

            printf("[FILS]  Reçu : %d\n", valeur);

            /* Calculer le double et le renvoyer au père via p2 */
            double_val = valeur * 2;
            if (write(p2[1], &double_val, sizeof(int)) == -1) {
                perror("fils: write p2");
                exit(EXIT_FAILURE);
            }
        }

        /* Fermer les extrémités restantes */
        close(p1[0]);
        close(p2[1]);

        exit(EXIT_SUCCESS);
    }

    /* ============================================================
     *  PROCESSUS PÈRE
     * ============================================================ */
    else {
        /* Fermer les extrémités inutilisées par le père */
        close(p1[0]); /* le père ne lit pas dans p1  */
        close(p2[1]); /* le père n'écrit pas dans p2 */

        int entiers[NB_ENTIERS] = {3, 7, 12, 25, 40};
        int resultat;

        for (int i = 0; i < NB_ENTIERS; i++) {
            printf("[PÈRE]  Envoi de : %d\n", entiers[i]);

            /* Envoyer un entier au fils via p1 */
            if (write(p1[1], &entiers[i], sizeof(int)) == -1) {
                perror("père: write p1");
                exit(EXIT_FAILURE);
            }

            /* Recevoir le double renvoyé par le fils via p2 */
            if (read(p2[0], &resultat, sizeof(int)) == -1) {
                perror("père: read p2");
                exit(EXIT_FAILURE);
            }

            printf("[PÈRE]  Double reçu : %d\n\n", resultat);
        }

        /* Fermer les extrémités restantes */
        close(p1[1]);
        close(p2[0]);

        /* Attendre la fin du fils */
        wait(NULL);
        printf("[PÈRE]  Fils terminé. Fin du programme.\n");
    }

    return EXIT_SUCCESS;
}
