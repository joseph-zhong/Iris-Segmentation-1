#include "ImageObj.h"
#include <math.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#pragma warning (disable:4996)

using namespace std;

//======================================================
// Default constructor for image class
//======================================================
Image::Image()
{
	numRows = 0;
	numColumns = 0;
	rowOffset = 0;
	columnOffset = 0;
	data = NULL;
}

Image::Image(int row, int col, unsigned char* oneDArray)
{
	numRows = row;
	numColumns = col;
	rowOffset = 0;
	columnOffset = 0;


	data = new unsigned char *[numRows];
	data[0] = new unsigned char[numRows*numColumns];

	for(int i=1; i < numRows; i++){
		data[i] = data[i-1] + numColumns;
	}
	int count = 0;
	for(int i = 0; i < numRows; i++){
		for(int j = 0; j < numColumns; j++){
			data[i][j] = oneDArray[count];
			count++;
		}
	}
}

//======================================================
// Image copy constructor
//======================================================
Image::Image(Image* inputImage)
{
	numRows = inputImage->getNumRows();
	numColumns = inputImage->getNumColumns();
	unsigned char **inputData = inputImage->getData();

	//allocating a contigous block of memory
	data = new unsigned char *[numRows];
	data[0] = new unsigned char[numRows*numColumns];

	//in order to be able to subscript using indices
	for(int k1 = 1; k1 < numRows; k1++)
		data[k1] = data[k1-1]+ numColumns;

	for(int k1 = 0; k1 < numRows; k1++)
	{
		for(int k2 = 0; k2 < numColumns; k2++)
			data[k1][k2] = inputData[k1][k2];
	}
}
//======================================================
// Overloaded constructor for image class
//======================================================
Image::Image(int row, int col)
{
	numRows = row;
	numColumns = col;

	//allocating a contigous block of memory
	data = new unsigned char *[numRows];
	data[0] = new unsigned char[numRows*numColumns];

	//in order to be able to subscript using indices
	for(int k1 = 1; k1 < numRows; k1++)
		data[k1] = data[k1-1]+ numColumns;

	//giving the data some initial values
	setAllData(0);
}
//======================================================
// Image Destructor
//======================================================
Image::~Image()
{
	//this will crash if the image has been declared but not initialized to NULL or another value
	if (data != NULL)
	{
		delete [] data[0];
		delete [] data;
	}
}
//======================================================
// returns element located at currentRow, currentColumn
//======================================================
unsigned char Image::getPixelElement(int currentRow, int currentColumn)
{
	if(currentRow > numRows)
		return -1;
	else if(currentColumn > numColumns)
		return -1;
	return data[currentRow][currentColumn];
}
//======================================================
// returns the number of rows in image
//======================================================
int Image::getNumRows()
{
	return numRows;
}
//=======================================================
// calculates the mean of image
//=======================================================
float Image::getMean()
{
	float imageMean = 0.0;
	for(int k1 = 0; k1 < numRows; k1++)
		for(int k2 = 0; k2 < numColumns; k2++)
			imageMean += data[k1][k2];
	imageMean /= (numRows*numColumns);

	return imageMean;
}
//=======================================================
// calculates the std dev of image
//=======================================================
float Image::getStandardDev(float imageMean)
{
	float imageStandardDev = 0.0;
	float sum = 0;
	imageMean = imageMean ? imageMean : getMean();
	imageStandardDev = 0.0;
	for(int k1 = 0; k1 < numRows; k1++)
	{
		for(int k2 =0; k2 < numColumns; k2++)
		{
			sum += (((float)data[k1][k2]-imageMean)*(data[k1][k2]-imageMean));
		}
	}
	imageStandardDev = sqrt(sum/(float)(numRows*numColumns-1));

	return imageStandardDev;
}
//======================================================
// returns the number of columns in image
//======================================================
int Image::getNumColumns()
{
	return numColumns;
}
//===============================
// Function for loading an image
//===============================
int Image::readBMP(string fileName, string pathName)
{
	unsigned char *holdRow;/* <holdRows each row to be read> */
	FILE* inputFile;		/* <the input file> */
	unsigned int* InfoPointer;		/* <pointer to holdRow header information>*/
	int sizeOfBitmap;		/* <Represents the size of the bitmap> */
	int extraBytes;			/* <Extra bytes in the image> */
	int fileSize;
	int imageOffset;
	string imageLocation = pathName.append(fileName);

	// open the image
	inputFile = fopen(imageLocation.c_str(), "rb");
	if(inputFile == NULL)
	{
		cout << "Error opening file." << endl;
		return -1;
	}
	header imageHeader;

	// load the image header
	size_t result = fread((void*) &imageHeader, sizeof(header), 1, inputFile);

	// get information from the image header
	InfoPointer = (unsigned int*) &imageHeader.data5;
	numRows = (int) *InfoPointer;
	InfoPointer = (unsigned int*) &imageHeader.data4;
	numColumns = (int)* InfoPointer;
	InfoPointer = (unsigned int*) &imageHeader.data2;
	imageOffset = (int)* InfoPointer;
	InfoPointer = (unsigned int*) &imageHeader.data;
	fileSize = (int)* InfoPointer;

	// now create the image matrix
	//allocating a contigous block in memory
	if(data != NULL){
		delete data[0];
		delete data;
	}
	data = new unsigned char *[numRows];
	data[0] = new unsigned char[numRows*numColumns];


	//in order to be able to subscript using indices
	for(int k1 = 1; k1 < numRows; k1++)
		data[k1] = data[k1-1]+ numColumns;

	// now we read the image data
	sizeOfBitmap = fileSize - imageOffset;
	extraBytes = sizeOfBitmap/numRows-numColumns;
	holdRow = new unsigned char[imageOffset];

	int remainingHeaderBytes = (size_t)imageOffset - sizeof(header);
	result = fread(holdRow, 1, remainingHeaderBytes, inputFile);
	delete [] holdRow;

	holdRow = new unsigned char[numColumns + extraBytes];
	for(int k1 = 0; k1 < numRows; k1++)
	{
		fread(holdRow, 1, numColumns+extraBytes, inputFile);
		for(int k2 = 0; k2 < numColumns; k2++)
		{
			data[numRows - k1 - 1][k2] = holdRow[k2];
		}
	}
	delete [] holdRow;
	fclose (inputFile);

	return 0;
}
//=====================================
// Function for saving an image to disk
//=====================================
int Image::writeBMP(string fileName, string pathName)
{
	// declare variables
	FILE* outputFile;
	int result = 0;
	int bitmapOffset = 1078;
	string imageLocation = pathName.append(fileName);
	unsigned char *colorValues;
	unsigned char *pixelValues;
	unsigned char maxValue;
	unsigned char value;
	double tempValue;
	int remainingHeaderBytes = 0;
	unsigned int columneighborhoodSize = (unsigned int)(ceil((float)numColumns/4.0)*4.0);

	// open the output file
	outputFile = fopen(imageLocation.c_str(), "wb");
	if(outputFile == NULL)
	{
		cout << "Error opening output file." << endl;
		return -1;
	}

	header outputHeader;
	// load the information for the header
	unsigned int* InfoPointer;
	outputHeader.FileType = 0x4D42;		/* < Always set to 4D42h > */
	outputHeader.Reserved1 = 0;			/* < Always 0> */
	outputHeader.Reserved2 = 0;			/* < Always 0> */
	InfoPointer = (unsigned int *)&outputHeader.data;
	*InfoPointer = (unsigned int)bitmapOffset+columneighborhoodSize*(unsigned int)numRows;	/* <Size of file in bytes > */
	InfoPointer = (unsigned int *)&outputHeader.data2;
	*InfoPointer = (unsigned int)bitmapOffset;	/* < 1078 specifies the offset from the beginning of the file to bitmap data > */
	InfoPointer = (unsigned int *)&outputHeader.data3;
	*InfoPointer = 40;							/* < 40 specifies the size of header structure in bytes> */
	InfoPointer = (unsigned int *)&outputHeader.data4;
	*InfoPointer = (unsigned int)numColumns;	/* < Image width in pixels > */
	InfoPointer = (unsigned int *)&outputHeader.data5;
	*InfoPointer = numRows;					/* < Image height in pixels > */
	outputHeader.Planes = 1;			/* < Number of planes of the target device > */
	outputHeader.BitsPerPixel = 8;		/* < Number of bits per pixel > */
	InfoPointer = (unsigned int *)&outputHeader.data6;
	*InfoPointer = 0;							/* < specifies the type of compression > */
	InfoPointer = (unsigned int *)&outputHeader.data7;
	*InfoPointer = columneighborhoodSize*(unsigned int)numRows;	/* < size of the image data in bytes > */
	InfoPointer = (unsigned int *)&outputHeader.data8;
	*InfoPointer = 0;
	InfoPointer = (unsigned int *)&outputHeader.data9;
	*InfoPointer = 0;
	InfoPointer = (unsigned int *)&outputHeader.data10;
	*InfoPointer = 0;
	InfoPointer = (unsigned int *)&outputHeader.data11;
	*InfoPointer = 0;

	// write the header
	result = fwrite((void*) &outputHeader, sizeof(header), 1, outputFile);

	// set display colors for each pixel value
	colorValues = new unsigned char[bitmapOffset];
	remainingHeaderBytes = (size_t)bitmapOffset - sizeof(header);
	maxValue = getMax();			/* < Get the maximum value in matrix > */
	for(int k1 = 0; k1 < 1024; k1 += 4)
	{
		tempValue = floor(floor((double)k1/4.0)*255.0/(double)maxValue+.5);
		if(tempValue > 255.0)
			tempValue = 255.0;
		value = (unsigned char)tempValue;
		colorValues[k1] = value;
		colorValues[k1 + 1] = value;
		colorValues[k1 + 2] = value;
		colorValues[k1 + 3] = 0;
	}

	// now save color values to disk
	result = fwrite(colorValues, 1, remainingHeaderBytes, outputFile);
	delete [] colorValues;

	// now write image data to disk in row reversed order
	pixelValues = new unsigned char[columneighborhoodSize];
	for(int k1 = numRows - 1; k1 >= 0; k1--)
	{
		for(int k2 = 0; k2 < numColumns; k2++)
		{
			pixelValues[k2] = (unsigned char)data[k1][k2];
		}
		result = fwrite((void*) pixelValues, 1, columneighborhoodSize, outputFile);
	};
	delete [] pixelValues;

	// close the file
	fclose(outputFile);
	return 0;
}
//=====================================
// Returns the maximum value in matrix
//=====================================
int Image::getMax()
{
	int tempMax = 0;
	tempMax = (unsigned char)data[0][0];
	// for each pixel
	for(int k1 = 0; k1 < numRows; k1++)
	{
		for(unsigned char* Ptr=&data[k1][0]; Ptr<&data[k1][numColumns];Ptr++)
		{
			// compare current element to tempMax
			if((unsigned char)*Ptr > tempMax)
				tempMax = *Ptr;
		}
	}
	return tempMax;
}
//======================================
// gets the location of max element
//======================================
int Image::getMaxLocation(int &rowCoordinate, int &columnCoordinate)
{
	int tempMax = 0;
	tempMax = (unsigned char)data[0][0];
	unsigned char* Ptr;
	// for each pixel
	for(int k1 = 0; k1 < numRows; k1++)
	{
		int k2 = 0;
		for(Ptr=&data[k1][0]; Ptr <=&data[k1][numColumns];Ptr++)
		{
			// if current element is greater than tempMax
			if((unsigned char)*Ptr > tempMax)
			{
				// set tempMax to current element
				tempMax = *Ptr;
				// store current element's location
				rowCoordinate = k1;
				columnCoordinate = k2;
			}
			k2++;
		}
	}
	return tempMax;
}
//======================================
// gets the maximum absolute difference
// between elements and a specified value
// in a row from a start column to an end
// column
//======================================
int Image::getMaxAbsoluteDifference(int startRow, int startColumn, int endRow, int endColumn, int value)
{
	int k1;

	int maxDifference = 0;
	unsigned char* Ptr;
	// for each row in the block
	for(k1 = startRow; k1 <= endRow; k1++)
	{
		int k2 = 0;
		// for each column in the block
		for(Ptr=&data[k1][startColumn]; Ptr<=&data[k1][endColumn];Ptr++)
		{
			// if k2 - value is greater than maxDifference
			if( (k2 - value) > maxDifference)
				maxDifference = k2 - value;
			// otherwise
			else if( (value - k2) > maxDifference)
				maxDifference = value - k2;
			k2++;
		}
	}

	return maxDifference;
}
//=====================================
// Function for saving a PGM to disk
//=====================================
int Image::writePGM(string fileName, string pathName)
{
	// declare variables
	FILE* outputFile;
	string imageLocation;
	unsigned char maxNum = 0;

	// open the output file
	imageLocation = pathName.append(fileName);
	outputFile = fopen(imageLocation.c_str(), "w");

	// load the information for the header
	if( outputFile != NULL)
	{
		maxNum = this->getMax();
		fprintf(outputFile, "P5\n");
		fprintf(outputFile, "%d %d\n",numColumns,numRows);
		fprintf(outputFile, "%d\n", maxNum);
		for(int k1=0; k1 < numRows; k1++)
		{
			for(int k2 = 0; k2 < numColumns; k2++)
			{
				char val = (char)data[k1][k2];
				fputc(val, outputFile);
			}
		}
		fclose(outputFile);
	}
	else
	{
		printf("Unable to open %s while attempting to perform PGM Write", imageLocation.c_str());
		return -1;
	}
	return 0;
}
//=====================================
// Sums all ones in image
//=====================================
int Image::sumAllOnes()
{
	int sum = 0;
	unsigned char* Ptr;
	for(int k1 = 0; k1 < numRows; k1++)
	{
		//for(int k2 = 0; k2 < numColumns; k2++)
		for(Ptr=&data[k1][0]; Ptr<&data[k1][numColumns]; Ptr++)
		{
			if(*Ptr)
				sum++;
		}
	}
	return sum;
}

