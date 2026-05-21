#define _GNU_SOURCE

#include <unistd.h>
#include <sys/syscall.h>

int main(int argc, char *argv[])
{
    long ret;

    if (argc != 2)
    {
        write(1, "Usage: ./mon_rmdir <dossier>\n", 30);
        return 1;
    }

    // Appel système bas niveau rmdir
    ret = syscall(SYS_rmdir, argv[1]);

    if (ret < 0)
    {
        write(1, "Erreur suppression dossier\n", 28);
        return 1;
    }

    write(1, "Dossier supprime\n", 18);

    return 0;
}
