/* suppr.c - Supprimer un fichier via unlink() */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    /* Vérification des arguments */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <nom_fichier>\n", argv[0]);
        return EXIT_FAILURE;
    }

    /* Suppression du fichier */
    if (unlink(argv[1]) == -1) {
        perror(argv[0]);
        return EXIT_FAILURE;
    }

    printf("Fichier '%s' supprimé avec succès.\n", argv[1]);
    return EXIT_SUCCESS;
}
