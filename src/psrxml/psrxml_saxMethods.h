#ifndef PSRXML_SAXMETHODS_H_
#define PSRXML_SAXMETHODS_H_
#include "psrxml.h"

#include <unistd.h>
#include <math.h>
#include <string.h>

#include <libxml/globals.h>
#include <libxml/xmlerror.h>
#include <libxml/parser.h>
#include <libxml/parserInternals.h> /* only for xmlNewInputFromFile() */
#include <libxml/tree.h>
#include <libxml/debugXML.h>
#include <libxml/xmlmemory.h>

int phx_isWhitespace(char c);
char* phx_trim(char* str);

typedef struct phx_reader_state {
	psrxml* theContent;
	dataFile *currentDataFile;
	process *currentProcess;
	extraKey *currentExtraKey;

	int statusFlags;
	char* charBuffer;
	int charBuffer_length; /* Size of the buffer array */
	int nchar; /* Number of valid chars in the buffer */

	// some atributes:
	char units[PHX_UNITS_LENGTH];
	char type[PHX_TYPE_LENGTH];
	char href[PHX_HREF_LENGTH];
	char name[PHX_HREF_LENGTH];
	int channel_number;
	int block_number;

	char is_in_flaggedChannel;
	char is_in_blockHeader;
	char is_in_telescope;
	char is_in_receiver;
	char is_in_backend;

	coordinate coord;
	az_el azel;

} phx_reader_state;

//static void phx_startElement(void *vs, const xmlChar *name, const xmlChar **atts);
//char phx_processElement(const char* name, phx_reader_state* state);
//static void phx_endElement(void *vs, const xmlChar *name);
//static void phx_endDocument(void* vs);
//static void phx_startDocument(void* vs);
//static void phx_characters(void *vs, const xmlChar *ch, int len);
//static void my_warning(void *user_data, const char *msg, ...) ;
//static void my_error(void *user_data, const char *msg, ...) ;
//static void my_fatalError(void *user_data, const char *msg, ...) ;


static void phx_startElement(void *vs, const xmlChar *name,
		const xmlChar **atts) {
	phx_reader_state* state; // The state
	int i;
	int attrcount;
	flaggedChannel* flaggedChannel_ptr;
	int version=0;

	state = (phx_reader_state*)vs; // set the state



	if (strcmp((char*)name, "telescope")==0)
		state->is_in_telescope=1;
	if (strcmp((char*)name, "receiver")==0)
		state->is_in_receiver=1;
	if (strcmp((char*)name, "backend")==0)
		state->is_in_backend=1;

	//if(state->currentExtraKey!=NULL)printf("Start: %s %d\n",(char*)name,state->currentExtraKey->attributes_length);

	attrcount = 0;

	state->units[0]='\0';
	state->type[0]='\0';
	state->href[0]='\0';
	strcpy(state->name, (char*)name);

	/* Read the attributes! */
	if ((char*)atts != NULL) {
		for (i = 0; ((char*)atts[i] != NULL); i++) {
			attrcount++;
		}
		for (i = 0; ((char*)atts[i] != NULL); i++) {
			if (strcmp((char*)atts[i], "units")==0) {
				// check for potential buffer overflow!
				if (strlen((char*)atts[++i]) > PHX_UNITS_LENGTH) {
					fprintf(stderr,"Error: units '%s' too long! (max size %d)\n",atts[i],PHX_UNITS_LENGTH);
				} else {
					strcpy(state->units, (char*)atts[i]);
				}
			} else if (strcmp((char*)atts[i], "type")==0) {
				// check for potential buffer overflow!
				if (strlen((char*)atts[++i]) > PHX_TYPE_LENGTH) {
					fprintf(stderr,"Error: type '%s' too long! (max size %d)\n",atts[i],PHX_TYPE_LENGTH);
				} else {
					strcpy(state->type, (char*)atts[i]);
				}
			} else if (strcmp((char*)atts[i], "href")==0) {
				// check for potential buffer overflow!
				if (strlen((char*)atts[++i]) > PHX_HREF_LENGTH) {
					fprintf(stderr,"Error: href '%s' too long! (max size %d)\n",atts[i],PHX_HREF_LENGTH);
				} else {
					strcpy(state->href, (char*)atts[i]);
				}
			} else if (strcmp((char*)atts[i], "name")==0) {
				// check for potential buffer overflow!
				if (strlen((char*)atts[++i]) > PHX_HREF_LENGTH) {
					fprintf(stderr,"Error: name '%s' too long! (max size %d)\n",atts[i],PHX_HREF_LENGTH);
				} else {
					strcpy(state->name, (char*)atts[i]);
				}
			} else if (strcmp((char*)atts[i], "version")==0) {
				version = atoi(phx_trim((char*)atts[++i]));
			} else if (strcmp((char*)atts[i], "channel_number")==0) {
				state->channel_number = atoi(phx_trim((char*)atts[++i]));
			} else if (strcmp((char*)atts[i], "block_number")==0) {
				state->block_number = atoi(phx_trim((char*)atts[++i]));
			}

		}
	}
	if (strcmp((char*)name, "psrxml")==0) {
		state->theContent->headerVersion = version;
	}

	if (strcmp((char*)name, "flagged_channel")==0) {
		state->is_in_flaggedChannel = 1;
		if (state->theContent->flaggedChannels_length < 1) {
			state->theContent->flaggedChannels = malloc(sizeof(flaggedChannel));
			state->theContent->flaggedChannels_length = 1;
		} else {
			state->theContent->flaggedChannels_length++;
			state->theContent->flaggedChannels = realloc(
					state->theContent->flaggedChannels, sizeof(flaggedChannel)
							*state->theContent->flaggedChannels_length);
		}
		// select the last flagged channel in the array... tasty pointer arithmatic!
		flaggedChannel_ptr = (state->theContent->flaggedChannels
				+state->theContent->flaggedChannels_length - 1);

		flaggedChannel_ptr->channelNumber = state->channel_number;
		flaggedChannel_ptr->comment = NULL;
		flaggedChannel_ptr->weight = 1;

	}

	if (strcmp((char*)name, "data")==0) {
		if (state->currentDataFile != NULL) {
			free(state->currentDataFile);
		}
		state->currentDataFile = malloc(sizeof(dataFile));
		state->currentDataFile->source = state->theContent;
		state->currentDataFile->isChannelInterleaved = 0;
		state->currentDataFile->extraKeys_length=0;
		state->currentDataFile->extraKeys = NULL;
		state->currentDataFile->firstSampleIsMostSignificantBit = 0;
		state->currentDataFile->isSigned = 0;
		state->currentDataFile->blockHeaderLength = 0;
		state->currentDataFile->blockHeaders_length = 0;
		state->currentDataFile->blockHeaders = NULL;
		state->currentDataFile->dataType[0]='\0';
	}
	if (strcmp((char*)name, "process")==0) {
		// new data block.
	}

	// free the extrakey

	if (state->currentExtraKey != NULL) {
		if (state->currentExtraKey->attributes_length > 0) {
			for (i = 0; i < state->currentExtraKey->attributes_length; i++) {
				free(state->currentExtraKey->attributes[i]);
			}
			free(state->currentExtraKey->attributes);
		}
		free(state->currentExtraKey);
		state->currentExtraKey = NULL;
	}
	// make the 'extrakey' in case we need it...

	state->currentExtraKey = malloc(sizeof(extraKey));
	strcpy(state->currentExtraKey->name, (char*)name);
	if ((char*)atts != NULL) {
		state->currentExtraKey->attributes = (char**)malloc(sizeof(char*)
				*attrcount);
		for (i = 0; i < attrcount; i++) {
			state->currentExtraKey->attributes[i] = (char*)malloc(sizeof(char)
					*(strlen((char*)atts[i])+1));
			strcpy(state->currentExtraKey->attributes[i], (char*)atts[i]);
		}
		state->currentExtraKey->attributes_length = attrcount;
	} else {
		state->currentExtraKey->attributes_length = 0;
	}

	// clear the char buffer:
	state->nchar = 0;

}

