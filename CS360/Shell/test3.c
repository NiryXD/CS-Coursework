static void execute_command(char **args, int argc, int confirm,
                           char *input, char *output, int append)
{
    pid_t pid;
    int fd, flags, status;
    
    pid = fork();
    
    if (pid < 0) {
        perror("fork");
        return;
    }
    
    if (pid == 0) {
        if (input != NULL) {
            fd = open(input, O_RDONLY);
            if (fd < 0) {
                perror(input);
                _exit(1);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }
        
        if (output != NULL) {
            flags = O_WRONLY | O_CREAT;
            if (append)
                flags |= O_APPEND;
            else
                flags |= O_TRUNC;
            fd = open(output, flags, 0644);
            if (fd < 0) {
                perror(output);
                _exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }
        
        execvp(args[0], args);
        perror(args[0]);
        _exit(1);
    } else {
        if (confirm)
            add_job(pid, args, argc);
        else
            waitpid(pid, &status, 0);
    }
}