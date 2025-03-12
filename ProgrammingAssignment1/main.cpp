#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <fstream>
#include <bitset>
#include <mutex> //Used for synchronization purposes to prevent multiple threads from accessing a shared resource at the same time

std::mutex mtx;	   // Used for cout
std::mutex id_mtx; // Used for the globalID

int globalID = 0;

void mergeSortInAThread(std::vector<int> &arr, int left, int right, int threadID);
void mergeSort(std::vector<int> &arr, int left, int right);

void merge(std::vector<int> &arr, int left, int mid, int right) // Merges the two sorted subarrays into one sorted array
{
	// Determining the size of each subarrays
	int arrLeft = mid - left + 1;
	int arrRight = right - mid;

	// Creating  temporary vectors that will store the elements from the original array
	std::vector<int> tempLeft(arrLeft);
	std::vector<int> tempRight(arrRight);

	// Here we will copy the elements of the array arr to the temporary vectors tempLeft[] and tempRight[] that will have the sorted halves of the array arr
	for (int i = 0; i < arrLeft; i++)
	{
		tempLeft[i] = arr[left + i];
	}
	for (int j = 0; j < arrRight; j++)
	{
		tempRight[j] = arr[mid + 1 + j];
	}

	int i = 0;	  // Index for tempLeft
	int j = 0;	  // Index for tempRight
	int k = left; // Index for arr
	// Merging the two temporary vectors back into the array keeping the sorted order
	while ((i < arrLeft) && (j < arrRight))
	{
		if (tempLeft[i] <= tempRight[j])
		{
			arr[k++] = tempLeft[i++];
		}
		else
		{
			arr[k++] = tempRight[j++];
		}
	}
	// For the elements remaining in the temporary vectors they are added at the end of the sorted array arr

	// Elements remaining from tempLeft[]
	while (i < arrLeft)
	{
		arr[k++] = tempLeft[i++];
	}
	// Elements remaining from tempRight[]
	while (j < arrRight)
	{
		arr[k++] = tempRight[j++];
	}
}
std::string printVector(std::vector<int> &arr, int left, int right) // Prints the handled subarrays during sorting
{
	std::string printResult;
	for (int i = left; i <= right; i++)
	{
		printResult += std::to_string(arr[i]) + " "; // Converting each number to a string and concatenates the numbers
	}
	return printResult;
}

void mergeSortInAThread(std::vector<int> &arr, int left, int right, int threadID) // This function is a wrapper for the mergeSort function managing multithreading
{
	// Ensuring resource acquisition is initialized
	{
		std::lock_guard<std::mutex> lock(mtx);													   // Used so cout from threads working simultaneously do not overlap
		std::cout << "Thread " << std::bitset<4>(threadID).to_string() << " started" << std::endl; // Converting threadID to a 4-bit binary string
	}
	mergeSort(arr, left, right); // Calling the function to sort the subarray from left to right
	// Making sure only one thread is printed at a time
	{
		std::lock_guard<std::mutex> lock(mtx);
		std::cout << "Thread " << std::bitset<4>(threadID).to_string() << " finished:  " << printVector(arr, left, right) << std::endl;
		// Make one big string so it can cout in one shot
	}
}

void mergeSort(std::vector<int> &arr, int left, int right) // The function recursively sorts the array arr with left being the starting index and right the ending index of the subarrays
{
	// Stopping the recursion for the case where there is one element meaning the subarray is already sorted or the invalid case where left is bigger than right
	if (left >= right)
		return;
	// Finding the correct middle index position by adding left to (right-left)/2
	int mid = left + (right - left) / 2;

	// Ensures that only one thread can access the globalID at a time
	{
		std::lock_guard<std::mutex> lock(id_mtx);
		globalID++;
	}
	std::thread lm(mergeSortInAThread, std::ref(arr), left, mid, globalID); // The left half (from left to mid) of the array arr is sorted in a thread
	{
		std::lock_guard<std::mutex> lock(id_mtx);
		globalID++;
	}
	std::thread rm(mergeSortInAThread, std::ref(arr), mid + 1, right, globalID); // The right half (from mid+1 to right) of the array arr is sorted in another thread

	//.join() makes sure that both sorting threads are done before merging
	lm.join();
	rm.join();

	merge(arr, left, mid, right); // Merges the sorting halves
}

// Prints all vector elements, original array and final sorted array
void printVector(std::vector<int> &arr)
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

	std::cout.rdbuf(out.rdbuf()); // Redirect std::cout to out.txt

	std::string inputMyText;				 // Create a text string, which is used to output the text file
	std::ifstream ReadMyInputFile("in.txt"); // Put cursor at the first line of the text

	while (std::getline(ReadMyInputFile, inputMyText)) // Get the first line of the cursor and put it in inputMyTest string
	{
		arr.push_back(stoi(inputMyText)); // Pushback inputMytext words into the array called arr
	}
	int n = arr.size();
	ReadMyInputFile.close();

	std::cout << "Unsorted array: ";
	printVector(arr);
	mergeSort(arr, 0, n - 1);
	std::cout << "Sorted array: ";
	printVector(arr);
	return 0;
}