double correctFreqUnits(double aDouble, char* units, const char* name) {
	if (strcmp(units, "Hz")==0) {
		aDouble /= 1e6;
	} else if (strcmp(units, "kHz")==0) {
		aDouble /=1e3;
	} else if (strcmp(units, "GHz")==0) {
		aDouble *= 1e3;
	} else if (strcmp(units, "THz")==0) {
		aDouble *= 1e6;
	} else if (strcmp(units, "MHz")!=0) {
		fprintf(stderr,"Unknown freq units '%s' for %s\n",units,name);
	}
	return aDouble;
}

double correctTimeUnits(double aDouble, char* units, const char* name) {
	if (strcmp(units, "ns")==0) {
		aDouble /= 1.0e9;
	} else if (strcmp(units, "us")==0) {
		aDouble /= 1.0e6;
	} else if (strcmp(units, "ms")==0) {
		aDouble /=1.0e3;
	} else if (strcmp(units, "minutes")==0) {
		aDouble *= 60.0;
	} else if (strcmp(units, "hours")==0) {
		aDouble *= 3600.0;
	} else if (strcmp(units, "s")!=0) {
		fprintf(stderr,"Unknown time units '%s' for %s\n",units,name);
	}
	return aDouble;
}

double correctPosnUnits(double aDouble, char* units, const char* name) {

	if (strcmp(units, "arcseconds")==0) {
		aDouble /= 3600.0;
	} else if (strcmp(units, "arcminutes")==0) {
		aDouble /= 60.0;
	} else if (strcmp(units, "radians")==0) {
		aDouble *= 57.29577951;
	} else if (strcmp(units, "hours")==0) {
		aDouble = aDouble * 360.0/24.0;
	} else if (strcmp(units, "minutes")==0) {
		aDouble = aDouble * 360.0 / (60.0*24.0);
	} else if (strcmp(units, "seconds")==0) {
		aDouble = aDouble * 360.0 / (3600.0*24.0);
	} else if (strcmp(units, "degrees")!=0) {
		fprintf(stderr,"Unknown coordinate units '%s' for %s\n",units,name);
	}
	return aDouble;
}

