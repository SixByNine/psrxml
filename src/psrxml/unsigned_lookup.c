#include <config.h>
#include "unpack_lookup.h"
#include <stdlib.h>
#include <stdio.h>


unsigned char **lookupUnsignedTable[9];
static char lookupUnsignedTableValid[] = { 0, 0, 0, 0, 0, 0, 0, 0 };

unsigned char ***getUnsignedLookupTable(){
	return lookupUnsignedTable;
}

void makeUnsignedLookup(int bitspersamp, int firstSampleIsMSB, int isSigned) {
	unsigned char bitmask;
	int negVal;
	int negConv;
	int i, j, k;
	int x;

	if (lookupUnsignedTableValid[bitspersamp]) {
		freeUnsignedLookup(bitspersamp);
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
	lookupUnsignedTable[bitspersamp] = malloc(sizeof(unsigned char*)* 8/(bitspersamp));
	for (i = 0; i < 8/(bitspersamp); i++) {
		lookupUnsignedTable[bitspersamp][i] = malloc(256*sizeof(unsigned char));
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
			lookupUnsignedTable[bitspersamp][i][j] = (unsigned char)k;
			//printf("%02x\t%02x\t%d %d\n",i,j, k,negVal);
		}
	}
	lookupUnsignedTableValid[bitspersamp] = 1;
	if (isSigned)
		lookupUnsignedTableValid[bitspersamp] = lookupUnsignedTableValid[bitspersamp] | 2;
	if (firstSampleIsMSB)
		lookupUnsignedTableValid[bitspersamp] = lookupUnsignedTableValid[bitspersamp] | 4;
}

void freeUnsignedLookup(int bitspersamp) {
	int i;

	for (i = 0; i < 8/(bitspersamp); i++) {
		free(lookupUnsignedTable[bitspersamp][i]);
	}
	free(lookupUnsignedTable[bitspersamp]);
	lookupUnsignedTableValid[bitspersamp] = 0;

}

char checkUnsignedLookup(int bitspersamp, int firstSampleIsMSB, int isSigned) {
	// !((!P ^ Q) V (P ^ !Q))
//	printf("lut: %d\n",lookupTableValid[bitspersamp]);
	if (( (!(lookupUnsignedTableValid[bitspersamp] & 2) && isSigned)
			|| ((lookupUnsignedTableValid[bitspersamp] & 2) && !isSigned))) {
		return 0;
	}
	if (( (!(lookupUnsignedTableValid[bitspersamp] & 4) && firstSampleIsMSB)
			|| ((lookupUnsignedTableValid[bitspersamp] & 4) && !firstSampleIsMSB))) {

		return 0;
	}

	return lookupUnsignedTableValid[bitspersamp];

}
