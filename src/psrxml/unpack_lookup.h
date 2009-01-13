#ifndef UNPACK_LOOKUP_H_
#define UNPACK_LOOKUP_H_
#ifdef __cplusplus
extern "C"{ 
#endif
	float ***getLookupTable();
	
void makeLookup(int bitspersamp,int firstSampleIsMSB, int isSigned);

void freeLookup(int bitspersamp);

char checkLookup(int bitspersamp,int firstSampleIsMSB, int isSigned);

#ifdef __cplusplus
}
#endif
#endif /*UNPACK_LOOKUP_H_*/