int correctDataUnits(int aLong, char* units, const char* name) {
	if (strcmp(units, "kilobytes")==0) {
		aLong = aLong * 1024;
	} else if (strcmp(units, "megabytes")==0) {
		aLong = aLong * 1048576;
	} else if (strcmp(units, "gigabytes")==0) {
		aLong = aLong * 1073741824;
	} else if (strcmp(units, "bytes")!=0) {
		fprintf(stderr,"Unknown data size units '%s' for %s\n",units,name);
	}
	return aLong;
}

char phx_processElement(const char* name, phx_reader_state* state, char* content) {
	double aDouble;
	unsigned long long aLong;
	char *aString;
	int anInt;
	flaggedChannel* flaggedChannel_ptr;

	/* Receiver elements */
	if (state->is_in_receiver) {
		if (strcmp(name, "name")==0) {
			strcpy(state->theContent->receiver.name, content);
			return 1;
		}
		if (strcmp(name, "feed_polarisation_basis")==0) {
			if ((strcmp(content, "CIRCULAR")==0)||(strcmp(content, "circular")
					==0) ||(strcmp(content, "Circular")==0)) {
				state->theContent->receiver.hasCircularFeeds = 1;
			} else {
				state->theContent->receiver.hasCircularFeeds = 0;
			}
			return 1;
		}

		if (strcmp(name, "feed_handedness")==0) {
			if ((strcmp(content, "RIGHT")==0)||(strcmp(content, "right")==0)
					||(strcmp(content, "Right")==0)) {
				state->theContent->receiver.feedRightHanded = 1;
			} else {
				state->theContent->receiver.feedRightHanded = 0;
			}
			return 1;
		}

		if (strcmp(name, "fwhm")==0) {
			aDouble = atof(content);
			aDouble = correctPosnUnits(aDouble, state->units, name);
			state->theContent->receiver.fwhm = aDouble;
			return 1;
		}

		if (strcmp(name, "number_of_polarisations")==0) {
			state->theContent->receiver.numberOfPolarisations = atoi(content);
			return 1;
		}
		if (strcmp(name, "feed_symetry")==0) {
			aDouble = atof(content);
			aDouble = correctPosnUnits(aDouble, state->units, name);
			state->theContent->receiver.feedSymetry = aDouble;
			return 1;
		}
		if (strcmp(name, "cal_phase")==0) {
			aDouble = atof(content);
			aDouble = correctPosnUnits(aDouble, state->units, name);
			state->theContent->receiver.calXYPhase = aDouble;
			return 1;
		}
	}

	/* Telescope elements */
	if (state->is_in_telescope) {
		if (strcmp(name, "name")==0) {
			strcpy(state->theContent->telescope.name, content);
			return 1;
		}
		if (strcmp(name, "zenith_limit")==0) {
			state->theContent->telescope.zenithLimit = atof(content);
			return 1;
		}
		if (strcmp(name, "longitude")==0) {
			state->theContent->telescope.longitude = atof(content);
			return 1;
		}
		if (strcmp(name, "lattitude")==0) {
			state->theContent->telescope.lattitude = atof(content);
			return 1;
		}
		if (strcmp(name, "x")==0) {
			state->theContent->telescope.x = atof(content);
			return 1;
		}
		if (strcmp(name, "y")==0) {
			state->theContent->telescope.y = atof(content);
			return 1;
		}
		if (strcmp(name, "z")==0) {
			state->theContent->telescope.z = atof(content);
			return 1;
		}
		if (strcmp(name, "sigproc_code")==0) {
			state->theContent->telescope.sigprocCode = atoi(content);
			return 1;
		}
		if (strcmp(name, "tempo_code")==0) {
			strcpy(state->theContent->telescope.tempoCode, content);
			return 1;
		}
		if (strcmp(name, "pulsarhunter_code")==0) {
			strcpy(state->theContent->telescope.pulsarhunterCode, content);
			return 1;
		}
	}

	/* Backend elements */
	if (state->is_in_backend) {
		if (strcmp(name, "name")==0) {
			strcpy(state->theContent->backend.name, content);
			return 1;
		}
		if (strcmp(name, "cross_phase")==0) {
			if ((strcmp(content, "REVERSE")==0)
					||(strcmp(content, "reverse")==0) ||(strcmp(content,
					"Reverse")==0)) {
				state->theContent->backend.reverseCrossPhase =1;
			} else {
				state->theContent->backend.reverseCrossPhase =0;
			}
			return 1;
		}
		if (strcmp(name, "sideband")==0) {
			if ((strcmp(content, "UPPER")==0)||(strcmp(content, "upper")==0)
					||(strcmp(content, "Upper")==0)) {
				state->theContent->backend.upperSideband =1;
			} else {
				state->theContent->backend.upperSideband =0;
			}
			return 1;
		}
		if (strcmp(name, "sigproc_code")==0) {
			state->theContent->backend.sigprocCode = atoi(content);
			return 1;
		}
	}
	/* PSRXML elements */

	if (strcmp(name, "source_name")==0) {
		strcpy(state->theContent->sourceName, content);
		return 1;
	}

	if (strcmp(name, "source_name_centre_beam")==0) {
		strcpy(state->theContent->sourceNameCentreBeam, content);
		return 1;
	}

	if (strcmp(name, "day_of_observation")==0) {
		//	aDouble = atof(content);
		if (strcmp(state->units, "MJD")==0) {
			state->theContent->mjdObs = (int)atoi(content);
			return 1;
		} else {
			fprintf(stderr,"Unknown Date units '%s' for %s\n",state->units,name);
			return 0;
		}

	}

	if (strcmp(name, "midnight_to_first_sample")==0) {
		aString = strtok(content, ".");
		aString = strtok(NULL,".");
		if (aString==NULL) {
			// if there are no dots, read as an integer
			sscanf(content, "%lld", &aLong);
			if (strcmp(state->units, "ns")!=0) {
				// we don't have ns as units, so convert...
				aLong = (unsigned long long)(correctTimeUnits((double)aLong,
						state->units, name) * 1e9);
			}
		} else {
			// else read as a float
			sscanf(content, "%lf", &aDouble);
			aLong = (unsigned long long)(correctTimeUnits(atof(content),
					state->units, name)*1e9);
		}

		state->theContent->timeToFirstSample = aLong;

		return 1;
	}

	if (strcmp(name, "epoch_first_sample")==0) {
		//	aDouble = atof(content);
		if (strcmp(state->units, "MJD")==0) {
			state->theContent->mjdObs = (int)atoi(strtok(content, "."));
			aString = strtok(NULL,".");
			if (aString!=NULL) {
				if (strlen(aString)> 16)
					aString[16]='\0';
				aLong = atoll(aString);
				state->theContent->timeToFirstSample = aLong * (long long)864;

				if (strlen(aString)> 11) {
					state->theContent->timeToFirstSample /= (long long)pow(10,
							strlen(aString)-11);
				} else {
					state->theContent->timeToFirstSample *= (long long)pow(10,
							11-strlen(aString));
				}
			}
		} else {
			state->theContent->timeToFirstSample
					= (unsigned long long)(correctTimeUnits(atof(content),
							state->units, name)*1e9);
		}
		return 1;
	}

	if (strcmp(name, "lst")==0) {
		strcpy(state->theContent->lst, content);
		return 1;
	}
	if (strcmp(name, "utc")==0) {
		strcpy(state->theContent->utc, content);
		return 1;
	}
	if (strcmp(name, "local_time")==0) {
		strcpy(state->theContent->localTime, content);
		return 1;
	}

	if (strcmp(name, "native_sample_rate")==0) {
		aDouble = atof(content);
		aDouble = correctTimeUnits(aDouble, state->units, name);
		state->theContent->nativeSampleRate = aDouble;
		return 1;
	}
	if (strcmp(name, "current_sample_interval")==0) {
		aDouble = atof(content);
		aDouble = correctTimeUnits(aDouble, state->units, name);
		state->theContent->currentSampleInterval = aDouble;
		return 1;
	}

	if (strcmp(name, "number_of_samples")==0) {
		state->theContent->numberOfSamples = atoi(content);
		return 1;
	}

	if (strcmp(name, "requested_obs_time")==0) {
		aDouble = atof(content);
		aDouble = correctTimeUnits(aDouble, state->units, name);
		state->theContent->requestedObsTime = aDouble;
		return 1;
	}

	if (strcmp(name, "actual_obs_time")==0) {
		aDouble = atof(content);
		state->theContent->actualObsTime = aDouble;
		return 1;
	}

	if (strcmp(name, "centre_freq_first_channel")==0) {
		aDouble = atof(content);
		aDouble = correctFreqUnits(aDouble, state->units, name);
		state->theContent->centreFreqCh1 = aDouble;
		return 1;
	}

	if (strcmp(name, "channel_offset")==0) {
		aDouble = atof(content);
		aDouble = correctFreqUnits(aDouble, state->units, name);
		state->theContent->freqOffset = aDouble;
		return 1;
	}

	if (strcmp(name, "number_of_channels")==0) {
		state->theContent->numberOfChannels = atoi(content);
		return 1;
	}

	if (strcmp(name, "flagged_channel")==0) {

		return 1;
	}

	if (strcmp(name, "reference_dm")==0) {
		aDouble = atof(content);
		state->theContent->referenceDm = aDouble;
		return 1;
	}

	if (strcmp(name, "comment")==0) {
		if (state->is_in_flaggedChannel) {

			if (state->is_in_blockHeader) {
				//@todo: block flaggedChannels
				return 1;

			} else {
				flaggedChannel_ptr = (state->theContent->flaggedChannels
						+state->theContent->flaggedChannels_length - 1);
				flaggedChannel_ptr->comment = malloc(sizeof(char)*state->nchar);
				strcpy(flaggedChannel_ptr->comment, content);
				flaggedChannel_ptr->comment = malloc(state->nchar*sizeof(char));
				return 1;

			}
		} else {
			state->theContent->comment = malloc(sizeof(char)*state->nchar);
			strcpy(state->theContent->comment, content);
			return 1;
		}
	}
	if (strcmp(name, "weight")==0) {
		if (state->is_in_flaggedChannel) {

			if (state->is_in_blockHeader) {
				//@todo: block flaggedChannels
			} else {
				flaggedChannel_ptr = (state->theContent->flaggedChannels
						+state->theContent->flaggedChannels_length - 1);
				flaggedChannel_ptr->weight = atof(content);
			}
			return 1;
		} else {
			return 0;
		}
	}
	if (strcmp(name, "start_paralactic_angle")==0) {
		aDouble = atof(content);
		state->theContent->startParalacticAngle = correctPosnUnits(aDouble,
				state->units, name);
		return 1;
	}
	if (strcmp(name, "end_paralactic_angle")==0) {
		aDouble = atof(content);
		state->theContent->endParalacticAngle = aDouble;
		return 1;
	}

	if (strcmp(name, "paralactic_angle_tracking")==0) {
		if ((strcmp(content, "TRUE") == 0) ||(strcmp(content, "true")==0)
				|| (strcmp(content, "True")==0)) {
			state->theContent->isParalacticAngleTracking = 1;
		} else {
			state->theContent->isParalacticAngleTracking = 0;
		}
		return 1;
	}

	if (strcmp(name, "ra")==0) {
		aDouble = atof(content);
		aDouble = correctPosnUnits(aDouble, state->units, name);
		state->coord.ra = aDouble;
		return 1;
	}
	if (strcmp(name, "dec")==0) {
		aDouble = atof(content);
		aDouble = correctPosnUnits(aDouble, state->units, name);
		state->coord.dec = aDouble;
		return 1;
	}
	if (strcmp(name, "position_error")==0) {
		aDouble = atof(content);
		aDouble = correctPosnUnits(aDouble, state->units, name);
		state->coord.posn_error = aDouble;
		return 1;
	}

	if (strcmp(name, "position_epoch")==0) {
		strcpy(state->coord.posn_epoch, content);
		return 1;
	}

	if (strcmp(name, "coordinate")==0) {
		return 1;
	}

	if (strcmp(name, "start_coordinate")==0) {
		state->theContent->startCoordinate.dec = state->coord.dec;
		state->theContent->startCoordinate.ra = state->coord.ra;
		state->theContent->startCoordinate.posn_error = state->coord.posn_error;

		strcpy(state->theContent->startCoordinate.posn_epoch,
				state->coord.posn_epoch);
		state->coord.ra = NAN;
		state->coord.dec = NAN;
		state->coord.posn_error = NAN;
		state->coord.posn_epoch[0] = '\0';
		return 1;
	}

	if (strcmp(name, "end_coordinate")==0) {
		state->theContent->endCoordinate.dec = state->coord.dec;
		state->theContent->endCoordinate.ra = state->coord.ra;
		state->theContent->endCoordinate.posn_error = state->coord.posn_error;
		strcpy(state->theContent->endCoordinate.posn_epoch,
				state->coord.posn_epoch);
		state->coord.ra = NAN;
		state->coord.dec = NAN;
		state->coord.posn_error = NAN;
		state->coord.posn_epoch[0] = '\0';

		return 1;
	}

	if (strcmp(name, "requested_coordinate")==0) {
		state->theContent->requestedCoordinate.dec = state->coord.dec;
		state->theContent->requestedCoordinate.ra = state->coord.ra;
		state->theContent->requestedCoordinate.posn_error
				= state->coord.posn_error;
		strcpy(state->theContent->requestedCoordinate.posn_epoch,
				state->coord.posn_epoch);
		state->coord.ra = NAN;
		state->coord.dec = NAN;
		state->coord.posn_error = NAN;
		state->coord.posn_epoch[0] = '\0';

		return 1;
	}

	if (strcmp(name, "az")==0) {
		aDouble = atof(content);
		aDouble = correctPosnUnits(aDouble, state->units, name);
		state->azel.az = aDouble;
		return 1;
	}
	if (strcmp(name, "el")==0) {
		aDouble = atof(content);
		aDouble = correctPosnUnits(aDouble, state->units, name);
		state->azel.el = aDouble;
		return 1;
	}
	if (strcmp(name, "start_telescope_position")==0) {
		state->theContent->startAzEl.az = state->azel.az;
		state->theContent->startAzEl.el = state->azel.el;
		return 1;
	}
	if (strcmp(name, "end_telescope_position")==0) {
		state->theContent->endAzEl.az = state->azel.az;
		state->theContent->endAzEl.el = state->azel.el;
		return 1;
	}
	//	if (strcmp(name, "receiver")==0) {
	//		//@todo: receiver
	//		strcpy(state->theContent->receiver.uri, state->href);
	//		return 1;
	//	}

	if (strcmp(name, "receiver_beam")==0) {
		state->theContent->receiverBeamNumber = atoi(content);
		return 1;
	}
	if (strcmp(name, "total_beams_recorded")==0) {
		state->theContent->totalBeamsRecorded = atoi(content);
		return 1;
	}
	if (strcmp(name, "sky_beam")==0) {
		state->theContent->skyBeamNumber = atoi(content);
		return 1;
	}
	//	if (strcmp(name, "backend")==0) {
	//		//@todo: receiver
	//		strcpy(state->theContent->backend.uri, state->href);
	//
	//		return 1;
	//	}

	if (strcmp(name, "recorded_polarisations")==0) {
		strcpy(state->theContent->recordedPol, content);
		state->theContent->nRecordedPol = strlen(content)/2;
		return 1;
	}
	if (strcmp(name, "observed_polarisations")==0) {
		strcpy(state->theContent->observedPol, content);
		return 1;
	}

	//	if (strcmp(name, "telescope")==0) {
	//		//@todo: receiver
	//		strcpy(state->theContent->telescope.uri, state->href);
	//
	//		return 1;
	//	}

	if (strcmp(name, "observing_programme")==0) {
		strcpy(state->theContent->observingProgramme, content);
		return 1;
	}
	if (strcmp(name, "observer_name")==0) {
		strcpy(state->theContent->observerName, content);
		return 1;
	}
	if (strcmp(name, "observation_type")==0) {
		strcpy(state->theContent->observationType, content);
		return 1;
	}
	if (strcmp(name, "observation_configuration")==0) {
		strcpy(state->theContent->observationType, content);
		return 1;
	}

	if (strcmp(name, "data_uid")==0) {
		if (state->currentDataFile == NULL) {
			fprintf(stderr,"PSRXML ERROR: datafile elements before <data> tag\n");
			return 0;
		}

		strcpy(state->currentDataFile->uid, content);
		strcpy(state->currentDataFile->uid_alg, state->type);
		return 1;
	}

	if (strcmp(name, "data_checksum")==0) {
		if (state->currentDataFile == NULL) {
			fprintf(stderr,"PSRXML ERROR: datafile elements before <data> tag\n");
			return 0;
		}

		strcpy(state->currentDataFile->checksum, content);
		strcpy(state->currentDataFile->checksum_type, state->type);

		return 1;
	}

	if (strcmp(name, "filename")==0) {
		if (state->currentDataFile == NULL) {
			fprintf(stderr,"PSRXML ERROR: datafile elements before <data> tag\n");
			return 0;
		}
		strcpy(state->currentDataFile->filename, content);

		return 1;
	}

	if (strcmp(name, "data_type")==0) {
		if (state->currentDataFile == NULL) {
			fprintf(stderr,"PSRXML ERROR: datafile elements before <data> tag\n");
			return 0;
		}
		strcpy(state->currentDataFile->dataType, content);

		return 1;
	}

	if (strcmp(name, "block_sha1")==0) {
		if (state->currentDataFile == NULL) {
			fprintf(stderr,"PSRXML ERROR: datafile elements before <data> tag\n");
			return 0;
		}
		if (state->block_number >= state->currentDataFile->blockHeaders_length) {
			fprintf(stderr,"PSRXML ERROR: <block_sha1> for invalid block %d\n",state->block_number);
			return 0;
		}
		if (strlen(content)> 40) {
			fprintf(stderr,"PSRXML ERROR: <block_sha1> for block %d is invalid, must be < 40 chars\n",state->block_number);
			return 0;
		}
		strcpy(
				state->currentDataFile->blockHeaders[state->block_number].sha1_hash,
				content);
		state->currentDataFile->blockHeaders[state->block_number].has_sha1_hash
				= 1;
		return 1;
	}

	if (strcmp(name, "endian")==0) {
		if (state->currentDataFile == NULL) {
			fprintf(stderr,"PSRXML ERROR: datafile elements before <data> tag\n");
			return 0;
		}

		if (strcmp(content, "LITTLE_ENDIAN")==0) {
			state->currentDataFile->endian=LITTLE;
		} else if (strcmp(content, "BIG_ENDIAN")==0) {
			state->currentDataFile->endian=BIG;
		} else {
			state->currentDataFile->endian=INDEPENDANT;
		}
		return 1;
	}

	if (strcmp(name, "header_length")==0) {
		if (state->currentDataFile == NULL) {
			fprintf(stderr,"PSRXML ERROR: datafile elements before <data> tag\n");
			return 0;
		}
		aLong = atoi(content);
		aLong = correctDataUnits(aLong, state->units, name);
		state->currentDataFile->headerLength = (int)aLong;
		return 1;
	}

	if (strcmp(name, "block_size")==0) {
		if (state->currentDataFile == NULL) {
			fprintf(stderr,"PSRXML ERROR: datafile elements before <data> tag\n");
			return 0;
		}
		aLong = atoi(content);
		aLong = correctDataUnits(aLong, state->units, name);
		state->currentDataFile->blockLength = aLong;
		return 1;
	}

	if (strcmp(name, "block_header_length")==0) {
		if (state->currentDataFile == NULL) {
			fprintf(stderr,"PSRXML ERROR: datafile elements before <data> tag\n");
			return 0;
		}
		aLong = atoi(content);
		aLong = correctDataUnits(aLong, state->units, name);
		state->currentDataFile->blockHeaderLength = (int)aLong;
		return 1;
	}

	if (strcmp(name, "bits_per_sample")==0) {
		if (state->currentDataFile == NULL) {
			fprintf(stderr,"PSRXML ERROR: datafile elements before <data> tag\n");
			return 0;
		}

		state->currentDataFile->bitsPerSample = atoi(content);
		return 1;
	}
	if (strcmp(name, "number_of_blocks")==0) {
		if (state->currentDataFile == NULL) {
			fprintf(stderr,"PSRXML ERROR: datafile elements before <data> tag\n");
			return 0;
		}

		state->currentDataFile->blockHeaders_length = atoi(content);

		// make the block header arrays.
		state->currentDataFile->blockHeaders = malloc(sizeof(dataBlockHeader)
				* state->currentDataFile->blockHeaders_length);
		for (anInt = 0; anInt < state->currentDataFile->blockHeaders_length; anInt++) {
			state->currentDataFile->blockHeaders[anInt].has_sha1_hash = 0;
		}

		return 1;
	}

	if (strcmp(name, "data_order")==0) {
		if (state->currentDataFile == NULL) {
			fprintf(stderr,"PSRXML ERROR: datafile elements before <data> tag\n");
			return 0;
		}
		if(strcmp(content, "TFP")==0){
			state->currentDataFile->isChannelInterleaved = 1;
		} else if(strcmp(content, "FTP")==0){
			state->currentDataFile->isChannelInterleaved = 0;
		} else{
			fprintf(stderr,"ERROR: PSRXML C library can not yet handle data orders other than FTP or TFP.\n");
			exit(1);
		}
		return 1;
	}
	if (strcmp(name, "bit_order_first_sample_in")==0) {
		if (state->currentDataFile == NULL) {
			fprintf(stderr,"PSRXML ERROR: datafile elements before <data> tag\n");
			return 0;
		}
		if ((strcmp(content, "MSB")==0)||(strcmp(content, "msb")==0)) {
			state->currentDataFile->firstSampleIsMostSignificantBit = 1;
		} else {
			state->currentDataFile->firstSampleIsMostSignificantBit = 0;
		}
		return 1;
	}
	if (strcmp(name, "signed")==0) {
		if (state->currentDataFile == NULL) {
			fprintf(stderr,"PSRXML ERROR: datafile elements before <data> tag\n");
			return 0;
		}
		if ((strcmp(content, "TRUE")==0)||(strcmp(content, "True")==0)
				||(strcmp(content, "true")==0)) {
			state->currentDataFile->isSigned = 1;
		} else {
			state->currentDataFile->isSigned = 0;
		}
		return 1;
	}

	if (strcmp(name, "data")==0) {
		// add a new data file to the array!
		if (state->theContent->files_length < 1) {
			state->theContent->files = malloc(sizeof(dataFile*));
			state->theContent->files_length = 1;
		} else {
			state->theContent->files_length++;
			state->theContent->files = realloc(state->theContent->files,
					sizeof(dataFile)*state->theContent->files_length);
		}
		// re-point the last element
		state->theContent->files[state->theContent->files_length-1]
				= state->currentDataFile;
		// make the current data file null so that it isn't free'd
		state->currentDataFile = NULL;
	}

	return 0;
}

