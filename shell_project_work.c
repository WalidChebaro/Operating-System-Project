#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define SIZE_OF_COMMAND_LINE 100
#define NUMBER_OF_COMMAND_LINE 1024

/* Selected Commands of the Shell */
void cmd_exit();
void cmd_help();
void cmd_pwd();
void cmd_cd(char *);
void cmd_ls();
void cmd_mkdir(char *);
void cmd_rmdir(char *);
void cmd_create_file(char *);
void cmd_read_file(char *);
void cmd_delete_file(char *);
void command_definer(int);

typedef struct cmd_line
{
    char cmd_line_content[SIZE_OF_COMMAND_LINE];
} cmd_line;

cmd_line cmd[NUMBER_OF_COMMAND_LINE];

typedef struct cmd_doc
{
    char *cmd;
    char *desc;
} cmd_doc;

cmd_doc cmd_table[] =
    {{"help", "help - show the help menu"},
     {"exit", "exit - exit the shell"},
     {"pwd", "pwd - print the working directory"},
     {"cd", "cd - change the directory"},
     {"ls", "ls - list the current directory"},
     {"mkdir", "mkdir - create a directory"},
     {"rmdir", "rmdir - remove a directory"},
     {"create_file", "create_file - create a new file"},
     {"read_file", "read_file - opens a created file"},
     {"delete_file", "delete_file - delete the current file"}};

void cmd_help()
{

    int i;
    for (i = 0; i < 10; i++)
    {
        printf("%s\n", cmd_table[i].desc);
    }
}

void cmd_exit()
{
    exit(0);
}

void cmd_pwd()
{
    execlp("pwd", "pwd", NULL);
}

void cmd_cd(char *token)
{
    chdir(token);
}

void cmd_ls()
{
    execlp("ls", "ls", "-l", NULL);
}

void cmd_mkdir(char *token)
{
    if (mkdir(token, S_IRWXU | S_IRWXG | S_IRWXO) == 0)
    {
        printf("%s directory created.\n", token);
    }
    else
    {
        printf("Unable to create directory, %s directory is already created.\n", token);
    }
}

void cmd_rmdir(char *token)
{
    if (rmdir(token) == 0)
    {
        printf("%s directory removed.\n", token);
    }
    else
    {
        printf("Unable to remove %s directory.\n", token);
    }
}

void cmd_create_file(char *token)
{
    FILE *fl;
    fl = fopen(token, "w");
    fclose(fl);
}

void cmd_read_file(char *token)
{
    FILE *fl;
    fl = fopen(token, "r");
    char c;
    while ((c = getc(fl)) != EOF)
    {
        printf("%c", c);
    }
    fclose(fl);
    printf("\nEnd of File\n");
}

void cmd_delete_file(char *token)
{
    remove(token);
}

// Divides the line from the command array into token and assignes the corresponding cmd.
void command_definer(int i)
{
    char *token;
    char *temp = cmd[i].cmd_line_content;

    while ((token = strtok_r(temp, " \t\r\n\a", &temp)))
    {
        if (strcmp(token, cmd_table[0].cmd) == 0)
        {
            cmd_help();
        }

        else if (strcmp(token, cmd_table[1].cmd) == 0)
        {
            cmd_exit();
        }

        else if (strcmp(token, cmd_table[2].cmd) == 0)
        {
            pid_t pid = fork();
            if (pid == 0)
            {
                cmd_pwd();
            }
            if (pid > 0)
            {
                wait(NULL);
            }

            if (pid < 0)
            {
                perror("ERROR");
            }
        }

        else if (strcmp(token, cmd_table[3].cmd) == 0)
        {
            while ((token = strtok_r(temp, " \t\r\n\a", &temp)))
            {
                cmd_cd(token);
            }
        }

        else if (strcmp(token, cmd_table[4].cmd) == 0)
        {
            pid_t pid = fork();
            if (pid == 0)
            {
                cmd_ls();
            }
            if (pid > 0)
            {
                wait(NULL);
            }

            if (pid < 0)
            {
                perror("ERROR");
            }
        }

        else if (strcmp(token, cmd_table[5].cmd) == 0)
        {
            while ((token = strtok_r(temp, " \t\r\n\a", &temp)))
            {
                cmd_mkdir(token);
            }
        }

        else if (strcmp(token, cmd_table[6].cmd) == 0)
        {
            while ((token = strtok_r(temp, " \t\r\n\a", &temp)))
            {
                cmd_rmdir(token);
            }
        }

        else if (strcmp(token, cmd_table[7].cmd) == 0)

        {
            while ((token = strtok_r(temp, " \t\r\n\a", &temp)))
            {
                cmd_create_file(token);
            }
        }

        else if (strcmp(token, cmd_table[8].cmd) == 0)

        {
            while ((token = strtok_r(temp, " \t\r\n\a", &temp)))
            {
                cmd_read_file(token);
            }
        }

        else if (strcmp(token, cmd_table[9].cmd) == 0)
        {
            while ((token = strtok_r(temp, " \t\r\n\a", &temp)))
            {
                cmd_delete_file(token);
            }
        }
    }
}

// Scans the line from the user and fills the command array with the line.
int line_reader(int i)
{
    char input[SIZE_OF_COMMAND_LINE];
    int index = 0;
    int j;
    while (scanf("%c", &input[index]))
    {
        if (input[index] == '\n')
        {
            break;
        }
        index++;
    }

    if (index == 0)
    {
        return 1;
    }
    else
    {
        for (j = 0; j <= sizeof(input); j++)
        {
            cmd[i].cmd_line_content[j] = input[j];
        }
        return 0;
    }
}

void main_loop()
{
    char cmd;
    int i;

    for (i = 0; i < NUMBER_OF_COMMAND_LINE; i++)
    {
        printf("%d: ", i);
        if (line_reader(i) == 1)
            ;
        else
        {
            command_definer(i);
        }
    }
}

int main(int argc, char *argv[])
{
    main_loop();
}