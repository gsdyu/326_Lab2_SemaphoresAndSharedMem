#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <semaphore.h>
#include "dungeon_info.h"

//shared memory global variable. Global because it is used both in main and handler
struct Dungeon *sh_dungeon;
static void dungHandler(int sig){
	//the format for shifting the letters is to convert the decoded letters into the range of 0-25. Then apply the key (the shift is backwards/-key) and then convert it back to the appropriate range.
	//offset for uppercase letter to be in range 0-25
	int upper_offset=65;
	//offset for lowercase letter to be in range 0-25
	int lower_offset=97;
	int key=(int)sh_dungeon->barrier.spell[0]%26;
	char decode[SPELL_BUFFER_SIZE];
	strncpy(decode,sh_dungeon->barrier.spell+1,SPELL_BUFFER_SIZE);
	for (int i=0;i<SPELL_BUFFER_SIZE;i++){
		int value = decode[i];
		// if value is an actual letter, capital or lower, the remainder will never be 26;26 remains a case where value is not a letter and cannot be shift
		int remainder=26;
		// created variable offset as offset to keep track of which offset (upper or lower) needs to be used when converting value back in the last line.
		int offset;
		//checks range of uppercase letters
		if (value>=65&&value<=90){
			offset = upper_offset;
			remainder = ((int)value-offset-key)%26;

		}
		//checks range of lowercase letters
		else if (value>=97&&value<=122){
			offset = lower_offset;
			remainder = ((int)value-offset-key)%26;
		}
		//remainder has not been changed;value is not a letter. code later converts the letter back to its value with offset; is not related to a value that is not a letter hence continue
		if (remainder == 26) continue;
		//% operator does not loop negative number back to the front. this if statement loops number back to the front.
		if (remainder < 0) remainder = 26+remainder;
		decode[i] = (char)(remainder+offset);
	}	
	memcpy(sh_dungeon->wizard.spell,decode,SPELL_BUFFER_SIZE);
	return;
}

static void semaHandler(int sig){
	sem_t* sema = sem_open(dungeon_lever_two,O_RDWR);
	sem_wait(sema);
	//will sleep until rogue sends a signal.
	sleep(10000);
	sem_post(sema);
	sem_close(sema);
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
		perror("sigaction");
		_exit(EXIT_FAILURE);
	}
	//set sigaction for SEMAPHORE_SIGNAL
	sigset_t semaMask;
	sigemptyset(&semaMask);
	struct sigaction semaSigaction;
	semaSigaction.sa_handler=&semaHandler;
	semaSigaction.sa_mask=semaMask;
	if(sigaction(SEMAPHORE_SIGNAL,&semaSigaction,NULL)==-1){
		perror("Semaphore wizard");
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
		perror("munmap wizard");
		_exit(EXIT_FAILURE);
	}	
}

