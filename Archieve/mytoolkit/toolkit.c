#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdbool.h>

#define MAXIMUM_INSTRUCTIONS 10
#define MAX_INS_LENGTH 80

void run_pipe(char *commands[], int n_commands) {
    int in = 0, fd[2];
    for (int i = 0; i < n_commands; i++, in = fd[0], close(fd[1])) {
        if (pipe(fd) == -1) {
            perror("Pipe failed");
            exit(EXIT_FAILURE);
        }

        int redirect_in = 0, redirect_out = 0, arg_index = 0;
        char *args[MAXIMUM_INSTRUCTIONS], *token, *input_file = NULL, *output_file = NULL;
        for (token = strtok(commands[i], " "); token != NULL; token = strtok(NULL, " ")) {
            if (strcmp(token, "<") == 0) {
                redirect_in = 1;
                input_file = strtok(NULL, " ");
            } else if (strcmp(token, ">") == 0) {
                redirect_out = 1;
                output_file = strtok(NULL, " ");
            } else {
                args[arg_index++] = token;
            }
        }

        args[arg_index] = NULL;
        pid_t pid = fork();
        if (pid == -1) {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }else if (pid == 0) {
            if (redirect_in) {
                int in_fd = open(input_file, O_RDONLY);
                if (in_fd == -1) {
                    perror("Failed to open input file");
                    exit(EXIT_FAILURE);
                }
                dup2(in_fd, STDIN_FILENO);
                close(in_fd);
            } else if (in != 0) {
                dup2(in, STDIN_FILENO);
                close(in);
            }
            if (redirect_out) {
                int out_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                if (out_fd == -1) {
                    perror("Failed to open output file");
                    exit(EXIT_FAILURE);
                }
                dup2(out_fd, STDOUT_FILENO);
                close(out_fd);
            } else if (i < n_commands - 1) {
                dup2(fd[1], STDOUT_FILENO);
                close(fd[1]);
            }
            if (execvp(args[0], args) == -1) {
                perror("Failed to execute command");
                exit(EXIT_FAILURE);
            }
        } else {
            wait(NULL);
            if (in != 0) {
                close(in);
            }
        }
    }
}

int main() {
    char *commands[MAXIMUM_INSTRUCTIONS];
    char input[MAX_INS_LENGTH];
    int i, j;
    bool done = false;

    while (!done) {
        printf("$ ");
        fgets(input, MAX_INS_LENGTH, stdin);

        if (!feof(stdin)) {
            if (strcmp(input, "myexit\n") == 0) {
                done = true;
            }
            else if (strcmp(input, "mypwd\n") == 0) {
                char cwd[1024];
                if (getcwd(cwd, sizeof(cwd)) != NULL) {
                    printf("%s\n", cwd);
                } else {
                    perror("getcwd() error");
                }
            }
            else if (strncmp(input, "mycd", 4) == 0) {
                char *path = strtok(input + 4, " \n\r");
                if (path == NULL) {
                    fprintf(stderr, "mycd: missing argument\n");
                }
                else if (chdir(path) != 0) {
                    perror("mycd");
                }
            }
            else {
                // tokenize the input into separate commands
                char *token = strtok(input, "|");
                i = 0;
                while (token != NULL) {
                    commands[i] = token;
                    i++;
                    token = strtok(NULL, "|");
                }
                int n_commands = i;

                // trim leading and trailing whitespace from each command
                for (i = 0; i < n_commands; i++) {
                    char *command = commands[i];
                    while (*command == ' ') {
                        command++;
                    }
                    j = strlen(command) - 1;
                    while (command[j] == '\n' || command[j] == '\r' || command[j] == ' ') {
                        command[j] = '\0';
                        j--;
                    }
                    commands[i] = command;
                }
                run_pipe(commands, n_commands);
            }
        }
        else {
            done = true;
        }
    }
    return 0;
}
