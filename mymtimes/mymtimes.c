#include<stdio.h>
#include<time.h>
#include<dirent.h>
#include<sys/stat.h>
#include<string.h>

int hr_count[24]={0};
time_t present;
char concat[1024], time1[40];
struct tm *tm_present;
void mymtimes(char *directory)
{
DIR *dp;
struct dirent *record;
struct stat statbuffer;
char path[1024];

 present = time(NULL) - 24*3600;
tm_present = localtime(&present);

if (directory == NULL)
{
directory = ".";

}
dp  = opendir(directory); 
	
if(dp != NULL)
{
while((record = readdir(dp)))
{
if((record->d_type==DT_DIR || record->d_type==DT_REG)&& strcmp(record->d_name,".")!=0 && strcmp(record->d_name, ".." )!=0)
{
snprintf(path, sizeof(path), "%s/%s", directory, record->d_name);
strcpy(concat,directory);
strcat(concat,"/");
strcat(concat,record->d_name);

if(stat(concat, &statbuffer) == 0)
{
	time_t mtime = statbuffer.st_mtime;
	if((record->d_type==DT_REG) && difftime(present, mtime)<= 86400)
	{
		int diff = difftime(present,mtime)+86400;
		hr_count[diff/3600]++;
	}
}
if(record->d_type ==DT_DIR && strcmp(record->d_name,".")!=0 && strcmp(record->d_name,"..")!=0)
{
char sub[1024]={0};
strcat(sub,directory);
strcat(sub,"/");
strcat(sub,record->d_name);
mymtimes(sub);
}
}
}
closedir(dp);
}
}
 int main(int argc, char *argv[])
{
if(argc > 1)
{
mymtimes(argv[1]);
}
else
{
mymtimes(NULL);
}
int i;
for(i=23;i>=0;i--)
{
present +=3600;
struct tm * tm_present = localtime(&present);
strftime(time1, sizeof(time1), "%a %b %d %T %Y", tm_present);
printf("%s: %d files\n", time1, hr_count[i]);

}
return 0;

}
