#include <iostream>
#include <fstream>
#include <queue>
#include <vector>
#include <chrono>
#include <thread>
#include <string>
#include <mutex>
#include <condition_variable>



int GLOBAL_TIME = 0; //global variable representing time
std::mutex schedulermtx; //mutex for the scheduler
std::condition_variable cv; //condition variable for processs schedulering
std::ofstream output_file; //output file stream

class Process
{
public:
	Process(int inReadyTime, int inServiceTime, int id) :
		readyTime(inReadyTime),
		serviceTime(inServiceTime), 
		remainingTime(inServiceTime),
		processNumber(id),
		completed(false)
	{

	}

	void run(const std::string& username)
	{
		std::unique_lock<std::mutex> lock(schedulermtx);
		output_file << "Time " << GLOBAL_TIME << ", User " << username <<", Process " << processNumber << ", Resumed " << std::endl;
		lock.unlock();
		std::this_thread::sleep_for(std::chrono::seconds(quantumTime));
		lock.lock();
		remainingTime -= quantumTime;
		GLOBAL_TIME += quantumTime;

		if (remainingTime <= 0)
		{
			completed = true;
			output_file << "Time " << GLOBAL_TIME << ", User " << username << ", Process " << processNumber << ", Finished " << std::endl;
		}
		else
		{
			output_file << "Time " << GLOBAL_TIME << ", User " << username << ", Process " << processNumber << ", Paused " << std::endl;
		}
		lock.unlock();
		cv.notify_all();
	}

	bool isReady() const { return readyTime <= GLOBAL_TIME; }
	bool isCompleted() const {return completed;}
	void setQuantumTime(int inputQT){quantumTime = inputQT;}


private:
	int readyTime;
	int serviceTime;
	int remainingTime;
	int quantumTime;
	bool completed;
	int processNumber;

};

class User
{
public:	
	std::vector<Process> listOfProcesses;
	std::string username;

	User(std::string& inUsername):username(inUsername){}

	void addProcess(int readyTime, int serviceTime)
	{
		listOfProcesses.emplace_back(readyTime, serviceTime, processNumberCounter++);
		// Process newProcess(readyTime, serviceTime, processNumberCounter);
		// processNumberCounter++;
		// listOfProcesses.push(newProcess);
	}

	std:: vector<Process*> getReadyProcesses(){
		std::vector<Process*> readyProcesses;
		for (auto& process : listOfProcesses){
			if (process.isReady() && !process.isCompleted()){
				readyProcesses.push_back(&process);
			}
		}
		return readyProcesses;
	}

	bool allProcessesCompleted()
	{
		for (auto& process : listOfProcesses){
			if (!process.isCompleted()){
				return false;
			}
		}
		return true;
	}

	void distributeQuantumTime(int totalQuantum)
	{
		int processNumberCounter = getReadyProcesses().size(); //get the number of processes that are ready
		if (processNumberCounter>0){
			int evenQuantumTime = totalQuantum/processNumberCounter;
			for (auto* process : getReadyProcesses()){
				process->setQuantumTime(evenQuantumTime);
			}
		}
	}

private:
	int processNumberCounter = 0;
};
class Scheduler
{
public:
	Scheduler(int inFixedTotalQuantumtime) ://member initiliazer list
		totalQuantumTime(inFixedTotalQuantumtime){}

	void scheduleListOfProcesses(std::vector<User>& listOfUsers){
		using namespace std::chrono_literals;
		while (!allUsersCompleted(listOfUsers)){
			distributeQuantum(listOfUsers);

			for (auto& user : listOfUsers){
				vector<Process*> readyProcesses = user.getReadyProcesses();
				for (auto* process : readyProcesses){
					if (process->isCompleted()){
						thread processThread(&Process::run, process, user.username);
						processThread.join();
					}
				}
			}
			GLOBAL_TIME++;//from the global variable
			std::this_thread::sleep_for(1s);//wait 1 second
		}
	}
	
	private:

	void distributeQuantum(std::vector<User>& listOfUsers)
	{
		std::vector<User*> activeUsers;
		for (auto& user : listOfUsers){
			if (!user.allProcessesCompleted())
			{
				activeUsers.push_back(&user);
			}
		}
		int perUserQuantum = totalQuantumTime/activeUsers.size();
		for (auto* user : activeUsers){
			user->distributeQuantumTime(perUserQuantum);
		}
	}

	bool allUsersCompleted(std::vector<User>& inListOfUsers)
	{
		for (auto& user : inListOfUsers){
			if (!user.allProcessesCompleted()){
				return false;
			}
		}
		return true;
	}
	
	int totalQuantumTime; //Total quantum time given for each User

};

int main()
{
	ifstream input_file("input.txt");
    output_file.open("output.txt");

	if (!input_file || !output_file)
	{
		std::cerr << "Error opening the file" << std::endl;
		return 1;
	}

	int totalQuantumTime;
	input_file >> totalQuantumTime;

	std::vector<User> users;
	string username;
	int processNumberCounter;

	while (input_file >> username >> processNumberCounter)
	{
		User newUser(username);
		for (int i = 0; i < processNumberCounter; i++)
		{
			int readyTime, serviceTime;
			input_file >> readyTime >> serviceTime;
			newUser.addProcess(readyTime, serviceTime);
		}
		users.push_back(newUser);
	}
	input_file.close();

	Scheduler scheduler(totalQuantumTime);
	scheduler.run(users);
	
	output_file.close();

	return 0;
}