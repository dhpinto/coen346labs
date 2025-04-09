#include <iostream>
#include <thread>
#include <string>
#include <chrono>
#include <fstream>
#include <vector>
#include <queue>
#include <condition_variable>
#include <sstream>
//might have two active process running at the same time
//portion of the cpu may be given at the same time


//Global variables: it exist during the whole lifetime of the application
int TIME = 0;//Global clock
std::string DELIMITER = ";";
std::mutex timeMutex;
std::mutex logMutex;
bool running= true;
//Global variables when reading commands.txt
std::vector<std::string> commandList;
std::mutex commandMutex;
int commandIndex=0;


//Logging the output function with the first part of the expected output
void logEvent(const std::string& event){
    std::lock_guard<std::mutex>lock(logMutex);
    std::ofstream  out("output.txt", std::ios::app);
    out<<"Clock: "<<TIME*1000<<", "<<event<<std::endl;
    out.close();
}
//Clock function, where the clock runs on its own thread
void clockThread(){
    while (running){
        std::this_thread::sleep_for(std::chrono::microseconds(1000));
        std::lock_guard<std::mutex> lock(timeMutex);
        TIME++;
    }
}

class largeDiskSpace
{
public:
	std::string retrievePage(std::string inId)
	{
		std::string iFileName = "vm.txt";
		std::ifstream iFile(iFileName);
		std::string oFileName = "temp.txt";
		std::ofstream oFile(oFileName);
		std::string line;
		std::string returnedString;

		if (iFile) {
			while (std::getline(iFile, line)) {
				std::string id = extractIdFromLine(line);
				if (inId == id)
				{
					returnedString = line;
				}
				else {
					oFile << line;
				}

			}
		}
		iFile.close();
		oFile.close();

		remove(iFileName.c_str());//the string into char start
		rename(oFileName.c_str(), iFileName.c_str());//we must use c_str conversion because these functions are old
		return returnedString;
	}
private:
	std::string extractIdFromLine(std::string incomingLine)
	{
		std::string token = incomingLine.substr(0, incomingLine.find(DELIMITER));
		return token;
	}
};

class Page
{
public:
	//default unassigned values
	std::string id = "";
	int value = -1;
	int lastAccessTime = -1;
};

class virtualMemoryManager
{

public:
	virtualMemoryManager(int inMaxMainMemorySpace) :
		maxMainMemorySpace(inMaxMainMemorySpace)
	{
		mainMemory = new Page[inMaxMainMemorySpace];//it feeds the max size to the array
	}

	~virtualMemoryManager()
	{
		delete[] mainMemory;
	}

	void store(std::string inVariableId, unsigned int inValue)
	{
        std::lock_guard<std::mutex> lock(memoryMutex);//Locking the critical section

		if (currentMainMemorySize < maxMainMemorySpace)
		{
			for (int i = 0; i < maxMainMemorySpace; i++)
			{
				if (mainMemory[i].id.empty())
				{
					mainMemory[i].id = inVariableId;//variable id
					mainMemory[i].value = inValue;//value
					mainMemory[i].lastAccessTime = TIME;//last access TIME
					currentMainMemorySize++;
					return;
				}
			}

		}
		else
		{
			writeToFile(inVariableId, inValue);
		}
	}

	void release(std::string inVariableId)
	{
        std::lock_guard<std::mutex> lock(memoryMutex);//Locking the critical section

		for (int i = 0; i < maxMainMemorySpace; i++)
		{
			if (mainMemory[i].id == inVariableId)
			{
				mainMemory[i].id = "";//variable id
				mainMemory[i].value = -1;//value
				mainMemory[i].lastAccessTime = TIME;//last access TIME
				return;
			}
		}
	}

	int lookup(std::string inVariableId)
	{
        std::lock_guard<std::mutex> lock(memoryMutex);//Locking the critical section

		for (int i = 0; i < maxMainMemorySpace; i++)
		{
			if (mainMemory[i].id == inVariableId)
			{
				mainMemory[i].lastAccessTime = TIME;//always want to update time	
				return mainMemory[i].value;
			}
		}
		//loop through all but did not find the id
		std::string retrievedLine = diskSpace.retrievePage(inVariableId);
		if (!retrievedLine.empty())
		{
			return swapFromDiskToMemory(retrievedLine);
		}

		return -1;


	}
	void printInfo()
	{
		for (int i = 0; i < maxMainMemorySpace; i++)
		{
			std::cout << "Page " << i + 1 << ": " << "ID: " << mainMemory[i].id << " "
				<< "Value: " << mainMemory[i].value << " "
				<< "Last Accessed Time: " << mainMemory[i].lastAccessTime << std::endl;
		}
	}
private:
    std::mutex memoryMutex;//To protect virtualMemoryManager function because store, lookup and release modify/access the same resources

