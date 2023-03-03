#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_COMMANDS 10
#define MAX_COMMAND_LENGTH 80
#define MAX_PIPE_COMMANDS 10

void execute_pipeline(char *commands[], int n_commands);

int main() {
    char *commands[MAX_COMMANDS];
    char input[MAX_COMMAND_LENGTH];
    int i, j;

    while (true) {
        std::cout << "$ ";
        std::cin.getline(input, MAX_COMMAND_LENGTH);
        
       if (strcmp(input, "myexit") == 0) {
            exit(EXIT_SUCCESS);
        }
        else if (strcmp(input, "mypwd") == 0) {
            char cwd[1024];
            if (getcwd(cwd, sizeof(cwd)) != nullptr) {
                std::cout << cwd << std::endl;
            } else {
                perror("getcwd() error");
            }
            continue;
        }
        else if (strncmp(input, "mycd", 4) == 0) {
            char *path = strtok(input + 4, " \n\r");
            if (path == NULL) {
                std::cerr << "mycd: missing argument\n";
                continue;
            }
            if (chdir(path) != 0) {
                perror("mycd");
            }
            continue;
        }
        else if (std::cin.eof()) {
            exit(EXIT_SUCCESS);
        }

        // tokenize the input into separate commands
        char *token = strtok(input, "|");
        i = 0;
        while (token != nullptr) {
            commands[i] = token;
            i++;
            token = strtok(nullptr, "|");
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
        execute_pipeline(commands, n_commands);
    }
    return 0;
}

void execute_pipeline(char *commands[], int n_commands) {
    int fd[2];
    int in = 0;
    int out = 1;
    pid_t pid;
    int i = 0;
    char *args[MAX_COMMANDS];
    int arg_index = 0;
    int redirect_in = 0;
    int redirect_out = 0;
    char *input_file;
    char *output_file;

    while (i < n_commands) {
        if (pipe(fd) < 0) {
            fprintf(stderr, "Pipe failed\n");
            exit(EXIT_FAILURE);
        }

        arg_index = 0;
        redirect_in = 0;
        redirect_out = 0;
        input_file = nullptr;
        output_file = nullptr;

        // Parse command arguments
        char *token = strtok(commands[i], " ");
        while (token != nullptr) {
            if (strcmp(token, "<") == 0) {
                redirect_in = 1;
                input_file = strtok(nullptr, " ");
            } else if (strcmp(token, ">") == 0) {
                redirect_out = 1;
                output_file = strtok(nullptr, " ");
            } else {
                args[arg_index++] = token;
            }
            token = strtok(nullptr, " ");
        }
        args[arg_index] = nullptr;

        // Fork a child process to execute the command
        if ((pid = fork()) == -1) {
            fprintf(stderr, "Fork failed\n");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            if (redirect_in) {
                int in_fd = open(input_file, O_RDONLY);
                if (in_fd < 0) {
                    fprintf(stderr, "Failed to open input file\n");
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
                if (out_fd < 0) {
                    fprintf(stderr, "Failed to open output file\n");
                    exit(EXIT_FAILURE);
                }
                dup2(out_fd, STDOUT_FILENO);
                close(out_fd);
            } else if (i < n_commands - 1) {
                dup2(fd[1], STDOUT_FILENO);
                close(fd[1]);
            }

            if (execvp(args[0], args) == -1) {
                fprintf(stderr, "Failed to execute command\n");
                exit(EXIT_FAILURE);
            }
        } else {
            wait(NULL);

            if (in != 0) {
                close(in);
            }
            close(fd[1]);
            in = fd[0];
            out = fd[1];
        }

        i++;
    }
}