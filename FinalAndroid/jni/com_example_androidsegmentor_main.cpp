#include "com_example_androidsegmentor_main.h"
#include <stdio.h>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <vector>
#include <cmath>
#include <ctime>
#include <utility>
#include <sstream>

#include <android/log.h>

#include "MedianFilter.h"
#include "ImageObj.h"
#include "CLAHE.h"

using namespace std;

pair<int, int> HoughTransform(double directional[480][640], Image *mask,
		Image *Hough, int gridSize, int minR, int maxR)
{

	int pixelVal;
	int** center = new int*[480];
	center[0] = new int[480 * 640];
	for (int i = 1; i < 480; i++)
	{
		center[i] = center[i - 1] + 640;
	}
	int max = 0;
	int tempPlus;

	int tempRow;
	int tempCol;

	int a;
	int b;
	double sinDirection;
	double cosDirection;
	pair<int, int> coor = make_pair(0, 0);

	for (int i = 0; i < 480; i++)
	{
		for (int j = 0; j < 640; j++)
		{
			center[i][j] = 0;
		}
	}

	for (int i = gridSize; i < mask->getNumRows() - gridSize; i++)
	{
		tempRow = i;
		for (int j = gridSize; j < mask->getNumColumns() - gridSize; j++)
		{
			tempCol = j;

			pixelVal = mask->getPixelElement(i, j);
			if (pixelVal > 0)
			{
				sinDirection = sin(directional[i][j]);
				cosDirection = cos(directional[i][j]);
				for (int k = minR; k < maxR; k++)
				{
					a = k * sinDirection;
					b = k * cosDirection;
					tempRow -= b;
					tempCol += a;

					if (tempCol < 640 - gridSize && tempRow < 480 - gridSize
							&& tempRow > gridSize && tempCol > gridSize)
					{
						center[tempRow][tempCol]++;
						if (center[tempRow][tempCol] > max)
						{
							max = center[tempRow][tempCol];
							coor = make_pair(tempRow, tempCol);
						}
						if (Hough->getPixelElement(tempRow, tempCol) < 255)
						{
							tempPlus = Hough->getPixelElement(tempRow, tempCol)
									+ 1;
							Hough->setPixelElement(tempRow, tempCol, tempPlus);
						}
					}

					tempCol = j;
					tempRow = i;
				}
			}
		}
	}

	delete center[0];
	delete center;
	return coor;

}

void makeMask(Image *medianImage, Image *mask, Image *gradient,
		double (&gradientArr)[480][640], int max, int maskThreshold)
{
	for (int i = medianImage->getNumRows() - 1; i > 1; i--)
	{
		for (int j = medianImage->getNumColumns() - 1; j > 1; j--)
		{

			gradientArr[i][j] /= max;
			gradientArr[i][j] *= 255;
			gradient->setPixelElement(i, j, gradientArr[i][j]);

			if (gradientArr[i][j] < maskThreshold)
				gradientArr[i][j] = 0;
			else
				gradientArr[i][j] = 255;

			mask->setPixelElement(i, j, gradientArr[i][j]);

		}
	}
}

int sobel(Image *medianImage, double (&directional)[480][640],
		double (&gradientArr)[480][640])
{
	int max = 0;
	int sobelVSum = 0;
	int sobelHSum = 0;
	int gradientDouble = 0;
	double direct = 0;
	for (int i = 1; i < medianImage->getNumRows() - 1; i++)
	{
		int topI = i - 1;
		int botI = i + 1;
		for (int j = 1; j < medianImage->getNumColumns() - 1; j++)
		{
			int leftJ = j - 1;
			int rightJ = j + 1;

			sobelVSum -= medianImage->getPixelElement(topI, leftJ);
			sobelVSum -= 2 * medianImage->getPixelElement(i, leftJ);
			sobelVSum -= medianImage->getPixelElement(botI, leftJ);

			sobelVSum += medianImage->getPixelElement(topI, rightJ);
			sobelVSum += 2 * medianImage->getPixelElement(i, rightJ);
			sobelVSum += medianImage->getPixelElement(botI, rightJ);

			sobelHSum -= medianImage->getPixelElement(topI, leftJ);
			sobelHSum -= 2 * medianImage->getPixelElement(topI, j);
			sobelHSum -= medianImage->getPixelElement(topI, rightJ);

			sobelHSum += medianImage->getPixelElement(botI, leftJ);
			sobelHSum += 2 * medianImage->getPixelElement(botI, j);
			sobelHSum += medianImage->getPixelElement(botI, rightJ);

			direct = -atan2((double) sobelVSum, (double) sobelHSum);

			directional[i][j] = direct;

			sobelVSum *= sobelVSum;
			sobelHSum *= sobelHSum;

			gradientDouble = sqrt((double) sobelVSum + (double) sobelHSum);
			if (gradientDouble > max)
				max = gradientDouble;
			gradientArr[i][j] = gradientDouble;

			sobelVSum = 0;
			sobelHSum = 0;

		}
	}
	return max;
}

