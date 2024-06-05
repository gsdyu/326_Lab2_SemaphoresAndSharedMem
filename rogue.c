#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include "dungeon_info.h"

//shared memory global variable. Global because it is used both in main and handler
struct Dungeon *sh_dungeon;
static void dungHandler(int sig){
	int max = MAX_PICK_ANGLE;
	int min = 0;
	int value = (max+min)/2;
	while(sh_dungeon->trap.locked){
		if (sh_dungeon->trap.direction=='u'){ 
			min = value;
			sh_dungeon->trap.direction='t';
		}
		else if (sh_dungeon->trap.direction=='d'){ 
			max = value;
			sh_dungeon->trap.direction='t';
		}
		else if (min==max){
			//if min and max are the same, value will not change. need to reset. min = 0
			min = 0;
			sh_dungeon->trap.direction='t';
		}
		else if (min==max-1){
			//same idea as checking for min==max. in this case min increments up to max since max has not changed, implying need a new and higher max value.
			max = MAX_PICK_ANGLE;
			sh_dungeon->trap.direction='t';
		}
		else continue;  
		value = (max+min)/2;
		sh_dungeon->rogue.pick=value;
		usleep(TIME_BETWEEN_ROGUE_TICKS+50);
	}
	printf("\nmin:%d,max:%d\n",min,max);
	return;
}

static void semaHandler(int sig){
	//the final level seems to be formatted as the string in treasure is added one by one in dungeon
	//rogue will constantly check the contents of treasure to see if a new value is added and add that content to spoils
	int current=0;
	char value;
	//until all content of treasure is added; is size 4
	while (current<4){
		value = sh_dungeon->treasure[current];
		//continue if value is NULL; the dungeon has not added a new content yet.
		if ((int)value==0) continue;
		sh_dungeon->spoils[current]=value;
		current+=1;
		sleep(1);
	}
	//sends a signal for wizard and barbarian. If the current fork method in game is used, rogue pid will be in between wizard and barbarian, hence -1 and +1.
	kill(getpid()-1,SIGINT);
	kill(getpid()+1,SIGINT);
	return;
}
int main(){

	//set sigaction for DUNGEON_SIGNAL
	sigset_t dungMask;
	sigemptyset(&dungMask);
	struct sigaction dungSigaction;
	dungSigaction.sa_handler=&dungHandler;
	dungSigaction.sa_mask=dungMask;
	if(sigaction(DUNGEON_SIGNAL,&dungSigaction,NULL)==-1){
		perror("dungeon_rogue");
		_exit(EXIT_FAILURE);
	}
	//set sigaction for SEMAPHORE_SIGNAL
	sigset_t semaMask;
	sigemptyset(&semaMask);
	struct sigaction semaSigaction;
	semaSigaction.sa_handler=&semaHandler;
	semaSigaction.sa_mask=semaMask;
	if(sigaction(SEMAPHORE_SIGNAL,&semaSigaction,NULL)==-1){
		perror("semaphore_rogue");
		_exit(EXIT_FAILURE);
	}
	//set up/connect to share memory
	int fd;
	fd=shm_open(dungeon_shm_name,O_CREAT|O_RDWR,0666);
	if (fd == -1){
		perror("shm_open");
		_exit(EXIT_FAILURE);
	}
	sh_dungeon=(struct Dungeon*)mmap(0,sizeof(struct Dungeon),PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
	if (sh_dungeon==MAP_FAILED){
		perror("mmap");
		_exit(EXIT_FAILURE);
	}
	close(fd);
	//explanation for bottom lines in barbarian.
	sleep(15);
	while(sh_dungeon->running){
	 sleep(10);
	}
	//cleanup
	if (munmap(sh_dungeon,sizeof(struct Dungeon))==-1){
		perror("munmap rogue");
		_exit(EXIT_FAILURE);
	}
}

