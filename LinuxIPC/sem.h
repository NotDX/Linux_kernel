#pragma once
#ifndef SEM
#define SEM

/*共享内存区和信号量地址定义，避免冲突*/

#define MEM_VEHICLE 13300
#define MEM_READER 11200
#define MEM_CONTENT 14500
#define SEM_WAITING 10100
#define SEM_RW 10200
#define SEM_W 10308
#define SEM_R 1048

#include <string>
#include <stdlib.h>  
#include <stdio.h>
#include <sys/shm.h>  
#include <sys/sem.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>

using namespace std;

extern void create_sem(int key, int number);
extern void create_sharedmem(int key, int number);
extern int load_sem(int key);
extern int P(int semid, int which);
extern int V(int semid, int which);
extern void *load_mem(int key);
extern string getTime();

#endif
