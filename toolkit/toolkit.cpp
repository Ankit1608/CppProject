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

void runPipeCommands(char *commands[]);

char** getSeperateCommands(char commandInput[MAX_COMMAND_LENGTH]);

char** removeExtraWhiteSpaces(char** commands);

bool runInternalCommands(char commandInput[MAX_COMMAND_LENGTH]) {
    // Define the internal command strings.
    const char* EXIT_COMMAND = "myexit";
    const char* PWD_COMMAND = "mypwd";
    const char* CD_COMMAND = "mycd";

   
    if (strcmp(commandInput, EXIT_COMMAND) == 0) {
        exit(EXIT_SUCCESS);
    }

    
    if (strcmp(commandInput, PWD_COMMAND) == 0) {
        char currentWorkingDirectory[MAX_COMMAND_LENGTH];
        if (getcwd(currentWorkingDirectory, sizeof(currentWorkingDirectory)) != nullptr) {
            std::cout << currentWorkingDirectory << std::endl;
            return true;
        } else {
            std::cerr << "Error: Could not get the current working directory." << std::endl;
            return false;
        }
    }

    
    if (strncmp(commandInput, CD_COMMAND, strlen(CD_COMMAND)) == 0) {
        
        char* argument = strtok(commandInput + strlen(CD_COMMAND), " \n\r");

        
        if (argument == nullptr) {
            std::cerr << "Error: Missing argument for cd command." << std::endl;
            return false;
        }

        
        if (chdir(argument) == -1) {
            perror("Error");
            return false;
        } else {
            std::cout << "Changed directory to " << argument << std::endl;
            return true;
        }
    }

   
    return false;
}

void setEnvironmentPath(char *pathGiven,char*pathName){

char *new_path =pathGiven;

    
    char *old_path = getenv(pathName);

    
    char *path = (char*)malloc(strlen(old_path) + strlen(new_path) + 2);

    
    strcpy(path, old_path);

    
    strcat(path, ":");
    strcat(path, new_path);

    
    setenv(pathName, path, 1);

    
    free(path);
}

int numberOfCommands=0;

int main() {
    char** commands = new char*[MAX_COMMANDS];
    char commandInput[MAX_COMMAND_LENGTH];

    X:
        numberOfCommands=0;
        cout << "$ ";
        cin.getline(commandInput, MAX_COMMAND_LENGTH);

        if(runInternalCommands(commandInput))
        {
            goto X;
        }
        if (cin.eof()) {
            exit(EXIT_SUCCESS);
        }

        
        commands = getSeperateCommands(commandInput);

        runPipeCommands
    (commands);

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


void runPipeCommands(char *commands[]) {
    int fd[2];
    int in = 0;
    int out = 1;
    const char *delimiter = " ";

    for (int i = 0; i < numberOfCommands; i++) {
        if (pipe(fd) < 0) {
            std::cerr << "Pipe failed\n";
            exit(EXIT_FAILURE);
        }

        char *args[MAX_COMMANDS];
        int argIndex = 0;
        int redirectIn = 0;
        int redirectOut = 0;
        char *inputFile = nullptr;
        char *outputFile = nullptr;

        // Parse command arguments
        char *command = commands[i];
        char *valueTokenIndex = strtok(command, delimiter);
        while (valueTokenIndex != nullptr) {
            if (std::string(valueTokenIndex) == "<") {
                redirectIn = 1;
                valueTokenIndex = strtok(nullptr, delimiter);
                inputFile = valueTokenIndex;
            } else if (std::string(valueTokenIndex) == ">") {
                redirectOut = 1;
                valueTokenIndex = strtok(nullptr, delimiter);
                outputFile = valueTokenIndex;
            } else {
                args[argIndex++] = valueTokenIndex;
            }
            valueTokenIndex = strtok(nullptr, delimiter);
        }
        args[argIndex] = nullptr;

        // Fork a child process to execute the command
        pid_t pid = fork();
        if (pid == -1) {
            std::cerr << "Fork failed\n";
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // Child process
            if (redirectIn) {
                int in_fd = open(inputFile, O_RDONLY);
                if (in_fd < 0) {
                    std::cerr << "Failed to open input file\n";
                    exit(EXIT_FAILURE);
                }
                dup2(in_fd, STDIN_FILENO);
                close(in_fd);
            } else if (in != 0) {
                dup2(in, STDIN_FILENO);
                close(in);
            }

            if (redirectOut) {
                int out_fd = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                if (out_fd < 0) {
                    std::cerr << "Failed to open output file\n";
                    exit(EXIT_FAILURE);
                }
                dup2(out_fd, STDOUT_FILENO);
                close(out_fd);
            } else if (i < numberOfCommands - 1) {
                dup2(fd[1], STDOUT_FILENO);
                close(fd[1]);
            }
            execvp(args[0], args);
            std::cerr << "Failed to execute command\n";
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
