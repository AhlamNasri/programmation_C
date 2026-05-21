/* cdroit.c - Modifier les droits d'un fichier */
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    /* Vérification des arguments : 3 ou 4 */
    if (argc < 3 || argc > 4) {
        fprintf(stderr, "Usage: %s <nom_fichier> <user|group|other> [lecture|ecriture|execution|all]\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *fichier        = argv[1];
    const char *type_utilisateur = argv[2];
    const char *droit          = (argc == 4) ? argv[3] : NULL;

    /* Récupérer le mode actuel */
    struct stat buffer;
    if (stat(fichier, &buffer) == -1) {
        perror(argv[0]);
        return EXIT_FAILURE;
    }

    mode_t mode = buffer.st_mode;

    /* Vérifier que type_utilisateur est valide */
    if (strcmp(type_utilisateur, "user")  != 0 &&
        strcmp(type_utilisateur, "group") != 0 &&
        strcmp(type_utilisateur, "other") != 0) {
        fprintf(stderr, "Erreur: type_utilisateur doit être 'user', 'group' ou 'other'\n");
        return EXIT_FAILURE;
    }

    /* Cas 1 : argument [droit] absent → supprimer tous les droits du type */
    if (droit == NULL) {
        if      (strcmp(type_utilisateur, "user")  == 0) mode &= ~(S_IRUSR | S_IWUSR | S_IXUSR);
        else if (strcmp(type_utilisateur, "group") == 0) mode &= ~(S_IRGRP | S_IWGRP | S_IXGRP);
        else                                              mode &= ~(S_IROTH | S_IWOTH | S_IXOTH);
    }
    /* Cas 2 : [droit] == "all" → donner tous les droits */
    else if (strcmp(droit, "all") == 0) {
        if      (strcmp(type_utilisateur, "user")  == 0) mode |= (S_IRUSR | S_IWUSR | S_IXUSR);
        else if (strcmp(type_utilisateur, "group") == 0) mode |= (S_IRGRP | S_IWGRP | S_IXGRP);
        else                                              mode |= (S_IROTH | S_IWOTH | S_IXOTH);
    }
    /* Cas 3 : droit spécifique → ajouter uniquement ce droit */
    else if (strcmp(droit, "lecture") == 0) {
        if      (strcmp(type_utilisateur, "user")  == 0) mode |= S_IRUSR;
        else if (strcmp(type_utilisateur, "group") == 0) mode |= S_IRGRP;
        else                                              mode |= S_IROTH;
    }
    else if (strcmp(droit, "ecriture") == 0) {
        if      (strcmp(type_utilisateur, "user")  == 0) mode |= S_IWUSR;
        else if (strcmp(type_utilisateur, "group") == 0) mode |= S_IWGRP;
        else                                              mode |= S_IWOTH;
    }
    else if (strcmp(droit, "execution") == 0) {
        if      (strcmp(type_utilisateur, "user")  == 0) mode |= S_IXUSR;
        else if (strcmp(type_utilisateur, "group") == 0) mode |= S_IXGRP;
        else                                              mode |= S_IXOTH;
    }
    else {
        fprintf(stderr, "Erreur: droit invalide. Choisir: lecture, ecriture, execution, all\n");
        return EXIT_FAILURE;
    }

    /* Appliquer le nouveau mode */
    if (chmod(fichier, mode) == -1) {
        perror(argv[0]);
        return EXIT_FAILURE;
    }

    printf("Droits modifiés avec succès sur '%s'\n", fichier);
    return EXIT_SUCCESS;
}