static void phx_endElement(void *vs, const xmlChar *name) {
	phx_reader_state* state; // the state
	char processed;
	char* content;
	int i;
	//	printf("End: %s\n",(char*)name);

	state = (phx_reader_state*)vs; // fill in the state
	processed = 0;
	/* NUL Terminate the string! */
	state->charBuffer[(state->nchar)]='\0';
	state->nchar++;

	// trim the content...
	content = phx_trim(state->charBuffer);

	// try and process the element...
	processed = phx_processElement((char*)name, state, content);

	//@todo: Fix the extra keys, and make extrakeys appear inside other tags... (harder)
	if (!processed && state->currentExtraKey != NULL) {

		if (state->currentDataFile == NULL) {
			if (state->theContent->extraKeys_length < 1) {
				state->theContent->extraKeys = malloc(sizeof(extraKey*));
				state->theContent->extraKeys_length = 1;

			} else {
				state->theContent->extraKeys_length++;
				state->theContent->extraKeys = realloc(
						state->theContent->extraKeys, sizeof(extraKey*)
								*state->theContent->extraKeys_length);
			}

			state->theContent->extraKeys[state->theContent->extraKeys_length-1]
					= state->currentExtraKey;
			state->currentExtraKey = NULL;

		} else {
			if (state->currentDataFile->extraKeys_length < 1) {
				state->currentDataFile->extraKeys = malloc(sizeof(extraKey*));
				state->currentDataFile->extraKeys_length = 1;

			} else {
				state->currentDataFile->extraKeys_length++;
				state->currentDataFile->extraKeys = realloc(
						state->currentDataFile->extraKeys, sizeof(extraKey*)
								*state->currentDataFile->extraKeys_length);
			}

			state->currentDataFile->extraKeys[state->currentDataFile->extraKeys_length-1]
					= state->currentExtraKey;
			state->currentExtraKey = NULL;
		}

	}

	if (state->currentExtraKey != NULL) {
		if (state->currentExtraKey->attributes_length > 0) {
			for (i = 0; i < state->currentExtraKey->attributes_length; i++) {
				free(state->currentExtraKey->attributes[i]);
			}
			free(state->currentExtraKey->attributes);
		}
		free(state->currentExtraKey);
		state->currentExtraKey = NULL;
	}
	state->currentExtraKey = NULL;

	if (strcmp((char*)name, "flaggedChannel")==0) {
		state->is_in_flaggedChannel=0;
	}

	if (strcmp((char*)name, "telescope")==0)
		state->is_in_telescope=0;
	if (strcmp((char*)name, "receiver")==0)
		state->is_in_receiver=0;
	if (strcmp((char*)name, "backend")==0)
		state->is_in_backend=0;

}

