#ifndef IMAGE_OBJ_H
#define IMAGE_OBJ_H

#include <string>

using namespace std;

// the header for loading .bmp images into the program
struct header
{
	unsigned short FileType;  /* File type, always 4D42h ("BM") */
	char data[4];	/* Size of the file in bytes*/
	unsigned short Reserved1;	/* Always 0 */
	unsigned short Reserved2;	/* Always 0 */
	char data2[4];	/* Starting position of image data */
	char data3[4];	/* Size of header in bytes */
	char data4[4];	/* Image width in pixels */
	char data5[4];	/* Image height in pixels */
	unsigned short Planes;	/* Number of color planes */
	unsigned short BitsPerPixel;	/* Number of bits per pixel */
	char data6[4];	/* Compression method used */
	char data7[4];	/* Size of bitmap in bytes */
	char data8[4];	/* the horizontal pixels per meter on the targer device, generally zero. */
	char data9[4];	/* the vertical pixels per meter on the targer device, generally zero. */
	char data10[4]; /* number of colors used in bitmap.  If zero, numbers calculated using biBitCount member */
	char data11[4];	/* number of important colors.  zero indicates all colors are equally important */
};

class Image
{
private:
	int numRows;
	int numColumns;
	unsigned char** data;
	int rowOffset;
	int columnOffset;
public:
	// constructors and destructors
	Image();
	Image(int, int, unsigned char*);
	Image(Image*);
	Image(int, int);
	~Image();
	// accessors and mutators
	int setSize(int, int);
	int setPixelElement(int, int, unsigned char);
	int setOffset(int, int);
	int getOriginRowOffset();
	int getOriginColumnOffset();
	int getNumRows();
	int getNumColumns();
	unsigned char getPixelElement(int, int);
	unsigned char **getData();
	// arithmetic operations
	int getMax();
	int getMaxLocation(int&, int&);
	int getMaxAbsoluteDifference(int, int, int, int, int);
	float getMean();
	float getStandardDev(float imageMean = NULL);
	int sumAllOnes();
	// file I/O
	int readBMP(string, string = "");
	int writeBMP(string, string = "");
	int readPGM(string, string);
	int writePGM(string, string);
	// image operations
	int SubImage(int, int, int, int, Image*&);
	int getSum();
	int getRowSum(int);
	int BWLabelObjects(int **&returnedAreas, int minimumArea);
	int setAllData(int value);


	//testing

};
#endif
