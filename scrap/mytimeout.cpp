#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

using namespace std;

void timeout(int seconds, char* command);

int main(int argc, char * argv[]){
    if (argc != 3) {
        std::cout << "Invalid number of arguments!" << std::endl;
        return 1;
    }

    int seconds = std::stoi(argv[1]);
    char* commands(argv[2]);
    timeout(seconds, commands);
    return 0;
}

void timeout(int seconds, char* command) {
    pid_t pid;
    int status;

    pid = fork();
    if (pid < 0) {
        std::cout << "Failed to fork" << std::endl;
        exit(1);
    } else if (pid == 0) {
        // Child process
        execl("/bin/sh", "sh", "-c", command, (char*)0);
        std::cout << "Failed to execute command: " << command << std::endl;
        exit(1);
    } else {
        // Parent process
        sleep(seconds);
        if (kill(pid, 0) == 0) {
            // Child process is still running
            kill(pid, SIGTERM);
            std::cout << "Process terminated after timeout of " << seconds << " seconds" << std::endl;
        }
        wait(&status);
    }
}