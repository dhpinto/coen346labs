#include <iostream>
#include <thread>
#include <string>
#include <chrono>
#include <fstream>
#include <vector>
#include <sstream>
//might have two active process running at the same time
//portion of the cpu may be given at the same time


//global variables: it exist during the whole lifetime of the application
int TIME = 0;
std::string DELIMITER = ";";
class largeDiskSpace
{
public:
	std::string retrievePage(std::string inId)
	{
		std::string iFileName = "vm.txt";
		std::ifstream iFile(iFileName);
		std::string oFileName = "temp.txt";
		std::ofstream oFile(oFileName);
		std::string line, id;
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
private:


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

			writeToFile(mainMemory[index].id, mainMemory[index].value);

			mainMemory[index].id = splittedString[0];
			mainMemory[index].value = stoi(splittedString[1]);//string convert to int because value is an int
			mainMemory[index].lastAccessTime = TIME;//last access TIME



			return mainMemory[index].value;
		}

	}
	void writeToFile(std::string inVariableId, int inValue)
	{
		std::ofstream out;

		// std::ios::app is the open mode "append" meaning
		// new data will be written to the end of the file.
		out.open("vm.txt", std::ios::app);

		std::string str = inVariableId + ";" + std::to_string(inValue);
		out << str;
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

class Process
{
public:
private:
};
class Scheduler
{
	//fifo
public:
private:
};
class Clock
{
public:
private:
};
class Logger
{
public:
private:
};
int main()
{
	virtualMemoryManager vmm(2);
	TIME = 1;
	std::cout << "************************ Time " << TIME << std::endl;
	vmm.store("1", 10);
	vmm.printInfo();
	TIME = 2;
	std::cout << "************************ Time " << TIME << std::endl;
	vmm.store("4", 5);
	vmm.printInfo();
	TIME = 3;
	std::cout << "************************ Time " << TIME << std::endl;
	vmm.store("3", 100);
	vmm.printInfo();
	TIME = 4;
	std::cout << "************************ Time " << TIME << std::endl;
	std::cout << vmm.lookup("1") << std::endl;
	vmm.printInfo();
	TIME = 5;
	std::cout << "************************ Time " << TIME << std::endl;
	std::cout << vmm.lookup("3") << std::endl;
	vmm.printInfo();
	return 0;
}