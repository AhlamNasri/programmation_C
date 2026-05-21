/* chgroup.c - Changer le groupe propriétaire d'un fichier */
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <grp.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    /* Vérification des arguments */
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <nom_fichier> <nouveau_groupe>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *fichier        = argv[1];
    const char *nouveau_groupe = argv[2];

    /* Récupérer le GID du nouveau groupe */
    struct group *grp = getgrnam(nouveau_groupe);
    if (grp == NULL) {
        fprintf(stderr, "groupe introuvable: %s\n", nouveau_groupe);
        return EXIT_FAILURE;
    }
    gid_t nouveau_gid = grp->gr_gid;

    /* Récupérer l'UID actuel du fichier (ne pas le modifier) */
    struct stat buffer;
    if (stat(fichier, &buffer) == -1) {
        perror(argv[0]);
        return EXIT_FAILURE;
    }

    /* Appliquer le changement de groupe en conservant l'UID */
    if (chown(fichier, buffer.st_uid, nouveau_gid) == -1) {
        perror(argv[0]);
        return EXIT_FAILURE;
    }

    printf("Groupe de '%s' changé en '%s'\n", fichier, nouveau_groupe);
    return EXIT_SUCCESS;
}
