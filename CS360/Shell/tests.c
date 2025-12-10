#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>

typedef struct Job {
    pid_t pid;
    char *command;
    struct Job *next;
} Job;

static Job *job_list = NULL;
static int num_jobs = 0;

static int is_valid_var_char(char c)
{
    return isalpha(c) || c == '_';
}

static char *expand_variables(const char *str)
{
    size_t result_size = 256;
    char *result = malloc(result_size);
    size_t result_idx = 0;
    size_t i = 0;
    
    if (result == NULL)
        return NULL;
    
    while (str[i] != '\0') {
        if (str[i] == '$') {
            size_t var_size = 64;
            size_t var_idx = 0;
            char *var_name = malloc(var_size);
            char *value;
            size_t value_len;
            
            if (var_name == NULL) {
                free(result);
                return NULL;
            }
            i++;
            
            while (str[i] != '\0' && is_valid_var_char(str[i])) {
                if (var_idx >= var_size - 1) {
                    var_size *= 2;
                    var_name = realloc(var_name, var_size);
                    if (var_name == NULL) {
                        free(result);
                        return NULL;
                    }
                }
                var_name[var_idx++] = str[i++];
            }
            var_name[var_idx] = '\0';
            
            if (var_idx > 0) {
                value = getenv(var_name);
                if (value != NULL) {
                    value_len = strlen(value);
                    while (result_idx + value_len >= result_size - 1) {
                        result_size *= 2;
                        result = realloc(result, result_size);
                        if (result == NULL) {
                            free(var_name);
                            return NULL;
                        }
                    }
                    strcpy(result + result_idx, value);
                    result_idx += value_len;
                }
            } else {
                if (result_idx >= result_size - 1) {
                    result_size *= 2;
                    result = realloc(result, result_size);
                    if (result == NULL) {
                        free(var_name);
                        return NULL;
                    }
                }
                result[result_idx++] = '$';
            }
            free(var_name);
        } else {
            if (result_idx >= result_size - 1) {
                result_size *= 2;
                result = realloc(result, result_size);
                if (result == NULL)
                    return NULL;
            }
            result[result_idx++] = str[i++];
        }
    }
    
    result[result_idx] = '\0';
    return result;
}

static void print_prompt(void)
{
    char *user = getenv("USER");
    char *home = getenv("HOME");
    char *cwd = getcwd(NULL, 0);
    
    if (user == NULL)
        user = "unknown";
    
    if (cwd == NULL) {
        printf("%s@cosc360:?> ", user);
        return;
    }
    
    if (home != NULL && strncmp(cwd, home, strlen(home)) == 0) {
        printf("%s@cosc360:~%s> ", user, cwd + strlen(home));
    } else {
        printf("%s@cosc360:%s> ", user, cwd);
    }
    free(cwd);
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
    
    if (len > 0 && line[len - 1] == '\n')
        line[len - 1] = '\0';
    
    return line;
}

static char **parse_line(char *line, int *argc, int *background,
                         char **input_file, char **output_file, int *append)
{
    size_t args_size = 8;
    char **args = malloc(args_size * sizeof(char *));
    char *token;
    char *rest = line;
    
    if (args == NULL)
        return NULL;
    
    *argc = 0;
    *background = 0;
    *input_file = NULL;
    *output_file = NULL;
    *append = 0;
    
    while ((token = strtok_r(rest, " \t", &rest)) != NULL) {
        size_t len = strlen(token);
        
        if (strcmp(token, "&") == 0) {
            *background = 1;
            continue;
        }
        
        if (token[0] == '<') {
            *input_file = expand_variables(token + 1);
            continue;
        }
        
        if (token[0] == '>' && token[1] == '>') {
            *output_file = expand_variables(token + 2);
            *append = 1;
            continue;
        }
        
        if (token[0] == '>') {
            *output_file = expand_variables(token + 1);
            *append = 0;
            continue;
        }
        
        if (len > 0 && token[len - 1] == '&') {
            token[len - 1] = '\0';
            *background = 1;
            if (strlen(token) == 0)
                continue;
        }
        
        if ((size_t)*argc >= args_size - 1) {
            args_size *= 2;
            args = realloc(args, args_size * sizeof(char *));
            if (args == NULL)
                return NULL;
        }
        
        args[*argc] = expand_variables(token);
        if (args[*argc] != NULL)
            (*argc)++;
    }
    
    args[*argc] = NULL;
    return args;
}