static void phx_endDocument(void* vs) {
	phx_reader_state* state;
	state = (phx_reader_state*)vs;
	if (state->charBuffer!=NULL) {

		free(state->charBuffer);
	}
}

static void phx_startDocument(void* vs) {

	phx_reader_state* state;
	state = (phx_reader_state*)vs;
	state->currentExtraKey=NULL;
	state->currentProcess = NULL;
	state->currentDataFile = NULL;
	state->coord.ra = NAN;
	state->coord.dec = NAN;
	state->coord.posn_error = NAN;
	state->coord.posn_epoch[0] = '\0';

	clearPsrXmlDoc(state->theContent);

	state->nchar = 0;
	state->units[0]='\0';
	state->type[0] ='\0';
	state->href[0] = '\0';

	state->is_in_flaggedChannel=0;
	state->is_in_blockHeader = 0;
	state->is_in_telescope=0;
	state->is_in_receiver=0;
	state->is_in_backend = 0;

	state->charBuffer_length = 1024*1024;

	state->charBuffer = (char*)malloc(sizeof(char)*state->charBuffer_length);
	if (state->charBuffer==NULL) {
		fprintf(stderr,"psrXML: Error, could not allocate memory");

		state->charBuffer_length = 80;
		state->charBuffer
				= (char*)malloc(sizeof(char)*state->charBuffer_length);

	}
}

