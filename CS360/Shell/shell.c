#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>

#define MAX_LINE 4096
#define MAX_ARGS 256
#define MAX_JOBS 256

typedef struct {
    pid_t pid;
    char *command;
} Job;

static Job jobs[MAX_JOBS];
static int num_jobs = 0;

static void print_prompt(void);
static char *read_line(void);
static char **parse_line(char *line, int *argc, int *background, 
                         char **input_file, char **output_file, int *append);
static char *expand_variables(const char *str);
static int is_valid_var_char(char c);
static void execute_command(char **args, int argc, int background,
                           char *input_file, char *output_file, int append);
static int handle_builtin(char **args, int argc);
static void add_job(pid_t pid, char **args, int argc);
static void remove_job(pid_t pid);
static void check_background_jobs(void);
static void print_jobs(void);
static void free_args(char **args, int argc);
static void cleanup_jobs(void);

int main(void)
{
    char *line;
    char **args;
    int argc;
    int background;
    char *input_file, *output_file;
    int append;

    while (1) {
        check_background_jobs();
        print_prompt();
        fflush(stdout);
        line = read_line();
        if (line == NULL) {
            printf("\n");
            break;
        }

        if (strlen(line) == 0) {
            free(line);
            continue;
        }

        args = parse_line(line, &argc, &background, &input_file, &output_file, &append);
        free(line);

        if (args == NULL || argc == 0) {
            if (args != NULL) {
                free(args);
            }
            continue;
        }

        int builtin_result = handle_builtin(args, argc);
        if (builtin_result == -1) {
            free_args(args, argc);
            free(args);
            if (input_file) free(input_file);
            if (output_file) free(output_file);
            break;
        } else if (builtin_result == 1) {
            free_args(args, argc);
            free(args);
            if (input_file) free(input_file);
            if (output_file) free(output_file);
            continue;
        }

        execute_command(args, argc, background, input_file, output_file, append);

        free_args(args, argc);
        free(args);
        if (input_file) free(input_file);
        if (output_file) free(output_file);
    }

    cleanup_jobs();

    return 0;
}

static void print_prompt(void)
{
    char *user = getenv("USER");
    char *home = getenv("HOME");
    char cwd[MAX_LINE];

    if (user == NULL) {
        user = "unknown";
    }

    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        strcpy(cwd, "?");
    }

    if (home != NULL && strncmp(cwd, home, strlen(home)) == 0) {
        char temp[MAX_LINE];
        temp[0] = '~';
        strcpy(temp + 1, cwd + strlen(home));
        strcpy(cwd, temp);
    }

    printf("%s@cosc360:%s> ", user, cwd);
}

static char *read_line(void)
{
    char *line = NULL;
    size_t bufsize = 0;
    ssize_t len;

    len = getline(&line, &bufsize, stdin);

    if (len == -1) {
        free(line);
        return NULL;
    }

    if (len > 0 && line[len - 1] == '\n') {
        line[len - 1] = '\0';
    }

    return line;
}

static int is_valid_var_char(char c)
{
    return isalpha(c) || c == '_';
}

static char *expand_variables(const char *str)
{
    char *result = malloc(MAX_LINE);
    if (result == NULL) return NULL;

    size_t result_idx = 0;
    size_t i = 0;

    while (str[i] != '\0' && result_idx < MAX_LINE - 1) {
        if (str[i] == '$') {
            i++;
            char var_name[MAX_LINE];
            size_t var_idx = 0;

            while (str[i] != '\0' && is_valid_var_char(str[i]) && var_idx < MAX_LINE - 1) {
                var_name[var_idx++] = str[i++];
            }
            var_name[var_idx] = '\0';

            if (var_idx > 0) {
                char *value = getenv(var_name);
                if (value != NULL) {
                    size_t value_len = strlen(value);
                    if (result_idx + value_len < MAX_LINE - 1) {
                        strcpy(result + result_idx, value);
                        result_idx += value_len;
                    }
                }
            } else {
                result[result_idx++] = '$';
            }
        } else {
            result[result_idx++] = str[i++];
        }
    }

    result[result_idx] = '\0';
    return result;
}