void houghSmooth(Image *tempHough, Image *Hough, float (&mexhatsmall)[5][5],
		float (&mexhatlarge)[17][17], bool LorS)
{
	for (int i = 100; i < Hough->getNumRows() - 100; i++)
	{
		for (int j = 150; j < Hough->getNumColumns() - 150; j++)
		{
			int total = 0;
			if (LorS)
			{
				for (int k = 0; k < 17; k++)
				{
					for (int l = 0; l < 17; l++)
					{
						total += mexhatlarge[k][l]
								* Hough->getPixelElement((i - k), (j - l));
					}
				}
			} else
			{
				for (int k = 0; k < 5; k++)
				{
					for (int l = 0; l < 5; l++)
					{
						total += mexhatsmall[k][l]
								* Hough->getPixelElement((i - k), (j - l));
					}
				}
			}
			if (total < 0)
			{
				total = 0;
				tempHough->setPixelElement(i, j, total);
			} else if (total < 255)
				tempHough->setPixelElement(i, j, total);
			else
				tempHough->setPixelElement(i, j, 255);

		}

	}
}

int findRad(int minRadDraw, int maxRadDraw, pair<int, int> centerCoor,
		Image *mask)
{
	int y;
	int row;
	int column;
	int maxCount = 0;
	int radius = 0;
	for (int j = minRadDraw; j < maxRadDraw; j++)
	{
		int testRadius = j;
		int count = 0;
		for (int i = 0; i < testRadius; i++)
		{
			y = sqrt(
					(pow((double) testRadius, (double) 2))
							- pow((double) i, (double) 2));

			if (i < (testRadius * .4))
			{
				row = centerCoor.first + i;
				column = centerCoor.second + y;
				if (row < 480 && column < 640 && row > 0 && column > 0)
					if (mask->getPixelElement(row, column) == 255)
						count++;

				row = centerCoor.first + i;
				column = centerCoor.second - y;
				if (row < 480 && column < 640 && row > 0 && column > 0)
					if (mask->getPixelElement(row, column) == 255)
						count++;

				row = centerCoor.first - i;
				column = centerCoor.second + y;
				if (row < 480 && column < 640 && row > 0 && column > 0)
					if (mask->getPixelElement(row, column) == 255)
						count++;

				row = centerCoor.first - i;
				column = centerCoor.second - y;
				if (row < 480 && column < 640 && row > 0 && column > 0)
					if (mask->getPixelElement(row, column) == 255)
						count++;
			}

			if (y < (testRadius * .4))
			{
				row = centerCoor.first + y;
				column = centerCoor.second + i;
				if (row < 480 && column < 640 && row > 0 && column > 0)
					if (mask->getPixelElement(row, column) == 255)
						count++;

				row = centerCoor.first + y;
				column = centerCoor.second - i;
				if (row < 480 && column < 640 && row > 0 && column > 0)
					if (mask->getPixelElement(row, column) == 255)
						count++;

				row = centerCoor.first - y;
				column = centerCoor.second + i;
				if (row < 480 && column < 640 && row > 0 && column > 0)
					if (mask->getPixelElement(row, column) == 255)
						count++;

				row = centerCoor.first - y;
				column = centerCoor.second - i;
				if (row < 480 && column < 640 && row > 0 && column > 0)
					if (mask->getPixelElement(row, column) == 255)
						count++;
			}

		}
		if (count > maxCount)
		{
			maxCount = count;
			radius = testRadius;
		}
	}
	if (maxCount == 0)
		return radius = 50;
	else
		return radius;
}

