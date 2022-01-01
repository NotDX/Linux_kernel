#include "allclass.h"
#include <stdlib.h>  
#include <stdio.h>  
#include <unistd.h>
#include <string>
#include <iostream>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <time.h>  
#include <sys/time.h>
using namespace std;

vehicle::vehicle(tunnel &T,int id)
{
    this->id = id;
    this->T = &T;
    my_r = T.r[id];
    my_w = T.w[id];
    content = (int *)load_mem(MEM_CONTENT);
    reader_count = (int *)load_mem(MEM_READER);
    sem_mutex_reader_count = load_sem(SEM_R);
    sem_mutex_read_or_write = load_sem(SEM_RW);
    sem_mutex_writer = load_sem(SEM_W);
    sem_waiting = load_sem(SEM_WAITING);
    count = (int *)load_mem(MEM_VEHICLE);

    this->mailboxs_pointers = new int[T.total_number_of_mailboxes];
    for (int i = 0; i < T.total_number_of_mailboxes; i++)
    {
        this->mailboxs_pointers[i] = 0;
    }
    
    idx_r = 0;
    idx_w = 0;
}

//写入邮箱
void vehicle::write_to_mailbox(w_message s)
{
    P(sem_mutex_read_or_write, s.mailbox_number);
    P(sem_mutex_writer, s.mailbox_number);
    int j = 0;
    /*判断条件：邮箱没有写满，并且该位置有写过
     j：统计邮箱内写了多少内容*/
    while (j < T->memory_segment_size && content[s.mailbox_number*T->memory_segment_size +j] != 0)
        j++;
    for (int i=j; i < T->memory_segment_size && i-j<s.message.length(); i++)
        content[s.mailbox_number*T->memory_segment_size +i] = s.message[i - j];
    sleep(s.duration);
    V(sem_mutex_writer, s.mailbox_number);
    V(sem_mutex_read_or_write, s.mailbox_number);
}

//从邮箱读取
void vehicle::read_from_mailbox(r_message s, vector<int> &rt)
{
    int semval;
    //确保无写者对同一邮箱进行操作
    while(true)
    {
        semval=semctl(sem_mutex_read_or_write, s.mailbox_number,GETVAL);
        if(semval>0)
            break;
    }
    reader_count[s.mailbox_number]++;
    sleep(s.duration);
    int j;
    for (j = this->mailboxs_pointers[s.mailbox_number]; j < s.length && content[s.mailbox_number* T->memory_segment_size +j] != 0; j++)
        rt.push_back(content[s.mailbox_number*T->memory_segment_size +j]);
    if (j - this->mailboxs_pointers[s.mailbox_number] < s.length)
        rt.push_back(-1);
    else
        rt.push_back(0);
    this->mailboxs_pointers[s.mailbox_number] = j;
    reader_count[s.mailbox_number]--;
    vector<int>* tmp = new  vector<int>(rt);
    this->readed_msg.push_back(*tmp);
}

//等待进入隧道
void vehicle::waiting_and_in()
{
	int semid = sem_waiting;
	
	while (true)
	{
		P(semid, 0);
		if (*count > 0) // 隧道还有空位置，车辆进入隧道，将隧道填满
		{
            printf("Car %d is in the tunnel\n", this->id);
			*count = *count - 1;
			V(semid, 0);
			break;
		}
		else
		{
			V(semid, 0);
            /*idx_r：该车的读指令读到的位置
             my_r.size()：该车读指令的数量*/
			if (idx_r >= my_r.size() && idx_w >= my_w.size()) //读写都已经完成
				continue;
            
			if (idx_r < my_r.size() && idx_w >= my_w.size())
			{
				printf("Car %d is outside the tunnel , reading attempt\n",this->id);
				sleep(my_r[idx_r].duration);
				idx_r++;
				continue;
			}

			if (idx_r >= my_r.size() && idx_w < my_w.size())
			{
				printf("Car %d is outside the tunnel , writing attempt\n", this->id);
				sleep(my_w[idx_w].duration);
				idx_w++;
				continue;
			}
            /*读指令的位置先于写指令，就先执行读*/
			if (my_r[idx_r].id < my_w[idx_w].id)
			{
				printf("Car %d is outside the tunnel , reading attempt\n", this->id);
				sleep(my_r[idx_r].duration);
				idx_r++;
			}
			else
			{
				printf("Car %d is outside the tunnel , writing attempt\n",  this->id);
				sleep(my_w[idx_w].duration);
				idx_w++;
			}
		}
	}
}