static void phx_characters(void *vs, const xmlChar *ch, int len) {
	phx_reader_state* state;
	int i;

	state = (phx_reader_state*)vs;

	if (len + state->nchar + 1 > state->charBuffer_length) {
		printf("Expand %d\n", state->charBuffer_length);

		/* We will overflow our buffer! */

		state->charBuffer= realloc(state->charBuffer, sizeof(char)
				*(state->charBuffer_length + len*2));
		state->charBuffer_length = state->charBuffer_length + len*2;

	}

	//printf("%d %d %d\n",len,state->nchar,state->sizeOfCharBuffer);
	for (i = 0; i < len; i++) {
		state->charBuffer[state->nchar + i] = ((char*)ch)[i];
	}
	state->nchar+=len;
	//	printf("ch %d %d\n",state->nchar,len);

}

static void my_warning(void *user_data, const char *msg, ...) {
	va_list args;
	va_start(args, msg);
	fprintf(stderr,"\nXML parser warning: %s\n\n",va_arg(args,char*));
	va_end(args);
}
static void my_error(void *user_data, const char *msg, ...) {
	va_list args;
	va_start(args, msg);
	fprintf(stderr,"\nXML parser error: %s\n\n",va_arg(args,char*));
	va_end(args);
}
static void my_fatalError(void *user_data, const char *msg, ...) {
	va_list args;
	va_start(args, msg);
	fprintf(stderr,"\nXML parser FATAL error: %s\n\n",va_arg(args,char*));
	va_end(args);
}

