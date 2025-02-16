#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <fstream>
#include <bitset>
int globalID = 0;
void mergeSortInAThread(std::vector<int>& arr, int left, int right, int threadID);
void mergeSort(std::vector<int>& arr, int left, int right);
//here we are creating two subarrays, one left and one right
void merge(std::vector<int>& arr, int left, int mid, int right)
{
	int arrLeft = mid - left + 1;
	int arrRight = right - mid;

	//Creating  temp vectors
	std::vector<int> tempLeft(arrLeft);
	std::vector<int> tempRight(arrRight);

	//Here we will transfer the data to the temporary vectors tempLeft[] and tempRight[]
	for (int i = 0; i < arrLeft; i++)
	{
		tempLeft[i] = arr[left + i];
	}
	for (int j = 0; j < arrRight; j++)
	{
		tempRight[j] = arr[mid + 1 + j];
	}
	//initializing to merge
	int i = 0;
	int j = 0;
	int k = left;
	//Merging the temporary back to the original arr[left right]
	while ((i < arrLeft) && (j < arrRight))
	{
		if (tempLeft[i] <= tempRight[j])
		{
			arr[k] = tempLeft[i];
			i++;
		}
		else 
		{
			arr[k] = tempRight[j];
			j++;
		}
		k++;
	}
	//For remaining elements

	//elements from tempLeft[]
	while (i < arrLeft)
	{
		arr[k] = tempLeft[i];
		i++;
		k++;
	}
	//elements from tempRight
	while (j < arrRight)
	{
		arr[k] = tempRight[j];
		j++;
		k++;
	}
}
std::string printVector(std::vector<int>& arr, int left, int right)//prepare the message to cout in one shot
{
	std::string printResult;
	for (int i = left; i <= right; i++)
	{
		printResult += std::to_string(arr[i]) + " ";
	}
	return printResult;
}
void mergeSortInAThread(std::vector<int>& arr, int left, int right, int threadID)//this function is an intermediate function called wrapping
{
	std::cout << "Thread " << std::bitset<4>(threadID).to_string() << " started" << std::endl;
	mergeSort(arr, left, right); //the parameter function call gets forward
	std::cout << "Thread " << std::bitset<4>(threadID).to_string() << " finished:  " << printVector(arr, left, right) << std::endl;
	//make one big string so it can cout in one shot
}

void mergeSort(std::vector<int>& arr, int left, int right)
{
	if (left >= right)
		return;

	int mid = left + (right - left) / 2;
	std::thread lm(mergeSortInAThread, std::ref(arr), left, mid, globalID++);//recursion 1, pass by reference
	std::thread rm(mergeSortInAThread, std::ref(arr), mid + 1, right, globalID++);//recursion 2
	lm.join();//waiting for other thread to finish
	rm.join();//waiting for other thread to finish 
	//mergeSort(arr, left, mid);
	//mergeSort(arr, mid + 1, right);
	merge(arr, left, mid, right);
}
//cout all vector elements
void printVector(std::vector<int>& arr)
{
	for (int i = 0; i < arr.size(); i++)
	{
		std::cout << arr[i] << " ";
	}
	std::cout << std::endl;
}

int main()
{
	std::vector<int> arr;

	std::ofstream out("out.txt");
	//   std::streambuf* coutbuf = std::cout.rdbuf(); //save old buf
	std::cout.rdbuf(out.rdbuf()); //redirect std::cout to out.txt!

	std::string inputMyText;//Create a text string, which is used to output the text file
	std::ifstream ReadMyInputFile("in.txt");//put cursor at the first line of the text

	while (std::getline(ReadMyInputFile, inputMyText))//get the first line of the cursor and put it in inputMyTest string
	{
		std::cout << inputMyText << std::endl;
		arr.push_back(stoi(inputMyText));//pushback inputMytext words into the array called arr
	}
	int n = arr.size();
	ReadMyInputFile.close();

	printVector(arr);
	mergeSort(arr, 0, n - 1);
	printVector(arr);
	return 0;
}