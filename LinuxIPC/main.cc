#include <unistd.h>
#include <sys/wait.h> 
#include <sys/types.h>
#include "allclass.h"
#include <iostream>
using namespace std;

int main()
{
	freopen("sample.in", "r", stdin);
	tunnel T;
	pid_t mypid;
	int vehicle_number=0;

	T.read();
	T.setup_ipc();
	for (int i = 0; i < T.total_number_of_cars; i++)
	{
		mypid = fork();
		if (mypid == 0)
		{
			vehicle_number = i;
			printf("Car %d is coming\n", vehicle_number);
			break;
		}
	}

	if (mypid == 0)//child
	{
		vehicle my_car(T,vehicle_number);
		my_car.run();
	}
	else
	{
		for (int i = 0; i < T.total_number_of_cars; i++)
			pid_t t= wait(NULL);
		T.print_memory();
	}
    //destroy_sharedmem（vehicle_number）;
	return 0;
}

