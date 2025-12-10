static void aJob(pid_t pid, char **args, int argc) {
    size_t length = 0;
    char *cmd;
    int i;

    if (nJobs >= MAXJOBS) {
        fprintf(stderr, "Too many jobs\n");
        return;
    }

    for (i = 0; i < argc; i++)
        length += strlen(args[i]) + 1;

    cmd = malloc(length);
    if (cmd == NULL) {
        perror("malloc");
        return;
    }

    cmd[0] = '\0';
    for (i = 0; i < argc; i++) {
        if (i > 0) strcat(cmd, " ");
        strcat(cmd, args[i]);
    }

    jobs[nJobs].pid = pid;
    jobs[nJobs].command = cmd;
    nJobs++;
}