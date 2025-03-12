#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <thread>
#include <string>
#include <mutex>

using namespace std;

mutex outputMtx;	  // mutex for the output
ofstream output_file; // output file stream

class Process
{
public:
	Process(int inReadyTime, int inServiceTime, int id) : readyTime(inReadyTime),
														  serviceTime(inServiceTime),
														  remainingTime(inServiceTime),
														  processNumber(id),
														  started(false),
														  completed(false) {}

	void execute(const string &username, int quantum, int &currentTime)
	{

		if (!started)
		{
			lock_guard<mutex> lock(outputMtx);
			output_file << "Time " << currentTime << ", User " << username << ", Process " << processNumber << ", Started" << endl;
			started = true;
		}

		{
			lock_guard<mutex> lock(outputMtx);
			output_file << "Time " << currentTime << ", User " << username << ", Process " << processNumber << ", Resumed" << endl;
		}

		int actualExecutionTime = min(quantum, remainingTime);

		this_thread::sleep_for(std::chrono::milliseconds(100));

		currentTime += actualExecutionTime;
		remainingTime -= actualExecutionTime;

		{
			lock_guard<mutex> lock(outputMtx);
			output_file << "Time " << currentTime << ", User " << username << ", Process " << processNumber << ", Paused" << endl;

			if (remainingTime <= 0)
			{
				completed = true;
				output_file << "Time " << currentTime << ", User " << username << ", Process " << processNumber << ", Finished" << endl;
			}
		}
	}

	bool isReady(int currentTime) const { return readyTime <= currentTime && !completed; }
	bool isCompleted() const { return completed; }
	int getRemainingTime() const { return remainingTime; }

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
	User(const string &inUsername) : username(inUsername) {}

	void addProcess(int readyTime, int serviceTime)
	{
		processes.emplace_back(readyTime, serviceTime, processes.size());
	}

	vector<Process *> getReadyProcesses(int currentTime)
	{
		vector<Process *> readyProcesses;
		for (auto &process : processes)
		{
			if (process.isReady(currentTime))
			{
				readyProcesses.push_back(&process);
			}
		}
		return readyProcesses;
	}

	bool hasReadyProcesses(int currentTime) const
	{
		for (const auto &process : processes)
		{
			if (process.isReady(currentTime) && !process.isCompleted())
			{
				return true;
			}
		}
		return false;
	}

	bool allProcessesCompleted() const
	{
		for (const auto &process : processes)
		{
			if (!process.isCompleted())
			{
				return false;
			}
		}
		return true;
	}

	string getUsername() const { return username; }

private:
	string username;
	vector<Process> processes;
};
class Scheduler
{
public:
	Scheduler(int quantum) : totalQuantum(quantum) {}

	void run(vector<User> &users)
	{
		int currentTime = 1; // start at time 1

		while (!allUsersCompleted(users))
		{

			vector<User *> activeUsers;
			for (auto &user : users)
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

			int userQuantum = totalQuantum / activeUsers.size();

			for (auto *user : activeUsers)
			{
				auto readyProcesses = user->getReadyProcesses(currentTime);
				if (readyProcesses.size() == 0)
				{
					continue;
				}

				int processQuantum = userQuantum / readyProcesses.size();

				for (auto *process : readyProcesses)
				{
					process->execute(user->getUsername(), processQuantum, currentTime);
				}
			}
		}
	}

private:
	bool allUsersCompleted(const vector<User> &users) const
	{
		for (const auto &user : users)
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
	ifstream input_file("input.txt");
	output_file.open("output.txt");

	if (!input_file || !output_file)
	{
		cerr << "Error opening the files" << endl;
		return 1;
	}

	int quantum;
	input_file >> quantum;

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

	Scheduler scheduler(quantum);
	scheduler.run(users);

	output_file.close();
	cout << "View output.txt for results" << endl;

	return 0;
}