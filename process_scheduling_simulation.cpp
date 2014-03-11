/*
	Name: Hai Tran
	Peoplesoft ID: 1056042
	Class: COSC 4330 - Operating System
*/

#include <iostream>
#include <string>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <queue>
#include <iomanip>
#include <algorithm>

using namespace std;

struct Process
{
	int process_num;		
	int status;				// check process is busy or not
	int ready_cpu;
	int require_cpu;
	int deadline;
	char step_type[40][5];
	int step_time[40]; 
	int current_step;				// current step that process is executing
	int cpu_run;					// if running, what cpu can run this process (0: not run)
	int num_step;
	int complete_time;
	int end_time;		// time end process
};

class CompareTimeDevice
{
	public:
		bool operator() (Process& p1, Process& p2)
		{
			if (p1.step_time[p1.current_step] > p2.step_time[p2.current_step]) return true;
			if (p1.step_time[p1.current_step]==p2.step_time[p2.current_step] && p1.process_num>p2.process_num) return true;
			return false;
		}

};

struct Device
{
	int status;
	int process_execute;		// process number that is executing in this device
	priority_queue <Process,vector<Process>,CompareTimeDevice> pQueue;
	queue <Process> nQueue;
	int complete_time;
};

struct ListStep
{
	int type;		// 0: process, 1: device
	int process_number;
	int device_number;
	int time;
};

class CompareTimeList
{
	public:
	bool operator() (ListStep& l1, ListStep& l2)
	{
		if (l1.time >l2.time) return true;
		if (l1.time == l2.time && l1.type>l2.type) return true;
		if (l1.time == l2.time && l1.type == l2.type && (l1.process_number > l2.process_number || l1.device_number > l2.device_number)) return true;
		return false;
	}
};

priority_queue<ListStep, vector<ListStep>, CompareTimeList> lQueue[5];		// global priority queue variable


void split_string( char src[], char dest[][5])
{
	char *prt;
	int i = 0;
	prt = strtok (src," ");
	while (prt != NULL)
	{
		strcpy(dest[i], prt);
		++i;
		prt = strtok(NULL," ");
	}
}
// checking_step_ type use to check this step is variable or not
bool checking_step_type (char src[5])
{
	bool found = false;
	char *ptr;
	ptr = &src[1];
	int a = atoi (ptr);
	++ptr;
	switch (src[0])
	{
		case 'R':
			found = true;
			break;
		case 'T':
			found = true;
			break;
		case 'D':
			if (strlen(src) == 1)
				found = true;
			break;
		case 'P':
			found = true;
			break;
		case 'C':
			found = true;
			break;
		case 'F':
			if ( (a > 0 && a < 5) && (strcmp(ptr,"w") == 0 || strcmp(ptr,"r") == 0))
				found = true;
			break;
	}
	return found;
}

// load all process from input file
int load_input (Process process[])
{
	int max_process = 0;
	ifstream input;
	input.open("Input.txt");
	while (!input.eof() && !input.fail())
	{
		char data[150] ="";
		char subData[40][5] ={""};
		int count = 0;
		input.getline(data,150);		// get line by line
		split_string (data, subData);	// split a string
		// load value into each process
		if (atoi(subData[0]) >= 0 )
		{
			process[max_process].process_num = max_process;
			process[max_process].current_step = 0;
			process[max_process].cpu_run = 0;
			if (atoi(subData[1]) > 0 && atoi(subData[1]) <5)
				process[max_process].ready_cpu = atoi (subData[1]);
			if (atoi(subData[2]) > 0 && atoi(subData[2]) <5)
				process[max_process].require_cpu = atoi(subData[2]);
			if (atoi(subData[3]) > 0)
				process[max_process].deadline = atoi (subData[3]);

			// load steps 
			for (int i =4; strcmp(subData[i],"\0") !=0; i += 2)
			{
				if (strcmp(subData[i],"C") == 0)
				{
					strcpy(process[max_process].step_type[count],subData[i]);
					++count; 
				}
				else
				{
					bool found = checking_step_type(subData[i]);
					if ( found == true && (atoi(subData[i+1]) > 0))
					{
						strcpy(process[max_process].step_type[count],subData[i]);
						process[max_process].step_time[count] = atoi(subData[i+1]);
						++count;
					}
				}
			}
			process[max_process].num_step = count;
			max_process +=1;
		}
		/*else
		{
			cout <<"error"<<endl;
		}*/
	}
	return max_process;
}

