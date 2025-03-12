#include <iostream>
#include <fstream>
#include <queue>
#include <vector>
#include <chrono>
#include <thread>
#include <string>
#include <mutex>
#include <condition_variable>

using namespace std;

int GLOBAL_TIME = 0;   // global variable representing time
mutex schedulermtx;	   // mutex for the scheduler
condition_variable cv; // condition variable for processs schedulering
ofstream output_file;  // output file stream

class Process
{
public:
	Process(int inReadyTime, int inServiceTime, int id) : readyTime(inReadyTime),
														  serviceTime(inServiceTime),
														  remainingTime(inServiceTime),
														  processNumber(id),
														  completed(false)
	{
	}

	void run(const string &username)
	{
		unique_lock<mutex> lock(schedulermtx);
		output_file << "Time " << GLOBAL_TIME << ", User " << username << ", Process " << processNumber << ", Resumed " << endl;
		lock.unlock();

		this_thread::sleep_for(chrono::seconds(quantumTime));
		lock.lock();
		remainingTime -= quantumTime;

		if (remainingTime <= 0)
		{
			completed = true;
			output_file << "Time " << GLOBAL_TIME + quantumTime << ", User " << username << ", Process " << processNumber << ", Finished " << endl;
		}
		else
		{
			output_file << "Time " << GLOBAL_TIME + quantumTime << ", User " << username << ", Process " << processNumber << ", Paused " << endl;
		}
		lock.unlock();
		cv.notify_all();
	}

	bool isReady() const { return readyTime <= GLOBAL_TIME; }
	bool isCompleted() const { return completed; }
	void setQuantumTime(int inputQT) { quantumTime = inputQT; }

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
	vector<Process> listOfProcesses;
	string username;

	User(const string &inUsername) : username(inUsername), processNumberCounter(0) {}

	void addProcess(int readyTime, int serviceTime)
	{
		listOfProcesses.emplace_back(readyTime, serviceTime, processNumberCounter++);
		// Process newProcess(readyTime, serviceTime, processNumberCounter);
		// processNumberCounter++;
		// listOfProcesses.push(newProcess);
	}

	vector<Process *> getReadyProcesses()
	{
		vector<Process *> readyProcesses;
		for (auto &process : listOfProcesses)
		{
			if (process.isReady() && !process.isCompleted())
			{
				readyProcesses.push_back(&process);
			}
		}
		return readyProcesses;
	}

	bool allProcessesCompleted()
	{
		for (auto &process : listOfProcesses)
		{
			if (!process.isCompleted())
			{
				return false;
			}
		}
		return true;
	}

	void distributeQuantumTime(int totalQuantum)
	{
		vector<Process *> readyProcesses = getReadyProcesses(); // get the number of processes that are ready
		int processCount = readyProcesses.size();

		if (processCount > 0)
		{
			int evenQuantumTime = totalQuantum / processNumberCounter;
			for (auto *process : readyProcesses)
			{
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
	Scheduler(int inFixedTotalQuantumtime) : totalQuantumTime(inFixedTotalQuantumtime) {}

	void scheduleListOfProcesses(vector<User> &listOfUsers)
	{
		using namespace chrono_literals;
		while (!allUsersCompleted(listOfUsers))
		{
			distributeQuantum(listOfUsers);

			for (auto &user : listOfUsers)
			{
				vector<Process *> readyProcesses = user.getReadyProcesses();
				for (auto *process : readyProcesses)
				{
					if (!process->isCompleted())
					{
						thread processThread(&Process::run, process, user.username);
						processThread.join();

						GLOBAL_TIME += process->isCompleted() ? 0 : totalQuantumTime / readyProcesses.size();
					}
				}
			}
			this_thread::sleep_for(1s); // wait 1 second
		}
	}

private:
	void distributeQuantum(vector<User> &listOfUsers)
	{
		vector<User *> activeUsers;
		for (auto &user : listOfUsers)
		{
			if (!user.allProcessesCompleted())
			{
				activeUsers.push_back(&user);
			}
		}

		if (activeUsers.size() == 0)
		{
			return;
		}

		int perUserQuantum = totalQuantumTime / activeUsers.size();
		for (auto *user : activeUsers)
		{
			user->distributeQuantumTime(perUserQuantum);
		}
	}

	bool allUsersCompleted(vector<User> &inListOfUsers)
	{
		for (auto &user : inListOfUsers)
		{
			if (!user.allProcessesCompleted())
			{
				return false;
			}
		}
		return true;
	}

	int totalQuantumTime; // Total quantum time given for each User
};

int main()
{
	ifstream input_file("input.txt");
	output_file.open("output.txt");

	if (!input_file || !output_file)
	{
		cerr << "Error opening the file" << endl;
		return 1;
	}

	int totalQuantumTime;
	input_file >> totalQuantumTime;

	vector<User> users;
	string username;
	int processNumberCounter;

	while (input_file >> username >> processNumberCounter)
	{
		users.emplace_back(username);
		for (int i = 0; i < processNumberCounter; i++)
		{
			int readyTime, serviceTime;
			input_file >> readyTime >> serviceTime;
			users.back().addProcess(readyTime, serviceTime);
		}
	}
	input_file.close();

	Scheduler scheduler(totalQuantumTime);
	scheduler.scheduleListOfProcesses(users);

	output_file.close();
	cout << "Output file has been created" << endl;

	return 0;
}