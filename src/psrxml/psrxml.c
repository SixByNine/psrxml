#include <pthread.h>

#include "psrxml.h"
#include "psrxml_saxMethods.h"
#include "unpack_lookup.h"
#include "psrxml_xml_writing.h"


#include <string.h>
#include <stdio.h>
#include <math.h>

#include <errno.h>

#ifdef USE_OPENSSL
#include <openssl/sha.h>
#include <openssl/md5.h>

#endif

int readPsrXml(psrxml *output, char* filename) {
	int v = -1;
	// declare variables...
	phx_reader_state state; //the state for the reader

	// initialise the state
	state.theContent = output;

	//call the parser:
	v = xmlSAXUserParseFile(&psrxml_xml_handler, &state, filename);
	//readPsrxmlExternalElements(output);
	return v;

}

int readPsrXMLPrepDataFile(dataFile *dataFile, const char* filename) {
	int err;
	dataFile->file = fopen(filename, "rb");

	if (dataFile->file==NULL) {
		fprintf(stderr,"psrxml: file '%s' cannot be read\n",filename);
		return -1;
	}

	if ((err=fseek(dataFile->file, dataFile->headerLength, SEEK_SET))) {

		fprintf(stderr,"psrxml: file could not be wound forward for reading in data block\n");
		fprintf(stderr,strerror(err));
		return -1;
	}

	dataFile->currentBlockNumber = 0;

	return 0;
}

int readPsrXmlSkipNBlocks(dataFile* dataFile, int nblocks) {
	int err;
	if ((err=fseek(dataFile->file, nblocks*(dataFile->blockLength
			+ dataFile->blockHeaderLength), SEEK_CUR))) {
		fprintf(stderr,"psrxml: file could not be wound for reading in data block\n");
		fprintf(stderr,strerror(err));
		return -1;
	}

	return 0;
}

int readPsrXmlNextDataBlock(dataFile *dataFile, unsigned char** buffer_ptr) {
	unsigned char* buffer;

	buffer = *buffer_ptr = malloc(dataFile->blockLength);

	if (buffer == NULL) {
		fprintf(stderr,"psrxml: Memory could not be allocated for reading in data block\n");
		return -1;
	}
	return readPsrXmlNextDataBlockIntoExistingArray(dataFile, buffer);
}

int readPsrXmlNextDataBlockIntoExistingArray(dataFile *dataFile,
		unsigned char* buffer) {

	int read;

	int err;

	read = 0;

	if (dataFile->currentBlockNumber < 0) {
		if ((err=fseek(dataFile->file, dataFile->headerLength, SEEK_SET))) {
			fprintf(stderr,"psrxml: file could not be wound forward for reading in data block\n");
			fprintf(stderr,strerror(err));
			return -1;
		}
		dataFile->currentBlockNumber = 0;
	}

	if ((err=fseek(dataFile->file, dataFile->blockHeaderLength, SEEK_CUR))) {
		fprintf(stderr,"psrxml: file could not be wound forward for reading in data block\n");
		fprintf(stderr,strerror(err));
		return -1;
	}
	while (!feof(dataFile->file) && read < dataFile->blockLength) {
		read += fread(buffer+read, 1, dataFile->blockLength-read,
				dataFile->file);
	}

#ifdef USE_OPENSSL

	//fprintf(stderr,"%s\n",hashStr);
	if (!psrxml_checkHash(dataFile, buffer, dataFile->currentBlockNumber)) {
		fprintf(stderr,"Warning, bad data block %d in file %s\n",dataFile->currentBlockNumber,dataFile->filename);
	}

#endif

	dataFile->currentBlockNumber++;
	return read;

}
#ifdef USE_OPENSSL

char psrxml_getHash(dataFile* dataFile, unsigned char* buffer, char* hashStr,
		int blockNumber) {
	unsigned char hash[SHA_DIGEST_LENGTH];
	char *ptr;
	int i;

	SHA1(buffer, dataFile->blockLength, hash);
	ptr = hashStr;
	for (i = 0; i < SHA_DIGEST_LENGTH; i++) {
		sprintf(ptr, "%02x", hash[i]);
		ptr+=2;

	}
	return 1;
}

char psrxml_checkHash(dataFile* dataFile, unsigned char* buffer, int blockNumber) {
	char hashStr[SHA_DIGEST_LENGTH*2+1];
	//	printf("testy1  %d %d %d\n",blockNumber,dataFile->blockHeaders_length,dataFile->blockHeaders[blockNumber].has_sha1_hash);

	if (blockNumber < dataFile->blockHeaders_length
			&& dataFile->blockHeaders[blockNumber].has_sha1_hash) {
		//		printf("testy2\n");

		if (psrxml_getHash(dataFile, buffer, hashStr, blockNumber)) {
			//			printf("testy3\n");

			if (strcmp(dataFile->blockHeaders[blockNumber].sha1_hash, hashStr)
					!=0) {
				//				printf("test4\n");

				// a bad data block... print error
				return 0;
			} else
				return 1;
		}
	}
	return 2;
}

#endif

int count=0;