void printCircle(pair<int, int> centerCoor, Image *images, int radius)
{
	int y;
	int row;
	int column;
	if (radius == 0)
		return;
	for (int i = 0; i < radius; i++)
	{
		y = sqrt(
				(pow((double) radius, (double) 2))
						- pow((double) i, (double) 2));

		row = centerCoor.first + i;
		column = centerCoor.second + y;
		if (row < 480 && column < 640 && row > 0 && column > 0)
			images->setPixelElement(row, column, 255);

		row = centerCoor.first + i;
		column = centerCoor.second - y;
		if (row < 480 && column < 640 && row > 0 && column > 0)
			images->setPixelElement(row, column, 255);

		row = centerCoor.first - i;
		column = centerCoor.second + y;
		if (row < 480 && column < 640 && row > 0 && column > 0)
			images->setPixelElement(row, column, 255);

		row = centerCoor.first - i;
		column = centerCoor.second - y;
		if (row < 480 && column < 640 && row > 0 && column > 0)
			images->setPixelElement(row, column, 255);

		row = centerCoor.first + y;
		column = centerCoor.second + i;
		if (row < 480 && column < 640 && row > 0 && column > 0)
			images->setPixelElement(row, column, 255);

		row = centerCoor.first + y;
		column = centerCoor.second - i;
		if (row < 480 && column < 640 && row > 0 && column > 0)
			images->setPixelElement(row, column, 255);

		row = centerCoor.first - y;
		column = centerCoor.second + i;
		if (row < 480 && column < 640 && row > 0 && column > 0)
			images->setPixelElement(row, column, 255);

		row = centerCoor.first - y;
		column = centerCoor.second - i;
		if (row < 480 && column < 640 && row > 0 && column > 0)
			images->setPixelElement(row, column, 255);

	}
}

void unwrap(int radLow, int radHigh, pair<int, int> centerCoor,
		Image *unwrapped, Image *images)
{

	double y = 0;
	double x = 0;
	double angle = 6.28 / 180;
	double tempAngle = 0;
	double radInc = ((double) radHigh - (double) radLow) / 96.0;
	double tempRad = 0;

	for (int i = 0; i < 180; i++)
	{
		for (int j = 0; j < 96; j++)
		{
			tempRad = radInc * j + radLow;
			tempAngle = angle * i;
			x = tempRad * sin(tempAngle) + centerCoor.second;
			y = tempRad * cos(tempAngle) + centerCoor.first;
			if (y < 480 && x < 640 && y > 0 && x > 0)
				unwrapped->setPixelElement(j, i, images->getPixelElement(y, x));
		}
	}

}

void Haar(Image *unwrap, Image *Template, Image *maskTemplate,
		int (&haar)[8][8])
{

//CLAHE
	float clipLimit = 5.8f;
	unsigned int xResolution = 180;
	unsigned int yResolution = 96;
	int numRegionsInXDirection = xResolution / 20;
	int numRegionsInYDirection = yResolution / 20;

	if (numRegionsInXDirection > 20)
		numRegionsInXDirection = 20;
	if (numRegionsInYDirection > 20)
		numRegionsInYDirection = 20;

	int errors = CLAHE(*unwrap->getData(), xResolution, yResolution, 0, 255,
			numRegionsInXDirection, numRegionsInYDirection, 256, clipLimit);

	double tempSum = 0;
	double oneSqrt2 = 1 / sqrt((double) 2);
	int setZero = 0;
	int tempRow = 0;
	int tempCol = 0;
	for (int i = 0; i < 96; i++)
	{
		for (int j = 0; j < 180; j++)
		{
			tempSum = 0;
			for (int k = -4; k <= 3; k++)
			{
				for (int l = -4; l <= 3; l++)
				{
					setZero = 0;
					if (i + k < 0 || i + k > 95)
						continue;
					else
					{
						tempRow = i + k;
						if (j + l < 0)
							tempCol = 180 + (j + l);
						else if (j + l > 179)
							tempCol = 0 - (180 - (j + l));
						else
							tempCol = j + l;
						tempSum += haar[k + 4][l + 4]
								* unwrap->getPixelElement(tempRow, tempCol);
					}
				}
			}
			tempSum *= oneSqrt2;
			tempSum = tempSum / 64;
			if (tempSum > 0)
				Template->setPixelElement(i, j, 255);
			else
				Template->setPixelElement(i, j, 0);

			if (tempSum > 1 || tempSum < -1)
				maskTemplate->setPixelElement(i, j, 255);
			else
				maskTemplate->setPixelElement(i, j, 0);
		}
	}

}

