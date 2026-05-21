#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <nombre> <prog1> [prog2 ...]\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    if (n <= 0) {
        fprintf(stderr, "Nombre invalide: %s\n", argv[1]);
        return 1;
    }

    for (int i = 2; i < argc; i++) {
        for (int j = 0; j < n; j++) {
            pid_t pid = fork();
            if (pid == 0) {
                execlp(argv[i], argv[i], NULL);
                perror(argv[i]);
                exit(1);
            } else if (pid < 0) {
                perror("fork");
            }
        }
    }

    return 0;
}
