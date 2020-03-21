/*
 * amean.cpp
 * 
 * Created on: Dec 1, 2018
 * 			Author: Raul Ymd <raul.ymd@gmail.com>
 *
 *
 *	
 *	arithmetic mean filter to remove short tailed noise 
 *
 */


#define WIN_DIM 5		//5*5 window for calculating average 
#define ULONG unsigned long 
#define UINT8 unsigned char 
#define UCHAR unsigned char

#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS


#include <iostream>
#include <cmath>
#include <fstream>

#include "/home/shahdaad/raul/images/hpp/peppers_gray.hpp"          //address of hpp image


#ifdef SNIPER
	#include "sim_api.h"
#endif

int saveChImage(const char* fileName) {		
	int i;
	int j;
	FILE *fp;
	fp = fopen(fileName, "w");
	if (!fp) {
		printf("[AMN] Warning: Oops! Cannot open %s!\n", fileName);
		return 0;
	}
  
    fprintf(fp,"%d\n",WIDHEI*WIDHEI*3);
    fwrite(dstImage , sizeof(char), sizeof(dstImage), fp);

	fclose(fp);
	return 1;
} 

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

int main(int argc, const char* argv[]) 
{
	//clock_t begin = clock();
    int x, y;
    int i, j;
    int sum;
    // int al = 10 ;
    // printf("srcImage address: %ld\n", (ULONG)&srcImage[0][0][0]);
    // printf("dstImage address: %ld\n", (ULONG)&dstImage[0][0][0]);
    // return 1;
    
#ifdef SNIPER
	SimRoiStart();
	//marking variables as approximate -> Raul Codes
	SimAddApproxAddrs((ULONG)&srcImage[0][0][0],WIDHEI*WIDHEI*3);
	SimAddApproxAddrs((ULONG)&dstImage[0][0][0],WIDHEI*WIDHEI*3);
    /*
    for(x=0; x<srcImageWidth; x++)
    {
        for(y=0; y<srcImageWidth; y++)
        {
            SimAddApproxAddrs((ULONG)&srcImage[x][y][0],1);
            SimAddApproxAddrs((ULONG)&srcImage[x][y][1],1);
            SimAddApproxAddrs((ULONG)&srcImage[x][y][2],1);
            SimAddApproxAddrs((ULONG)&dstImage[x][y][0],1);
            SimAddApproxAddrs((ULONG)&dstImage[x][y][1],1);
            SimAddApproxAddrs((ULONG)&dstImage[x][y][2],1);
        }
    }
    */
#endif
    
       
    for(x=0; x<srcImageWidth; x++)
    {
        for(y=0; y<srcImageWidth; y++)
        {
            sum = 0;
            for (i=0; i<WIN_DIM; i++)
            {
                for (j=0; j<WIN_DIM; j++)
                {
                    int zx = x+i-WIN_DIM/2;
                    int zy = y+j-WIN_DIM/2;
                    if(zx >= 0 && zy >= 0 && zx<srcImageWidth && zy<srcImageWidth)
					{
                        sum += (srcImage[zx][zy][0]);            //WARNING: the image must be grayscale
						//if(x == 63 && y ==0)
							//printf("(%d,%d) value: %d\n",zx,zy,(int)srcImage[zx][zy][0]);
                        
					}
                }
            }
            int val = sum/WIN_DIM/WIN_DIM;      if(val > 255) val = 255;
            dstImage[x][y][0] = val;
            dstImage[x][y][1] = val;
            dstImage[x][y][2] = val;
            
#ifdef SNIPER        
		//adding ADD counters based on algorithm
		SimApproxAddCnt((ULONG)&dstImage[x][y][0],8);
		SimApproxAddCnt((ULONG)&dstImage[x][y][1],8);
		SimApproxAddCnt((ULONG)&dstImage[x][y][2],8);		   
#endif                        
        }
    }
    
    //saveRgbImage( "amean_outimg.rgb");
    if(argc == 1)
        saveChImage( "amean_outimg_0.ch");		//because saveChImage also contributes in fault and DataSig
    else
    {
        std::string fname = (std::string)"amean_outimg_" + (std::string)argv[1] + ".ch";
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
