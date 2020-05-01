#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <wait.h>
#include <sys/resource.h>

void cont(int sig_num){
	signal(SIGCONT,cont);
	printf("Process %d starts...\n",getpid());
}

void stop(int sig_num){
	signal(SIGUSR1,stop);
	printf("Process %d ends...\n",getpid());
	pause();
}

int main(int argc, char const *argv[])
{
	int i, num;
	num = atoi(&argv[0][1]);
	signal(SIGUSR1,stop);
	signal(SIGCONT,cont);
	setpriority(PRIO_PROCESS,0,-20);
	pause();
	for (i = -5; i <= num; i++)
	{
		printf("This is program %s and it prints for the %d time...\n",argv[0],i+6 );
		usleep(10000);
	}
}