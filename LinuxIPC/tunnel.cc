#include "allclass.h"
#include <assert.h>
using namespace std;


tunnel::tunnel()
{
}

//打印隧道信息
void tunnel::print_memory()
{
	int *content = (int *)load_mem(MEM_CONTENT);
	printf("\n");
	printf("——————————Tunnel Info——————————\n");
	for (int i = 1; i <= total_number_of_mailboxes; i++)//遍历打印所有邮箱信息
	{
		printf("mailbox No.%d:", i);
		for (int j = 0; j < memory_segment_size; j++)
		{
			if (content[i*memory_segment_size + j] == 0)
				break;
			printf("%c", content[i*memory_segment_size + j]);
		}
		printf("\n");
	}
	printf("\n");
}

//信号量及共享内存区的创建
void tunnel::setup_ipc()
{
	create_sharedmem(MEM_CONTENT, sizeof(int)*total_number_of_mailboxes*memory_segment_size);
	create_sharedmem(MEM_READER, sizeof(int)*total_number_of_mailboxes);
	create_sharedmem(MEM_VEHICLE, sizeof(int));
	create_sem(SEM_WAITING, 1);
	create_sem(SEM_R, total_number_of_mailboxes);
	create_sem(SEM_RW, total_number_of_mailboxes);
	create_sem(SEM_W, total_number_of_mailboxes);
	int *count = (int *)load_mem(MEM_VEHICLE);
	*count = maximum_number_of_cars_in_tunnel;
}

void tunnel::clear_ipc()
{
}

//读取输入文件内容
void tunnel::read()
{
	cin >> total_number_of_cars >> maximum_number_of_cars_in_tunnel >> tunnel_travel_time >> total_number_of_mailboxes >> memory_segment_size;
	string x;
	for (int i = 0; i < total_number_of_cars; i++)
	{
		cin >> x;
		assert(x[7] - 1 - '0' == i);
		string op, msg;
		int duration, mailboxnumber, id = 0, length;
		vector<r_message>* tmp_r = new vector<r_message>;
		vector<w_message>* tmp_w = new vector<w_message>;
		while (true)
		{
			id++;
			cin >> op;
			if (op == "end.")
				break;
			if (op == "w")
			{
				cin >> msg >> duration >> mailboxnumber;
				tmp_w->push_back(w_message(msg, mailboxnumber, duration, id));
			}
			else if (op == "r")
			{
				cin >> length >> duration >> mailboxnumber;
				tmp_r->push_back(r_message(length, mailboxnumber, duration, id));
			}
		}
		r.push_back(*tmp_r);
		w.push_back(*tmp_w);
	}

}

tunnel::~tunnel()
{
}
