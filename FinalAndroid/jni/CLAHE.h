#ifndef CLAHE_H
#define CLAHE_H

/*********************** Local prototypes ************************/
typedef unsigned char kz_pixel_t;	 /* for 8 bit-per-pixel images */
#define uiNR_OF_GREY (256)

static void ClipHistogram (unsigned long*, unsigned int, unsigned long);
static void MakeHistogram (kz_pixel_t*, unsigned int, unsigned int, unsigned int,
		unsigned long*, unsigned int, kz_pixel_t*);
static void MapHistogram (unsigned long*, kz_pixel_t, kz_pixel_t,
	       unsigned int, unsigned long);
static void MakeLut (kz_pixel_t*, kz_pixel_t, kz_pixel_t, unsigned int);
static void Interpolate (kz_pixel_t*, int, unsigned long*, unsigned long*,
	unsigned long*, unsigned long*, unsigned int, unsigned int, kz_pixel_t*);

/**************	 Start of actual code **************/
#include <stdlib.h>			 /* To get prototypes of malloc() and free() */

const unsigned int uiMAX_REG_X = 20;	  /* max. # contextual regions in x-direction */
const unsigned int uiMAX_REG_Y = 20;	  /* max. # contextual regions in y-direction */


int CLAHE (kz_pixel_t *, unsigned int, unsigned int, kz_pixel_t, 
		   kz_pixel_t, unsigned int, unsigned int, unsigned int, float);
void ClipHistogram (unsigned long *, unsigned int, unsigned long);
void MakeHistogram (kz_pixel_t *, unsigned int,	unsigned int, unsigned int, unsigned long*,
				    unsigned int, kz_pixel_t*);
void MapHistogram (unsigned long*, kz_pixel_t, kz_pixel_t, unsigned int, unsigned long);
void MakeLut (kz_pixel_t *, kz_pixel_t, kz_pixel_t, unsigned int);
void Interpolate (kz_pixel_t *, int, unsigned long *, unsigned long *, 
				  unsigned long *,  unsigned long *, unsigned int, unsigned int, kz_pixel_t *);

#endif