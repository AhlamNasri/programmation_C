/*
 * ls.c — Implémentation de la commande ls avec appels bas niveau
 *
 * Appels système utilisés :
 *   open(), getdents64(), write(), stat(), close()
 *
 * Compilation : gcc -o ls ls.c
 * Usage       : ./ls [répertoire]
 */

#define _GNU_SOURCE
#include <sys/syscall.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>

/* ------------------------------------------------------------------ */
/*  Utilitaires d'écriture bas niveau (pas de printf/puts)            */
/* ------------------------------------------------------------------ */

static void my_write(const char *s)
{
    size_t len = 0;
    while (s[len]) len++;
    write(STDOUT_FILENO, s, len);
}

static void my_write_nl(void)
{
    write(STDOUT_FILENO, "\n", 1);
}

/* Conversion entier → chaîne décimale dans un buffer fourni par l'appelant */
static void uint_to_str(unsigned long long v, char *buf, int bufsize)
{
    int i = bufsize - 1;
    buf[i] = '\0';
    if (v == 0) { buf[--i] = '0'; }
    while (v && i > 0) { buf[--i] = '0' + (v % 10); v /= 10; }
    /* déplacer au début du buffer */
    int len = bufsize - 1 - i;
    memmove(buf, buf + i, len + 1);
}

/* ------------------------------------------------------------------ */
/*  Affichage du type de fichier (premier caractère du mode ls -l)    */
/* ------------------------------------------------------------------ */

static char file_type_char(mode_t mode)
{
    if (S_ISDIR(mode))  return 'd';
    if (S_ISLNK(mode))  return 'l';
    if (S_ISBLK(mode))  return 'b';
    if (S_ISCHR(mode))  return 'c';
    if (S_ISFIFO(mode)) return 'p';
    if (S_ISSOCK(mode)) return 's';
    return '-';
}

/* ------------------------------------------------------------------ */
/*  Affichage des permissions (rwxrwxrwx)                             */
/* ------------------------------------------------------------------ */

static void print_permissions(mode_t mode)
{
    char perms[10];
    perms[0] = (mode & S_IRUSR) ? 'r' : '-';
    perms[1] = (mode & S_IWUSR) ? 'w' : '-';
    perms[2] = (mode & S_IXUSR) ? 'x' : '-';
    perms[3] = (mode & S_IRGRP) ? 'r' : '-';
    perms[4] = (mode & S_IWGRP) ? 'w' : '-';
    perms[5] = (mode & S_IXGRP) ? 'x' : '-';
    perms[6] = (mode & S_IROTH) ? 'r' : '-';
    perms[7] = (mode & S_IWOTH) ? 'w' : '-';
    perms[8] = (mode & S_IXOTH) ? 'x' : '-';
    perms[9] = '\0';
    write(STDOUT_FILENO, perms, 9);
}

/* ------------------------------------------------------------------ */
/*  Structure linux_dirent64 (pour getdents64)                        */
/* ------------------------------------------------------------------ */

struct linux_dirent64 {
    ino64_t        d_ino;
    off64_t        d_off;
    unsigned short d_reclen;
    unsigned char  d_type;
    char           d_name[];
};

/* ------------------------------------------------------------------ */
/*  Listage d'un répertoire                                           */
/* ------------------------------------------------------------------ */

#define BUF_SIZE 4096

static int list_dir(const char *path)
{
    /* 1. Ouvrir le répertoire */
    int fd = open(path, O_RDONLY | O_DIRECTORY);
    if (fd < 0) {
        my_write("ls: impossible d'ouvrir : ");
        my_write(path);
        my_write_nl();
        return 1;
    }

    char buf[BUF_SIZE];
    long nread;

    /* 2. Lire les entrées par blocs via getdents64 */
    while ((nread = syscall(SYS_getdents64, fd, buf, BUF_SIZE)) > 0) {

        for (long pos = 0; pos < nread; ) {
            struct linux_dirent64 *d = (struct linux_dirent64 *)(buf + pos);

            /* Ignorer . et .. */
            if (!(d->d_name[0] == '.' &&
                  (d->d_name[1] == '\0' ||
                   (d->d_name[1] == '.' && d->d_name[2] == '\0')))) {

                /* 3. stat() pour obtenir les métadonnées */
                char fullpath[4096];
                size_t plen = strlen(path);
                size_t nlen = strlen(d->d_name);
                memcpy(fullpath, path, plen);
                fullpath[plen] = '/';
                memcpy(fullpath + plen + 1, d->d_name, nlen + 1);

                struct stat st;
                if (stat(fullpath, &st) == 0) {
                    char num_buf[32];

                    /* Type + permissions */
                    char tc = file_type_char(st.st_mode);
                    write(STDOUT_FILENO, &tc, 1);
                    print_permissions(st.st_mode);
                    write(STDOUT_FILENO, "  ", 2);

                    /* Nombre de liens */
                    uint_to_str((unsigned long long)st.st_nlink, num_buf, 32);
                    my_write(num_buf);
                    write(STDOUT_FILENO, "  ", 2);

                    /* Taille en octets */
                    uint_to_str((unsigned long long)st.st_size, num_buf, 32);
                    my_write(num_buf);
                    write(STDOUT_FILENO, "\t", 1);
                }

                /* Nom du fichier */
                my_write(d->d_name);
                my_write_nl();
            }

            pos += d->d_reclen;
        }
    }

    if (nread < 0) {
        my_write("ls: erreur lecture répertoire\n");
        close(fd);
        return 1;
    }

    /* 4. Fermer le descripteur */
    close(fd);
    return 0;
}

/* ------------------------------------------------------------------ */
/*  Point d'entrée                                                    */
/* ------------------------------------------------------------------ */

int main(int argc, char *argv[])
{
    if (argc < 2) {
        return list_dir(".");
    }

    int ret = 0;
    for (int i = 1; i < argc; i++) {
        if (argc > 2) {
            my_write(argv[i]);
            my_write(":\n");
        }
        ret |= list_dir(argv[i]);
        if (argc > 2 && i < argc - 1)
            my_write_nl();
    }
    return ret;
}
