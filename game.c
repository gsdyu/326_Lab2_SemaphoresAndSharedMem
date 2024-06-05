#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "dungeon_info.h"
#include <signal.h>
#include <semaphore.h>

int main(int argc, char *argv[])
{
	//create shared memory
	int fd;
	fd = shm_open(dungeon_shm_name,O_CREAT|O_RDWR,0666);
	if (fd == -1){
		perror("shm error");
		_exit(EXIT_FAILURE);
	}
	if (ftruncate(fd,sizeof(struct Dungeon))==-1){
		perror("shm trunc error");
		_exit(EXIT_FAILURE);
	}
	//create semaphores for the two levers of final round
	sem_t* lev_o = sem_open(dungeon_lever_one,O_CREAT,0666,1);
	sem_t* lev_t = sem_open(dungeon_lever_two,O_CREAT,0666,1);
	//create the 3 classes
	pid_t wizard,rogue,barbarian;
	wizard=fork();
	if (wizard==0){
		char *argVec[3];
		argVec[0]="wizard";
		execv("wizard",argVec);
		perror("wizard");
		_exit(EXIT_FAILURE);
	}
	else{
		printf("wizard created\n");
	}
	rogue=fork();
	if (rogue==0){
		char *argVec[3];
		argVec[0]="rogue";
		execv("rogue",argVec);
		perror("rogue");
		_exit(EXIT_FAILURE);
	}
	else{
		printf("rogue created\n");
	}
	barbarian=fork();
	if (barbarian==0){
		char *argVec[3];
		argVec[0]="barbarian";
		int number = execv("barbarian",argVec);
		perror("barbarian");
		_exit(EXIT_FAILURE);
	}
	else{
		printf("barbarian created\n");
	}
	printf("This is parent:%d, wizard:%d, rogue:%d, barbarian:%d\n",getpid(),wizard,rogue,barbarian);	
	//wait some seconds for all the processes to get ready after fork is called
	sleep(5);
	RunDungeon(wizard, rogue, barbarian);
	//cleanup
	if (close(fd)==-1){
		perror("fd close");
		_exit(EXIT_FAILURE);
	}
	//RunDungeon seems to already unlink;running this code tells that the file does not exist
	/*if (shm_unlink(dungeon_shm_name)==-1){
		perror("fd unlink");
		_exit(EXIT_FAILURE);
	}*/
}