char testPsrXml(psrxml *header) {
	int version;

	version = header->headerVersion;

	if (version > PHX_HEADER_VERSION) {
		fprintf(stderr,"warning, file header version %d greater than software version %d.\n",version,PHX_HEADER_VERSION);
	}

	// test version 1
	// require sourcename
	if (strlen(header->sourceName) < 1) {
		fprintf(stderr,"V1 REQUIRED ELEMENT: source_name\n");
		return 1;
	} else {
		printf("source name: %s\n", header->sourceName);
	}
	// require obs epoch
	if (header->mjdObs ==0 || header->timeToFirstSample==0) {
		fprintf(stderr,"V1 REQUIRED ELEMENT: epoch_first_sample\n");
		return 1;
	} else {
		printf("epoch: %d  +%lld ns\n", header->mjdObs,
				header->timeToFirstSample);
	}

	// require current sample rate
	if (header->currentSampleInterval==NAN) {
		fprintf(stderr,"V1 REQUIRED ELEMENT: current_sample_interval\n");
		return 1;
	} else {
		printf("sample interval: %lf s\n", header->currentSampleInterval);
	}

	// require number of samples
	if (header->numberOfSamples < 1) {
		fprintf(stderr,"V1 REQUIRED ELEMENT: number_of_samples\n");
		return 1;
	} else {
		printf("number of samples: %d\n", header->numberOfSamples);
	}

	// require freq ch1
	if (header->centreFreqCh1==NAN) {
		fprintf(stderr,"V1 REQUIRED ELEMENT: centre_freq_first_channel\n");
		return 1;
	} else {
		printf("centre freq ch 1: %lf MHz\n", header->centreFreqCh1);
	}

	// require freq ch1
	if (isnan(header->freqOffset)) {
		fprintf(stderr,"V1 REQUIRED ELEMENT: channel_offset\n");
		return 1;
	} else {
		printf("freq offset: %lf MHz\n", header->freqOffset);
	}

	// require nchans
	if (isnan(header->freqOffset)) {
		fprintf(stderr,"V1 REQUIRED ELEMENT: number_of_channels\n");
		return 1;
	} else {
		printf("number of chans: %d\n", header->numberOfChannels);
		printf("band: %lf - %lf\n", header->centreFreqCh1-header->freqOffset
				/2.0, header->centreFreqCh1-header->freqOffset/2.0
				+ (header->numberOfChannels)*header->freqOffset);
	}

	// require start Coordinate
	if (isnan(header->startCoordinate.ra) || isnan(header->startCoordinate.dec)
			|| strlen(header->startCoordinate.posn_epoch)<1) {
		fprintf(stderr,"V1 REQUIRED ELEMENT: start_coordinate\n");
		return 1;
	} else {
		printf("start position : %lf, %lf (deg)\n", header->startCoordinate.ra,
				header->startCoordinate.dec);
		printf("Epoch: %s\n", header->startCoordinate.posn_epoch);
		if (isnan(header->startCoordinate.posn_error)) {
			printf("Posn error: NOT PROVIDED\n");
		} else {
			printf("Posn error: %lf deg\n", header->startCoordinate.posn_error);
		}
	}
	// require telescope ID
	if (strlen(header->telescope.name) < 1) {
		fprintf(stderr,"V1 REQUIRED ELEMENT: telescope\n");
		return 1;

	} else {
		printf("Telescope: %s\n", header->telescope.name);

	}

	// require reciever ID
	if (strlen(header->receiver.name) < 1) {
		fprintf(stderr,"V1 REQUIRED ELEMENT: receiver\n");
		return 1;
	} else {
		printf("Receiver: %s\n", header->receiver.name);

	}

	// require backend ID
	if (strlen(header->backend.name) < 1) {
		fprintf(stderr,"V1 REQUIRED ELEMENT: backend\n");
		return 1;

	} else {
		printf("Backend: %s\n", header->backend.name);

	}

	printf("Compliant to version 1.\n");

	return 0;
}

void freePsrXml(psrxml* source) {
	int i, j, k;

	if (source->comment!=NULL) {
		free(source->comment);
	}

	if (source->extraKeys_length > 0) {
		for (i=0; i < source->extraKeys_length; i++) {
			printf("\n'%s'\n", source->extraKeys[i]->content);
			free(source->extraKeys[i]->content);
			for (j=0; j< source->extraKeys[i]->attributes_length; j++) {
				free(source->extraKeys[i]->attributes[j]);
			}
			free(source->extraKeys[i]->attributes);
			free(source->extraKeys[i]);
		}
		free(source->extraKeys);
	}

	if (source->files_length > 0) {
		for (k=0; k < source->files_length; k++) {
			// free any extrakeys if set.
			if (source->extraKeys_length > 0) {
				for (i=0; i < source->files[k]->extraKeys_length; i++) {
					free(source->files[k]->extraKeys[i]->content);
					for (j=0; j
							< source->files[k]->extraKeys[i]->attributes_length; j++) {
						free(source->files[k]->extraKeys[i]->attributes[j]);
					}
					free(source->files[k]->extraKeys[i]->attributes);
					free(source->files[k]->extraKeys[i]);
				}
				free(source->files[k]->extraKeys);
			}
			if (source->files[k]->file!=NULL) {
				fclose(source->files[k]->file);
				source->files[k]->file = NULL;
			}
			free(source->files[k]);
		}
		free(source->files);
	}

	if (source->flaggedChannels_length > 0) {
		for (i=0; i < source->flaggedChannels_length; i++) {
			free(source->flaggedChannels[i].comment);
		}
		free(source->flaggedChannels);
	}

	if (source->processes_length > 0) {
		for (i = 0; i < source->processes_length; i++) {
			free(source->processes[i]);
		}
		free(source->processes);
	}

	free(source);

}