//隧道中行驶
void vehicle::run()
{
	clock_gettime(CLOCK_MONOTONIC, &(this->start_time_including_waiting));
	waiting_and_in();
	gettimeofday(&(this->start_time_excluding_waiting), NULL);
	//读写都未完成
	while (idx_r < my_r.size() && idx_w < my_w.size())
	{
		struct timeval now;
		gettimeofday(&(now), NULL);
        //超时
		if (now.tv_sec-this->start_time_excluding_waiting.tv_sec >= this->T->tunnel_travel_time)
			break;
        //读指令的位置先于写指令，就先执行读
		if (my_r[idx_r].id < my_w[idx_w].id)
		{
			vector<int>	rt;
			read_from_mailbox(my_r[idx_r], rt);
            printf("\n");
			printf("Car %d is Reading\n", this->id);
			printf("Read in mailbox No. %d\n", my_r[idx_r].mailbox_number);
			printf("The content:");
			for (int i = 0; i < rt.size(); i++)
			{
				if (rt[i] == -1)
					printf("reach the bottom of the mailbox!!!");
				else
					printf("%c", rt[i]);
			}
			printf("\n");
            printf("\n");
			idx_r++;
		}
		else
		{
			write_to_mailbox(my_w[idx_w]);
            printf("\n");
			printf("Car %d is Writing\n", this->id);
			printf("Write in mailbox No. %d\n", my_w[idx_w].mailbox_number);
			printf("The content:");
			cout << my_w[idx_w].message << endl;
            printf("\n");
			idx_w++;
		}
	}
    //没有读完 继续读
	while (idx_r < my_r.size())
	{
		struct timeval now;
		gettimeofday(&(now), NULL);
		if (now.tv_sec - this->start_time_excluding_waiting.tv_sec >= this->T->tunnel_travel_time)
			break;
		vector<int>	rt;
		read_from_mailbox(my_r[idx_r], rt);
        printf("\n");
		printf("Car %d is Reading\n", this->id);
		printf("Read in mailbox No. %d\n", my_r[idx_r].mailbox_number);
		printf("The content:");
		for (int i = 0; i < rt.size(); i++)
		{
			if (rt[i] == -1)
				printf("reach the bottom of the mailbox!!!");
			else
				printf("%c", rt[i]);
		}
		printf("\n");
        printf("\n");
		idx_r++;
	}
    //没有写完 继续写
	while (idx_w < my_w.size()) 
	{
		struct timeval now;
		gettimeofday(&(now), NULL);
		if (now.tv_sec - this->start_time_excluding_waiting.tv_sec >= this->T->tunnel_travel_time)
			break;
		write_to_mailbox(my_w[idx_w]);
        printf("\n");
		printf("Car %d is Writing\n",  this->id);
		printf("Write in mailbox No. %d\n", my_w[idx_w].mailbox_number);
		printf("The content:");
		cout << my_w[idx_w].message << endl;
        printf("\n");
		idx_w++;
	}
	
	struct timeval now;
	gettimeofday(&(now), NULL);
	if (now.tv_sec - this->start_time_excluding_waiting.tv_sec < this->T->tunnel_travel_time)
	{
		printf("Car %d is Runing\n", this->id);
		sleep(this->T->tunnel_travel_time - ((double)(now.tv_sec - this->start_time_excluding_waiting.tv_sec)));
	}
	leave();

}

//离开隧道
void vehicle::leave()
{
    P(this->sem_waiting, 0);
    *count=*count+1;//隧道空位加一
    V(this->sem_waiting, 0);
    struct timeval now;
    struct timespec tpend;
    gettimeofday(&now, NULL);
    clock_gettime(CLOCK_MONOTONIC, &(tpend));
    printf("\n");
    printf("Car %d is leaving\n", this->id);
    //printf("[Detail]:        Memory Summary:\n");
    for (int i = 1; i <= readed_msg.size(); i++)
    {
        printf("Message No. %d:",i);
        for (int j = 0; j < readed_msg[i].size(); j++)
        {
            if (readed_msg[i][j] == -1)
            {
                printf("reach the bottom of the mailbox!!!");
                break;
            }
            else
                printf("%c", readed_msg[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

vehicle::~vehicle()
{
}