	int swapFromDiskToMemory(std::string inLine)
	{
		std::vector<std::string> splittedString = split(inLine, ';');
		if (currentMainMemorySize < maxMainMemorySpace)
		{
			for (int i = 0; i < maxMainMemorySpace; i++)
			{
				if (mainMemory[i].id.empty())
				{
					mainMemory[i].id = splittedString[0];//variable id
					mainMemory[i].value = stoi(splittedString[1]);//value
					mainMemory[i].lastAccessTime = TIME;//last access TIME
					currentMainMemorySize++;
					return mainMemory[i].value;
				}
			}

		}
		else
		{
			int min = mainMemory[0].lastAccessTime;
			int index = 0;
			for (int i = 0; i < maxMainMemorySpace; i++)
			{
				if (mainMemory[i].lastAccessTime < min)
				{
					min = mainMemory[i].lastAccessTime;
					index = i;
				}
			}
            //Logging before replacing the page
            logEvent("Memory Manager, SWAP: Variable "+ splittedString[0]+" with Variable "+ mainMemory[index].id);

			writeToFile(mainMemory[index].id, mainMemory[index].value);

			mainMemory[index].id = splittedString[0];
			mainMemory[index].value = stoi(splittedString[1]);//string convert to int because value is an int
			mainMemory[index].lastAccessTime = TIME;//last access TIME

			return mainMemory[index].value;
		}
        return -1;//In case of failure, it will fail safely
	}
	void writeToFile(std::string inVariableId, int inValue)
	{
		std::ofstream out;

		// std::ios::app is the open mode "append" meaning
		// new data will be written to the end of the file.
		out.open("vm.txt", std::ios::app);

		std::string str = inVariableId + ";" + std::to_string(inValue);
		out << str<<std::endl;
	}

	std::vector<std::string> split(const std::string& s, char delim) {
		std::vector<std::string> result;
		std::stringstream ss(s);
		std::string item;

		while (std::getline(ss, item, delim)) {
			result.push_back(item);
		}

		return result;
	}

	//mapping is used it has a key value association, there exist function to store, erase and lookup
	Page* mainMemory;
	largeDiskSpace diskSpace;
	int maxMainMemorySpace;
	int currentMainMemorySize = 0;


};

struct Process
{
int id, startTime, duration;
};

//Function to read the processes from processes.txt
int readProcessFromFile(const std::string& filename, std::vector<Process>& processes){
    std::ifstream file(filename);

    if(!file){
        std::cout<<"Couldn't open processes.txt\n";
        return -1;
    }
    int numCores, numProcesses;
    file>>numCores>>numProcesses;
    for (int i = 0; i < numProcesses; ++i) {
        int start, duration;
        file>>start>>duration;
        processes.push_back({i+1, start,duration});
    }
    return numCores;
}
//Function to get the next command
std::string getNextCommand(){
    std::lock_guard<std::mutex> lock(commandMutex);
    std::string command=commandList[commandIndex];

    if(commandList.empty())
    {
        return "";//For error handling
    }

    commandIndex=(commandIndex+1)%commandList.size();
    return command;
}

std::queue<Process> readyQueue;
std::mutex queueMutex;
std::condition_variable cv;

int maxCores=0;
int runningProcesses=0;

//Function to simulate processes using threads
void simulatingProcess(Process process, virtualMemoryManager* vmm){
    //Waiting until the clock reaches the start time
    while(TIME<process.startTime)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        readyQueue.push(process);
        cv.notify_all();
    }

    //Waiting until the scheduler starts the process
    std::unique_lock<std::mutex> lock(queueMutex);
    cv.wait(lock,[&](){
        return runningProcesses<maxCores && readyQueue.front().id==process.id;
    });

    readyQueue.pop();
    runningProcesses++;
    lock.unlock();

    logEvent("Process "+std::to_string(process.id)+ ": Started");

    int startTime=TIME;

    while(TIME-startTime<process.duration){
        std::string command=getNextCommand();
        std::stringstream ss(command);
        std::string action,id;
        int value;

        ss>>action>>id;

        if(action=="Store"){
            ss>>value;
            vmm->store(id,value);
            logEvent("Process "+ std::to_string(process.id)+ ", Store: Variable "+id+", Value: "+std::to_string(value));
        }

        else if (action=="Release"){
            vmm->release(id);
            logEvent("Process "+ std::to_string(process.id)+", Release: Variable "+id);
        }

        else if(action=="Lookup"){
            int result=vmm->lookup(id);
            logEvent("Process "+std::to_string(process.id)+", Lookup: Variable "+id+", Value: "+std::to_string(result));
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(rand()%1000+1));
    }
    logEvent("Process "+ std::to_string(process.id)+": Finished");

    //Freeing the core
    std::lock_guard<std::mutex> lck(queueMutex);
    runningProcesses--;
    cv.notify_all();//To wake up the scheduler or to wait for the processes
}

//Function to read the commands from commands.txt
void readCommandsFromFile(const std::string& filename){
    std::ifstream file(filename);
    if(!file){
        std::cout<<"Couldn't open commands.txt\n";
        return;
    }
    std::string line;
    while(std::getline(file,line)){
        if(!line.empty())
            commandList.push_back(line);//Storing the commands in the vector
    }
}

//Function to read the memconfig.txt
int readMemorySizeFromFile(const std::string& filename){
    std::ifstream file(filename);
    if (!file){
        std::cout<<"Couldn't open memconfig.txt\n";
        return -1;
    }

    int size;
    file>>size;
    return size;
}

int main()
{
    std::ofstream ("output.txt").close();//Clears the logs
    std::ofstream("vm.txt").close();//Clears everything in the file at the start

    srand(time(0));//Random time selected

    std::thread clock(clockThread);

    std::vector<Process> processes;
    maxCores=readProcessFromFile("processes.txt",processes);
    readCommandsFromFile("commands.txt");

    int memorySize= readMemorySizeFromFile("memconfig.txt");
    virtualMemoryManager vmm(memorySize);//Passing the memory size to virtualMemoryManager

    std::vector<std::thread>processThreads;
    for(auto& process:processes){
        processThreads.emplace_back(simulatingProcess,process,&vmm);
    }

    for(auto& thread:processThreads){
        if(thread.joinable())
            thread.join();
    }

    running= false;
    clock.join();
	return 0;
}