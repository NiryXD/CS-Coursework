#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <errno.h>

struct Job {
    pid_t pid;
    char *cmdline;
    struct Job *next;
};

static struct Job *jobs_head = NULL;

static void free_jobs(void)
{
    struct Job *j = jobs_head;
    while (j) {
        struct Job *next = j->next;
        free(j->cmdline);
        free(j);
        j = next;
    }
    jobs_head = NULL;
}

static void add_job(pid_t pid, const char *cmdline)
{
    struct Job *job = malloc(sizeof(*job));
    if (!job) {
        return;
    }
    job->pid = pid;
    if (cmdline) {
        job->cmdline = strdup(cmdline);
    } else {
        job->cmdline = strdup("");
    }
    job->next = NULL;

    if (!jobs_head) {
        jobs_head = job;
    } else {
        struct Job *j = jobs_head;
        while (j->next) {
            j = j->next;
        }
        j->next = job;
    }
}

static void remove_job(pid_t pid)
{
    struct Job *prev = NULL;
    struct Job *j = jobs_head;
    while (j) {
        if (j->pid == pid) {
            if (prev) {
                prev->next = j->next;
            } else {
                jobs_head = j->next;
            }
            free(j->cmdline);
            free(j);
            return;
        }
        prev = j;
        j = j->next;
    }
}

static int count_jobs(void)
{
    int c = 0;
    struct Job *j = jobs_head;
    while (j) {
        c++;
        j = j->next;
    }
    return c;
}

static void reap_background_jobs(void)
{
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        remove_job(pid);
    }
}

static void print_jobs(void)
{
    reap_background_jobs();
    int n = count_jobs();
    if (n == 1) {
        printf("1 job.\n");
    } else {
        printf("%d jobs.\n", n);
    }
    struct Job *j = jobs_head;
    while (j) {
        printf("   %-6d - %s\n", (int)j->pid, j->cmdline ? j->cmdline : "");
        j = j->next;
    }
}

static void print_prompt(void)
{
    char *user = getenv("USER");
    if (!user) {
        user = "unknown";
    }
    char cwd[PATH_MAX];
    char *pwd = NULL;
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        char *home = getenv("HOME");
        if (home) {
            size_t home_len = strlen(home);
            if (strncmp(cwd, home, home_len) == 0 &&
                (cwd[home_len] == '\0' || cwd[home_len] == '/')) {
                static char display[PATH_MAX];
                size_t extra_len = strlen(cwd + home_len);
                if (1 + extra_len < sizeof(display)) {
                    display[0] = '~';
                    memcpy(display + 1, cwd + home_len, extra_len + 1);
                    pwd = display;
                } else {
                    pwd = cwd;
                }
            } else {
                pwd = cwd;
            }
        } else {
            pwd = cwd;
        }
    } else {
        pwd = "?";
    }

    printf("%s@cosc360:%s> ", user, pwd);
    fflush(stdout);
}

static char *expand_vars(const char *s)
{
    size_t cap = 64;
    size_t len = 0;
    char *out = malloc(cap);
    if (!out) return NULL;

    for (size_t i = 0; s[i] != '\0'; ++i) {
        if (s[i] == '$') {
            size_t j = i + 1;
            if (!s[j] || !(isalpha((unsigned char)s[j]) || s[j] == '_')) {
                if (len + 1 >= cap) {
                    cap *= 2;
                    char *tmp = realloc(out, cap);
                    if (!tmp) { free(out); return NULL; }
                    out = tmp;
                }
                out[len++] = '$';
                continue;
            }
            while (s[j] && (isalpha((unsigned char)s[j]) || s[j] == '_')) {
                j++;
            }
            size_t vlen = j - (i + 1);
            char *var = malloc(vlen + 1);
            if (!var) { free(out); return NULL; }
            memcpy(var, s + i + 1, vlen);
            var[vlen] = '\0';
            char *val = getenv(var);
            free(var);
            if (!val) val = "";
            size_t vallen = strlen(val);
            if (len + vallen >= cap) {
                while (len + vallen >= cap) cap *= 2;
                char *tmp = realloc(out, cap);
                if (!tmp) { free(out); return NULL; }
                out = tmp;
            }
            memcpy(out + len, val, vallen);
            len += vallen;
            i = j - 1;
        } else {
            if (len + 1 >= cap) {
                cap *= 2;
                char *tmp = realloc(out, cap);
                if (!tmp) { free(out); return NULL; }
                out = tmp;
            }
            out[len++] = s[i];
        }
    }
    if (len + 1 >= cap) {
        cap += 1;
        char *tmp = realloc(out, cap);
        if (!tmp) { free(out); return NULL; }
        out = tmp;
    }
    out[len] = '\0';
    return out;
}

