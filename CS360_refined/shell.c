#include <stdio.h>
#include <unistd.h>

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>

// When using the Run button above, this will
// pipe in the input from tests.txt
// Compilation: gcc -Wall -Wextra -o /tmp/runner shell.c
// Running: /tmp/runner < tests.txt


// Place holders for now. LL if needed
#define MAXLINE 4096
#define MAXARGS 256
#define MAXJOBS 256

typedef struct {
    pid_t pid;
    char *command;
} Job;

static Job jobs[MAXJOBS];
static Job *lJob = NULL;
static int nJobs = 0;

static int validChar(char n) {
    return isalpha(n) || n == '_';
}

static void rJob(pid_t pid) {
    int i;
    int j;
    for (i = 0; i < nJobs; i++) {
        if (jobs[i].pid == pid) {
            free(jobs[i].command);
            for(j = i; j < nJobs - 1; j++)
            jobs[j] = jobs[j + 1];
            nJobs--;
            return;
        }
    }
}

static char *variableExpansion(const char *string) {
    char *result = malloc(MAXLINE);
    size_t rIndex = 0;
    size_t i = 0;
    if (result == NULL) return NULL;

    while (string[i] != '\0' && rIndex < MAXLINE - 1) {
        if (string[i] == '$') {
            char vName[MAXLINE];
            size_t vIndex = 0;
            i++;

            while (string[i] != '\0' && validChar(string[i]) && vIndex < MAXLINE - 1) {
                vName[vIndex++] = string[i++];
            }
            vName[vIndex] = '\0';

            if (vIndex > 0) {
                char *value = getenv(vName);
                // read enviroment variables
                if (value != NULL) {
                    size_t vLen = strlen(value);
                    if (rIndex + vLen < MAXLINE - 1) {
                        strcpy(result + rIndex, value);
                        rIndex += vLen;
                    }
                }
            }
            else {
                result[rIndex++] = '$';
            }
        }
        else {
            result[rIndex++] = string[i++];
        }
    }

    result[rIndex++] = '\0';
    return result;
}

static void Promptlol(void) {
    char *user = getenv("USER");
    char *home = getenv("HOME");
    char cwd[MAXLINE];

    if (user == NULL) user = "?";
    if (getcwd(cwd, sizeof(cwd)) == NULL) strcpy(cwd, "?");
    if (home != NULL && strncmp(cwd, home, strlen(home)) == 0) {
        char temp[MAXLINE];
        temp[0] = '~';
        strcpy(temp + 1, cwd + strlen(home));
        strcpy(cwd, temp);
    }

    printf("%s@cosc360:%s> ", user, cwd);
}

static char *reads(void) {
    char *line = NULL;
    size_t buffer = 0;
    ssize_t length;

    length = getline(&line, &buffer, stdin);

    if (length == -1){
        free(line);
        return NULL;
    }

        if(length > 0 && line[length - 1] == '\n') line[length - 1] = '\0';

        return line;
    
}

static void freeArguments (char **args, int argc) {
    for (int i = 0; i < argc; i++) free (args[i]);
}

static char **parsing(char *line, int *argc, int *confirm, char **input, char **output, int *append) {
    char **args = malloc(MAXARGS * sizeof(char *));
    char *token;
    char *remain = line;

    if (args == NULL) return NULL;

    *argc = 0;
    *confirm = 0;
    *input = NULL;
    *output = NULL;
    *append= 0;

    while ((token = strtok_r(remain, " \t", &remain)) != NULL) {
    // https://www.geeksforgeeks.org/cpp/strtok-strtok_r-functions-c-examples/
    size_t length = strlen(token);

    if (strcmp(token, "&") == 0) {
        *confirm = 1;
        continue;
    }

    if (token[0] == '<') {
        *input = variableExpansion(token + 1);
        continue;
    }

    if (token[0] == '>' && token[1] == '>') {
        *output = variableExpansion(token + 2);
        *append = 1;
        continue;
    }

    if (token[0] == '>') {
        *output = variableExpansion(token + 1);
        *append = 0;
        continue;
    }

    if (length > 0 && token[length - 1] == '&') {
        token[length - 1] = '\0';
        *confirm = 1;
        if (strlen(token) == 0) continue;
    }

    args[*argc] = variableExpansion(token);
    if (args[*argc] != NULL)
    (*argc)++;
    }

    args[*argc] = NULL;
    return args;
}