//checkingCpu to check a CPU free or not
void checkingCPU(int device_ID, int case_num, int process_ID, int current_time, Device device[], Process process[])
{
	if (device[device_ID].status == 1 )	// busy
	{
		if (case_num == 1 || case_num ==2)
			device[device_ID].nQueue.push(process[process_ID]);
		else
			device[device_ID].pQueue.push(process[process_ID]);
	}
	else	// free
	{
		device[device_ID].status = 1;
		device[device_ID].process_execute = process_ID;
		if ( case_num == 3 || case_num ==4)
			device[device_ID].complete_time= current_time + process[process_ID].step_time[process[process_ID].current_step];
		else
			device[device_ID].complete_time = current_time + 10;
		ListStep list;
		list.type = 1;
		list.device_number = device_ID;
		list.time = device[device_ID].complete_time;
		lQueue[case_num].push(list);
		
	}
	process[process_ID].status = 1;
	process[process_ID].cpu_run = 0;
}
void running (int case_num, int current_time, Device device[], Process process[], int process_ID)
{
	int i = process_ID;
	if (process[i].cpu_run == 0)
	{
		if (process[i].ready_cpu != 0)
		{
			if (process[i].ready_cpu != process[i].require_cpu && process[i].require_cpu != 0)
			{
				process[i].status = 0;
				process[i].cpu_run = process[i].require_cpu;
				process[i].complete_time = current_time + 15;
				ListStep list;
				list.type = 0;		
				list.process_number = i;
				list.time = process[i].complete_time;
				lQueue[case_num].push(list);
			}
			else
			{
				if (process[i].ready_cpu == process[i].require_cpu || process[i].require_cpu == 0)
				{
					checkingCPU (process[i].ready_cpu,case_num,i,current_time,device, process);
				}
			}
		}
		else
		{
			if (process[i].require_cpu != 0)
			{
				checkingCPU (process[i].require_cpu,case_num,i,current_time,device, process);
			}
		}
	}
	else
	{
		checkingCPU(process[i].cpu_run,case_num,i,current_time,device, process);
	}
}

void request_disk (int device_ID,int case_num, int current_time, Device device[], Process process[], int process_ID)
{
	if (device[device_ID].status == 1)			// busy
	{
		if (case_num == 1 || case_num == 3)
			device[device_ID].nQueue.push(process[process_ID]);
		else
			device[device_ID].pQueue.push(process[process_ID]);
	}
	else						// free
	{
		device[device_ID].status = 1;
		device[device_ID].process_execute = process_ID;
		if (case_num == 1 || case_num == 3)
		{
			device[device_ID].complete_time = current_time + 10;
		}
		else
			device[device_ID].complete_time = current_time + process[process_ID].step_time[process[process_ID].current_step];
		ListStep list;
		list.type = 1;
		list.device_number = device_ID;
		list.time = device[device_ID].complete_time;
		lQueue[case_num].push(list);

	}
}
void request_DVD (int device_ID,int case_num, int current_time, Device device[], Process process[], int process_ID)
{
	process[process_ID].complete_time = current_time + process[process_ID].step_time[process[process_ID].current_step];
	process[process_ID].step_time[process[process_ID].current_step] = process[process_ID].complete_time;		// tiime run is at complete time
	if (device[device_ID].status == 1)
	{
		if (process[process_ID].complete_time >= device[device_ID].complete_time)
			device[device_ID].pQueue.push(process[process_ID]);
		else
		{
			int i = device[device_ID].process_execute;
			process[i].step_time[process[i].current_step] = device[device_ID].complete_time;
			device[device_ID].pQueue.push(process[i]);
			device[device_ID].process_execute = process_ID;
			device[device_ID].complete_time = process[process_ID].step_time[process[process_ID].current_step];
			ListStep list;
			list.type = 1;
			list.device_number =  device_ID;
			list.time = device[device_ID].complete_time;
			lQueue[case_num].push(list);
		}
	}
	else
	{
		device[device_ID].status = 1;
		device[device_ID].process_execute = process_ID;
		device[device_ID].complete_time = process[process_ID].complete_time;
		ListStep list;
		list.type = 1;
		list.device_number =  device_ID;
		list.time = device[device_ID].complete_time;
		lQueue[case_num].push(list);
	}
}

