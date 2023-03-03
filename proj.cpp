#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

using namespace std;

#define MAX_COMMANDS 10
#define MAX_COMMAND_LENGTH 80
#define MAX_PIPE_COMMANDS 10

void execute_pipeline(char *commands[]);

char** getSeperateCommands(char commandInput[MAX_COMMAND_LENGTH]);

char** removeExtraWhiteSpaces(char** commands);


int numberOfCommands=0;

int main() {
    char** commands = new char*[MAX_COMMANDS];
    char commandInput[MAX_COMMAND_LENGTH];

    X:
        numberOfCommands=0;
        cout << "$ ";
        cin.getline(commandInput, MAX_COMMAND_LENGTH);
        
       if (strcmp(commandInput, "myexit") == 0) {
            exit(EXIT_SUCCESS);
        }
        else if (strcmp(commandInput, "mypwd") == 0) {
            char cwd[1024];
            if (getcwd(cwd, sizeof(cwd)) != nullptr) {
                cout << cwd << endl;
            } else {
                perror("getcwd() error");
            }
            goto X;
        }
        else if (strncmp(commandInput, "mycd", 4) == 0) {
            char *path = strtok(commandInput + 4, " \n\r");
            if (path == NULL) {
                cerr << "mycd: missing argument\n";
                goto X;
            }
            if (chdir(path) != 0) {
                perror("mycd");
            }
            goto X;
        }
        else if (cin.eof()) {
            exit(EXIT_SUCCESS);
        }

        
        commands = getSeperateCommands(commandInput);

        execute_pipeline(commands);

    goto X;

    return 0;
}

char** getSeperateCommands(char commandInput[MAX_COMMAND_LENGTH])
{
    char** commands = new char*[MAX_COMMANDS];
    int i = 0;
    int j = 0;
    while (commandInput[j] != '\0') {
        int k = j;
        while (commandInput[k] != '\0' && commandInput[k] != '|') {
            k++;
        }
        if (k > j) {
            commands[i] = new char[k - j + 1];
            strncpy(commands[i], commandInput + j, k - j);
            commands[i][k - j] = '\0';
            i++;
        }
        if (commandInput[k] == '|') {
            j = k + 1;
        } else {
            j = k;
        }
    }
    
    for (int k = i; k < MAX_COMMANDS; k++) {
        commands[k] = nullptr;
    }
    numberOfCommands = i;
    commands = removeExtraWhiteSpaces(commands);
    return commands;
}




char** removeExtraWhiteSpaces(char** commands)
{
    char** result = new char*[numberOfCommands];
    for (int i = 0; i < numberOfCommands; i++) {
        char* command = commands[i];
        char* trimmed = new char[strlen(command) + 1];
        int j = 0;
        bool leadingSpace = true;
        for (int k = 0; k < strlen(command); k++) {
            if (command[k] == ' ' && leadingSpace) {
                continue;
            } else {
                leadingSpace = false;
            }
            trimmed[j++] = command[k];
        }
        trimmed[j] = '\0';
        int end = strlen(trimmed) - 1;
        while (end >= 0 && (trimmed[end] == ' ' || trimmed[end] == '\n' || trimmed[end] == '\r')) {
            trimmed[end--] = '\0';
        }
        result[i] = trimmed;
    }
    return result;
}


void execute_pipeline(char *commands[]) {
int fd[2];
int in = 0;
int out = 1;
for (int i = 0; i < numberOfCommands; i++) {
    if (pipe(fd) < 0) {
        fprintf(stderr, "Pipe failed\n");
        exit(EXIT_FAILURE);
    }

    char *args[MAX_COMMANDS];
    int arg_index = 0;
    int redirect_in = 0;
    int redirect_out = 0;
    char *input_file = nullptr;
    char *output_file = nullptr;

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
    pid_t pid = fork();
    if (pid == -1) {
        fprintf(stderr, "Fork failed\n");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Child process
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
        } else if (i < numberOfCommands - 1) {
            dup2(fd[1], STDOUT_FILENO);
            close(fd[1]);
        }

        execvp(args[0], args);
        fprintf(stderr, "Failed to execute command\n");
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        wait(NULL);

        if (in != 0) {
            close(in);
        }
        close(fd[1]);
        in = fd[0];
        out = fd[1];
    }
}

}
