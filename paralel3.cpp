#include<fstream> //read and write from a file
#include<iostream> 
#include <string> // substr and append
#include <omp.h> //to use openmp functions and blocks 
#include<sys/time.h> // timeval and gettimeofday
#define windowSize 3 
#define tempSize 9 
using namespace std;
int findMedian(int[]); // function prototype
int main(int argc, char* argv[])
{
	if (argc != 2) // if the number of arguments doesnt equal to 2 
	{
		cout << "you must provide one argument to this program.\n";
		cout << "usage ./a.out inputFileName.\n";
		return 1;
	}
	ifstream infile(argv[1]);
	if (!infile) // if input file doesnt exist 
	{
		cout << "ERROR please check the spelling of the file.\n";
		return 1;
	}
	struct timeval currentTime;
	double startTime,endTime,elapsed;
	int rows, cols, median,nThreads;
	int tempArray[tempSize];
	//int nThreads, threadId;
	infile >> rows >> cols;
	// allocate the two dynamic arrays 
	int **anaMatrixPtr = new int*[rows];
	int **sonucMatrixPtr = new int*[rows];
	for (int i = 0; i<rows; i++)
	{
		anaMatrixPtr[i] = new int[cols];
	}
	for (int i = 0; i<rows; i++)
	{
		sonucMatrixPtr[i] = new int[cols];
	}
	// read all the values from the input folder to the dynamic array		
	for (int i = 0; i<rows; i++)
		for (int j = 0; j<cols; j++)
			infile >> anaMatrixPtr[i][j];
	// do some changes on the input file to get the output file name 
	string outputFileName(argv[1]);
	string str = "_filtered.txt";
	outputFileName = outputFileName.substr(0, outputFileName.length() - 4);
	outputFileName = outputFileName.append(str);
	remove(outputFileName.c_str()); // if file exists delete it (we use c style strings because type string doesmt exist in c++ and
	// because of that we have to convert it back to c style 	
	ofstream ofile(outputFileName.c_str(), ios::app);
	gettimeofday(&currentTime,NULL);
	startTime=currentTime.tv_sec+(currentTime.tv_usec/1000000.0);
	#pragma omp parallel private(median,tempArray)
	{
		nThreads=omp_get_num_threads();
		#pragma omp for collapse(2)
			for (int i = 0; i<rows; i++)
			{
				for (int j = 0; j<cols; j++)
				{// for all the rows and cols that we have to do the filtering operation on
					if ((i != 0) && (i != (rows - 1)) && (j != 0) && (j != (cols - 1)))
					{
						for (int a = 0, k = i - 1; a<3 && k <= i + 1; a++, k++)
						{
							for (int b = 0, l = j - 1; b<3 && l <= j + 1; b++, l++)
							{ //represent two dimension array in one dimentional array
								tempArray[a*windowSize + b] = anaMatrixPtr[k][l];
							}
						}// find the median and then throw the value into the resault matrix
						median = findMedian(tempArray);
						sonucMatrixPtr[i][j] = median;
						//ofile << median << " ";
					}//if satir 41
					else// the first and last row and col we dont have to do any filtering on them so we add the values
						// without any change from first matrix to the second matrix
						sonucMatrixPtr[i][j] = anaMatrixPtr[i][j];
				}//for satir 39
				//ofile << "\n";
			}//for satir 37
	}//pragma code
	gettimeofday(&currentTime,NULL); 
	endTime=currentTime.tv_sec+(currentTime.tv_usec/1000000.0);
	elapsed = endTime-startTime; // calculate elapsed time in seconds 
	printf("\nThe filtering proccess with %d thread took : %.7f ms\n",nThreads,elapsed*1000);	
	 // here i chose to do the writing to the output file in another loop so it doesnt affect the filtering time that we are aiming to analyse 
	 // while writing out the resaulys we have to be careful for the whitespaces in order to keep the array in shape 
		for (int i = 0; i<rows; i++)
		{
			for (int j = 0; j<cols; j++)
			{// control the number of digits 
				if (sonucMatrixPtr[i][j] / 100 >= 1)
					ofile << sonucMatrixPtr[i][j] << " ";
				else if (sonucMatrixPtr[i][j] / 10 >= 1)
					ofile << sonucMatrixPtr[i][j] << "  ";
				else
					ofile << sonucMatrixPtr[i][j] << "   ";
			}
			ofile << endl;
		}

	
	infile.close();// close input file 
	ofile.close();// close output file
	//deallocate the two dynamic arrays 
	for (int i = 0; i<rows; i++)
		delete[] anaMatrixPtr[i];
	delete[] anaMatrixPtr;
	for (int i = 0; i<rows; i++)
		delete[] sonucMatrixPtr[i];
	delete[] sonucMatrixPtr;
	return 0;
}
// a function to sort an array and then get the median value
int findMedian(int arr[])
{ // insertion sort algorithm 
	for (int i = 0; i<tempSize; i++)
	{
		for (int j = 0; j<tempSize - 1; j++)
		{
			if (arr[j]>arr[j + 1])
			{
				int temp = arr[j + 1];
				arr[j + 1] = arr[j];
				arr[j] = temp;
			}
		}
	}
	return 	arr[tempSize / 2];
}