void request_printer (int case_num, int current_time, Device device[], Process process[], int process_ID)
{
	if(device[7].status == 1 && device[8].status == 1)
	{
		device[7].nQueue.push(process[process_ID]);
	}
	else
	{
		ListStep list;
		if ( (device[7].status == 0 && device[8].status == 1) || (device[7].status == 0 && device[8].status == 0))
		{
			device[7].status = 1;
			device[7].process_execute = process_ID;
			device[7].complete_time = current_time + process[process_ID].step_time[process[process_ID].current_step];
			list.type = 1;
			list.device_number = 7;
			list.time = device[7].complete_time;
			lQueue[case_num].push(list);
		}
		else
		{
			if ( device[7].status == 1 && device[8].status == 0)
			{
				device[8].status = 1;
				device[8].process_execute = process_ID;
				device[8].complete_time = current_time + process[process_ID].step_time[process[process_ID].current_step];
				list.type = 1;
				list.device_number = 8;
				list.time = device[8].complete_time;
				lQueue[case_num].push(list);
			}
		}
	}
}

void request_table (int device_ID, int case_num, int current_time, Device device[], Process process[], int process_ID)
{
	process[process_ID].complete_time = current_time + process[process_ID].step_time[process[process_ID].current_step];
	if(device[device_ID].status == 0 )    // free
	{
		device[device_ID].status = 1;
		device[device_ID].process_execute = process_ID;
		device[device_ID].complete_time = process[process_ID].complete_time;
		ListStep list;
		list.type = 1;
		list.device_number = device_ID;
		list.time = device[device_ID].complete_time;
		lQueue[case_num].push(list);
	}
	else
	{
		int i = device[device_ID].process_execute;			// get process_ID from executing device
		if (process[i].step_type[process[i].current_step][2] =='w')			// device is written
		{
			device[device_ID].nQueue.push(process[process_ID]);
		}
		else			// device is read
		{
			if (process[process_ID].step_type[process[process_ID].current_step][2] == 'r')
			{
				if (device[device_ID].complete_time <= process[process_ID].complete_time)
				{
					if (!device[device_ID].nQueue.empty())
					{
						device[device_ID].nQueue.push(process[process_ID]);
					}
					else
					{
						process[process_ID].step_time[process[process_ID].current_step] = process[process_ID].complete_time;
						device[device_ID].pQueue.push(process[process_ID]);
					}
				}
				else
				{
					process[i].step_time[process[i].current_step] = device[device_ID].complete_time;
					device[device_ID].pQueue.push(process[i]);
					device[device_ID].process_execute = process_ID;
					device[device_ID].complete_time = process[process_ID].complete_time;
					ListStep list;
					list.type = 1;
					list.device_number = device_ID;
					list.time = device[device_ID].complete_time;
					lQueue[case_num].push(list);
				}
			}
			else
			{
				device[device_ID].nQueue.push(process[process_ID]);
			}
		}
	}
}

void run_process_step(int case_num, int current_time, Device device[], Process process[], int process_ID)
{
	int i = process_ID;
	int a = atoi(&process[process_ID].step_type[process[process_ID].current_step][1]) + 8;
	switch(process[i].step_type[process[i].current_step][0])
	{
		case 'R':
			running(case_num,current_time,device,process,process_ID);
			break;
		case 'D':
			request_disk(5,case_num,current_time,device,process,process_ID);
			break;
		case 'T':
			request_DVD(6,case_num,current_time,device,process,process_ID);
			break;
		case 'P':
			request_printer(case_num,current_time,device,process,process_ID);
			break;
		case 'F':
			request_table(a,case_num,current_time,device,process,process_ID);
			break;
		case 'C':
			process[process_ID].end_time = current_time;
			break;
	}
}

