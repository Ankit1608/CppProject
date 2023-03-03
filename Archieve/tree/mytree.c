#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include<unistd.h>
#include <sys/stat.h>
#include <string.h>

#define MAX_NAME_LEN 256

// function to recursively print the tree structure of a directory
void print_tree(char *path, int depth) {
    // pointer to the directory stream
    DIR *dir;
    // structure to store information about each directory entry
    struct dirent *entry;
    // structure to store information about a file
    struct stat statbuf;
    // buffer to store the new path for each subdirectory
    char new_path[MAX_NAME_LEN];

    // open the directory stream
    if (!(dir = opendir(path)))
        // return if unable to open the directory
        return;

    // read each entry in the directory
    while ((entry = readdir(dir)) != NULL) {
        // skip the current and parent directories
        if (entry->d_name[0] == '.')
            continue;

        // create the new path for each subdirectory
        size_t len = snprintf(NULL, 0, "%s/%s", path, entry->d_name);
        char *new_path = malloc(len + 1);
        snprintf(new_path, len + 1, "%s/%s", path, entry->d_name);
        
        // retrieve information about the file
        lstat(new_path, &statbuf);

        // print the appropriate number of '|   ' for each level of depth
        for (int i = 0; i < depth; i++)
            printf("|   ");
        // print the entry name and '/' if it's a directory, '\'n' otherwise
        printf("|--- %s", entry->d_name);
        if (S_ISDIR(statbuf.st_mode)) {
            printf("/\n");
            // recursively call the function for subdirectories
            print_tree(new_path, depth + 1);
        } else
            printf("\n");
    }
    // close the directory stream
    closedir(dir);
}

int main(int argc, char **argv) {
    // buffer to store the path to be printed
    char path[MAX_NAME_LEN];

    // determine the path to print based on the input arguments
    switch (argc) {
        // if no arguments are provided, print the current directory
        case 1:
            if (getcwd(path, MAX_NAME_LEN) == NULL) {
                printf("Error: Failed to get current directory path\n");
                return 1;
            }
            break;
        // if one argument is provided, use it as the path to print
        case 2:
            strncpy(path, argv[1], MAX_NAME_LEN);
            break;
        // if more than one argument is provided, print an error message
        default:
            printf("Error: Too many arguments\n");
            return 1;
    }

    // check if the provided path is a directory
    struct stat statbuf;
    if (lstat(path, &statbuf) != 0) {
        printf("Error: Failed to get information about the path\n");
        return 1;
    }
    if (!S_ISDIR(statbuf.st_mode)) {
        printf("Error: The provided path is not a directory\n");
        return 1;
    }

    // print the path
    printf("%s\n", path);
    // call the function to print the tree structure of the directory
    print_tree(path, 0);

    return 0;
}
