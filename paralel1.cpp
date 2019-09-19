#include<fstream> //read and write from a file
#include<iostream> 
#include <string> // substr and append 
#include<mpi.h>
#define windowSize 3 
#define tempSize 9 
using namespace std;
int findMedian(int[]); // function prototype
int main(int argc,char* argv[])
{
	//declaring some variables
	int size,myRank,rows,cols,median,matrixPtrIJ,partRows,partCols,tempArray[tempSize],partArraySize,arraySize;
	
	int *anaMatrixPtr;
	int *kismiAnaMatrixPtr;
	int *sonucMatrixPtr;
	int *kismiSonucMatrixPtr;
	int *tempSonucMatrixPtr;
	
    double startTime,endTime,elapsed;
    
    MPI_Status status;
    
    MPI_Init(&argc,&argv); // initialize the mpi encironment
    MPI_Comm_size(MPI_COMM_WORLD,&size); //learn the size of the comm world
    MPI_Comm_rank(MPI_COMM_WORLD,&myRank); // let every process learn its rank 
      
   	if(myRank==0)
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
		infile>>rows>>cols;
		arraySize = rows*cols;
		if((arraySize%size) !=0)
		{
			cout << "The main array can not be divided equally to the nubmer of processes given aborting....\n";
			MPI_Abort(MPI_COMM_WORLD,99); // abort from the comm world with error code 99 			
		}
				
		partRows=rows/size;
		partCols=cols;
		partArraySize = partRows*partCols; // find the divided array size  
		// allocate the main dynamic arrays
		anaMatrixPtr = new int[arraySize];
		sonucMatrixPtr = new int[arraySize];
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
		// send the values of the variables that need to be shared from master node to the other computing nodes		
		for(int i=1;i<size;i++)
		{//send the divided array size 
			MPI_Send(&partArraySize,1,MPI_INT,i,25,MPI_COMM_WORLD);
		}
		for(int i=1;i<size;i++)
		{//send the divided arrays rows number
			MPI_Send(&partRows,1,MPI_INT,i,25,MPI_COMM_WORLD);
		}		
		for(int i=1;i<size;i++)
		{//send the divided arrays cols number
			MPI_Send(&partCols,1,MPI_INT,i,25,MPI_COMM_WORLD);
		}
		for(int i=1;i<size;i++)
		{//send the divided arrays elements to each computing node in order to do the filtering 
			MPI_Send(&anaMatrixPtr[i*partArraySize],partArraySize,MPI_INT,i,25,MPI_COMM_WORLD);
		}
		startTime=MPI_Wtime();
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
							tempArray[a*windowSize+b]=anaMatrixPtr[k*partCols+l];
						}
					}// find the median and then throw the value into the resault matrix
			 		median = findMedian(tempArray);
			 		tempSonucMatrixPtr[matrixPtrIJ]=median;
			 				   
				}//if satir 103 
				else// the first and last row and col we dont have to do any filtering on them so we add the values 
				// without any change from first matrix to the second matrix 
					tempSonucMatrixPtr[matrixPtrIJ]=anaMatrixPtr[matrixPtrIJ];					 		
			}// the inside for satir 101

		}// the outside for satir 99
		for(int i=1;i<size;i++)
		{//recieve the filtering resaults from the other computing nodes 
			MPI_Recv(&tempSonucMatrixPtr[i*partArraySize],partArraySize,MPI_INT,i,25,MPI_COMM_WORLD,&status);
		}

		for(int i=partRows-1;i<rows-1;i=i+partRows)
		{
			
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
		{
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
			 				   
					}//if satir 103 
					else// the first and last row and col we dont have to do any filtering on them so we add the values 
				// without any change from first matrix to the second matrix 
						tempSonucMatrixPtr[matrixPtrIJ]=anaMatrixPtr[matrixPtrIJ];					 		
				}// the inside for satir 101

			}// the outside for satir 99			
		}
				
		
		
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
			{// control the number of digits 
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
				
		
		infile.close();// close input file 
		ofile.close();// close output file
		//deallocate the two dynamic arrays 
		delete [] anaMatrixPtr;
		delete [] sonucMatrixPtr;
		delete [] tempSonucMatrixPtr;			   		
		   	
	}// if line 29
	else // for the other computing nodes
	{
		MPI_Recv(&partArraySize,1,MPI_INT,0,25,MPI_COMM_WORLD,&status);
		kismiAnaMatrixPtr=new int[partArraySize];
		kismiSonucMatrixPtr=new int[partArraySize];
		MPI_Recv(&partRows,1,MPI_INT,0,25,MPI_COMM_WORLD,&status);
		MPI_Recv(&partCols,1,MPI_INT,0,25,MPI_COMM_WORLD,&status);
		MPI_Recv(kismiAnaMatrixPtr,partArraySize,MPI_INT,0,25,MPI_COMM_WORLD,&status);
		
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
		MPI_Send(kismiSonucMatrixPtr,partArraySize,MPI_INT,0,25,MPI_COMM_WORLD);
		
		
		delete [] kismiAnaMatrixPtr;
		delete [] kismiSonucMatrixPtr;
		
	}
	     	
	
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

			