static char **parse_tokens(const char *line, int *out_count)
{
    int capacity = 8;
    int count = 0;
    char **tokens = malloc(sizeof(char*) * capacity);
    if (!tokens) return NULL;

    const char *p = line;
    while (*p) {
        while (*p && isspace((unsigned char)*p)) p++;
        if (!*p) break;
        const char *start = p;
        while (*p && !isspace((unsigned char)*p)) p++;
        size_t tlen = (size_t)(p - start);
        char *tmp = malloc(tlen + 1);
        if (!tmp) {
            for (int i = 0; i < count; ++i) free(tokens[i]);
            free(tokens);
            return NULL;
        }
        memcpy(tmp, start, tlen);
        tmp[tlen] = '\0';
        char *expanded = expand_vars(tmp);
        free(tmp);
        if (!expanded) {
            for (int i = 0; i < count; ++i) free(tokens[i]);
            free(tokens);
            return NULL;
        }
        if (count >= capacity) {
            capacity *= 2;
            char **tmp_tokens = realloc(tokens, sizeof(char*) * capacity);
            if (!tmp_tokens) {
                free(expanded);
                for (int i = 0; i < count; ++i) free(tokens[i]);
                free(tokens);
                return NULL;
            }
            tokens = tmp_tokens;
        }
        tokens[count++] = expanded;
    }
    if (count >= capacity) {
        capacity += 1;
        char **tmp_tokens = realloc(tokens, sizeof(char*) * capacity);
        if (!tmp_tokens) {
            for (int i = 0; i < count; ++i) free(tokens[i]);
            free(tokens);
            return NULL;
        }
        tokens = tmp_tokens;
    }
    tokens[count] = NULL;
    if (out_count) *out_count = count;
    return tokens;
}

static void free_tokens(char **tokens, int count)
{
    if (!tokens) return;
    for (int i = 0; i < count; ++i) {
        free(tokens[i]);
    }
    free(tokens);
}

static void builtin_cd(int argc, char **argv)
{
    const char *target = NULL;
    if (argc < 2 || argv[1] == NULL || argv[1][0] == '\0' || strcmp(argv[1], "~") == 0) {
        target = getenv("HOME");
        if (!target) target = "";
    } else if (argv[1][0] == '~') {
        const char *home = getenv("HOME");
        if (!home) home = "";
        size_t home_len = strlen(home);
        size_t rest_len = strlen(argv[1] + 1);
        char *buf = malloc(home_len + rest_len + 2);
        if (!buf) return;
        strcpy(buf, home);
        if (rest_len > 0) {
            strcat(buf, argv[1] + 1);
        }
        if (chdir(buf) != 0) {
            perror(buf);
        } else {
            char cwd[PATH_MAX];
            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                setenv("PWD", cwd, 1);
            }
        }
        free(buf);
        return;
    } else {
        target = argv[1];
    }

    if (target[0] == '\0') {
        return;
    }
    if (chdir(target) != 0) {
        perror(target);
    } else {
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            setenv("PWD", cwd, 1);
        }
    }
}

