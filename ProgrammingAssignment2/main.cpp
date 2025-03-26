#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <thread>
#include <string>
#include <mutex>

using namespace std;

mutex outputMtx, timeMtx;	  // mutex for the output
ofstream output_file; // output file stream

class Process
{
public:
	Process(int inReadyTime, int inServiceTime, int id) : readyTime(inReadyTime),		// time when the process is ready to be executed
		serviceTime(inServiceTime),	// total time needed to complete the process
		remainingTime(inServiceTime), // remaining time to complete the process
		processNumber(id),			// unique id for each process
		started(false),				// flag to check if the process has started
		completed(false)
	{
	} // flag to check if the process has completed

	void execute(const string& username, int quantum, int& currentTime)
	{

		if (!started) // if it's the first time the process is executed, log as started
		{
			lock_guard<mutex> lock(outputMtx);
			output_file << "Time " << currentTime << ", User " << username << ", Process " << processNumber << ", Started" << endl;
			started = true;
		}

		// else, if process already started, process is resumed
		else
		{
			lock_guard<mutex> lock(outputMtx);
			output_file << "Time " << currentTime << ", User " << username << ", Process " << processNumber << ", Resumed" << endl;
		}

		int actualExecutionTime = min(quantum, remainingTime);

		this_thread::sleep_for(std::chrono::milliseconds(100)); // simulate execution

		{
			lock_guard<mutex> lock(timeMtx);
			currentTime += actualExecutionTime;
			remainingTime -= actualExecutionTime; // update current time and remaining time
		}

		{
			// after execution pauses:
			lock_guard<mutex> lock(outputMtx);
			output_file << "Time " << currentTime << ", User " << username << ", Process " << processNumber << ", Paused" << endl;
			// if process is completed, log as finished
			if (remainingTime <= 0)
			{
				completed = true;
				output_file << "Time " << currentTime << ", User " << username << ", Process " << processNumber << ", Finished" << endl;
			}
		}
	}

	bool isReady(int currentTime) const { return readyTime <= currentTime && !completed; } // check if the process is ready to execute
	bool isCompleted() const { return completed; }										   // check if the process is completed
	int getRemainingTime() const { return remainingTime; }								   // get the remaining time to complete the process

private:
	int readyTime;
	int serviceTime;
	int remainingTime;
	int processNumber;
	bool completed;
	bool started;
};

class User
{
public:
	User(const string& inUsername) : username(inUsername) {}

	void addProcess(int readyTime, int serviceTime)
	{
		processes.emplace_back(readyTime, serviceTime, processes.size()); // adds process to the back of the vector
	}

	vector<Process*> getReadyProcesses(int currentTime) // get list of processes that are ready to execute
	{
		vector<Process*> readyProcesses;
		for (auto& process : processes)
		{
			if (process.isReady(currentTime))
			{
				readyProcesses.push_back(&process);
			}
		}
		return readyProcesses;
	}

	bool hasReadyProcesses(int currentTime) const // check if there are any processes ready to execute
	{
		for (const auto& process : processes)
		{
			if (process.isReady(currentTime) && !process.isCompleted())
			{
				return true;
			}
		}
		return false;
	}

	bool allProcessesCompleted() const // check if all the user's processes are completed
	{
		for (const auto& process : processes)
		{
			if (!process.isCompleted())
			{
				return false;
			}
		}
		return true;
	}

	string getUsername() const { return username; } // get the username as string

private:
	string username;
	vector<Process> processes; // vector of processes belonging to the user
};
class Scheduler
{
public:
	Scheduler(int quantum) : totalQuantum(quantum) {}

	void run(vector<User>& users) // run the scheduler until all processes are completed
	{
		int currentTime = 1; // start at time 1

		while (!allUsersCompleted(users))
		{

			vector<User*> activeUsers;
			for (auto& user : users)
			{
				if (user.hasReadyProcesses(currentTime))
				{
					activeUsers.push_back(&user);
				}
			}

			if (activeUsers.size() == 0)
			{
				currentTime++;
				this_thread::sleep_for(std::chrono::milliseconds(50)); // small sleep when idle
				continue;
			}

			int userQuantum = totalQuantum / activeUsers.size(); // divide the total quantum time among the active users

			for (auto* user : activeUsers) // execute processes for each active user
			{
				auto readyProcesses = user->getReadyProcesses(currentTime);
				if (readyProcesses.size() == 0) // if no processes are ready to execute, exit for loop
				{
					continue;
				}

				int processQuantum = userQuantum / readyProcesses.size(); // divide the user quantum time among the ready processes

				vector<thread> threads;

				for (auto* process : readyProcesses)
				{
					threads.emplace_back([=, &currentTime]() {
						process->execute(user->getUsername(), processQuantum, currentTime);
						});
				}

				// Wait for all threads to finish
				for (auto& t : threads)
				{
					if (t.joinable())
						t.join();
				}
			}
		}
	}

private:
	bool allUsersCompleted(const vector<User>& users) const // check if all users have completed all their processes
	{
		for (const auto& user : users)
		{
			if (!user.allProcessesCompleted())
			{
				return false;
			}
		}
		return true;
	}

	int totalQuantum; // Total quantum time given for each User
};

int main()
{
	ifstream input_file("input.txt"); // open input and output files
	output_file.open("output.txt");

	if (!input_file || !output_file) // error handling for file opening
	{
		cerr << "Error opening the files" << endl;
		return 1;
	}

	int quantum;
	input_file >> quantum; // read quantum time from input file

	vector<User> users;
	string username;
	int processNumberCounter; // number of processes for each user

	// read input file and create users and processes
	while (input_file >> username >> processNumberCounter)
	{
		users.emplace_back(username); // create a new user
		for (int i = 0; i < processNumberCounter; i++)
		{
			int readyTime, serviceTime;
			input_file >> readyTime >> serviceTime;
			users.back().addProcess(readyTime, serviceTime); // add process to the user
		}
	}
	input_file.close();

	Scheduler scheduler(quantum); // create a scheduler with the given quantum time
	scheduler.run(users);		  // run the scheduler

	output_file.close();						   // close the output file
	cout << "View output.txt for results" << endl; // print message to show that execution is completed

	return 0;
}