void CPU(int cpu_ID, Process process[], Device device[],int case_num, int current_time)
{
	int i = device[cpu_ID].process_execute;		// get process number
	if (case_num == 3 || case_num == 4)
	{
		process[i].current_step += 1;
		process[i].cpu_run = 0;
		run_process_step (case_num,current_time,device,process,i);
		if (!device[cpu_ID].pQueue.empty())
		{
			Process p = device[cpu_ID].pQueue.top();
			device[cpu_ID].pQueue.pop();
			//cout<<"next process 
			device[cpu_ID].process_execute = p.process_num;
			device[cpu_ID].complete_time = current_time + p.step_time[p.current_step];
			ListStep list;
			list.type = 1;
			list.device_number = cpu_ID;
			list.time = device[cpu_ID].complete_time;
			lQueue[case_num].push(list);
		}
		else
		{
			device[cpu_ID].status = 0;
			device[cpu_ID].process_execute = 0;
			device[cpu_ID].complete_time = 0;
		}
		
	}
	else
	{
		if ( case_num == 1 || case_num == 2)
		{
			process[i].step_time[process[i].current_step] -= 10;
			int time = process[i].step_time[process[i].current_step];
			if ( time <= 0)
			{
				process[i].current_step += 1;
				process[i].cpu_run = 0;
				run_process_step (case_num,current_time,device,process,i);
			}
			else
			{
				device[cpu_ID].nQueue.push (process[i]);
			}
			if (!device[cpu_ID].nQueue.empty())
			{
				Process p = device[cpu_ID].nQueue.front();
				device[cpu_ID].nQueue.pop();
				device[cpu_ID].process_execute = p.process_num;
				device[cpu_ID].complete_time = current_time + 10;
				ListStep list;
				list.type = 1;
				list.device_number = cpu_ID;
				list.time = device[cpu_ID].complete_time;
				lQueue[case_num].push(list);
			}
			else
			{
				device[cpu_ID].status = 0;
				device[cpu_ID].process_execute = 0;
				device[cpu_ID].complete_time = 0;
			}
		}
	}
}

void hardDisk(Process process[], Device device[],int case_num, int current_time)
{
	int i = device[5].process_execute;		// get process number
	if (case_num == 2 || case_num == 4)
	{
		process[i].current_step += 1;
		process[i].cpu_run = 0;
		run_process_step (case_num,current_time,device,process,i);
		if (!device[5].pQueue.empty())
		{
			Process p = device[5].pQueue.top();
			device[5].pQueue.pop();
			device[5].process_execute = p.process_num;
			device[5].complete_time = current_time + p.step_time[p.current_step];
			ListStep list;
			list.type = 1;
			list.device_number = 5;
			list.time = device[5].complete_time;
			lQueue[case_num].push(list);
		}
		else
		{
			device[5].status = 0;
			device[5].process_execute = 0;
			device[5].complete_time = 0;
		}
		
	}
	else
	{
		if ( case_num == 1 || case_num == 3)
		{
			process[i].step_time[process[i].current_step] -= 10;
			int time = process[i].step_time[process[i].current_step];
			if ( time <= 0)
			{		
				process[i].current_step += 1;
				process[i].cpu_run = 0;
				run_process_step (case_num,current_time,device,process,i);
			}
			else
			{
				device[5].nQueue.push (process[i]);
			}
			if (!device[5].nQueue.empty())
			{
				Process p = device[5].nQueue.front();
				device[5].nQueue.pop();
				device[5].process_execute = p.process_num;
				device[5].complete_time = current_time + 10;
				ListStep list;
				list.type = 1;
				list.device_number = 5;
				list.time = device[5].complete_time;
				lQueue[case_num].push(list);
			}
			else
			{
				device[5].status = 0;
				device[5].process_execute = 0;
				device[5].complete_time = 0;
			}
		}
	}
}