//=====================================
// Function for reading a PGM from disk
//=====================================
int Image::readPGM(string fileName, string pathName)
{
	string imageLocation;
	FILE *inputFile;
	char tempBuffer[81];
	int currentRows = 0;
	int currentColumns = 0;
	int imageMax = 0;

	imageLocation = pathName.append(fileName);
	inputFile = fopen(imageLocation.c_str(), "r");
	if(inputFile != NULL)
	{
 		fgets(tempBuffer, 81, inputFile);
		if(strstr(tempBuffer, "P5") == NULL)
		{
			printf("Unable to open %s for PGM read, input not a PGM file\n", imageLocation.c_str());
			numRows = -1;
			numColumns = -1;
			data = NULL;
			return -1;
		}
		fscanf(inputFile, "%d %d\n", &currentColumns, &currentRows);
		fscanf(inputFile, "%d\n", &imageMax);

		setSize(currentRows, currentColumns);

		for(int k1 = 0; k1 < numRows; k1++)
		{
			for(int k2 = 0; k2 < numColumns; k2++)
			{
				data[k1][k2] = fgetc(inputFile);
			}
		}
		fclose(inputFile);
	}
	else
	{
		printf("Unable to open %s for PGM read, failure to initialize\n", imageLocation.c_str());
		return -1;
	}

	return 0;
}
//======================================
// Sets the size of image matrix
//======================================
int Image::setSize(int rowSize, int colSize)
{

	//freeing the memory from the previous data
	if(numRows != 0 || numColumns != 0)
	{
		delete [] data[0];
		delete [] data;
	}

	numRows = rowSize;
	numColumns = colSize;

	//allocating a contigous block of memory
	data = new unsigned char *[numRows];
	data[0] = new unsigned char[numRows*numColumns];

	//in order to subscript using indices
	for(int k1 = 1; k1 < numRows; k1++)
		data[k1] = data[k1-1]+ numColumns;

	return 0;
}
//======================================
// sets a specific element in the matrix
//======================================
int Image::setPixelElement(int rowCoordinate, int columnCoordinate, unsigned char setValue)
{
	// size checking
	if(rowCoordinate > numRows || columnCoordinate > numColumns)
		return -1;
	if(setValue > 255)
		return -2;

	data[rowCoordinate][columnCoordinate] = setValue;
	return 0;
}
//=============================================
// returns a pointer to data matrix
//=============================================
unsigned char **Image::getData()
{
	return data;
}

