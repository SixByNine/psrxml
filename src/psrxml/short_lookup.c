#include <stdlib.h>
#include <stdio.h>
#include <config.h>
#include "unpack_lookup.h"
#include "short_lookup.h"


unsigned short **lookupShortTable[9];
static char lookupShortTableValid[] = { 0, 0, 0, 0, 0, 0, 0, 0 };

unsigned short ***getShortLookupTable(){
	return lookupShortTable;
}

void freeShortLookup(int bitspersamp) {
	int i;

	for (i = 0; i < 8/(bitspersamp); i++) {
		free(lookupShortTable[bitspersamp][i]);
	}
	free(lookupShortTable[bitspersamp]);
	lookupShortTableValid[bitspersamp] = 0;

}

void makeShortLookup(int bitspersamp, int firstSampleIsMSB, int isSigned) {
	unsigned short bitmask;
	int negVal;
	int negConv;
	int i, j, k;
	int x;

	if (lookupShortTableValid[bitspersamp]) {
		freeShortLookup(bitspersamp);
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
	lookupShortTable[bitspersamp] = malloc(sizeof(unsigned short*)* 8/(bitspersamp));
	for (i = 0; i < 8/(bitspersamp); i++) {
		lookupShortTable[bitspersamp][i] = malloc(256*sizeof(unsigned short));
		for (j = 0; j < 256; j++) {//=bitspersamp){
			k = j;
			if (firstSampleIsMSB)
				k = k >> ((x-i)*bitspersamp);
			else
				k = k >> ((i)*bitspersamp);
			k = k & bitmask;
			if (isSigned && k >= negVal)
				k = k - negConv;
			if (isSigned){
				k+= 128;
			}
			lookupShortTable[bitspersamp][i][j] = (unsigned short)k;
			//printf("%02x\t%02x\t%d %d\n",i,j, k,negVal);
		}
	}
	lookupShortTableValid[bitspersamp] = 1;
	if (isSigned)
		lookupShortTableValid[bitspersamp] = lookupShortTableValid[bitspersamp] | 2;
	if (firstSampleIsMSB)
		lookupShortTableValid[bitspersamp] = lookupShortTableValid[bitspersamp] | 4;
}

char checkShortLookup(int bitspersamp, int firstSampleIsMSB, int isSigned) {
	// !((!P ^ Q) V (P ^ !Q))
//	printf("lut: %d\n",lookupTableValid[bitspersamp]);
	if (( (!(lookupShortTableValid[bitspersamp] & 2) && isSigned)
			|| ((lookupShortTableValid[bitspersamp] & 2) && !isSigned))) {
		return 0;
	}
	if (( (!(lookupShortTableValid[bitspersamp] & 4) && firstSampleIsMSB)
			|| ((lookupShortTableValid[bitspersamp] & 4) && !firstSampleIsMSB))) {

		return 0;
	}

	return lookupShortTableValid[bitspersamp];

}