int phx_isWhitespace(char c) {
	return (c==' ' || c=='\t' || c=='\n' || c=='\r' );
}

char* phx_trim(char* str) {
	int i;
	char* end;

	i = 0;

	while (phx_isWhitespace(*str)) {
		str++;
	}

	end = str;

	while (*end != '\0') {
		end++;
	}

	end--;

	while (end > str && (phx_isWhitespace(*end))) {
		end--;
	}

	*(end+1) = '\0';

	return str;
}

// The SAX handler definition
static xmlSAXHandler psrxml_xml_handler = { NULL, /* internalSubset */
NULL, /* isStandalone */
NULL, /* hasInternalSubset */
NULL, /* hasExternalSubset */
NULL, /* resolveEntity */
NULL, /* getEntity */
NULL, /* entityDecl */
NULL, /* notationDecl */
NULL, /* attributeDecl */
NULL, /* elementDecl */
NULL, /* unparsedEntityDecl */
NULL, /* setDocumentLocator */
phx_startDocument, /* startDocument */
phx_endDocument, /* endDocument */
phx_startElement, /* startElement */
phx_endElement, /* endElement */
NULL, /* reference */
phx_characters, /* characters */
NULL, /* ignorableWhitespace */
NULL, /* processingInstruction */
NULL, /* comment */
my_warning, /* xmlParserWarning */
my_error, /* xmlParserError */
my_fatalError, /* xmlParserError */
NULL, /* getParameterEntity */
NULL, /* cdataBlock; */
NULL, /* externalSubset; */
1, NULL, NULL, /* startElementNs */
NULL, /* endElementNs */
NULL /* xmlStructuredErrorFunc */
};
#endif /*PSRXML_SAXMETHODS_H_*/