void dvdDisk(Process process[], Device device[],int current_time, int case_num)
{
	if (current_time == device[6].complete_time)
	{
		// run next step
		int i = device[6].process_execute;
		process[i].current_step += 1;
		process[i].cpu_run = 0;
		run_process_step(case_num, current_time,device,process,i);
		//take next process
		if(!device[6].pQueue.empty())
		{
			Process p = device[6].pQueue.top();
			device[6].pQueue.pop();
			device[6].process_execute = p.process_num;
			device[6].complete_time = p.step_time[p.current_step];
			ListStep list;
			list.type = 1;
			list.device_number = 6;
			list.time = device[6].complete_time;
			lQueue[case_num].push(list);
		}
		else
		{
			device[6].status = 0;
			device[6].process_execute = 0;
			device[6].complete_time = 0;
		}
	}
}

void printer (int printer_ID,int current_time,int case_num,Device device[], Process process[])
{
	// run next step
	int i = device[printer_ID].process_execute;
	process[i].current_step += 1;
	process[i].cpu_run = 0;
	run_process_step(case_num, current_time,device,process,i);
	// take next process
	if (!device[7].nQueue.empty())
	{
		Process p = device[7].nQueue.front();
		device[7].nQueue.pop();
		device[printer_ID].process_execute = p.process_num;
		device[printer_ID].complete_time = current_time + p.step_time[p.current_step];
		ListStep list;
		list.type = 1;
		list.device_number = printer_ID;
		list.time = device[printer_ID].complete_time;
		lQueue[case_num].push(list);
	}
	else
	{
		device[printer_ID].status = 0;
		device[printer_ID].process_execute = 0;
		device[printer_ID].complete_time = 0;
	}
}

void table (int table_ID, int current_time, int case_num,Device device[], Process process[])
{
	if(current_time == device[table_ID].complete_time)
	{
		// run next step
		int i = device[table_ID].process_execute;
		process[i].current_step += 1;
		process[i].cpu_run = 0;
		run_process_step(case_num, current_time,device,process,i);
		// take next process
		if ( !device[table_ID].pQueue.empty())
		{
			Process p = device[table_ID].pQueue.top();
			device[table_ID].pQueue.pop();
			device[table_ID].process_execute = p.process_num;
			device[table_ID].complete_time = p.step_time[p.current_step];
			ListStep list;
			list.type = 1;
			list.device_number= table_ID;
			list.time = device[table_ID].complete_time;
			lQueue[case_num].push(list);
		}
		else
		{
			if (!device[table_ID].nQueue.empty())
			{
				Process p = device[table_ID].nQueue.front();
				device[table_ID].nQueue.pop();
				if (p.step_type[p.current_step][2] == 'w')
				{
					device[table_ID].process_execute = p.process_num;
					device[table_ID].complete_time = current_time + p.step_time[p.current_step];
					ListStep list;
					list.type = 1;
					list.device_number = table_ID;
					list.time = device[table_ID].complete_time;
					lQueue[case_num].push(list);
				}
				else
				{
					p.complete_time = current_time + p.step_time[p.current_step];
					p.step_time[p.current_step] = p.complete_time;
					device[table_ID].pQueue.push(p);
					if (!device[table_ID].nQueue.empty())
					{
						Process p1 = device[table_ID].nQueue.front();
						while (p1.step_type[p1.current_step][2] != 'w')
						{
							p1.complete_time = current_time + p1.step_time[p1.current_step];
							p1.step_time[p1.current_step] = p1.complete_time;
							device[table_ID].pQueue.push (p1);
							device[table_ID].nQueue.pop();
							if(!device[table_ID].nQueue.empty())
							{
								p1 = device[table_ID].nQueue.front();
							}
							else
								break;
						}
					}

					Process p2 = device[table_ID].pQueue.top();
					device[table_ID].pQueue.pop();
					device[table_ID].process_execute = p2.process_num;
					device[table_ID].complete_time = p2.step_time[p2.current_step];
					ListStep list;
					list.type = 1;
					list.device_number = table_ID;
					list.time = device[table_ID].complete_time;
					lQueue[case_num].push(list);
				}
			}
			else
			{
				device[table_ID].status = 0;
				device[table_ID].process_execute = 0;
				device[table_ID].complete_time = 0;
			}
		}
	}
}

