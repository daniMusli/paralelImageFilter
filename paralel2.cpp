#include<fstream> //read and write from a file
#include<iostream> // input output stream 
#include <string> // substr and append 
#include<mpi.h> // mpi variables and methods 
#define windowSize 3 
#define tempSize 9 
using namespace std;
int findMedian(int[]); // function prototype
int main(int argc,char* argv[])
{
	//declaring some variables
	int size,myRank,rows,cols,median,matrixPtrIJ,partRows,partCols,tempArray[tempSize],partArraySize,arraySize;
	
	// declaring the dinamik arrays 
	int *anaMatrixPtr;
	int *kismiAnaMatrixPtr;
	int *kismiSonucMatrixPtr;
	int *tempSonucMatrixPtr;
	
	// declaring variables to count the filtering time 
    double startTime,endTime,elapsed;
   
    MPI_Init(&argc,&argv); // initialize the mpi encironment
    MPI_Comm_size(MPI_COMM_WORLD,&size); //learn the size of the comm world
    MPI_Comm_rank(MPI_COMM_WORLD,&myRank); // let every process learn its rank 
   	if(myRank==0)// the master node 
   	{
		if(argc!=2) // if the number of arguments doesnt equal to 2 
		{
			cout<<"you must provide one argument to this program.\n";
			cout << "usage ./a.out inputFileName.\n";
			MPI_Abort(MPI_COMM_WORLD,99); // abort from the comm world with error code 99 
		}   		
   		ifstream infile(argv[1]); // open the file to read from
    	if(!infile) // if input file doesnt exist 
        {
            cout<<"ERROR please check the spelling of the file.\n";
            MPI_Abort(MPI_COMM_WORLD,99); // abort from the comm world with error code 99 
        }
		infile>>rows>>cols; // read the number of lines and coloumns from input folder 
		arraySize = rows*cols;
		if((arraySize%size) !=0) // if the number of elements of the array cant be equally devided to the number of proccesses then abort 
		{
			cout << "The main array can not be divided equally to the nubmer of processes given aborting....\n";
			MPI_Abort(MPI_COMM_WORLD,99); // abort from the comm world with error code 99 			
		}
		// in this algorithm we are only dividing the array by its rows in order to keep the shape of 
		// coloumns and rows of the main array if we dont do so the values of the neighboring elements will change and 
		// the filtering resaults wont be the same.		
		partRows=rows/size; 
		partCols=cols;
		partArraySize = partRows*partCols; // find the divided array size  
		// allocate the main dynamic arrays
		anaMatrixPtr = new int[arraySize];
		tempSonucMatrixPtr = new int[arraySize];
	    // read all the values from the input folder to the dynamic array		
		for(int i=0;i<rows;i++)
		{
			for(int j=0;j<cols;j++)
			{
				matrixPtrIJ=i*cols+j;
				infile>>anaMatrixPtr[matrixPtrIJ];				
			}
		} 
	infile.close();// close input file 	
	// we start the filtering proccess here the master node will take care of the first divided array 
	//and store the resaults in the resaults array 
	startTime=MPI_Wtime();
	   	
	}// if myRank == 0 
	// we broadcast the variables that we are going to use in other nodes 
	// so this code should be written in the global range and be executed by all the nodes 
	// the master node will do the broadcasting and the scattering 
	MPI_Bcast(&partArraySize,1,MPI_INT,0,MPI_COMM_WORLD);
	MPI_Bcast(&partRows,1,MPI_INT,0,MPI_COMM_WORLD);	
	MPI_Bcast(&partCols,1,MPI_INT,0,MPI_COMM_WORLD);
	// allocate space for the part arrays 
	kismiAnaMatrixPtr = new int[partArraySize];
	kismiSonucMatrixPtr = new int[partArraySize];
	
	MPI_Scatter(anaMatrixPtr,partArraySize,MPI_INT,kismiAnaMatrixPtr,partArraySize,MPI_INT,0,MPI_COMM_WORLD);	
	for(int i=0;i<partRows;i++)
	{
		for(int j=0;j<partCols;j++)
		{// for all the rows and cols that we have to do the filtering operation on
			matrixPtrIJ=i*partCols+j;
			if((i!=0) && (i!=(partRows-1)) && (j!=0) && (j!=(partCols-1)) )
			{
				for(int a=0,k=i-1; a<3 && k<=i+1 ;a++,k++)
				{
					for(int b=0,l=j-1; b<3 && l<=j+1 ;b++,l++)
					{ //represent two dimension array in one dimentional array
							tempArray[a*windowSize+b]=kismiAnaMatrixPtr[k*partCols+l];
					}
				}// find the median and then throw the value into the resault matrix
			 	median = findMedian(tempArray);
			 	kismiSonucMatrixPtr[matrixPtrIJ]=median;
			 				   
			}//if satir 147 
			else// the first and last row and col we dont have to do any filtering on them so we add the values 
				// without any change from first matrix to the second matrix 
				kismiSonucMatrixPtr[matrixPtrIJ]=kismiAnaMatrixPtr[matrixPtrIJ];					 		
		}// the inside for satir 144
			
	}// the outside for satir 142
	// here we are gathering the small resault matrixes into our final resault matris 	
	MPI_Gather(kismiSonucMatrixPtr,partArraySize,MPI_INT,tempSonucMatrixPtr,partArraySize,MPI_INT,0,MPI_COMM_WORLD);
	if(myRank==0)
	{
		// now the resault array that we have is still missing some rows that we couldnt preform the filtering operation on 
		// and these lines are the first and last line of each divided array because these lines elements may have neighbors 
		// in other arrays which they cant reach due to them being in other array on some other node 
		// which may cause a problem for us thats why we left out these line to the master node to preform the filtering on them 
		for(int i=partRows-1;i<rows-1;i=i+partRows)
		{ // the last row of each divided array 
			
			for(int j=1;j<cols-1;j++)
			{
				matrixPtrIJ=i*cols+j;
				for(int a=0,k=i-1; a<3 && k<=i+1 ;a++,k++)
				{
					for(int b=0,l=j-1; b<3 && l<=j+1 ;b++,l++)
					{ //represent two dimension array in one dimentional array
						tempArray[a*windowSize+b]=anaMatrixPtr[k*cols+l];
					}
				}// find the median and then throw the value into the resault matrix
			 	median = findMedian(tempArray);
			 	tempSonucMatrixPtr[matrixPtrIJ]=median;					
				}
				
		}
		for(int i=partRows;i<rows-1;i=i+partRows)
		{// the first row of each divided array 
			for(int j=1;j<cols-1;j++)
			{
				matrixPtrIJ=i*cols+j;
				for(int a=0,k=i-1; a<3 && k<=i+1 ;a++,k++)
				{
					for(int b=0,l=j-1; b<3 && l<=j+1 ;b++,l++)
					{ //represent two dimension array in one dimentional array
						tempArray[a*windowSize+b]=anaMatrixPtr[k*cols+l];
					}
				}// find the median and then throw the value into the resault matrix
			 	median = findMedian(tempArray);
			 	tempSonucMatrixPtr[matrixPtrIJ]=median;					
			}
				
		}
		// in our algorithm we are only dividing by numbers bu some arrays cant be fully divided only by rows division some lines 
		// in worst case 8 at maximum will be left after we divide the array into smaller ones these line will be operated on by the
		// master node as well 
		if(rows%size!=0)
		{
			for(int i=rows-(rows%size);i<rows;i++)
			{
				for(int j=0;j<cols;j++)
				{// for all the rows and cols that we have to do the filtering operation on
					matrixPtrIJ=i*partCols+j;
					if((i!=(rows-1)) && (j!=0) && (j!=(cols-1)) )
					{
						for(int a=0,k=i-1; a<3 && k<=i+1 ;a++,k++)
						{
							for(int b=0,l=j-1; b<3 && l<=j+1 ;b++,l++)
							{ //represent two dimension array in one dimentional array
							tempArray[a*windowSize+b]=anaMatrixPtr[k*cols+l];
							}
						}// find the median and then throw the value into the resault matrix
			 			median = findMedian(tempArray);
			 			tempSonucMatrixPtr[matrixPtrIJ]=median;
			 				   
					}//if  
					else// the first and last row and col we dont have to do any filtering on them so we add the values 
				// without any change from first matrix to the second matrix 
						tempSonucMatrixPtr[matrixPtrIJ]=anaMatrixPtr[matrixPtrIJ];					 		
				}// the inside for 

			}// the outside for 			
		}
		
		// now that we finished all the filtering operations on the resault array we are going to calculate the time and write out the resaults into the output folder 
		endTime=MPI_Wtime();
		// do some changes on the input file to get the output file name 
		string outputFileName(argv[1]);
		string str = "_filtered.txt";
		outputFileName=outputFileName.substr(0, outputFileName.length()-4);
		outputFileName=outputFileName.append(str);
		remove(outputFileName.c_str()); // if file exists delete it (we use c style strings because type string doesmt exist in c++ and
		// because of that we have to convert it back to c style 	
		ofstream ofile(outputFileName.c_str(), ios::app );
		elapsed = endTime-startTime; // calculate elapsed time in seconds
		printf("\nthe number of milliseconds taken by the program to do the filtering process on file %s  is: %.7f ms\n",outputFileName.c_str(),elapsed*1000);
		// while writing out the resaulys we have to be careful for the whitespaces in order to keep the array in shape 
		for(int i=0;i<rows;i++)
		{
			for(int j=0;j<cols;j++)
			{// control the number of digits in order to put equal number of spaces between the elements
				matrixPtrIJ=i*cols+j;
				if(tempSonucMatrixPtr[matrixPtrIJ]/100 >=1 )
					ofile << tempSonucMatrixPtr[matrixPtrIJ]<<" ";
				else if(tempSonucMatrixPtr[matrixPtrIJ]/10 >=1)
					ofile <<tempSonucMatrixPtr[matrixPtrIJ]<<"  ";
				else
					ofile <<tempSonucMatrixPtr[matrixPtrIJ]<<"   ";	
			}
			ofile <<endl;
		}		
				
		
		
		ofile.close();// close output file
		//deallocate the two dynamic arrays 
		delete [] anaMatrixPtr;
		delete [] tempSonucMatrixPtr;
	}	

	delete [] kismiAnaMatrixPtr;
	delete [] kismiSonucMatrixPtr;
	MPI_Finalize();
	return 0;
}
// a function to sort an array and then get the median value
int findMedian(int arr[])
{ // insertion sort algorithm 
	for(int i=0;i<tempSize;i++)
	{
		for(int j=0;j<tempSize-1;j++)
		{
			if(arr[j]>arr[j+1])
			{
				int temp = arr[j+1];
				arr[j+1]=arr[j];
				arr[j]=temp;	
			}	
		}	
	}
	return 	arr[tempSize/2];	
}

			

