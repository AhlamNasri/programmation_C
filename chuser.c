/* chuser.c - Changer le propriétaire (user) d'un fichier */
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    /* Vérification des arguments */
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <nom_fichier> <nouveau_user>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *fichier      = argv[1];
    const char *nouveau_user = argv[2];

    /* Récupérer l'UID du nouvel utilisateur */
    struct passwd *pw = getpwnam(nouveau_user);
    if (pw == NULL) {
        fprintf(stderr, "utilisateur introuvable: %s\n", nouveau_user);
        return EXIT_FAILURE;
    }
    uid_t nouveau_uid = pw->pw_uid;

    /* Récupérer le GID actuel du fichier (ne pas le modifier) */
    struct stat buffer;
    if (stat(fichier, &buffer) == -1) {
        perror(argv[0]);
        return EXIT_FAILURE;
    }

    /* Appliquer le changement de propriétaire en conservant le GID */
    /* Note : chown() nécessite les privilèges root sur Linux */
    if (chown(fichier, nouveau_uid, buffer.st_gid) == -1) {
        perror(argv[0]);
        return EXIT_FAILURE;
    }

    printf("Propriétaire de '%s' changé en '%s'\n", fichier, nouveau_user);
    return EXIT_SUCCESS;
}