void run_device_step(int case_num, int current_time, Device device[], Process process[], int device_ID)
{
	int i = device_ID;
	if ( i == 1 || i == 2 || i == 3 || i == 4)
		CPU(i,process,device,case_num,current_time);
	else
	{
		if (i == 5)
			hardDisk(process,device,case_num, current_time);
		else
		{
			if (i == 6)
				dvdDisk(process,device,current_time,case_num);
			else
			{
				if ( i == 7 || i == 8)
					printer(i,current_time,case_num,device,process);
				else
				{
					if ( i == 9 || i == 10 || i == 11 || i == 12)
						table (i,current_time,case_num,device, process);
				}
			}
		}
	}
}

void init_device(Device device[])
{
	for (int i = 1; i<13; ++i)
	{
		device[i].status = 0;
		device[i].process_execute = 0;
		device[i].complete_time = 0;
	}
}

ListStep lookup_list(int case_num)
{
	ListStep list;
	if (!lQueue[case_num].empty())
	{
		list = lQueue[case_num].top();
		lQueue[case_num].pop();
	}
	return list;
}

void print_normalQueue(ofstream &output,queue <Process> q)
{
	while (!q.empty())
	{
		Process p = q.front();
		output<<p.process_num<<",";
		q.pop();
	}

}

void print_priorityQueue( ofstream &output, priority_queue<Process,vector<Process>,CompareTimeDevice> q)
{
	while (!q.empty())
	{
		Process p = q.top();
		output<<p.process_num<<",";
		q.pop();
	}
}
void print_cpu (ofstream &output, Device device[],int case_num)
{
	for (int i = 1; i < 5;++i)
	{
		if (device[i].status == 1)
		{
			queue<Process> q1 = device[i].nQueue;
			priority_queue<Process,vector<Process>,CompareTimeDevice> q2 = device[i].pQueue;
			output<<"CPU "<<i<<": "<<device[i].process_execute<<"(";
			if ( case_num == 1 || case_num == 2)
				print_normalQueue(output,q1);
			else
				print_priorityQueue(output,q2);
			output<<")"<<endl;
		}
		else
			output<<"CPU "<<i<<": ()"<<endl;
	}
}

void print_hardDisk(ofstream &output,Device device[],int case_num)
{
	if(device[5].status == 1)
	{
		queue<Process> q1 = device[5].nQueue;
		priority_queue<Process,vector<Process>,CompareTimeDevice> q2 = device[5].pQueue;
		output<<"Hard Disk: "<<": "<<device[5].process_execute<<"(";
		if ( case_num == 1 || case_num == 3)
			print_normalQueue(output,q1);
		else
			print_priorityQueue(output,q2);
		output<<")"<<endl;
	}
	else
		output<<"Hard Disk: ()"<<endl;
}

void print_DVD(ofstream &output, Device device[])
{
	if(device[6].status == 1)
	{
		output<<"DVD: "<<device[6].process_execute<<",";
		priority_queue<Process,vector<Process>,CompareTimeDevice> q = device[6].pQueue;
		print_priorityQueue(output,q);
		output<<endl;
	}
	else
	{
		output<<"DVD: "<<endl;
	}
}
void print_printer(ofstream &output, Device device[])
{
	queue<Process> q = device[7].nQueue;
	output<<"Printer: ";
	if(device[7].status == 1 && device[8].status == 1)
		output<<device[7].process_execute <<", "<< device[8].process_execute <<"(";
	else
	{
		if (device[7].status == 0 && device[8].status == 1)
			output<<device[8].process_execute<<"(";
		else
		{
			if (device[7].status == 1 && device[8].status == 0)
				output<<device[7].process_execute<<"(";
			else
				output<<"(";
		}
	}
	print_normalQueue(output,q);
	output<<")"<<endl;
}