int main(void)
{
    char *line = NULL;
    size_t cap = 0;

    for (;;) {
        reap_background_jobs();
        print_prompt();

        ssize_t nread = getline(&line, &cap, stdin);
        if (nread < 0) {
            break;
        }

        if (nread > 0 && line[nread - 1] == '\n') {
            line[nread - 1] = '\0';
        }

        int background = 0;
        int end = (int)strlen(line) - 1;
        while (end >= 0 && isspace((unsigned char)line[end])) {
            line[end] = '\0';
            end--;
        }
        if (end >= 0 && line[end] == '&') {
            background = 1;
            line[end] = '\0';
            end--;
            while (end >= 0 && isspace((unsigned char)line[end])) {
                line[end] = '\0';
                end--;
            }
        }

        char *cmdline = line;
        while (*cmdline && isspace((unsigned char)*cmdline)) {
            cmdline++;
        }
        if (*cmdline == '\0') {
            continue;
        }

        char *invocation = strdup(cmdline);
        if (!invocation) {
            invocation = NULL;
        }

        int ntokens = 0;
        char **tokens = parse_tokens(cmdline, &ntokens);
        if (!tokens) {
            free(invocation);
            continue;
        }
        if (ntokens == 0) {
            free_tokens(tokens, ntokens);
            free(invocation);
            continue;
        }

        if (strcmp(tokens[0], "exit") == 0) {
            free_tokens(tokens, ntokens);
            free(invocation);
            free_jobs();
            free(line);
            return 0;
        }

        if (strcmp(tokens[0], "cd") == 0) {
            builtin_cd(ntokens, tokens);
            free_tokens(tokens, ntokens);
            free(invocation);
            continue;
        }

        if (strcmp(tokens[0], "jobs") == 0) {
            print_jobs();
            free_tokens(tokens, ntokens);
            free(invocation);
            continue;
        }

        enum { OUT_NONE = 0, OUT_TRUNC, OUT_APPEND };
        int out_type = OUT_NONE;
        char *infile = NULL;
        char *outfile = NULL;

        char **argv = malloc(sizeof(char*) * (ntokens + 1));
        if (!argv) {
            free_tokens(tokens, ntokens);
            free(invocation);
            continue;
        }
        int argc = 0;
        for (int i = 0; i < ntokens; ++i) {
            char *t = tokens[i];
            if (t[0] == '<' && t[1] != '\0') {
                infile = t + 1;
            } else if (t[0] == '>' && t[1] == '>' && t[2] != '\0') {
                out_type = OUT_APPEND;
                outfile = t + 2;
            } else if (t[0] == '>' && t[1] != '\0') {
                out_type = OUT_TRUNC;
                outfile = t + 1;
            } else {
                argv[argc++] = t;
            }
        }
        argv[argc] = NULL;

        if (argc == 0) {
            free(argv);
            free_tokens(tokens, ntokens);
            free(invocation);
            continue;
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            free(argv);
            free_tokens(tokens, ntokens);
            free(invocation);
            continue;
        }

        if (pid == 0) {
            if (infile) {
                int fd = open(infile, O_RDONLY);
                if (fd < 0) {
                    perror(infile);
                    _exit(1);
                }
                if (dup2(fd, STDIN_FILENO) < 0) {
                    perror("dup2");
                    close(fd);
                    _exit(1);
                }
                close(fd);
            }
            if (outfile) {
                int flags = O_WRONLY | O_CREAT;
                if (out_type == OUT_APPEND) {
                    flags |= O_APPEND;
                } else {
                    flags |= O_TRUNC;
                }
                int fd = open(outfile, flags, 0666);
                if (fd < 0) {
                    perror(outfile);
                    _exit(1);
                }
                if (dup2(fd, STDOUT_FILENO) < 0) {
                    perror("dup2");
                    close(fd);
                    _exit(1);
                }
                close(fd);
            }
            execvp(argv[0], argv);
            perror(argv[0]);
            _exit(1);
        } else {
            if (background) {
                if (invocation) {
                    add_job(pid, invocation);
                } else {
                    add_job(pid, argv[0]);
                }
            } else {
                int status;
                if (waitpid(pid, &status, 0) < 0) {
                    perror("waitpid");
                }
            }
            free(argv);
            free_tokens(tokens, ntokens);
            free(invocation);
        }
    }

    free(line);
    free_jobs();
    return 0;
}