static void cJob(void) {
    int i;
    int check;
    pid_t pid;

    for (i = nJobs - 1; i >= 0; i--) {
        // https://stackoverflow.com/questions/33508997/waitpid-wnohang-wuntraced-how-do-i-use-these
        pid = waitpid(jobs[i].pid, &check, WNOHANG);
        if (pid > 0) rJob(pid);
    }
}

static void pJobs(void) {
    if (nJobs == 1) printf ("1 job.\n");
    else printf ("%d jobs.\n", nJobs);

    for (int i = 0; i < nJobs; i++) 
    printf("   %-6d - %s\n", jobs[i].pid, jobs[i].command);

}

static void cleanUp(void) {
    for (int i = 0; i < nJobs; i++)
    free(jobs[i].command);
}

static int executeOrder(char **args, int argc) {
    char *path;

    if (argc == 0) return 0;

    if (strcmp(args[0], "exit") == 0) return -1;

    if (strcmp(args[0], "cd") == 0) {
        if (argc == 1) path = getenv("HOME");
        else if (strcmp(args[1], "~") == 0)
        path = getenv("HOME");
        else path = args[1];
        if (path == NULL)
        fprintf(stderr, "cd: HOME not set\n");
        else if (chdir(path) != 0)
        fprintf(stderr, "%s: %s\n", path, strerror(errno));
        return 1;
    }

    if (strcmp(args[0], "jobs" ) == 0 ) {
        cJob();
        pJobs();
        return 1;
    }

    return 0;
}

static void aJob(pid_t pid, char **args, int argc) {
    size_t length = 0;
    char *cmd;
    int i;

    if (nJobs >= MAXJOBS) {
        return;
    }

    for (i = 0; i < argc; i++) 
    length + strlen(args[i]) + 1;

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

static void executeAgain(char **args, int argc, int confirm, char *input, char *output, int append) {
    pid_t pid;
    int fd;
    int flags;
    int status;

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
                exit(1);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }

        if(output != NULL) {
            flags = O_WRONLY | O_CREAT;
            if (append)
            flags |= O_APPEND;
            else 
            flags |= O_TRUNC;
            fd = open(output, flags, 0644);
            if (fd < 0) {
                perror(output);
                exit(1);
            }

            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        execvp(args[0], args);
        perror(args[0]);
        exit(1);
    }

    else {
        if (confirm)
        aJob(pid, args, argc);
        else
        waitpid(pid, &status, 0);
    }
}


int main()
{
    char *line;
    char **args;
    int argc;
    int confirm;
    int append;
    int result;
    char *input;
    char *output;

    while (1) {
       cJob();
       Promptlol();
        fflush(stdout);

        line = reads();
        if (line == NULL) {
            printf("\n");
            break;
        }

        if (strlen(line) == 0) {
            free(line);
            continue;
        }

        args = parsing(line, &argc, &confirm, &input, &output, &append);
        free(line);

        if (args == NULL || argc == 0) {
            if (args != NULL) 
            free(args); continue;
        }

        result = executeOrder(args, argc);
        if (result == -1) {
            freeArguments(args, argc);
            free(args);
            if (input) free(input);
            if (output) free (output);
            break;
        }
        else if (result == 1) {
            freeArguments(args, argc);
            free(args);
            if (input) free(input);
            if (output) free (output);
            continue;
        }

        executeAgain(args, argc, confirm, input, output, append);

        freeArguments(args, argc);
        free(args);
        if (input) free(input);
        if (output) free (output);
    }

    cleanUp();
    return 0;
}
