#pragma once
#include"sem.h"
#include<vector>
#include<iostream>
#include <sys/types.h>

using namespace std;

//消息长度，读时长，邮箱号，车辆号
class r_message
{
public:
    int length, mailbox_number, duration,id;
    r_message(int length, int mailbox_number, int duration,int id)
    {
        this->length = length;
        this->mailbox_number = mailbox_number;
        this->duration = duration;
        this->id = id;
    }
};

//消息内容，写时长，邮箱号，车辆号
class w_message
{
public:
    string message;
    int mailbox_number, duration,id;
    w_message(string message, int mailbox_number, int duration,int id)
    {
        this->message = message;
        this->mailbox_number = mailbox_number;
        this->duration = duration;
        this->id = id;
    }
};

class tunnel
{
public:
	vector<vector<r_message> > r;
	vector<vector<w_message> > w;
	int total_number_of_mailboxes, memory_segment_size;
	int total_number_of_cars, maximum_number_of_cars_in_tunnel, tunnel_travel_time;
    
	tunnel();
	void print_memory();
	void setup_ipc();
	void clear_ipc();
	void read();
	~tunnel();
};

class vehicle
{
private:
    int idx_r, idx_w;
    vector<r_message> my_r;
    vector<w_message> my_w;
    int id;
    tunnel *T;
    struct timeval start_time_excluding_waiting;
    struct timespec start_time_including_waiting;
    int *count,
        *content,
        *reader_count,
        *mailboxs_pointers;
    int sem_mutex_reader_count,
        sem_mutex_read_or_write,
        sem_mutex_writer,
        sem_waiting;
    vector<vector<int> > readed_msg;
public:
    vehicle(tunnel &T, int id);
    void waiting_and_in();
    void leave();
    void write_to_mailbox(w_message);
    void read_from_mailbox(r_message, vector<int> &rt);
    void run();
    ~vehicle();
};


