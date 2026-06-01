#define _GNU_SOURCE
#include <sys/syscall.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>

/* ----------------- I/O helpers ----------------- */

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

/* int → string */
static void uint_to_str(unsigned long long v, char *buf, int bufsize)
{
    int i = bufsize - 1;
    buf[i] = '\0';

    if (v == 0) {
        buf[--i] = '0';
    }

    while (v && i > 0) {
        buf[--i] = '0' + (v % 10);
        v /= 10;
    }

    int len = bufsize - 1 - i;
    memmove(buf, buf + i, len + 1);
}

/* ----------------- file type ----------------- */

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

/* ----------------- permissions ----------------- */

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

/* ----------------- getdents struct ----------------- */

struct linux_dirent64 {
    ino64_t        d_ino;
    off64_t        d_off;
    unsigned short d_reclen;
    unsigned char  d_type;
    char           d_name[];
};

/* ----------------- core function ----------------- */

#define BUF_SIZE 4096

static int list_dir(const char *path, int recursive)
{
    int fd = open(path, O_RDONLY | O_DIRECTORY);
    if (fd < 0) {
        my_write("ls: cannot open ");
        my_write(path);
        my_write_nl();
        return 1;
    }

    char buf[BUF_SIZE];
    long nread;

    while ((nread = syscall(SYS_getdents64, fd, buf, BUF_SIZE)) > 0) {

        for (long pos = 0; pos < nread; ) {

            struct linux_dirent64 *d =
                (struct linux_dirent64 *)(buf + pos);

            /* skip . and .. */
            if (!(d->d_name[0] == '.' &&
                (d->d_name[1] == '\0' ||
                (d->d_name[1] == '.' && d->d_name[2] == '\0')))) {

                char fullpath[4096];
                size_t plen = strlen(path);
                size_t nlen = strlen(d->d_name);

                memcpy(fullpath, path, plen);
                fullpath[plen] = '/';
                memcpy(fullpath + plen + 1, d->d_name, nlen + 1);

                struct stat st;
                if (stat(fullpath, &st) == 0) {

                    char num_buf[32];

                    /* type */
                    char tc = file_type_char(st.st_mode);
                    write(STDOUT_FILENO, &tc, 1);

                    /* permissions */
                    print_permissions(st.st_mode);
                    write(STDOUT_FILENO, "  ", 2);

                    /* links */
                    uint_to_str(st.st_nlink, num_buf, 32);
                    my_write(num_buf);
                    write(STDOUT_FILENO, "  ", 2);

                    /* size */
                    uint_to_str(st.st_size, num_buf, 32);
                    my_write(num_buf);
                    write(STDOUT_FILENO, "\t", 1);
                }

                my_write(d->d_name);
                my_write_nl();

                /* ----------------- RECURSIVE -r ----------------- */
                if (recursive && S_ISDIR(st.st_mode)) {

                    if (strcmp(d->d_name, ".") != 0 &&
                        strcmp(d->d_name, "..") != 0) {

                        my_write("\n");
                        my_write(fullpath);
                        my_write(":\n");

                        list_dir(fullpath, recursive);
                    }
                }
            }

            pos += d->d_reclen;
        }
    }

    if (nread < 0) {
        my_write("ls: read error\n");
        close(fd);
        return 1;
    }

    close(fd);
    return 0;
}

/* ----------------- main ----------------- */

int main(int argc, char *argv[])
{
    int recursive = 0;
    int start = 1;

    /* check -r */
    if (argc > 1 && strcmp(argv[1], "-r") == 0) {
        recursive = 1;
        start = 2;
    }

    if (start >= argc) {
        return list_dir(".", recursive);
    }

    int ret = 0;

    for (int i = start; i < argc; i++) {

        if (argc - start > 1) {
            my_write(argv[i]);
            my_write(":\n");
        }

        ret |= list_dir(argv[i], recursive);

        if (i < argc - 1) my_write_nl();
    }

    return ret;
}
