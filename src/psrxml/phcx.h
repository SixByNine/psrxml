#ifndef PHCX_H_
#define PHCX_H_
#ifdef __cplusplus
extern "C" {
#endif

typedef struct phcx_header{
	char telescope[20];
	char sourceID[80];
	double ra; /* Degrees */
	double dec; /* Degrees */
	double centreFreq; /* MHz */
	double bandwidth; /* MHz */
	double mjdStart;
	double observationLength; /* Seconds */

}phcx_header;

typedef struct phcx_snr_block{
	double**** block;
	double* dmIndex;
	double* periodIndex;
	double* accnIndex;
	double* jerkIndex;
	int ndm;
	int nperiod;
	int naccn;
	int njerk;

} phcx_SNRBlock;

typedef struct phcx_section{
	char* name;
	double bestTopoPeriod;
	double bestBaryPeriod;
	double bestDm;
	double bestAccn;
	double bestJerk;
	double bestSnr;
	double bestWidth;
	double tsamp;

	float** subints;
	int nsubints;
	float** subbands;
	int nsubbands;
	float* pulseProfile;
	int nbins;

	phcx_SNRBlock snrBlock;

	int nextrakey;
	char** extrakey;
	char** extravalue;



}phcx_section;

typedef struct phcx{

	phcx_header header;
	phcx_section* sections;
	int nsections;
	int nextrakey;
	char** extrakey;
	char** extravalue;

}phcx;


typedef struct phcx_reader_state{
	phcx* theContent;
	phcx_section *currentSection;
	int statusFlags;
	char* charBuffer;
	int sizeOfCharBuffer; /* Size of the buffer array */
	int nchar; /* Number of valid chars in the buffer */

	char units[40];
	int nVals;
	int nBins;
	int nSub;
	char name[100];
	char format[10];
	char key[128];
	double min;
	double max;

}phcx_reader_state;

void free_phcx(phcx* thePhcx);
phcx* read_phcx(char* filename);
void write_phcx(char* fileName, phcx* cand);


#ifdef __cplusplus
}
#endif
#endif
