#include<stdio.h>
#include<stdlib.h>
#include<sys/time.h>
#include<unistd.h>
#include<sys/resource.h>
#include<sys/wait.h>

void mytime(char **argv)
{
	struct rusage usg;
	struct timeval start, end;
	pid_t pid;
	int status;

	gettimeofday(&start, NULL);

	pid = fork();
	if (pid == 0)
	{
		execvp(argv[0], argv);
		exit(0);
	}
	else
	{
		waitpid(pid, &status, 0);
		getrusage(RUSAGE_CHILDREN, &usg);
		gettimeofday(&end, NULL);
	}

	double user_time = (double) usg.ru_utime.tv_sec + (double) usg.ru_utime.tv_usec / 1000000;
	double system_time = (double) usg.ru_stime.tv_sec + (double) usg.ru_stime.tv_usec / 1000000;
	double elapsed_time = (double) (end.tv_sec - start.tv_sec) + (double) (end.tv_usec - start.tv_usec) / 1000000;

	printf("user time: %lf\n", user_time);
	printf("system time: %lf\n", system_time);
	printf("elapsed time: %lf\n", elapsed_time);
}

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		printf("Error occured\n");
		return 1;
	}
	mytime(&argv[1]);
	return 0;
}


