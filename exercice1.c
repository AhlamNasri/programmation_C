/*
 * Exercice 1 - Programmation Système en C sous Unix
 * Sauvegarde de données dans un fichier lors d'une interruption Ctrl-C (SIGINT)
 * Université Hassan 1er - FIGI
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

/* Données globales à sauvegarder */
int donnees[] = {10, 20, 30, 40, 50};
int nb_donnees = 5;

/* Gestionnaire du signal SIGINT */
void gestionnaire_sigint(int sig) {
    printf("\n[SIGINT reçu] Sauvegarde des données en cours...\n");

    /* Ouvre (ou crée) le fichier en écriture, remplace s'il existe */
    FILE *f = fopen("sauvegarde.txt", "w");
    if (f == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < nb_donnees; i++) {
        fprintf(f, "%d\n", donnees[i]);
    }

    fclose(f);
    printf("Données sauvegardées dans 'sauvegarde.txt'.\n");
    exit(EXIT_SUCCESS);
}

int main(void) {
    /* Modifier le comportement à la réception de SIGINT */
    if (signal(SIGINT, gestionnaire_sigint) == SIG_ERR) {
        perror("signal");
        exit(EXIT_FAILURE);
    }

    printf("Programme en cours d'exécution. Appuyez sur Ctrl-C pour sauvegarder et quitter.\n");

    /* Boucle infinie simulant un traitement */
    while (1) {
        printf("Traitement en cours...\n");
        sleep(2);
    }

    return EXIT_SUCCESS;
}