static char **parse_line(char *line, int *argc, int *background,
                         char **input_file, char **output_file, int *append)
{
    char **args = malloc(MAX_ARGS * sizeof(char *));
    if (args == NULL) return NULL;

    *argc = 0;
    *background = 0;
    *input_file = NULL;
    *output_file = NULL;
    *append = 0;

    char *token;
    char *rest = line;

    while ((token = strtok_r(rest, " \t", &rest)) != NULL) {
        if (strcmp(token, "&") == 0) {
            *background = 1;
            continue;
        }

        if (token[0] == '<') {
            char *filename = token + 1;
            char *expanded = expand_variables(filename);
            *input_file = expanded;
            continue;
        }

        if (token[0] == '>' && token[1] == '>') {
            char *filename = token + 2;
            char *expanded = expand_variables(filename);
            *output_file = expanded;
            *append = 1;
            continue;
        }

        if (token[0] == '>') {
            char *filename = token + 1;
            char *expanded = expand_variables(filename);
            *output_file = expanded;
            *append = 0;
            continue;
        }

        size_t len = strlen(token);
        if (len > 0 && token[len - 1] == '&') {
            token[len - 1] = '\0';
            *background = 1;
            if (strlen(token) == 0) {
                continue;
            }
        }

        char *expanded = expand_variables(token);
        if (expanded != NULL) {
            args[*argc] = expanded;
            (*argc)++;
        }
    }

    args[*argc] = NULL;
    return args;
}

static int handle_builtin(char **args, int argc)
{
    if (argc == 0) return 0;

    if (strcmp(args[0], "exit") == 0) {
        return -1;
    }

    if (strcmp(args[0], "cd") == 0) {
        char *target;

        if (argc == 1) {
            target = getenv("HOME");
        } else if (strcmp(args[1], "~") == 0) {
            target = getenv("HOME");
        } else {
            target = args[1];
        }

        if (target == NULL) {
            fprintf(stderr, "cd: HOME not set\n");
        } else if (chdir(target) != 0) {
            fprintf(stderr, "%s: %s\n", target, strerror(errno));
        }

        return 1;
    }

    if (strcmp(args[0], "jobs") == 0) {
        check_background_jobs();
        print_jobs();
        return 1;
    }

    return 0;
}

static void execute_command(char **args, int argc, int background,
                           char *input_file, char *output_file, int append)
{
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        if (input_file != NULL) {
            int fd = open(input_file, O_RDONLY);
            if (fd < 0) {
                perror(input_file);
                _exit(1);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }

        if (output_file != NULL) {
            int flags = O_WRONLY | O_CREAT;
            if (append) {
                flags |= O_APPEND;
            } else {
                flags |= O_TRUNC;
            }
            int fd = open(output_file, flags, 0644);
            if (fd < 0) {
                perror(output_file);
                _exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        execvp(args[0], args);

        perror(args[0]);
        _exit(1);
    } else {
        if (background) {
            add_job(pid, args, argc);
        } else {
            int status;
            waitpid(pid, &status, 0);
        }
    }
}

static void add_job(pid_t pid, char **args, int argc)
{
    if (num_jobs >= MAX_JOBS) {
        fprintf(stderr, "Too many background jobs\n");
        return;
    }

    size_t cmd_len = 0;
    for (int i = 0; i < argc; i++) {
        cmd_len += strlen(args[i]) + 1;
    }

    char *cmd = malloc(cmd_len);
    if (cmd == NULL) {
        perror("malloc");
        return;
    }

    cmd[0] = '\0';
    for (int i = 0; i < argc; i++) {
        if (i > 0) strcat(cmd, " ");
        strcat(cmd, args[i]);
    }

    jobs[num_jobs].pid = pid;
    jobs[num_jobs].command = cmd;
    num_jobs++;
}

static void remove_job(pid_t pid)
{
    for (int i = 0; i < num_jobs; i++) {
        if (jobs[i].pid == pid) {
            free(jobs[i].command);
            for (int j = i; j < num_jobs - 1; j++) {
                jobs[j] = jobs[j + 1];
            }
            num_jobs--;
            return;
        }
    }
}

static void check_background_jobs(void)
{
    int status;
    pid_t pid;

    for (int i = num_jobs - 1; i >= 0; i--) {
        pid = waitpid(jobs[i].pid, &status, WNOHANG);
        if (pid > 0) {
            remove_job(pid);
        }
    }
}

static void print_jobs(void)
{
    if (num_jobs == 1) {
        printf("1 job.\n");
    } else {
        printf("%d jobs.\n", num_jobs);
    }

    for (int i = 0; i < num_jobs; i++) {
        printf("   %-6d - %s\n", jobs[i].pid, jobs[i].command);
    }
}

static void free_args(char **args, int argc)
{
    for (int i = 0; i < argc; i++) {
        free(args[i]);
    }
}

static void cleanup_jobs(void)
{
    for (int i = 0; i < num_jobs; i++) {
        free(jobs[i].command);
    }
}