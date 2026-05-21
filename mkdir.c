#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage : %s <nom_dossier>\n", argv[0]);
        return 1;
    }

    // Création du dossier avec permissions rwxr-xr-x
    if (mkdir(argv[1], 0755) == -1)
    {
        printf("Erreur : %s\n", strerror(errno));
        return 1;
    }

    printf("Dossier '%s' créé avec succès.\n", argv[1]);

    return 0;
}
