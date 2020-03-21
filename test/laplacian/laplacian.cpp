/*
 * laplacian.cpp
 * 
 * Created on: Dec 1, 2018
 * 			Author: Raul Ymd <raul.ymd@gmail.com>
 *
 *
 *	
 *	image sharpening algorithm by 5*5 laplacian filter
 *
 */
 

#define ULONG unsigned long
#define UCHAR unsigned char
#define UINT8 unsigned char

#include <iostream>
#include <cmath>
#include <fstream>


#include "/home/shahdaad/raul/images/hpp/peppers_gray.hpp"          //address of hpp image
//#include IMHEADER

#ifdef SNIPER
	#include "sim_api.h"
#endif

/*
int saveRgbImage(const char* fileName) {		
	int i;
	int j;
	FILE *fp;
	fp = fopen(fileName, "w");
	if (!fp) {
		printf("[AMN] Warning: Oops! Cannot open %s!\n", fileName);
		return 0;
	}
	fprintf(fp, "%d,%d\n", WIDHEI, WIDHEI);
	for(i = 0; i < WIDHEI; i++) {
		for(j = 0; j < (WIDHEI - 1); j++) {
			fprintf(fp, "%d,%d,%d,", int((dstImage[i][j][0])), int((dstImage[i][j][1])), int((dstImage[i][j][2])));
		}
		fprintf(fp, "%d,%d,%d\n", int((dstImage[i][j][0])), int((dstImage[i][j][1])), int((dstImage[i][j][2])));
	}

	fprintf(fp, "Manipulated by Raul Ymd");
	fclose(fp);
	return 1;
} 
*/

int saveChImage(const char* fileName) {		
	int i;
	int j;
	FILE *fp;
	fp = fopen(fileName, "w");
	if (!fp) {
		printf("[LAP] Warning: Oops! Cannot open %s!\n", fileName);
		return 0;
	}	
    fprintf(fp,"%d\n",WIDHEI*WIDHEI*3);
    fwrite(dstImage , sizeof(char), sizeof(dstImage), fp);

	fclose(fp);
	return 1;
} 


int main(int argc, const char* argv[]) 
{
	//clock_t begin = clock();
    int x, y;
    int i, j;
    int n, val;   

#ifdef SNIPER
	SimRoiStart();
	//marking variables as approximate -> Raul Codes
	SimAddApproxAddrs((ULONG)&srcImage[0][0][0],WIDHEI*WIDHEI*3);
	SimAddApproxAddrs((ULONG)&dstImage[0][0][0],WIDHEI*WIDHEI*3);
#endif    
    
    n= 5;       //laplacian filter size
    double lap_mask[5][5] =
		{
			{ -1, -1, -1,  -1,  -1 },
			{ -1, -1, -1,  -1,  -1 },
			{ -1, -1, 24,  -1,  -1 },
			{ -1, -1, -1,  -1,  -1 },
			{ -1, -1, -1,  -1,  -1 }
		} ;
		
	//makeGrayscale(srcImage);    
    for(x=0; x<srcImageWidth; x++)
    {
        for(y=0; y<srcImageWidth; y++)
        {
            val = 0;
            for (i=0; i<n; i++)
            {
                for (j=0; j<n; j++)
                {
                    int zx = x+i-n/2;
                    int zy = y+j-n/2;
                    if(zx >= 0 && zy >= 0 && zx<srcImageWidth && zy<srcImageWidth)
                        val += (srcImage[zx][zy][0])*lap_mask[i][j];            //WARNING: the image must be grayscale
                }
            }
            if(val > 255)   val = 255;          //saturation
            if(val < 0)     val = 0;            //saturation
            dstImage[x][y][0] = val;
            dstImage[x][y][1] = val;
            dstImage[x][y][2] = val;
            
#ifdef SNIPER        
		//adding ADD counters based on algorithm
		SimApproxAddCnt((ULONG)&dstImage[x][y][0],24);
		SimApproxAddCnt((ULONG)&dstImage[x][y][1],24);
		SimApproxAddCnt((ULONG)&dstImage[x][y][2],24);

		SimApproxMulCnt((ULONG)&dstImage[y][x][0],25);
		SimApproxMulCnt((ULONG)&dstImage[y][x][1],25);
		SimApproxMulCnt((ULONG)&dstImage[y][x][2],25);	         
#endif  
            
        }
    }
    //saveRgbImage( "laplacian_outimg.rgb");
    if(argc == 1)
        saveChImage( "laplacian_outimg_0.ch");		//because saveChImage also contributes in fault and DataSig
    else
    {
        std::string fname = (std::string)"laplacian_outimg_" + (std::string)argv[1] + ".ch";
        saveChImage(fname.c_str());		
    }

#ifdef SNIPER
	SimDelApproxAddrs((ULONG)&srcImage[0][0][0],WIDHEI*WIDHEI*3);
	SimDelApproxAddrs((ULONG)&dstImage[0][0][0],WIDHEI*WIDHEI*3);
	SimRoiEnd();
	//deleting variables as approximate -> Raul Codes
#endif	 
	
	//clock_t end = clock();
	//double time_spent = (double)(end - begin) ;
	//printf("execution time is: %f\n",time_spent);
	
    return  0;
}
