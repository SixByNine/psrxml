#ifndef SHORT_LOOKUP_H_
#define SHORT_LOOKUP_H_
#ifdef __cplusplus
extern "C"{ 
#endif

unsigned short ***getShortLookupTable();

void makeShortLookup(int bitspersamp, int firstSampleIsMSB, int isSigned);

void freeShortLookup(int bitspersamp);

char checkShortLookup(int bitspersamp, int firstSampleIsMSB, int isSigned);


#ifdef __cplusplus
}
#endif
#endif /*SHORT_LOOKUP_H_*/