void print_table(ofstream &output,Device device[],Process process[])
{
	for (int i = 9;i<13;++i)
	{
		if(device[i].status == 1)
		{
			queue<Process> q1 = device[i].nQueue;
			priority_queue<Process,vector<Process>,CompareTimeDevice> q2 = device[i].pQueue;
			output<<"Table "<<i-8<<": ";
			int j = device[i].process_execute;
			if(process[j].step_type[process[j].current_step][2] == 'w')
				output<< "W"<<j<<"(";
			else
			{
				output<<"R"<<j<<",";
				//print_priorityQueue(output,q2);
				while (!q2.empty())
				{
					Process p = q2.top();
					output<<"R"<<p.process_num<<",";
					q2.pop();
				}
				output<<"(";
			}

			//print_normalQueue(output,q1);
			while (!q1.empty())
			{
				Process p = q1.front();
				if (p.step_type[p.current_step][2] == 'w')
					output<<"W"<<p.process_num<<",";
				else
					output<<"R"<<p.process_num<<",";
				q1.pop();
			}
			output<<")"<<endl;
		}
		else
		{
			output<<"Table "<<i-8<<": "<<"()"<<endl;;
		}
	}
}

void display (ofstream &output, Device device[], Process process[],int case_num)
{
	print_cpu(output,device,case_num);
	print_hardDisk(output,device,case_num);
	print_DVD(output,device);
	print_printer(output,device);
	print_table(output,device,process);
}
int main()
{

	int case_num = 1;  // there are 4 cases
	for (case_num =1; case_num < 5; ++case_num)
	{
		// declare output file
		char buffer[2];
		sprintf(buffer,"%d",case_num);
		char filename[13]="";
		strcat(filename,"output");
		strcat(filename,buffer);
		strcat(filename,".txt");
		ofstream output;
		output.open (filename,ios::app);
		output <<" Case "<<case_num<<":"<<endl;

		//declare system
		int max_process = 0;
		Process process[50];
		Device device[13]; // 1-4: CPU, 5:Disk, 6:DVD, 7-8:printer, 9-12: table
		init_device (device);
		max_process = load_input (process);

		int current_time = 0;
		// run at current_time = 0
		for (int i =0;i <max_process; ++i)
		{
			run_process_step (case_num,current_time,device, process,i);// work for t = 0
		}
		output<<"------------ Time tick "<<current_time <<" -------------------"<<endl;
		display(output,device,process,case_num);
		
		while (!lQueue[case_num].empty())
		{
			ListStep list, nextList;
			list = lookup_list(case_num);
			//cout<<"List time is: "<< list.time<<endl;
			current_time = list.time;
			switch (list.type)					
			{
				case 0:
					run_process_step (case_num, current_time,device,process,list.process_number);
					break;
				case 1:
					run_device_step (case_num,current_time,device,process, list.device_number);
					break;
			}
			nextList = lQueue[case_num].top();
			if (current_time != nextList.time)
			{
				output<<"------------ Time tick "<<current_time <<" -------------------"<<endl;
				display (output,device,process,case_num);
			}
		}
		output<<"------------------------------------------------------"<<endl;
		int miss_deadline[max_process];
		int counter = 0;
		for(int i = 0; i<max_process;++ i)
		{
			if ((process[i].deadline - process[i].end_time) < 0 && process[i].deadline > 0)
			{
				miss_deadline[counter] = i;
				++counter;
			}
		}
		output<<"The total number of processes: "<<max_process<<endl;
		output<<"The number of processes missing their deadlines: "<<counter<<endl;
		output<<"The process IDs of late processes: ";
		for(int i =0; i<counter;++i)
		{
			output<<miss_deadline[i]<<",";
		}
		output<<endl;
	}
	
	return 0;
}
