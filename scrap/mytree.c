#include<stdio.h>
#include<string.h>
#include<dirent.h>

void mytree(char* path, int level)
{
DIR* directory = opendir(path);
//char next_path[1024];

if(directory == NULL)
{
return;
}
struct dirent* record;
while((record = readdir(directory)) != NULL)
{
if(strcmp(record->d_name, ".") == 0 || strcmp(record->d_name, "..") == 0)
{
continue;
}
int i;
char character='.';
if(record->d_name[0]!=character)
{
for(i = 0; i < level; i++)
{
printf("|   ");
}
if(record->d_type == DT_DIR)
{
printf("|--- %s/\n",record->d_name);
}
else
printf("|--- %s\n",record->d_name);
}
if (record->d_type == DT_DIR)
{

char next_path[1024] = {0};

snprintf(next_path, sizeof(next_path), "%s/%s", path, record->d_name);
mytree(next_path, level + 1);
}
}
closedir(directory);
}
int main(int argc, char* argv[])
{
char* path = ".";
if (argc > 1)
{
path = argv[1];
}
printf("%s\n", path);
mytree(path, 0);
return 0;
}

