#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

// https://brennan.io/2015/01/16/write-a-shell-in-c/
// https://github.com/brenns10/lsh/blob/407938170e8b40d231781576e05282a41634848c/src/main.c

/* Selected Commands of the Shell */
int cmd_exit();
int cmd_help();
int cmd_pwd();
int cmd_cd();
int cmd_wait();

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

int shell_init(char **args)
{
  pid_t pid, wpid;
  int status;

  pid = fork();
  if (pid == 0) {
    // Child process
    if (execvp(args[0], args) == -1) {
      perror("lsh");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // Error forking
    perror("lsh");
  } else {
    // Parent process
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

int shell_exec(char **args)
{
  int i;

  if (args[0] == NULL) {
    // An empty command was entered.
    return 1;
  }

/* Check what command has been entered by the user by comparing it to the list of commands available */
  for (i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++) {
    if (strcmp(args[0], cmd_table[i].cmd) == 0) {
      return (*cmd_table[i].cmd)(args);
    }
  }

  return shell_launch(args);
}

void shell_loop(){
    do {
        printf("> ");
        char line = shell_read_line();
        char *args[]= shell_split_line(line);
        int status = shell_executs(args);
    }while(status);

}

int main (int argc, char *argv[]){
    shell_loop();

    return EXIT_SUCCESS;
}