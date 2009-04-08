#include <config.h>
#include "unpack_lookup.h"
#include <stdlib.h>
#include <stdio.h>


float **lookupTable[9];
static char lookupTableValid[] = { 0, 0, 0, 0, 0, 0, 0, 0 };

float ***getLookupTable(){
	return lookupTable;
}

void makeLookup(int bitspersamp, int firstSampleIsMSB, int isSigned) {
	unsigned char bitmask;
	int negVal;
	int negConv;
	int i, j, k;
	int x;

	if (lookupTableValid[bitspersamp]) {
		freeLookup(bitspersamp);
	}
	// make the lookup table...
	switch (bitspersamp) {
	case 1:
		bitmask = 0x01;
		negVal = 1;
		negConv = 2;
		break;
	case 2:
		bitmask = 0x03;
		negVal = 2;
		negConv = 4;
		break;
	case 4:
		bitmask = 0x0F;
		negVal = 8;
		negConv = 16;
		break;
	case 8:
		bitmask = 0xFF;
		negVal = 128;
		negConv = 256;
		break;
	default:
		fprintf(stderr,"phx: Cannot deal with %d bit data!\n",bitspersamp);
		exit(9);
	}
	x = 8/(bitspersamp) - 1;
	lookupTable[bitspersamp] = malloc(sizeof(float*)* 8/(bitspersamp));
	for (i = 0; i < 8/(bitspersamp); i++) {
		lookupTable[bitspersamp][i] = malloc(256*sizeof(float));
		for (j = 0; j < 256; j++) {//=bitspersamp){
			k = j;
			if (firstSampleIsMSB)
				k = k >> ((x-i)*bitspersamp);
			else
				k = k >> ((i)*bitspersamp);
			k = k & bitmask;
			if (isSigned && k >= negVal)
				k = k - negConv;
			lookupTable[bitspersamp][i][j] = (float)k;
			//printf("%02x\t%02x\t%d %d\n",i,j, k,negVal);
		}
	}
	lookupTableValid[bitspersamp] = 1;
	if (isSigned)
		lookupTableValid[bitspersamp] = lookupTableValid[bitspersamp] | 2;
	if (firstSampleIsMSB)
		lookupTableValid[bitspersamp] = lookupTableValid[bitspersamp] | 4;
}

void freeLookup(int bitspersamp) {
	int i;

	for (i = 0; i < 8/(bitspersamp); i++) {
		free(lookupTable[bitspersamp][i]);
	}
	free(lookupTable[bitspersamp]);
	lookupTableValid[bitspersamp] = 0;

}

char checkLookup(int bitspersamp, int firstSampleIsMSB, int isSigned) {
	// !((!P ^ Q) V (P ^ !Q))
//	printf("lut: %d\n",lookupTableValid[bitspersamp]);
	if (( (!(lookupTableValid[bitspersamp] & 2) && isSigned)
			|| ((lookupTableValid[bitspersamp] & 2) && !isSigned))) {
		return 0;
	}
	if (( (!(lookupTableValid[bitspersamp] & 4) && firstSampleIsMSB)
			|| ((lookupTableValid[bitspersamp] & 4) && !firstSampleIsMSB))) {

		return 0;
	}

	return lookupTableValid[bitspersamp];

}
