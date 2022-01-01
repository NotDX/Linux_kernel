#include <string>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include "sem.h"
#include "allclass.h"
/*struct sembuf
{
    short sem_num; //信号量的编号
    short sem_op;  //信号量一次PV操作时加减的数值 +1,-1
    short sem_flg; //SEM_UNDO,使操作系统跟踪当前进程对该信号量的修改情况
}*/

union semun
{
    int val;    //用于SETVAL命令
    struct semid_ds *buf;    //用于IPC_STAT, IPC_SET
    unsigned short  *array;  //用于GETALL, SETALL
    struct seminfo  *__buf;  //用于IPC_INFO
};

//创建信号量集合
void create_sem(int key, int num_sems)
{
    /*key:信号量所对应的键值（可取任意值，只需保证不同信号量所取键值不同即可）
    num_sems:指定需要的信号量数目*/
    int semid = semget((key_t)key, num_sems, IPC_CREAT|0666);//创建信号量
    if (semid < 0)
    {
        perror("Semget error");
        return;
    }
    union semun sem_un;
    for(int i=0;i<num_sems;i++)
    {
        sem_un.val = 1;
        int rt=semctl(semid, i, SETVAL, sem_un); //赋初值，初始化信号量
        if(rt<0)
        {
            perror("Semctl Set val error");
            return;
        }
    }
}

//创建单个信号量
int load_sem(int key)
{
    int semid = semget((key_t)key, 0, IPC_CREAT);
    if (semid == -1)
        perror("load error");
    return semid;
}

// P操作，信号量-1（wait）
int P(int semid, int which)
{
    sembuf mysembuf;
    mysembuf.sem_num = which;
    mysembuf.sem_op = -1;
    mysembuf.sem_flg = SEM_UNDO;
    return semop(semid, &mysembuf, 1);
}

// V操作，信号量+1（signal）
int V(int semid, int which)
{
    sembuf mysembuf;
    mysembuf.sem_num = which;
    mysembuf.sem_op = 1;
    mysembuf.sem_flg = SEM_UNDO;
    return semop(semid, &mysembuf, 1);
}


//创建共享内存区
void create_sharedmem(int key, int size)
{
    /*key:共享内存对应的键值
    size:共享内存的容量（以字节为单位）*/
    int shmid = shmget((key_t)key, size, 0666 | IPC_CREAT); //创建共享内存
    if (shmid == -1)
    {
        printf("shmget failed\n");
        return;
    }
}

//建立共享内存链接
void *load_mem(int key)
{
    int shmid = shmget((key_t)key, 0, IPC_CREAT);
    if (shmid == -1)
    {
        perror("shmget failed\n");
        exit(EXIT_FAILURE);;
    }
    void *shm = shmat(shmid, (void*)0, 0);//建立链接，指定共享内存连接到当前进程中的地址位置
    if (shm == (void*)-1)
    {
        perror("shmat failed\n");
        exit(EXIT_FAILURE);;
    }
    return shm;
}

//销毁共享内存区
void destroy_sharedmem(int key)
{
    int shmid = shmget((key_t)key, 0, IPC_CREAT);
    if (shmid == -1) //未创建成功
    {
            printf("Nothing to destroy.Destroy null\n");
            return;
    }
    else
    {
        int ret = shmctl(shmid, IPC_RMID, 0);//删除共享内存
        if (ret < 0)
        {
            printf("Destroy failed\n");
            return;
        }
    }
}


//获取时间，计时函数
string getTime()
{
    time_t timep;
    time(&timep);
    char tmp[64];
    strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", localtime(&timep));
    return tmp;
}
