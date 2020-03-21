/*
 * convolution.hpp
 * 
 * Created on: Sep 9, 2013
 * 			Author: Amir Yazdanbakhsh <a.yazdanbakhsh@gatech.edu>
 *
 *	Annotated by: Raul Yarmand
 *	to add approximate variables for both sniper and data significance analysis 
 */

 #ifndef __CONVOLUTION_HPP__
 #define __CONVOLUTION_HPP__

#include <iostream>
#include <string>

#define ULONG unsigned long
#define UCHAR unsigned char
#define UINT8 unsigned char


extern int srcImageHeight;
extern int srcImageWidth;
extern UINT8 dstImage[WIDHEI][WIDHEI][3];
extern UINT8 srcImage[WIDHEI][WIDHEI][3];


float sobel(float w[][3]) ;
 #define WINDOW(image, x, y, window) {					\
 	window[0][0] = (image[ y - 1 ][ x - 1 ][0]) ;	\
 	window[0][1] = (image[ y - 1 ][ x ][0]	) ; \
 	window[0][2] = (image[ y - 1 ][ x + 1 ][0]) ;	\
																\
	window[1][0] = (image[ y ][ x - 1 ][0]	);	\
 	window[1][1] = (image[ y ][ x ][0]	    );	\
 	window[1][2] = (image[ y ][ x + 1 ][0]	);	\
																\
	window[2][0] = (image[ y + 1 ][ x - 1 ][0]) ;	\
 	window[2][1] = (image[ y + 1 ][ x ][0] 	) ; \
 	window[2][2] = (image[ y + 1 ][ x + 1 ][0]) ;	\
}

#define HALF_WINDOW(image, x, y, window) {																			\
	window[0][0] = (x == 0 || y == 0										) ? 0 : (image[y - 1][x - 1][0]);	\
	window[0][1] = (y == 0													) ? 0 : (image[y - 1][x][0]	 );	\
	window[0][2] = (x == srcImageWidth -1 || y == 0						) ? 0 : (image[y - 1][x + 1][0]);	\
																															\
	window[1][0] = (x == 0 													) ? 0 : (image[y][x - 1][0]	 );	\
	window[1][1] = 																	(image[y][x][0]		 );	\
	window[1][2] = (x == srcImageWidth -1									) ? 0 : (image[y][x + 1][0]	 );	\
																															\
	window[2][0] = (x == 0 || y == srcImageHeight - 1						) ? 0 : (image[y + 1][x - 1][0]);	\
	window[2][1] = (y == srcImageHeight - 1								) ? 0 : (image[y + 1][x][0]	 );	\
	window[2][2] = (x == srcImageWidth -1 || y == srcImageHeight - 1	) ? 0 : (image[y + 1][x + 1][0]);	\
}
/* 
 #define WINDOW(imagePtr, x, y, window) {					\
 	window[0][0] = (imagePtr->pixels[ y - 1 ][ x - 1 ]->r) ;	\
 	window[0][1] = (imagePtr->pixels[ y - 1 ][ x ]->r 	) ; \
 	window[0][2] = (imagePtr->pixels[ y - 1 ][ x + 1 ]->r) ;	\
																\
	window[1][0] = (imagePtr->pixels[ y ][ x - 1 ]->r 	);	\
 	window[1][1] = (imagePtr->pixels[ y ][ x ]->r 	    );	\
 	window[1][2] = (imagePtr->pixels[ y ][ x + 1 ]->r 	);	\
																\
	window[2][0] = (imagePtr->pixels[ y + 1 ][ x - 1 ]->r) ;	\
 	window[2][1] = (imagePtr->pixels[ y + 1 ][ x ]->r 	) ; \
 	window[2][2] = (imagePtr->pixels[ y + 1 ][ x + 1 ]->r) ;	\
}

#define HALF_WINDOW(imagePtr, x, y, window) {																			\
	window[0][0] = (x == 0 || y == 0										) ? 0 : (imagePtr->pixels[y - 1][x - 1]->r);	\
	window[0][1] = (y == 0													) ? 0 : (imagePtr->pixels[y - 1][x]->r	 );	\
	window[0][2] = (x == imagePtr->width -1 || y == 0						) ? 0 : (imagePtr->pixels[y - 1][x + 1]->r);	\
																															\
	window[1][0] = (x == 0 													) ? 0 : (imagePtr->pixels[y][x - 1]->r	 );	\
	window[1][1] = 																	(imagePtr->pixels[y][x]->r		 );	\
	window[1][2] = (x == imagePtr->width -1									) ? 0 : (imagePtr->pixels[y][x + 1]->r	 );	\
																															\
	window[2][0] = (x == 0 || y == imagePtr->height - 1						) ? 0 : (imagePtr->pixels[y + 1][x - 1]->r);	\
	window[2][1] = (y == imagePtr->height - 1								) ? 0 : (imagePtr->pixels[y + 1][x]->r	 );	\
	window[2][2] = (x == imagePtr->width -1 || y == imagePtr->height - 1	) ? 0 : (imagePtr->pixels[y + 1][x + 1]->r);	\
}
*/
/*
 #define WINDOW(imagePtr, x, y, window) {					\
 	window[0][0] = imagePtr->pixels[ y - 1 ][ x - 1 ]->r ;	\
 	window[0][1] = imagePtr->pixels[ y - 1 ][ x ]->r ;	    \
 	window[0][2] = imagePtr->pixels[ y - 1 ][ x + 1 ]->r ;	\
\
	window[1][0] = imagePtr->pixels[ y ][ x - 1 ]->r ;		\
 	window[1][1] = imagePtr->pixels[ y ][ x ]->r ;	    	\
 	window[1][2] = imagePtr->pixels[ y ][ x + 1 ]->r ;		\
\
	window[2][0] = imagePtr->pixels[ y + 1 ][ x - 1 ]->r ;	\
 	window[2][1] = imagePtr->pixels[ y + 1 ][ x ]->r ;	    \
 	window[2][2] = imagePtr->pixels[ y + 1 ][ x + 1 ]->r ;	\
}

#define HALF_WINDOW(imagePtr, x, y, window) {																			\
	window[0][0] = (x == 0 || y == 0										) ? 0 : imagePtr->pixels[y - 1][x - 1]->r;	\
	window[0][1] = (y == 0													) ? 0 : imagePtr->pixels[y - 1][x]->r;		\
	window[0][2] = (x == imagePtr->width -1 || y == 0						) ? 0 : imagePtr->pixels[y - 1][x + 1]->r;	\
\
	window[1][0] = (x == 0 													) ? 0 : imagePtr->pixels[y][x - 1]->r;		\
	window[1][1] = 																	imagePtr->pixels[y][x]->r;			\
	window[1][2] = (x == imagePtr->width -1									) ? 0 : imagePtr->pixels[y][x + 1]->r;		\
\
	window[2][0] = (x == 0 || y == imagePtr->height - 1						) ? 0 : imagePtr->pixels[y + 1][x - 1]->r;	\
	window[2][1] = (y == imagePtr->height - 1								) ? 0 : imagePtr->pixels[y + 1][x]->r;		\
	window[2][2] = (x == imagePtr->width -1 || y == imagePtr->height - 1	) ? 0 : imagePtr->pixels[y + 1][x + 1]->r;	\
}

*/

 #endif