extern "C" JNIEXPORT void JNICALL Java_com_cse3345_f13_Tanner_JNI_Segment(
		JNIEnv *env, jobject obj, jstring jfileName, jstring jpath,
		jobject jstats, jbyteArray passed)
{
	clock_t start;

	int lenJArr = env->GetArrayLength(passed);
	unsigned char* toBMP = new unsigned char[lenJArr];
	env->GetByteArrayRegion(passed, 0, lenJArr,
			reinterpret_cast<jbyte*>(toBMP));


	Image* makeBMP = new Image();
	int pos = 0;
	__android_log_print(ANDROID_LOG_ERROR, "MyProject",
			"pre setting toBMP");



/*
	for (int i = 0; i < 480; i++)
	{
		for (int j = 0; j < 640; j++)
		{
			makeBMP->setPixelElement(i,j,toBMP[pos]);
			pos++;
		}
	}
	*/
	__android_log_print(ANDROID_LOG_ERROR, "MyProject",
			"post setting toBMP");

	string fileName = env->GetStringUTFChars(jfileName, 0);
	string newFileName = env->GetStringUTFChars(jfileName, 0);

	fileName.append(".bmp");
	newFileName.append("_Segmented.bmp");



	string path = env->GetStringUTFChars(jpath, 0);

	jclass EyeStats = env->GetObjectClass(jstats);

	jmethodID setStats = env->GetMethodID(EyeStats, "setStats", "(DDII)V");



	__android_log_print(ANDROID_LOG_ERROR, "MyProject",
			"pre setting up bmp");




	//ofstream output;
	//output.open((path + fileName).c_str());

	__android_log_print(ANDROID_LOG_ERROR, "MyProject",
			(path + fileName).c_str());

	Image tempTest = new Image(480,640,toBMP);
	tempTest.writeBMP(fileName, path);

	//for(int i= 0; i < lenJArr; i++){
		//if(i % 7 == 0)
		//	output << toBMP[i] << endl;
	//	output << toBMP[i];
	//}
//
	//output.close();

	__android_log_print(ANDROID_LOG_ERROR, "MyProject",
			"pre read bmp");

	makeBMP->readBMP(fileName, path);

	__android_log_print(ANDROID_LOG_ERROR, "MyProject",
			"post read bmp");

	//makeBMP->writeBMP(fileName, path);



	//makeBMP->writeBMP(fileName, path);

	__android_log_print(ANDROID_LOG_ERROR, "MyProject",
			"post writing bmp");

	double pupil;
	double Limbic;
	int X;
	int Y;

	//__android_log_print(ANDROID_LOG_ERROR, "MyProject", "test");

	double clocks = (double) CLOCKS_PER_SEC;

	int radius = 0;
	int LimbicRadius = 0;

	/*
	 *
	 *
	 *
	 *
	 *
	 *
	 *
	 */
	int maskThreshold = 25;
	int maskThreshold2 = 35;
	int maskThreshold3 = 18;

	int gridSize = 21;
	int minR = 10;
	int maxR = 100;

	int minRadDraw = 30;
	int maxRadDraw = 110;
	int maxRadDrawSmall = 70;

	int minLimbRadDraw = 90;
	int maxLimbRadDraw = 170;

	//bool maskRetried = false;
	bool maskRetried = false;
	/*
	 *
	 *
	 *
	 *
	 */
	int imageRuns = 3000;
	bool print = false;
	bool printImages = true;

	/*
	 *
	 *
	 *
	 *
	 *
	 *
	 *
	 */

	double progress;

	Image *templates[imageRuns];
	Image *masks[imageRuns];

	string buffer;

	string eyes[imageRuns];
	string subjects[imageRuns];
	string fileNames[imageRuns];

	int falseCount = 0;
	int genCount = 0;

	double meanFalse = 0;
	double meanGen = 0;

	int falseCountTotal;
	int genCountTotal;

	double meanFalseTotal;
	double meanGenTotal;

	Image *images = new Image(480, 640);
	Image *medianImage = new Image(480, 640);
	Image *gradient = new Image(480, 640);
	Image *mask = new Image(480, 640);
	Image *Hough = new Image(480, 640);
	Image *tempHough = new Image(480, 640);

	Image *maskRetry1 = new Image(480, 640);
	Image *maskRetry2 = new Image(480, 640);

	Image *unwrapped = new Image(96, 180);
	Image *Template = new Image(96, 180);
	Image *maskTemplate = new Image(96, 180);

	int haar[8][8] = { { -1, -1, -1, -1, 1, 1, 1, 1 }, { -1, -1, -1, -1, 1, 1,
			1, 1 }, { -1, -1, -1, -1, 1, 1, 1, 1 },
			{ -1, -1, -1, -1, 1, 1, 1, 1 }, { -1, -1, -1, -1, 1, 1, 1, 1 }, {
					-1, -1, -1, -1, 1, 1, 1, 1 },
			{ -1, -1, -1, -1, 1, 1, 1, 1 }, { -1, -1, -1, -1, 1, 1, 1, 1 } };

	double declarationDuration = 0;
	double medianFilterDuration = 0;
	double sobelDuration = 0;
	double maskDuration = 0;
	double HoughDuration = 0;
	double findDuration = 0;
	double drawDuration = 0;
	double writeDuration = 0;
	double insideSobelDuration = 0;

	double minHams = 10;
	double maxHams = 0;

	double minHam = 10;
	double maxHam = 0;

	int posMaxGenI = 0;
	int posMaxGenJ = 0;

	int posMinGenI = 0;
	int posMinGenJ = 10;

	int maxPosI = 0;
	int maxPosJ = 0;

	int minPosI = 10;
	int minPosJ = 10;

	int genAboveMin = 0;

	pair<int, int> centerCoor;

	static double directional[480][640];
	static double gradientArr[480][640];

	bool firstRun = true;

//while (true) {

	int maxRad = 0;
	int maxRadLimb = 0;

	int max;

	if (print)
		start = clock();
	maskRetried = false;

	images->setAllData(0);
	gradient->setAllData(0);
	medianImage->setAllData(0);
	mask->setAllData(0);
	Hough->setAllData(0);
	tempHough->setAllData(0);
	Template->setAllData(0);
	maskTemplate->setAllData(0);
	maskRetry1->setAllData(0);
	maskRetry2->setAllData(0);
	//}
	//firstRun = false;

//return;

	images->readBMP(fileName, path);

	max = 0;

	if (print)
	{
		declarationDuration += (clock() - start) / clocks;
		start = clock();
	}

	MedianFilter(images, medianImage, 21);

	if (print)
	{
		medianFilterDuration += (clock() - start) / clocks;
		start = clock();
	}

	max = sobel(medianImage, directional, gradientArr);

	if (print)
	{
		sobelDuration += (clock() - start) / clocks;
		start = clock();
	}

	makeMask(medianImage, mask, gradient, gradientArr, max, maskThreshold);

	if (print)
	{
		maskDuration += (clock() - start) / clocks;
		start = clock();
	}

	centerCoor = HoughTransform(directional, mask, Hough, gridSize, minR, maxR);
	if (print)
	{
		HoughDuration += (clock() - start) / clocks;
		start = clock();
	}

	//first find the pupil radius and draw it
	//then find the limbic radius and draw it as well
	radius = findRad(minRadDraw, maxRadDraw, centerCoor, mask);
	if (radius > maxRad)
		maxRad = radius;
	LimbicRadius = findRad(minLimbRadDraw, maxLimbRadDraw, centerCoor, mask);
	if (LimbicRadius > maxRadLimb)
		maxRadLimb = LimbicRadius;
	if (LimbicRadius == maxRadLimb)
	{
		makeMask(medianImage, maskRetry1, gradient, gradientArr, max,
				maskThreshold2);
		LimbicRadius = findRad(minLimbRadDraw, maxLimbRadDraw, centerCoor,
				maskRetry1);
		if (LimbicRadius == maxRadLimb)
			maskRetried = true;
		if (maskRetried)
		{
			makeMask(medianImage, maskRetry2, gradient, gradientArr, max,
					maskThreshold3);
			LimbicRadius = findRad(minLimbRadDraw, maxLimbRadDraw, centerCoor,
					maskRetry2);
			maskRetried = false;
		}

	}

	if (radius >= LimbicRadius - 30)
	{
		radius = findRad(minRadDraw, maxRadDrawSmall, centerCoor, mask);
	}

	if (print)
	{
		findDuration += (clock() - start) / clocks;
		start = clock();
	}

	unwrap(radius, LimbicRadius, centerCoor, unwrapped, images);
	Haar(unwrapped, Template, maskTemplate, haar);

	printCircle(centerCoor, images, radius);
	printCircle(centerCoor, images, LimbicRadius);
	if (print)
	{
		drawDuration += (clock() - start) / clocks;
		start = clock();
	}

	pupil = radius;
	Limbic = LimbicRadius;
	X = centerCoor.first;
	Y = centerCoor.second;

	//string pathName = "/storage/emulated/0/Pictures/segmented/";

	env->CallVoidMethod(jstats, setStats, pupil, Limbic, X, Y);

	images->writeBMP(newFileName, path);

}