psrxml* clonePsrXml(psrxml* source) {

	return NULL;
}

void clearPsrXmlDoc(psrxml *doc) {
	doc->actualObsTime=NAN;
	strcpy(doc->backend.name, "UNKNOWN");
	doc->backend.upperSideband=0;
	doc->backend.reverseCrossPhase=0;
	doc->backend.sigprocCode=-1;

	doc->sourceNameCentreBeam[0]='\0';
	doc->catReference[0]='\0';
	doc->centreFreqCh1=NAN;
	doc->comment=NULL;
	doc->currentSampleInterval=NAN;
	doc->endAzEl.az=NAN;
	doc->startAzEl.el=NAN;
	doc->endCoordinate.ra=NAN;
	doc->endCoordinate.dec=NAN;
	doc->endCoordinate.posn_error=NAN;
	doc->endCoordinate.posn_epoch[0]='\0';
	doc->endParalacticAngle=NAN;
	doc->extraKeys=NULL;
	doc->extraKeys_length=0;
	doc->files=NULL;
	doc->files_length=0;
	doc->flaggedChannels=NULL;
	doc->flaggedChannels_length=0;
	doc->freqOffset=NAN;
	doc->isParalacticAngleTracking=0;
	doc->mjdObs=0;
	doc->nRecordedPol=0;
	doc->nativeSampleRate=NAN;
	doc->numberOfChannels=0;
	doc->numberOfProcesses=0;
	doc->numberOfSamples=0;
	doc->observerName[0]='\0';
	doc->observingProgramme[0]='\0';
	doc->observationType[0]='\0';
	doc->observedPol[0]='\0';
	doc->isParalacticAngleTracking = 0;
	doc->processes=NULL;
	doc->processes_length=0;
	doc->receiverBeamNumber=0;
	strcpy(doc->receiver.name, "UNKNOWN");
	doc->receiver.calXYPhase=0;
	doc->receiver.feedSymetry=0;
	doc->receiver.hasCircularFeeds=0;
	doc->receiver.numberOfPolarisations=0;
	doc->recordedPol[0]='\0';
	doc->requestedCoordinate.ra=NAN;
	doc->requestedCoordinate.dec=NAN;
	doc->requestedCoordinate.posn_error=NAN;
	doc->requestedCoordinate.posn_epoch[0]='\0';
	doc->requestedObsTime=NAN;
	doc->skyBeamNumber=0;
	doc->sourceName[0]='\0';
	doc->startAzEl.az=NAN;
	doc->startAzEl.el=NAN;
	doc->startCoordinate.ra=NAN;
	doc->startCoordinate.dec=NAN;
	doc->startCoordinate.posn_error=NAN;
	doc->startCoordinate.posn_epoch[0]='\0';
	doc->startParalacticAngle=NAN;
	strcpy(doc->telescope.name, "UNKNOWN");
	doc->telescope.pulsarhunterCode[0]='\0';
	doc->telescope.sigprocCode=-1;
	doc->telescope.tempoCode[0]='\0';
	doc->telescope.x=0;
	doc->telescope.y=0;
	doc->telescope.z=0;
	doc->telescope.longitude = 0;
	doc->telescope.lattitude = 0;
	doc->telescope.zenithLimit = 0;

	doc->referenceDm=0;

	doc->timeToFirstSample=0;
	doc->totalBeamsRecorded=0;
	doc->headerVersion=0;

	doc->utc[0]='\0';
	doc->lst[0]='\0';
	doc->localTime[0]='\0';
	doc->observationConfiguration[0]='\0';

	doc->telescopeIdentifyingString[0]='\0';
	doc->receiverIdentifyingString[0]='\0';
	doc->backendIdentifyingString[0]='\0';

	doc->telescopeConfigString[0]='\0';
	doc->receiverConfigString[0]='\0';
	doc->backendConfigString[0]='\0';

}


void zapDataChunk(float* outData, psrxml* header, int fileNum, int nSamps) {
	int chan, samp, flag;
	float weight;
	for (flag = 0; flag < header->flaggedChannels_length; flag++) {
		chan = header->flaggedChannels[flag].channelNumber;
		weight = header->flaggedChannels[flag].weight;
		if (chan >= header->numberOfChannels)
			continue;
		for (samp=0; samp < nSamps; samp++) {
			outData[chan*nSamps + samp] *= weight;
		}
	}

}
