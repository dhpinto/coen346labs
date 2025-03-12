#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <thread>
#include <string>
#include <mutex>

using namespace std;

mutex outputMtx;	  // mutex to synchronize the output
ofstream output_file; // output file stream

class Process //Represents each user's individual process
{
public:
    Process(int inReadyTime, int inServiceTime, int id) : readyTime(inReadyTime),// time when the process is ready to be executed
                                                          serviceTime(inServiceTime),// total time needed to complete the process
                                                          remainingTime(inServiceTime),// remaining time to complete the process
                                                          processNumber(id),// unique id for each process
                                                          started(false),// flag to check if the process has started
                                                          completed(false) {}// flag to check if the process has completed

    void execute(const string &username, int quantum, int &currentTime)//Function to execute a process at a given time quantum
    {
        //Logging the start fo the process done only once
        if (!started)
        {
            lock_guard<mutex> lock(outputMtx);//Ensuring only one thread can write at a time preventing race conditions and corrupted outputs and handling automatically locking/unlocking
            output_file << "Time " << currentTime << ", User " << username << ", Process " << processNumber << ", Started" << endl;
            started = true;//Signaling that the process has started
        }

        //Logging the processes being resumed
        {
            lock_guard<mutex> lock(outputMtx);//Ensuring only one thread can write at a time preventing race conditions and corrupted outputs and handling automatically locking/unlocking
            output_file << "Time " << currentTime << ", User " << username << ", Process " << processNumber << ", Resumed" << endl;
        }

        int actualExecutionTime = min(quantum, remainingTime);//Calculating the actual execution time

        this_thread::sleep_for(std::chrono::milliseconds(100));//Simulating the process execution by using sleep

        //Updating the time and remaining execution time
        currentTime += actualExecutionTime;
        remainingTime -= actualExecutionTime;

        //Logging the paused and finished processes' states
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

    bool isReady(int currentTime) const { return readyTime <= currentTime && !completed; }//Checking if the process is ready to run
    bool isCompleted() const { return completed; }//Checking if the process has been completed

private:
    int readyTime;
    int serviceTime;
    int remainingTime;
    int processNumber;
    bool completed;
    bool started;
};

class User//Representing a user with multiple processes
{
public:
    User(const string &inUsername) : username(inUsername) {}

    void addProcess(int readyTime, int serviceTime)//Adding a new process to list of process
    {
        processes.emplace_back(readyTime, serviceTime, processes.size());
    }

    vector<Process *> getReadyProcesses(int currentTime)//Getting a list of processes that are for execution
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

    bool hasReadyProcesses(int currentTime) const//Checking if a user has a process that is ready
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

    bool allProcessesCompleted() const//Checking if all the users' processes have been completed
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

    string getUsername() const { return username; }//Getting the username info for logging purposes

private:
    string username;
    vector<Process> processes;// vector of processes belonging to the user
};
class Scheduler
{
public:
    Scheduler(int quantum) : totalQuantum(quantum) {}//Manages and executes all the processes

    void run(vector<User> &users)//Running the scheduler to handle the processes
    {
        int currentTime = 1; // start at time 1

        //Running the scheduling cycle until all users' processes have been completed
        while (!allUsersCompleted(users))
        {

            vector<User *> activeUsers;
            //To identify the processes that are ready with their users
            for (auto &user : users)
            {
                if (user.hasReadyProcesses(currentTime))
                {
                    activeUsers.push_back(&user);
                }
            }

            //Waiting if there is no users have processes that are ready
            if (activeUsers.size() == 0)
            {
                currentTime++;
                this_thread::sleep_for(std::chrono::milliseconds(50)); // small sleep when idle
                continue;
            }

            int userQuantum = totalQuantum / activeUsers.size();// divide the total quantum time among the active users

            //Distributing the time quantum among the active users and their processes
            for (auto *user : activeUsers)
            {
                auto readyProcesses = user->getReadyProcesses(currentTime);
                if (readyProcesses.size() == 0)// if no processes are ready to execute, exit for loop
                {
                    continue;
                }

                int processQuantum = userQuantum / readyProcesses.size();// divide the user quantum time among the ready processes

                for (auto *process : readyProcesses)// execute each ready process
                {
                    process->execute(user->getUsername(), processQuantum, currentTime);
                }
            }
        }
    }

private:
    //Checking if all the users have completed their processes
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
    // open input and output files
    ifstream input_file("ProgrammingAssignment2/input.txt");
    output_file.open("ProgrammingAssignment2/output.txt");

    // error handling for file opening
    if (!input_file || !output_file)
    {
        cerr << "Error opening the files" << endl;
        return 1;
    }

    int quantum;
    input_file >> quantum;//Reading the total time quantum from the input file

    vector<User> users;
    string username;
    int processNumberCounter;// number of processes for each user

    //Reading the users and number of processes
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

    Scheduler scheduler(quantum);// create a scheduler with the given quantum time
    scheduler.run(users);// run the scheduler

    output_file.close();// close the output file
    cout << "View output.txt for results" << endl; // print message to show that execution is completed


    return 0;
}