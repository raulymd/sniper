#include "sim_api.h"

#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS
#include <stdint.h>
#include <stdio.h>

int value; 

int loop() {

	int i;
	for (i = 0 ; i < 6000; i++) {
		value += i;
	}
	return value;
}



int main() {

    float A = -1;
    float B = 10;
	SimRoiStart();

	value = 0;
	loop();

	if (SimInSimulator()) {
		printf("API Test: Running in the simulator\n"); fflush(stdout);
	} else {
		printf("API Test: Not running in the simulator\n"); fflush(stdout);
	}

	SimAddApproxAddrs(1000,1);
	printf("[API] -1 * 10 = %ld\n",SimApproxMulOp(A,B));
	if(SimApproxMulCnt(1000,1))
		printf("[API] mul count incremented!\n");
	else
		printf("[API] mul count not incremented!\n");
	SimDelApproxAddrs(1000,1);	
	SimRoiEnd();
	return 0;
}
