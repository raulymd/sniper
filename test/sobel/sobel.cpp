/*
 * sobel.cpp
 * 
 * Created on: Sep 9, 2013
 * 			Author: Amir Yazdanbakhsh <a.yazdanbakhsh@gatech.edu>
 *
 *	Annotated by: Raul Yarmand
 *	to add approximate variables for both sniper and data significance analysis 
 */

#include "convolution.hpp"
#include <iostream>
#include <cmath>

#define ULONG unsigned long
#define UCHAR unsigned char
#define UINT8 unsigned char

#include "/home/shahdaad/raul/images/hpp/peppers_gray.hpp"


#ifdef SNIPER
	#include "sim_api.h"
#endif


int saveChImage(const char* fileName) {		
	int i;
	int j;
	FILE *fp;
	fp = fopen(fileName, "w");
	if (!fp) {
		printf("[SOB] Warning: Oops! Cannot open %s!\n", fileName);
		return 0;
	}  
    fprintf(fp,"%d\n",WIDHEI*WIDHEI*3);
    fwrite(dstImage , sizeof(char), sizeof(dstImage), fp);

	fclose(fp);
	return 1;
} 

/*
int saveRgbImage(const char* fileName) 	//dstImage  must be used instead of srcImage for sobel
{		
	int i;
	int j;
	FILE *fp;
	fp = fopen(fileName, "w");
	if (!fp) {
		printf("Warning: Oops! Cannot open %s!\n", fileName);
		return 0;
	}
	fprintf(fp, "%d,%d\n", WIDHEI, WIDHEI);
	for(i = 0; i < WIDHEI; i++) {
		for(j = 0; j < (WIDHEI - 1); j++) {
			fprintf(fp, "%d,%d,%d,", int((dstImage[i][j][0])), int((dstImage[i][j][1])), int((dstImage[i][j][2])));
		}
		fprintf(fp, "%d,%d,%d\n", int((dstImage[i][j][0])), int((dstImage[i][j][1])), int((dstImage[i][j][2])));
	}

	fprintf(fp, "Manipulated by Raul Yarmand");
	fclose(fp);
	return 1;
}

void makeGrayscale(UCHAR image[][WIDHEI][3])
{
	float luminance ;

	float rC = 0.30  ;		//changed -> no scaling
	float gC = 0.59  ;		//changed
	float bC = 0.11  ;		//changed

	for(int w = 0 ; w < WIDHEI ; w++)
	{
		for(int h = 0 ; h < WIDHEI ; h++)
		{
			luminance = 	   (( rC * ((UCHAR)image[h][w][0]) ) + 		    //( rC * ((UCHAR)image[h][w][0]) ) + 
								( gC * ((UCHAR)image[h][w][1]) ) +          //( gC * ((UCHAR)image[h][w][1]) ) + 
								( bC * ((UCHAR)image[h][w][2]) )) ;         //( bC * ((UCHAR)image[h][w][2]) ) ;

			luminance = ( rC * this->pixels[h][w]->r ) + 
						( gC * this->pixels[h][w]->g ) + 
						( bC * this->pixels[h][w]->b ) ;			
						

			image[h][w][0] = (UCHAR)(luminance + 0.0001);
			image[h][w][1] = (UCHAR)(luminance + 0.0001); 
			image[h][w][2] = (UCHAR)(luminance + 0.0001); 
		}
	}
}
*/

