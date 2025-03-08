#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <thread>
#include <string>


int GLOBAL_TIME = 0;//global variable representing time


class Process
{
public:
	Process(int inReadyTime, int inServiceTime) :
		readyTime(inReadyTime),
		serviceTime(inServiceTime)
	{

	}
	void run(std::string& username)
	{
		std::cout << "Time " << GLOBAL_TIME << "Process " << processNumber << "Resumed " << std::endl;
	}
	int readyTime = 0;
	int serviceTime = 0;

	int currentServicedTime = 0;//incremental of service time, until ones meet service time, the process is not done
	int quantumTime = 0;
	bool completed = false;
	int processNumber;
private:

};

class User
{
public:
	User(std::string& inUsername):
		username(inUsername)
	{

	}
	void distributeCycleTime()
	{
		int totalNumberOfProcess = listOfProcesses.size();//size n that has the total number of user with processes (dynamic)
		int quantumTimeEvenlyDistributed = 0;
		quantumTimeEvenlyDistributed = quantumTime / totalNumberOfProcess;
		for (int i = 0; i < listOfProcesses.size(); i++)
		{
			listOfProcesses[i].quantumTime = quantumTimeEvenlyDistributed;
		}
	}
	void addProcessToUser(Process& inProcess)
	{
		inProcess.processNumber = processNumberCounter;
		processNumberCounter++;
		listOfProcesses.push_back(inProcess);
	}
	std::vector<Process> listOfArrivedProcessesAndStillRequireProcessing()//return the processes that are ready
	{
		std::vector<Process> processesThatHasArrived;
		for (int i = 0; i < listOfProcesses.size(); i++)
		{
			if ((listOfProcesses[i].readyTime >= GLOBAL_TIME)&&(!listOfProcesses[i].completed))
			{
				processesThatHasArrived.push_back(listOfProcesses[i]);
			}
		}
		return processesThatHasArrived;
	}
	bool areAllProcessesComplete()
	{
		for (int i = 0; i < listOfProcesses.size(); i++)
		{
			if (!listOfProcesses[i].completed)
			{
				return false;
			}
		}
		return true;
	}

	int quantumTime;//every user knows their own time, depending on the number of process, user will have different quantumTime
	std::vector<Process> listOfProcesses;
	std::string username;
	int processNumberCounter = 0;
private:
	
};
class Scheduler
{
public:
	Scheduler(int inFixedTotalQuantumtime) ://member initiliazer list
		totalQuantumTime(inFixedTotalQuantumtime)
	{

	}
	void schedulingListOfProcesses(std::vector<User>& listOfUsers)
	{
		using namespace std::chrono_literals;
		while (!areAllProcessesCompleteForAllUsers(listOfUsers))
		{
			distributeCycleTime(listOfUsers);
			std::vector<User>& retrieveListOfUsersThatHasProcesses(listOfUsers);
			for (int i = 0; i < retrieveListOfUsersThatHasProcesses.size(); i++)
			{
				
			}













			GLOBAL_TIME++;//from the global variable
			std::this_thread::sleep_for(1s);//wait 1 second
		}
	}
		
	void distributeCycleTime(std::vector<User>& listOfUsers)//calculation of the totalQuantumTime A,B,C but C has no process,
		//and A and B must be at 50%
	{
		//I receive the total list of user but must make another list of user the only has processes
		std::vector<User> listOfUsersThatHasProcesses = retrieveListOfUsersThatHasProcesses(listOfUsers);
		int totalNumberOfUsersWithProcess = listOfUsersThatHasProcesses.size();//size n that has the total number of user with processes (dynamic)
		//TotalQuantumTime*100%/totalNumberOfUserWithProcess
		// which becomes TotalQuantumTime*1/totalNumberOfUserWithProcess
		int quantumTimeEvenlyDistributed = 0;
		quantumTimeEvenlyDistributed = totalQuantumTime / totalNumberOfUsersWithProcess;
		//give this time evenly to each user that has processes
		for (int i = 0; i < listOfUsersThatHasProcesses.size(); i++)
		{
			listOfUsersThatHasProcesses[i].quantumTime = quantumTimeEvenlyDistributed;
		}
	}
	bool areAllProcessesCompleteForAllUsers(std::vector<User>& inListOfUsers)
	{
		for (int i = 0; i < inListOfUsers.size(); i++)
		{
			if (!inListOfUsers[i].areAllProcessesComplete())
			{
				return false;
			}
		}
		return true;
	}

private:
	
	std::vector<User> retrieveListOfUsersThatHasProcesses(std::vector<User>& listOfUsers)//responsible to only retrieve the list of User that has processes, useful cause we can use it multiple time
	{	//helper function and is not exposed to the main
		//I receive the total list of user but must make another list of user the only has processes
		std::vector<User> listOfUsersThatHasProcesses;
		for (int i = 0; i < listOfUsers.size(); i++)
		{
			//if the !list of user vector is not empty, which means there exist process, we push back the listofUsers
			//in the vector of listOfUsersThatHasProcesses
			if (!listOfUsers[i].listOfArrivedProcessesAndStillRequireProcessing().empty())//this returns a boolean, user has process
			{
				listOfUsersThatHasProcesses.push_back(listOfUsers[i]);
			}
		}
		return listOfUsersThatHasProcesses;
	}
	int totalQuantumTime;//Total quantum time given for each User

};

int main()
{
	int totalQuantumTime = 4;
	std::vector<User> users;
	Scheduler scheduler(totalQuantumTime);
	std::string usernameA = "A";
	User a(usernameA);//own process P1 and P2
	Process a_P0(4,3);
	Process a_P1(1,5);
	a.addProcessToUser(a_P0);//process P1 is added to User a
	a.addProcessToUser(a_P1);
	std::string usernameB = "B";
	User b(usernameB);//owns P3 and ready for execution
	Process b_P0(5,6);
	b.addProcessToUser(b_P0);

	//someone does input and output files please, it's same as last assignment redirect the cout to the file
	//someone else does threads (maybe use conditonal variable)
	return 0;
}