//=============================================
// sets the image offset
//=============================================
int Image::setOffset(int rowOffset, int columnOffset)
{
	this->rowOffset = rowOffset;
	this->columnOffset = columnOffset;
	return 0;
}
//=============================================
// gets the image offset
//=============================================
int Image::getOriginRowOffset()
{
	return rowOffset;
}
//=============================================
// gets the image offset
//=============================================
int Image::getOriginColumnOffset()
{
	return columnOffset;
}
//======================================================
// Sums an image
//======================================================
int Image::getSum()
{
	int imageSum = 0;
	unsigned char* Ptr;
	// for each pixel
	for(int k1 = 0; k1 < numRows; k1++)
	{
		for(Ptr=&data[k1][0]; Ptr<&data[k1][numColumns]; Ptr++)
		{
			imageSum += *Ptr;
		}
	}
	return imageSum;
}
//=====================================================
// Sums a specific row in the image
//=====================================================
int Image::getRowSum(int startRow)
{
	int rowSum = 0;


	// for each pixel in the row
	for(unsigned char* Ptr=&data[startRow][0]; Ptr<&data[startRow][numColumns];Ptr++)
	{
		rowSum += *Ptr;
	}

	return rowSum;
}
//=============================================
// Returns an image that is a portion of the
// original image
//=============================================
int Image::SubImage(int startRow, int startColumn, int rowSize, int colSize, Image*& outputImage)
{
	// get the output data
	unsigned char **newImageData = outputImage->getData();
	// check the bounds
	if((startRow >= 0 && startRow <= numRows && startColumn >= 0 && startColumn <= numColumns) &&
		(startRow+rowSize-1 > 0 && startRow+rowSize-1 < numRows && startColumn+colSize-1 > 0 && startColumn+colSize-1 < numColumns))
	{
		// for each pixel in original image
		for(int k1 = startRow, k2 = 0; k1 < startRow+rowSize; k1++, k2++)
		{
			for(int k3 = startColumn, k4 = 0; k3 < startColumn+colSize; k3++,k4++)
			{
				// current pixel in output data equals current pixel in original data
				newImageData[k2][k4] = data[k1][k3];
			}
		}
		outputImage->setOffset(startRow, startColumn);
	}
	return 0;
}
//======================================
// labels black and white objects in
// an image
//======================================
int Image::BWLabelObjects(int **&returnedAreas, int minimumArea)
{
	int maxPoints = (numRows*numColumns)/2 + 1;
	int numObjectsAboveMinArea = 0;
	int currentRunStart, currentRunStop, prevRunStart, prevRunStop;
	int currentRunLabel;
	int newLabel = 1;
	int currentRun = 0;
	int firstRun;

	//coordinate
	int x = 0;
	int y = 0;

	/**************************************
			Start equivalence table
		The following arrays make up the
	equivalence table used by the function to
				store run data
	**************************************/
	// array to hold the first run on the row
	int *rowStart = new int[numRows];
	// array to hold the last run on the row
	int *rowEnd = new int[numRows];
	// array to hold the row each run contains
	int *row = new int[maxPoints];
	// holds the start column of each run
	int *columnStart = new int[maxPoints];
	// holds the last column of each run
	int *columnEnd = new int[maxPoints];
	// the permanent label of each run
	int *permLabel = new int[maxPoints];
	// the temporary label of each run
	int *label = new int[maxPoints];
	// holds the area of each run
	int *area;
	/**************************************
			End equivalence table
	**************************************/
	//counters
	int currentRow, currentColumn;

	// initialize rowStart and rowEnd elements to zero
	for (int i = 0; i < numRows; i++)
	{
		rowStart[i] = 0;
		rowEnd[i] = 0;
	}

	// initialize everything else to zero
	for (int i = 0; i < maxPoints; i++)
	{
		row[i] = 0;
		columnStart[i] = 0;
		columnEnd[i] = 0;
		permLabel[i] = 0;
		label[i] = 0;
	}

	// for each row in image
	for (currentRow = 0; currentRow < numRows; currentRow++)
	{
		// initialize first elements of rowStart and rowEnd to -1
		rowStart[currentRow] = -1;
		rowEnd[currentRow] = -1;
		currentColumn = 0;

		// for each column in the image
		while (currentColumn < numColumns)
		{
			// if current pixel is a one, then run exists
			if (data[currentRow][currentColumn])
			{
				// if run's start row has not been set yet
				if (rowStart[currentRow] == -1)
				{
					// set the current row to show that row # currentRun
					// begins at row currentRow
					rowStart[currentRow] = currentRun;
				}

				// set current run to contain row currentRow
				row[currentRun] = currentRow;
				// set current run to start at column currentColumn
				columnStart[currentRun] = currentColumn;

				// while the rows run continues advance to the next column
				while ((data[currentRow][currentColumn]) && (currentColumn < numColumns))
					currentColumn++;

				// when the run has ended:
				// set the endColumn of currentRun to currentColumn
				columnEnd[currentRun] = currentColumn;
				// assign the run a temporary label of currentRun
				label[currentRun] = currentRun;
				// since it is the first run, it has a permanent label of zero
				permLabel[currentRun] = 0;
				// temporarily set the endRow of currentRun to currentRow
				rowEnd[currentRow] = currentRun;

				// advance one run
				currentRun++;
				// advance to next column to check for another row
				currentColumn++;
			}
			// else if the current pixel is not a one
			else
				// advance to next column
				currentColumn++;
		}
	}

	// if a run has been found, we must simplify the run(s)
	if (currentRun > 0)
	{
		// the first run is the run located at row[0]
		firstRun = row[0];
		label[newLabel] = newLabel;
		// permLabel zero should should be newLabel + 1
		permLabel[0] = newLabel++;
		// start at the second row from the top
		currentRow = 1;

		// while the run on currentRow is still part of the first run
		while ((row[currentRow] == firstRun) && (currentRow < currentRun))
		{
			// assign label
			label[newLabel] = newLabel;
			// set the permLabel of currentRow
			permLabel[currentRow] = newLabel++;
			// advance a row
			currentRow++;
		}

		//top down pass
		for (int k1 = 1; k1 < numRows; k1++) //for each remaining row in image
		{
			//k1 is a row index
			//P is index of RLL run in current row
			//Q is index of RLL run in row above current row
			currentRunStart = rowStart[k1];
			prevRunStart = rowEnd[k1];
			currentRunStop = rowStart[k1-1];
			prevRunStop = rowEnd[k1-1];

			if ((currentRunStart >= 0) && (currentRunStop >= 0)) //if there are RLL runs in both rows
			{
				// top down scan #1
				while ((currentRunStart <= prevRunStart) && (currentRunStop <= prevRunStop))
				{
					// P ends before Q begins so increment P to next RLL run
					if (columnEnd[currentRunStart] < columnStart[currentRunStop])
					{
						currentRunStart++;
					}
					else //else #1
					{
						//currentRunStart ends after currentRunStop begins
						if (columnEnd[currentRunStop] < columnStart[currentRunStart])
						{
							currentRunStop++;
						}
						else //else #2, there is overlap between run P and run Q
						{
							currentRunLabel = permLabel[currentRunStart];

							// if P has no label, give it Q's
							if (currentRunLabel == 0)
							{
								permLabel[currentRunStart] = permLabel[currentRunStop];
							}
							else //else #3
							{
								//overlapping runs with different labels
								if (currentRunLabel != permLabel[currentRunStop])
								{
									label[permLabel[currentRunStop]] = currentRunLabel;
								}
							} //end else #3

							//Q ends before P ends so increment Q to next RLL run
							if (columnEnd[currentRunStart] > columnEnd[currentRunStop])
							{
								currentRunStop++;
							}
							else //else #4
							{
								//P ends before Q ends so increment P to next RLL run
								if (columnEnd[currentRunStop] > columnEnd[currentRunStart])
								{
									currentRunStart++;
								}
								//P and Q end at the same time to increment both
								else
								{
									currentRunStop++;
									currentRunStart++;
								}
							} // end else #4

						} //end else #2
					} //end else #1
				} // end while (top down scan #1)

				//top down scan #2
				//change labels in row above current row to be what they should be using translation table
				currentRunStart = rowStart[k1];
				while (currentRunStart <= prevRunStart)
				{
					currentRunLabel = permLabel[currentRunStart]; //get P's label

					//if P has no label, give it one
					if (currentRunLabel == 0)
					{
						label[newLabel] = newLabel;
						permLabel[currentRunStart] = newLabel++;
					}
					else //else #5
					{
						// if P's label needs to be updated
						if (currentRunLabel != label[currentRunLabel])
						{
							permLabel[currentRunStart] = label[currentRunLabel];
						}
					} //end else #5

					currentRunStart++;
				} //end while (top scan #2)
			} // end if (currentRunStart >= 0) && (currentRunStop >= 0)

			//either row k1 or k1-1 contains no RLL runs
			else //else #6
			{
				currentRunStart = rowStart[k1];

				//if row k1 contains RLL runs
				if (currentRunStart >= 0)
				{
					while (currentRunStart <= prevRunStart)
					{
						label[newLabel] = newLabel;
						permLabel[currentRunStart] = newLabel++;
						currentRunStart++;
					}
				}
			} // end else #6
		} //end for (top up pass)

		// Bottom up pass
		for (int k1 = numRows-2; k1 >= 0; k1--)
		{
			//k1 is row index
			currentRunStart = rowStart[k1];
			prevRunStart = rowEnd[k1];
			currentRunStop = rowStart[k1+1];
			prevRunStop = rowEnd[k1+1];

			if ((currentRunStart >= 0) && (currentRunStop >= 0))
			{
				//bottom up scan #1
				while ((currentRunStart <= prevRunStart) && (currentRunStop <= prevRunStop))
				{
					if (columnEnd[currentRunStart] < columnStart[currentRunStop])
					{
						currentRunStart++;
					}
					else //else #1
					{
						if (columnEnd[currentRunStop] < columnStart[currentRunStart])
						{
							currentRunStop++;
						}
						else //else #2 there is overlap between run P and run Q
						{
							if (permLabel[currentRunStart] != permLabel[currentRunStop])
							{
								label[permLabel[currentRunStart]] = permLabel[currentRunStop];
								permLabel[currentRunStart] = permLabel[currentRunStop];
							}

							if (columnEnd[currentRunStart] > columnEnd[currentRunStop])
							{
								currentRunStop++;
							}
							else //else #3
							{
								if (columnEnd[currentRunStop] > columnEnd[currentRunStart])
								{
									currentRunStart++;
								}
								else
								{
									currentRunStop++;
									currentRunStart++;
								}
							} //end else #3
						} //end else #2
					} //end else #1
				} //end while (bottom up scan #1)

				//bottom up scan #2
				currentRunStart = rowStart[k1];
				while (currentRunStart <= prevRunStart)
				{
					permLabel[currentRunStart] = label[permLabel[currentRunStart]];
					currentRunStart++;
				} //end while (bottom up scan #2)
			} // end if ((currentRunStart > 0) && (currentRunStop >= 0))
		} // end for (bottom up pass)
	} // end if (currentRun < 0)


	int maxValue = permLabel[0];

	//adding -1 to all values in columnEnd
	//also finding the maximum value inside of permLabel at the same time
	for (int k1 = 0; k1 < maxPoints; k1++)
	{
		columnEnd[k1] += -1;

		if (permLabel[k1] > maxValue)
			maxValue = permLabel[k1];
	}

	//allocating area
	maxValue++;
	area = new int[maxValue];

	//zeroing all the values in area
	for (int k1 = 0; k1 < maxValue; k1++)
		area[k1] = 0;

	//assigning the values in area
	for (int k1 = 0; k1 < currentRun; k1++)
	{
		area[permLabel[k1]] += (columnEnd[k1] - columnStart[k1] + 1);
	}

	//counting the number of objects above the area limit
	for (int k1 = 0; k1 < maxValue; k1++)
	{
		if (area[k1] > minimumArea)
			numObjectsAboveMinArea++;
	}

	if (numObjectsAboveMinArea > 255)
		numObjectsAboveMinArea = 255;

	//allocating returned areas to the appropriate size
	//5 locations for area and bounds
	returnedAreas = new int *[numObjectsAboveMinArea+1];
	for (int k1 = 0; k1 < numObjectsAboveMinArea+1; k1++)
	{
		returnedAreas[k1] = new int[5];

		//zeroing out the array
		for (int k2 = 0; k2 < 5; k2++)
			returnedAreas[k1][k2] = 0;
	}

	//clearing this image
	setAllData(0);

	//needed for finding max location
	int maxLoc;
	int tempMax;

	for (int k1 = 1; k1 <= numObjectsAboveMinArea; k1++)
	{
		//resetting their values
		maxLoc = 0;
		tempMax = area[0];

		//finding the label of the largest object
		for (int k4 = 1; k4 < maxValue; k4++)
		{
			if (area[k4] > tempMax)
			{
				maxLoc = k4;
				tempMax = area[k4];
			}
		}

		// store data in returnedAreas
		currentRunLabel = maxLoc;
		returnedAreas[k1][0] = area[currentRunLabel];
		returnedAreas[k1][1] = numRows;
		returnedAreas[k1][2] = numColumns;
		returnedAreas[k1][3] = -1;
		returnedAreas[k1][4] = -1;
		area[currentRunLabel] = 0; //clearing this label so the next largest object is found next iteration

		// for each run up to currentRun
		for (int k2 = 0; k2 < currentRun; k2++)
		{
			// if permLabel[k2] == currentRunLabel
			if (permLabel[k2] == currentRunLabel)
			{
				// for each column in the current run
				for (int k3 = columnStart[k2]; k3 <= columnEnd[k2]; k3++)
				{
					// color the objects according to run
					data[row[k2]][k3] = k1;
				}

				if (row[k2] < returnedAreas[k1][1])
					returnedAreas[k1][1] = row[k2];
				if (columnStart[k2] < returnedAreas[k1][2])
					returnedAreas[k1][2] = columnStart[k2];
				if (row[k2] > returnedAreas[k1][3])
					returnedAreas[k1][3] = row[k2];
				if (columnEnd[k2] > returnedAreas[k1][4])
					returnedAreas[k1][4] = columnEnd[k2];
			}
		}

	}

	// free memory
	delete [] area;
	delete [] rowStart;
	delete [] rowEnd;
	delete [] row;
	delete [] columnStart;
	delete [] columnEnd;
	delete [] label;
	delete [] permLabel;
	return 0;
}
//========================================================
// sets all data in a matrix to a passed in value
//========================================================
int Image::setAllData(int value)
{
	unsigned char* Ptr;
	for(int k1 = 0; k1 < numRows; k1++)
	{
		//for(int k2 = 0; k2 < numColumns; k2++)
		for(Ptr =&data[k1][0]; Ptr<&data[k1][numColumns]; Ptr++)
		{
			*Ptr = value;
		}
	}
	return 0;
}