static void free_args(char **args, int argc)
{
    int i;
    for (i = 0; i < argc; i++)
        free(args[i]);
}

static void remove_job(pid_t pid)
{
    Job *curr = job_list;
    Job *prev = NULL;
    
    while (curr != NULL) {
        if (curr->pid == pid) {
            if (prev == NULL)
                job_list = curr->next;
            else
                prev->next = curr->next;
            free(curr->command);
            free(curr);
            num_jobs--;
            return;
        }
        prev = curr;
        curr = curr->next;
    }
}

static void check_background_jobs(void)
{
    Job *curr = job_list;
    Job *next;
    int status;
    pid_t result;
    
    while (curr != NULL) {
        next = curr->next;
        result = waitpid(curr->pid, &status, WNOHANG);
        if (result > 0)
            remove_job(curr->pid);
        curr = next;
    }
}

static void print_jobs(void)
{
    Job *curr = job_list;
    
    if (num_jobs == 1)
        printf("1 job.\n");
    else
        printf("%d jobs.\n", num_jobs);
    
    while (curr != NULL) {
        printf("   %-6d - %s\n", curr->pid, curr->command);
        curr = curr->next;
    }
}

static int handle_builtin(char **args, int argc)
{
    char *target;
    
    if (argc == 0)
        return 0;
    
    if (strcmp(args[0], "exit") == 0)
        return -1;
    
    if (strcmp(args[0], "cd") == 0) {
        if (argc == 1)
            target = getenv("HOME");
        else if (strcmp(args[1], "~") == 0)
            target = getenv("HOME");
        else
            target = args[1];
        
        if (target == NULL)
            fprintf(stderr, "cd: HOME not set\n");
        else if (chdir(target) != 0)
            fprintf(stderr, "%s: %s\n", target, strerror(errno));
        
        return 1;
    }
    
    if (strcmp(args[0], "jobs") == 0) {
        check_background_jobs();
        print_jobs();
        return 1;
    }
    
    return 0;
}

static void add_job(pid_t pid, char **args, int argc)
{
    Job *new_job;
    size_t cmd_len = 0;
    char *cmd;
    int i;
    
    for (i = 0; i < argc; i++)
        cmd_len += strlen(args[i]) + 1;
    
    cmd = malloc(cmd_len);
    if (cmd == NULL) {
        perror("malloc");
        return;
    }
    
    cmd[0] = '\0';
    for (i = 0; i < argc; i++) {
        if (i > 0)
            strcat(cmd, " ");
        strcat(cmd, args[i]);
    }
    
    new_job = malloc(sizeof(Job));
    if (new_job == NULL) {
        perror("malloc");
        free(cmd);
        return;
    }
    
    new_job->pid = pid;
    new_job->command = cmd;
    new_job->next = job_list;
    job_list = new_job;
    num_jobs++;
}

static void execute_command(char **args, int argc, int background,
                           char *input_file, char *output_file, int append)
{
    pid_t pid;
    int fd, flags, status;
    
    pid = fork();
    
    if (pid < 0) {
        perror("fork");
        return;
    }
    
    if (pid == 0) {
        if (input_file != NULL) {
            fd = open(input_file, O_RDONLY);
            if (fd < 0) {
                perror(input_file);
                exit(1);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }
        
        if (output_file != NULL) {
            flags = O_WRONLY | O_CREAT;
            if (append)
                flags |= O_APPEND;
            else
                flags |= O_TRUNC;
            fd = open(output_file, flags, 0644);
            if (fd < 0) {
                perror(output_file);
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }
        
        execvp(args[0], args);
        perror(args[0]);
        exit(1);
    } else {
        if (background)
            add_job(pid, args, argc);
        else
            waitpid(pid, &status, 0);
    }
}

static void cleanup_jobs(void)
{
    Job *curr = job_list;
    Job *next;
    
    while (curr != NULL) {
        next = curr->next;
        free(curr->command);
        free(curr);
        curr = next;
    }
}

int main(void)
{
    char *line;
    char **args;
    int argc, background, append, builtin_result;
    char *input_file, *output_file;

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
            if (args != NULL)
                free(args);
            continue;
        }
        
        builtin_result = handle_builtin(args, argc);
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