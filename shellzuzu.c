#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

#include "tokenizer.h"

/* Convenience macro to silence compiler warnings about unused function parameters. */
#define unused __attribute__((unused))

/* Whether the shell is connected to an actual terminal or not. */
bool shell_is_interactive;

/* File descriptor for the shell input */
int shell_terminal;

/* Terminal mode settings for the shell */
struct termios shell_tmodes;

/* Process group id for the shell */
pid_t shell_pgid;

int cmd_exit(struct tokens *tokens);
int cmd_help(struct tokens *tokens);
int cmd_pwd(struct tokens *tokens);
int cmd_cd(struct tokens *tokens);
int cmd_wait(struct tokens *tokens);

/* Built-in command functions take token array (see parse.h) and return int */
typedef int cmd_fun_t(struct tokens *tokens);

/* Built-in command struct and lookup table */
typedef struct fun_desc
{
    cmd_fun_t *fun;
    char *cmd;
    char *doc;
} fun_desc_t;

fun_desc_t cmd_table[] = {
    {cmd_help, "?", "show this help menu"},
    {cmd_exit, "exit", "exit the command shell"},
    {cmd_pwd, "pwd", "print working directory"},
    {cmd_cd, "cd", "change current directory"},
    {cmd_wait, "wait", "wait for all processes to finish"}};

/* Prints a helpful description for the given command */
int cmd_help(unused struct tokens *tokens)
{
    for (unsigned int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++)
        printf("%s - %s\n", cmd_table[i].cmd, cmd_table[i].doc);
    return 1;
}

/* Exits this shell */
int cmd_exit(unused struct tokens *tokens)
{
    exit(0);
}

/* Prints current working directory */
int cmd_pwd(unused struct tokens *tokens)
{
    char cwd[4096];
    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
        fprintf(stdout, "%s\n", cwd);
    }
    return 0;
}

/* Changes the current directory */
int cmd_cd(struct tokens *tokens)
{
    if (!(tokens_get_length(tokens) <= 1))
    {
        if (chdir(tokens_get_token(tokens, 1)))
        {
            fprintf(stdout, "cd: no such file or directory: %s\n", tokens_get_token(tokens, 1));
        }
    }
    return 0;
}

/* Waits on process to finish */
int cmd_wait(struct tokens *tokens)
{
    int status;
    while (wait(&status) != -1)
        continue;
    return 1;
}

/* Looks up the built-in command, if it exists. */
int lookup(char cmd[])
{
    for (unsigned int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++)
        if (cmd && (strcmp(cmd_table[i].cmd, cmd) == 0))
            return i;
    return -1;
}

/* Intialization procedures for this shell */
void init_shell()
{
    /* Our shell is connected to standard input. */
    shell_terminal = STDIN_FILENO;
    
    /* Check if we are running interactively */
    shell_is_interactive = isatty(shell_terminal);
    
    if (shell_is_interactive)
    {
        /* If the shell is not currently in the foreground, we must pause the shell until it becomes a
         * foreground process. We use SIGTTIN to pause the shell. When the shell gets moved to the
         * foreground, we'll receive a SIGCONT. */
        while (tcgetpgrp(shell_terminal) != (shell_pgid = getpgrp()))
            kill(-shell_pgid, SIGTTIN);
        
        /* Saves the shell's process id */
        shell_pgid = getpid();
        
        /* Take control of the terminal */
        tcsetpgrp(shell_terminal, shell_pgid);
        
        /* Save the current termios to a variable, so it can be restored later. */
        tcgetattr(shell_terminal, &shell_tmodes);
    }
}

