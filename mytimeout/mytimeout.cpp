#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

using namespace std;

int timeout(int seconds, char* command);

int main(int argc, char * argv[]){
    if (argc != 3) {
        std::cout << "Invalid number of arguments!" << std::endl;
        return 1;
    }

    int seconds = std::stoi(argv[1]);
    char* commands(argv[2]);
    int exit_status = timeout(seconds, commands);
    return exit_status;
}

int timeout(int seconds, char* command) {
    pid_t pid;
    int status, exit_status;

    pid = fork();
    if (pid < 0) {
        std::cerr << "Failed to fork: " << strerror(errno) << std::endl;
        exit_status = 1;
    } else if (pid == 0) {
        // Child process
        execl("/bin/sh", "sh", "-c", command, (char*)0);
        std::cerr << "Failed to execute command: " << strerror(errno) << std::endl;
        exit(1);
    } else {
        // Parent process
        sleep(seconds);
        if (kill(pid, 0) == 0) {
            // Child process is still running
            if (kill(pid, SIGTERM) != 0) {
                std::cerr << "Failed to terminate process: " << strerror(errno) << std::endl;
                exit_status = 1;
            } else {
                std::cout << "Process terminated after timeout of " << seconds << " seconds" << std::endl;
                exit_status = 2;
            }
        } else {
            wait(&status);
            if (WIFEXITED(status)) {
                exit_status = WEXITSTATUS(status);
            } else if (WIFSIGNALED(status)) {
                exit_status = 128 + WTERMSIG(status);
            } else {
                exit_status = 1;
            }
        }
    }
    return exit_status;
}