int main ( int argc, const char* argv[])
{
	int x, y;
	float s = 0;
	float w[][3] = {
		{0, 0, 0},
		{0, 0, 0},
		{0, 0, 0}
	};


#ifdef SNIPER
	SimRoiStart();
	//marking variables as approximate -> Raul Codes
	SimAddApproxAddrs((ULONG)&srcImage[0][0][0],WIDHEI*WIDHEI*3);
	SimAddApproxAddrs((ULONG)&dstImage[0][0][0],WIDHEI*WIDHEI*3);
#endif


	y = 0 ;
	// Start performing Sobel operation
	for( x = 0 ; x < srcImageWidth ; x++ ) {
		HALF_WINDOW(srcImage, x, y, w) ;


			s = sobel(w);
		
		dstImage[y][x][0] = (UCHAR)s ;
		dstImage[y][x][1] = (UCHAR)s ;
		dstImage[y][x][2] = (UCHAR)s ;
		
#ifdef SNIPER        
		//adding ADD counters based on algorithm
		SimApproxAddCnt((ULONG)&dstImage[y][x][0],16);
		SimApproxAddCnt((ULONG)&dstImage[y][x][1],16);
		SimApproxAddCnt((ULONG)&dstImage[y][x][2],16);		
        
		SimApproxMulCnt((ULONG)&dstImage[y][x][0],18);
		SimApproxMulCnt((ULONG)&dstImage[y][x][1],18);
		SimApproxMulCnt((ULONG)&dstImage[y][x][2],18);        
#endif
	}
    //return 0;       //first row
    
	for (y = 1 ; y < (srcImageHeight - 1) ; y++) {
		x = 0 ;
		HALF_WINDOW(srcImage, x, y, w);

			s = sobel(w);

	
		dstImage[y][x][0] = (UCHAR)s ;
		dstImage[y][x][1] = (UCHAR)s ;
		dstImage[y][x][2] = (UCHAR)s ;
        
#ifdef SNIPER
		//adding ADD counters based on algorithm
		SimApproxAddCnt((ULONG)&dstImage[y][x][0],16);
		SimApproxAddCnt((ULONG)&dstImage[y][x][1],16);
		SimApproxAddCnt((ULONG)&dstImage[y][x][2],16);	
        
		SimApproxMulCnt((ULONG)&dstImage[y][x][0],18);
		SimApproxMulCnt((ULONG)&dstImage[y][x][1],18);
		SimApproxMulCnt((ULONG)&dstImage[y][x][2],18);	        
#endif
		for( x = 1 ; x < srcImageWidth - 1 ; x++ ) {
			WINDOW(srcImage, x, y, w) ;
				
				s = sobel(w);
				

			dstImage[y][x][0] = (UCHAR)s ;
			dstImage[y][x][1] = (UCHAR)s ;
			dstImage[y][x][2] = (UCHAR)s ;
            
#ifdef SNIPER
			//adding ADD counters based on algorithm
			SimApproxAddCnt((ULONG)&dstImage[y][x][0],16);
			SimApproxAddCnt((ULONG)&dstImage[y][x][1],16);
			SimApproxAddCnt((ULONG)&dstImage[y][x][2],16);					
            
			SimApproxMulCnt((ULONG)&dstImage[y][x][0],18);
			SimApproxMulCnt((ULONG)&dstImage[y][x][1],18);
			SimApproxMulCnt((ULONG)&dstImage[y][x][2],18);		            
#endif
		}

		x = srcImageWidth - 1 ;
		HALF_WINDOW(srcImage, x, y, w) ;
		

			s = sobel(w);
			

		dstImage[y][x][0] = (UCHAR)s ;
		dstImage[y][x][1] = (UCHAR)s ;
		dstImage[y][x][2] = (UCHAR)s ;

#ifdef SNIPER
		//adding ADD counters based on algorithm
		SimApproxAddCnt((ULONG)&dstImage[y][x][0],16);
		SimApproxAddCnt((ULONG)&dstImage[y][x][1],16);
		SimApproxAddCnt((ULONG)&dstImage[y][x][2],16);		

		SimApproxMulCnt((ULONG)&dstImage[y][x][0],18);
		SimApproxMulCnt((ULONG)&dstImage[y][x][1],18);
		SimApproxMulCnt((ULONG)&dstImage[y][x][2],18);		
#endif        

	}

	y = srcImageHeight - 1;

	for(x = 0 ; x < srcImageWidth ; x++) {
		HALF_WINDOW(srcImage, x, y, w) ;
		
			s = sobel(w);
			

		dstImage[y][x][0] = (UCHAR)s ;
		dstImage[y][x][1] = (UCHAR)s ;
		dstImage[y][x][2] = (UCHAR)s ;

#ifdef SNIPER
		//adding ADD counters based on algorithm
		SimApproxAddCnt((ULONG)&dstImage[y][x][0],16);
		SimApproxAddCnt((ULONG)&dstImage[y][x][1],16);
		SimApproxAddCnt((ULONG)&dstImage[y][x][2],16);		

		SimApproxMulCnt((ULONG)&dstImage[y][x][0],18);
		SimApproxMulCnt((ULONG)&dstImage[y][x][1],18);
		SimApproxMulCnt((ULONG)&dstImage[y][x][2],18);	        
#endif
	}

    if(argc == 1)
        saveChImage( "sobel_outimg_0.ch");
    else 
    {
        std::string fname = (std::string)"sobel_outimg_" + (std::string)argv[1] + ".ch";
        saveChImage(fname.c_str());		
    }

#ifdef SNIPER
	SimDelApproxAddrs((ULONG)&srcImage[0][0][0],WIDHEI*WIDHEI*3);
	SimDelApproxAddrs((ULONG)&dstImage[0][0][0],WIDHEI*WIDHEI*3);
	SimRoiEnd();
	//deleting variables as approximate -> Raul Codes
#endif	

	return 0 ;
}