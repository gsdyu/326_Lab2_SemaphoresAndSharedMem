#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <semaphore.h>
#include "dungeon_info.h"

//shared memory global variable. Global because used in in main and handler.
struct Dungeon *sh_dungeon;
static void dungHandler(int sig){
	sh_dungeon->barbarian.attack=sh_dungeon->enemy.health;
	return;
}

static void semaHandler(int sig){
	sem_t* sema=sem_open(dungeon_lever_one,O_RDWR);
	sem_wait(sema);
	//will sleep until it recieves a signal. 
	sleep(10000);
	sem_post(sema);
	sem_close(sema);
	return;
}

int main(){
	//set up sigaction for DUNGEON_SIGNAL
	sigset_t dungMask;
	sigemptyset(&dungMask);
	struct sigaction dungSigaction;
	dungSigaction.sa_handler=&dungHandler;
	dungSigaction.sa_mask=dungMask;
	if(sigaction(DUNGEON_SIGNAL,&dungSigaction,NULL)==-1){
		perror("dungeon_barb");
		_exit(EXIT_FAILURE);
	}
	//set up sigaction for SEMAPHORE_SIGNAL
	sigset_t semaMask;
	sigemptyset(&semaMask);
	struct sigaction semaSigaction;
	semaSigaction.sa_handler=&semaHandler;
	semaSigaction.sa_mask=semaMask;
	if(sigaction(SEMAPHORE_SIGNAL,&semaSigaction,NULL)==-1){
		perror("semaphore_barb");
		_exit(EXIT_FAILURE);
	}
	perror("sig");
	//set/connect shared memory
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
	//barbarian is created in game, but runDungeon is not called yet on creation for sh_dungeon->running to exist
	//sleep first to give game time to initialize runDungeon and make sh_dungeon.
	//after the first signal, while barbarian is potentially sleeping (with sleep(15)), the barbarian will exist the sleep and enter the appropriate sh_dungeon->running while loop
	sleep(15);
	while(sh_dungeon->running){
	 sleep(10);
	}
	//cleanup
	if (munmap(sh_dungeon,sizeof(struct Dungeon))==-1){
		perror("close");
		_exit(EXIT_FAILURE);
	}
	
}