int main(unused int argc, unused char *argv[])
{
 
    
    static char line[4096];
    int line_num = 0;
    
    /* Please only print shell prompts when standard input is not a tty */
    if (shell_is_interactive)
        fprintf(stdout, "%d: ", line_num);
    
    while (fgets(line, 4096, stdin))
    {
        /* Split our line into words. */
        struct tokens *tokens = tokenize(line);
        
        /* Find which built-in function to run. */
        int fundex = lookup(tokens_get_token(tokens, 0));
        
        if (fundex >= 0)
        {
            cmd_table[fundex].fun(tokens);
        }
        else
        {
            size_t token_length = tokens_get_length(tokens);
            
            if (token_length)
            {
                
                signal(SIGINT, SIG_IGN);
                signal(SIGSTOP, SIG_IGN);
                signal(SIGTTOU, SIG_IGN);
                
                pid_t pid = fork();
                int status;
                
                if (!pid)
                {
                    setpgid(pid, getpid());
                    tcsetpgrp(shell_terminal, getpgid(pid));
                    
                    signal(SIGINT, SIG_DFL);
                    signal(SIGSTOP, SIG_DFL);
                    signal(SIGTTOU, SIG_DFL);
                    
                    char *path = tokens_get_token(tokens, 0);
                    char **argv = (char **)malloc((token_length + 1) * sizeof(char *));
                    
                    for (unsigned int i = 0; i < token_length; i++)
                    {
                        argv[i] = tokens_get_token(tokens, i);
                        // Check if one of the arguments is a simple '>' or '<'
                        // Then do I/O redirection
                        if (argv[i][0] == '<')
                        {
                            if (i + 1 >= token_length)
                                fprintf(stderr, "%s\n", "Syntax error.");
                            
                            int newfd;
                            
                            if ((newfd = open(tokens_get_token(tokens, i + 1), O_RDONLY)) < 0)
                            {
                                fprintf(stderr, "%s\n", "Error opening file for reading.");
                                exit(1);
                            }
                            // Replace standard input with file
                            dup2(newfd, 0);
                            argv[i] = NULL;
                            break;
                        }
                        else if (argv[i][0] == '>')
                        {
                            
                            if (i + 1 >= token_length)
                                fprintf(stderr, "%s\n", "Syntax error.");
                            
                            int newfd;
                            if ((newfd = open(tokens_get_token(tokens, i + 1), O_CREAT | O_TRUNC | O_WRONLY, 0644)) < 0)
                            {
                                fprintf(stderr, "%s\n", "Error opening file for writing.");
                                exit(1);
                            }
                            // Replace standard output with file
                            dup2(newfd, 1);
                            argv[i] = NULL;
                            break;
                        }
                        /* Background processes */
                        if (argv[i][0] == '&')
                        {
                            tcsetpgrp(shell_terminal, shell_pgid);
                            argv[i] = NULL;
                            break;
                        }
                    }
                    argv[token_length] = NULL;
                    
                    // Check if path is indicated in program
                    if (strchr((const char *)argv[0], '/') == NULL)
                    {
                        
                        /* Path resolution */
                        char *pathbuf;
                        char *pathtok;
                        size_t path_len;
                        static char resolved[4096];
                        
                        path_len = confstr(_CS_PATH, NULL, (size_t)0);
                        pathbuf = malloc(path_len);
                        if (pathbuf == NULL)
                            abort();
                        confstr(_CS_PATH, pathbuf, path_len);
                        pathtok = strtok(pathbuf, ":");
                        while (pathtok != NULL)
                        {
                            memset(resolved, '\0', path_len);
                            strcpy(resolved, pathtok);
                            strcat(resolved, "/");
                            strcat(resolved, path);
                            execv(resolved, (char *const *)argv);
                            pathtok = strtok(NULL, ":");
                        }
                        free(pathbuf);
                    }
                    else
                    {
                        execv(path, (char *const *)argv);
                    }
                }
                else
                {
                    bool background = token_length > 1 && tokens_get_token(tokens, token_length - 1)[0] == '&';
                    if (background)
                        waitpid(pid, &status, WNOHANG);
                    else
                        waitpid(pid, &status, 0);
                    
                    tcsetpgrp(shell_terminal, shell_pgid);
                    
                    signal(SIGINT, SIG_DFL);
                    signal(SIGSTOP, SIG_DFL);
                    signal(SIGTTOU, SIG_DFL);
                }
            }
        }
        if (shell_is_interactive)
        /*+ Please only print shell prompts when standard input is not a tty */
            fprintf(stdout, "%d: ", ++line_num);
        
        /* Clean up memory */
        tokens_destroy(tokens);
    }
    return 0;
}
