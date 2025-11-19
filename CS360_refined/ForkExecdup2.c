#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
    if (argc < 4) {
        printf("Usage: %s <output> <error> <command> [args]\n", argv[0]);
        return 1;
    }

    int fd1 = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    int fd2 = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);

    pid_t pid;
    pid = fork();

    if (pid == 0){
        dup2(fd1, STDOUT_FILENO);
        dup2(fd2, STDERR_FILENO);

        execv(argv[3], argv + 3);
    }
    else{
        int status;
        waitpid(pid, &status, 0);
        if(WIFEXITED(status) != 0){
            printf("Child failed.\n");
            return 1;
        }
    }

    return 0;
}
