#ifndef PSRXML_H_
#define PSRXML_H_

#ifndef NAN
#define NAN (0.0/0.0)
#endif

#define PHX_TYPE_LENGTH 40
#define PHX_HREF_LENGTH 1024
#define PHX_UNITS_LENGTH 40

#ifdef HAVE_OPENSSL_SHA_H
#define USE_OPENSSL
#endif

#define PHX_HEADER_VERSION 1
#define PSRXML_IO_VERSION 101

#include <stdio.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef struct extraKey {

	char name[80];

	char **attributes; //  array of strings
	int attributes_length;

	char *content; // a string

} extraKey;

typedef struct flaggedChannel {
	int channelNumber;
	float weight;
	char* comment;
} flaggedChannel;

typedef struct coordinate {
	double ra; //degrees
	double dec; //degrees
	double posn_error; //degrees
	char posn_epoch[PHX_TYPE_LENGTH];

} coordinate;

typedef struct az_el {
	double az;
	double el;

} az_el;

enum endianness {BIG,LITTLE,INDEPENDANT};

typedef struct telescope_t {
	char name[PHX_HREF_LENGTH];
	//	char uri[PHX_HREF_LENGTH];
	double longitude, latitude, zenithLimit;
	double x, y, z;
	int sigprocCode;
	char tempoCode[80];
	char pulsarhunterCode[PHX_HREF_LENGTH];
} telescope_t;

typedef struct backend_t {
	char name[PHX_HREF_LENGTH];
	//	char uri[PHX_HREF_LENGTH];
	int sigprocCode;
	char upperSideband;
	char reverseCrossPhase;
} backend_t;

typedef struct receiver_t {
	char name[PHX_HREF_LENGTH];
	//	char uri[PHX_HREF_LENGTH];
	char hasCircularFeeds;
	int numberOfPolarisations;
	char feedRightHanded;
	double feedSymmetry;
	double fwhm;
	double calXYPhase;

} receiver_t;

typedef struct dataBlockHeader {
	char has_sha1_hash;
	char sha1_hash[41];

} dataBlockHeader;

typedef struct dataFile {
	struct psrxml *source;

	char filename[PHX_HREF_LENGTH];
	char dataType[PHX_HREF_LENGTH];

	char uid[PHX_HREF_LENGTH];
	char uid_alg[PHX_HREF_LENGTH];
	char checksum[PHX_HREF_LENGTH];
	char checksum_type[PHX_HREF_LENGTH];

	enum endianness endian;
	int headerLength;
	int blockLength;
	int blockHeaderLength;

	int bitsPerSample;

	char isChannelInterleaved;
	char firstSampleIsMostSignificantBit;
	char isSigned;

	extraKey **extraKeys;
	int extraKeys_length;

	FILE *file;
	int currentBlockNumber;

	int blockHeaders_length;
	dataBlockHeader* blockHeaders;

} dataFile;

typedef struct process {
	char softwareName[PHX_HREF_LENGTH];
	char softwareVersion[PHX_HREF_LENGTH];
	long long timeOfProcessing;
	char machine[PHX_HREF_LENGTH];
	char location[PHX_HREF_LENGTH];
	char user[PHX_HREF_LENGTH];

} process;

typedef struct psrxml {

	/*
	 * Time units: seconds
	 * Freq Units: MHz
	 * Posn Units: deg
	 */

	int headerVersion;
	char sourceName[PHX_HREF_LENGTH];
	char sourceNameCentreBeam[PHX_HREF_LENGTH];
	char catReference[PHX_HREF_LENGTH];

	int mjdObs;
	unsigned long long timeToFirstSample; // nanoseconds

	char utc[80];
	char lst[80];
	char localTime[80];

	double nativeSampleRate;
	double currentSampleInterval; //sec
	unsigned int numberOfSamples;

	double requestedObsTime;
	double actualObsTime;

	double centreFreqCh1;
	double freqOffset;
	int numberOfChannels;

	flaggedChannel *flaggedChannels;
	int flaggedChannels_length;

	coordinate startCoordinate;
	coordinate endCoordinate;
	coordinate requestedCoordinate; // Central beam posn

	double startParalacticAngle;
	double endParalacticAngle;
	char isParalacticAngleTracking;

	az_el startAzEl;
	az_el endAzEl;

	char observingProgramme[PHX_HREF_LENGTH];
	char observerName[PHX_HREF_LENGTH];
	char observationType[PHX_HREF_LENGTH];
	char observationConfiguration[PHX_HREF_LENGTH];

	char telescopeIdentifyingString[PHX_HREF_LENGTH];
	char receiverIdentifyingString[PHX_HREF_LENGTH];
	char backendIdentifyingString[PHX_HREF_LENGTH];

	char telescopeConfigString[PHX_HREF_LENGTH];
	char receiverConfigString[PHX_HREF_LENGTH];
	char backendConfigString[PHX_HREF_LENGTH];

	receiver_t receiver;
	int receiverBeamNumber;

	int totalBeamsRecorded;
	int skyBeamNumber;

	backend_t backend;

	char recordedPol[PHX_TYPE_LENGTH];
	char observedPol[PHX_TYPE_LENGTH];
	int nRecordedPol;

	telescope_t telescope;

	float referenceDm;

	char *comment;

	dataFile **files;
	int files_length;

	process** processes;
	int processes_length; // Length of the array
	int numberOfProcesses; // number of processes in array.


	extraKey **extraKeys;
	int extraKeys_length;

} psrxml;

int readPsrXml(psrxml *output, char* filename);

int readReceiverXml(receiver_t *output, char* filename);
int readTelescopeXml(telescope_t *output, char* filename);
int readBackendXml(backend_t *output, char* filename);

void readPsrxmlExternalElements(psrxml* header);

int readPsrXmlNextDataBlock(dataFile* dataFile, unsigned char** buffer_ptr);
int readPsrXmlNextDataBlockIntoExistingArray(dataFile *dataFile,
		unsigned char* buffer);
int readPsrXmlSkipNBlocks(dataFile* dataFile, int nblocks);

#ifdef USE_OPENSSL
char
		psrxml_checkHash(dataFile* dataFile, unsigned char* buffer,
				unsigned int bufsize, int blocknum);
char psrxml_getHash(dataFile* dataFile, unsigned char* buffer, char* hashStr,
	unsigned int bufsize);
#endif

char testPsrXml(psrxml *header);

int readPsrXMLPrepDataFile(dataFile *dataFile, const char* filename);

void freePsrXml(psrxml* source);
void writePsrXml(psrxml* header, char* fileName);

void clearPsrXmlDoc(psrxml *doc);

void unpackDataChunk(unsigned char* raw, float* outData, psrxml* header,
				int fileNum, int nSamps, int nsampStart, int nsampEnd,
				char swapChannels);
void unpackDataChunk_1to8bit_toshort(unsigned char* raw, unsigned short* outdata, int nbits,
		        int nchans, unsigned int nsamps, char fsInMSB, char isSigned,
			        char isChannelInterleaved, int nsampStart, int nsampEnd,
				        char swapChannels);
void unpackToChannels(float* indata, float** out, int nchans, int nsamp);

void zapDataChunk(float* outData, psrxml* header, int fileNum, int nSamps);
void deg2sex(double angle, char* string, int ndec);
void deg2hms(double angle, char* string, int ndec);


#ifdef __cplusplus
}
#endif
#endif /*PSRXML_H_*/
