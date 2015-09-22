#include <config.h>
#include <psrxml.h>
#include <stdlib.h>

void closeTag(FILE* file, int* indent, char* tagName) {
	int i;
	(*indent)--;

	for (i = 0; i < *indent; i++)
		fprintf(file, "\t");
	fprintf(file, "</%s>\n", tagName);
}

void emptyTag(FILE* file, int* indent, char* tagName, char** attributes,
		int attributes_length) {
	int i, j;
	attributes_length*=2;
	for (i = 0; i < *indent; i++)
		fprintf(file, "\t");
	fprintf(file, "<%s", tagName);
	i=0;
	while (i < attributes_length) {
		j=i+1;
		fprintf(file, " %s='%s'", attributes[i], attributes[j]);
		i+=2;
	}
	fprintf(file, " />\n");
}

void writeTagAttr(FILE* file, int* indent, char* tagName, char** attributes,
		int attributes_length, char* content) {
	int i, j;
	attributes_length*=2;
	for (i = 0; i < *indent; i++)
		fprintf(file, "\t");
	i=0;
	fprintf(file, "<%s", tagName);
	while (i < attributes_length) {
		j=i+1;
		fprintf(file, " %s='%s'", attributes[i], attributes[j]);
		i+=2;
	}
	fprintf(file, ">");
	if (content==NULL) {
		fprintf(file, "\n");
	} else {
		fprintf(file, content);
		fprintf(file, "</%s>\n", tagName);
		(*indent)--;
	}
	(*indent)++;
}

void openTagAttr(FILE* file, int* indent, char* tagName, char** attributes,
		int attributes_length) {
	writeTagAttr(file, indent, tagName, attributes, attributes_length, NULL);
}

void openTag(FILE* file, int* indent, char* tagName) {
	openTagAttr(file, indent, tagName, NULL, 0);
}

void writeTag(FILE* file, int* indent, char* tagName, char* content) {
	writeTagAttr(file, indent, tagName, NULL, 0, content);
}

void writePsrXml(psrxml* header, char* fileName) {
	FILE* file;
	int indent;
	int i, block;
	char *attr[64];
	char content[PHX_HREF_LENGTH];

	for (i=0; i<64; i++)
		attr[i] = malloc(PHX_HREF_LENGTH);

	indent = 0;

	file = fopen(fileName, "w");
	fprintf(file, "<?xml version='1.0'?>\n");

	strcpy(attr[0], "version");
	sprintf(attr[1], "%d", PHX_HEADER_VERSION);
	openTagAttr(file, &indent, "psrxml", attr, 1); // <pxh>

	writeTag(file, &indent, "source_name", header->sourceName);

	if (strlen(header->sourceNameCentreBeam))
		writeTag(file, &indent, "source_name_centre_beam",
				header->sourceNameCentreBeam);

	//epoch_first_sample	
	strcpy(attr[0], "units");
	strcpy(attr[1], "MJD");
	sprintf(content, "%d", header->mjdObs);
	writeTagAttr(file, &indent, "day_of_observation", attr, 1, content);
	strcpy(attr[1], "ns");
	sprintf(content, "%lld", header->timeToFirstSample);
	writeTagAttr(file, &indent, "midnight_to_first_sample", attr, 1, content);

	//native_sample_rate OPTIONAL
	if (!isnan(header->nativeSampleRate)) {
		sprintf(content, "%lf", header->nativeSampleRate*1e6);
		strcpy(attr[1], "us");
		writeTagAttr(file, &indent, "native_sample_rate", attr, 1, content);
	}

	//current_sample_interval
	sprintf(content, "%lf", header->currentSampleInterval*1e6);
	strcpy(attr[1], "us");
	writeTagAttr(file, &indent, "current_sample_interval", attr, 1, content);

	writeTag(file, &indent, "lst", header->lst);

	writeTag(file, &indent, "utc", header->utc);

	writeTag(file, &indent, "local_time", header->localTime);

	//number_of_samples
	sprintf(content, "%d", header->numberOfSamples);
	writeTag(file, &indent, "number_of_samples", content);

	//requested_obs_time OPTIONAL
	if (!isnan(header->requestedObsTime)) {
		sprintf(content, "%lf", header->requestedObsTime);
		strcpy(attr[1], "s");
		writeTagAttr(file, &indent, "requested_obs_time", attr, 1, content);
	}

	//actual_obs_time
	sprintf(content, "%lf", header->actualObsTime);
	strcpy(attr[1], "s");
	writeTagAttr(file, &indent, "actual_obs_time", attr, 1, content);

	//centre_freq_first_channel
	sprintf(content, "%lf", header->centreFreqCh1);
	strcpy(attr[1], "MHz");
	writeTagAttr(file, &indent, "centre_freq_first_channel", attr, 1, content);

	//channel_offset
	sprintf(content, "%lf", header->freqOffset);
	strcpy(attr[1], "MHz");
	writeTagAttr(file, &indent, "channel_offset", attr, 1, content);

	//number_of_channels
	sprintf(content, "%d", header->numberOfChannels);
	writeTag(file, &indent, "number_of_channels", content);

	//reference_dm OPTIONAL
	if(header->referenceDm!=0){
		sprintf(content, "%f", header->referenceDm);
		writeTag(file, &indent, "reference_dm", content);
	}

	//flagged_channel OPTIONAL
	for (i=0; i<header->flaggedChannels_length; i++) {
		strcpy(attr[0], "channel_number");
		sprintf(attr[1], "%d", header->flaggedChannels[i].channelNumber);
		openTagAttr(file, &indent, "flagged_channel", attr, 1);

		if (header->flaggedChannels[i].comment!=NULL)
			writeTag(file, &indent, "comment",
					header->flaggedChannels[i].comment);
		sprintf(content, "%f", header->flaggedChannels[i].weight);
		if (abs(header->flaggedChannels[i].weight-1)< 0.0001)
			writeTag(file, &indent, "weight", content);

		closeTag(file, &indent, "flagged_channel");

	}

	//start_paralactic_angle OPTIONAL
	if (!isnan(header->startParalacticAngle)) {
		sprintf(content, "%lf", header->startParalacticAngle);
		strcpy(attr[0], "units");
		strcpy(attr[1], "degrees");
		writeTagAttr(file, &indent, "start_paralactic_angle", attr, 1, content);
	}

	//end_paralactic_angle OPTIONAL
	if (!isnan(header->endParalacticAngle)) {
		sprintf(content, "%lf", header->endParalacticAngle);
		strcpy(attr[0], "units");
		strcpy(attr[1], "degrees");
		writeTagAttr(file, &indent, "end_paralactic_angle", attr, 1, content);
	}

	if (header->isParalacticAngleTracking) {
		writeTag(file, &indent, "paralactic_angle_tracking", "TRUE");
	} else {
		writeTag(file, &indent, "paralactic_angle_tracking", "FALSE");
	}

	//start_coordinate
	openTag(file, &indent, "start_coordinate");
	openTag(file, &indent, "coordinate");

	strcpy(attr[0], "units");
	strcpy(attr[1], "degrees");
	sprintf(content, "%lf", header->startCoordinate.ra);
	writeTagAttr(file, &indent, "ra", attr, 1, content);
	sprintf(content, "%lf", header->startCoordinate.dec);
	writeTagAttr(file, &indent, "dec", attr, 1, content);
	writeTag(file, &indent, "position_epoch",
			header->startCoordinate.posn_epoch);
	//position_error OPTIONAL
	if (!isnan(header->startCoordinate.posn_error)) {
		sprintf(content, "%lf", header->startCoordinate.posn_error);
		writeTagAttr(file, &indent, "position_error", attr, 1, content);
	}
	closeTag(file, &indent, "coordinate");

	closeTag(file, &indent, "start_coordinate");

	//end_coordinate OPTIONAL
	if (!isnan(header->endCoordinate.ra)) {
		openTag(file, &indent, "end_coordinate");
		openTag(file, &indent, "coordinate");

		strcpy(attr[0], "units");
		strcpy(attr[1], "degrees");
		sprintf(content, "%lf", header->endCoordinate.ra);
		writeTagAttr(file, &indent, "ra", attr, 1, content);
		sprintf(content, "%lf", header->endCoordinate.dec);
		writeTagAttr(file, &indent, "dec", attr, 1, content);
		writeTag(file, &indent, "position_epoch",
				header->endCoordinate.posn_epoch);
		//position_error OPTIONAL
		if (!isnan(header->startCoordinate.posn_error)) {
			sprintf(content, "%lf", header->endCoordinate.posn_error);
			writeTagAttr(file, &indent, "position_error", attr, 1, content);
		}
		closeTag(file, &indent, "coordinate");

		closeTag(file, &indent, "end_coordinate");
	}

	//requested_position OPTIONAL
	if (!isnan(header->requestedCoordinate.ra)) {
		openTag(file, &indent, "requested_coordinate");
		openTag(file, &indent, "coordinate");

		strcpy(attr[0], "units");
		strcpy(attr[1], "degrees");

		sprintf(content, "%lf", header->requestedCoordinate.ra);
		writeTagAttr(file, &indent, "ra", attr, 1, content);
		sprintf(content, "%lf", header->requestedCoordinate.dec);
		writeTagAttr(file, &indent, "dec", attr, 1, content);
		writeTag(file, &indent, "position_epoch",
				header->requestedCoordinate.posn_epoch);
		//position_error OPTIONAL
		if (!isnan(header->startCoordinate.posn_error)) {
			sprintf(content, "%lf", header->requestedCoordinate.posn_error);
			writeTagAttr(file, &indent, "position_error", attr, 1, content);
		}
		closeTag(file, &indent, "coordinate");

		closeTag(file, &indent, "requested_coordinate");
	}

	//start_telescope_position OPTIONAL
	if (!isnan(header->startAzEl.az)) {
		openTag(file, &indent, "start_telescope_position");
		strcpy(attr[0], "units");
		strcpy(attr[1], "degrees");
		sprintf(content, "%lf", header->startAzEl.az);
		writeTagAttr(file, &indent, "az", attr, 1, content);
		sprintf(content, "%lf", header->startAzEl.el);
		writeTagAttr(file, &indent, "el", attr, 1, content);
		closeTag(file, &indent, "start_telescope_position");
	}
	//end_telescope_position OPTIONAL
	if (!isnan(header->endAzEl.az)) {
		openTag(file, &indent, "end_telescope_position");
		strcpy(attr[0], "units");
		strcpy(attr[1], "degrees");
		sprintf(content, "%lf", header->endAzEl.az);
		writeTagAttr(file, &indent, "az", attr, 1, content);
		sprintf(content, "%lf", header->endAzEl.el);
		writeTagAttr(file, &indent, "el", attr, 1, content);
		closeTag(file, &indent, "end_telescope_position");

	}

	//reciever
	openTag(file, &indent, "receiver");
	strcpy(content, header->receiver.name);
	writeTag(file, &indent, "name", content);
	if (header->receiver.hasCircularFeeds) {
		writeTag(file, &indent, "feed_polarisation_basis", "CIRCULAR");
	} else {
		writeTag(file, &indent, "feed_polarisation_basis", "LINEAR");
	}
	if (header->receiver.feedRightHanded) {
		writeTag(file, &indent, "feed_handedness", "RIGHT");
	} else {
		writeTag(file, &indent, "feed_handedness", "LEFT");
	}
	sprintf(content, "%d", header->receiver.numberOfPolarisations);
	writeTag(file, &indent, "number_of_polarisations", content);

	strcpy(attr[0], "units");
	strcpy(attr[1], "degrees");
	sprintf(content, "%lf", header->receiver.calXYPhase);
	writeTagAttr(file, &indent, "cal_phase", attr, 1, content);
	sprintf(content, "%lf", header->receiver.feedSymmetry);
	writeTagAttr(file, &indent, "feed_symetry", attr, 1, content);
	sprintf(content, "%lf", header->receiver.fwhm);
	writeTagAttr(file, &indent, "fwhm", attr, 1, content);

	closeTag(file, &indent, "receiver");

	//reciever_beam OPTIONAL
	if (header->receiverBeamNumber> 0) {
		sprintf(content, "%d", header->receiverBeamNumber);
		writeTag(file, &indent, "receiver_beam", content);
	}

	// total_beams_recorded OPTIONA:
	if (header->totalBeamsRecorded> 0) {
		sprintf(content, "%d", header->totalBeamsRecorded);
		writeTag(file, &indent, "total_beams_recorded", content);
	}
	//sky_beam
	if (header->skyBeamNumber> 0) {
		sprintf(content, "%d", header->skyBeamNumber);
		writeTag(file, &indent, "sky_beam", content);
	}

	//backend
	openTag(file, &indent, "backend");
	strcpy(content, header->backend.name);
	writeTag(file, &indent, "name", content);
	if (header->backend.upperSideband) {
		writeTag(file, &indent, "sideband", "UPPER");
	} else {
		writeTag(file, &indent, "sideband", "LOWER");
	}
	if (header->backend.reverseCrossPhase) {
		writeTag(file, &indent, "cross_phase", "REVERSE");
	} else {
		writeTag(file, &indent, "cross_phase", "STANDARD");
	}
	sprintf(content, "%d", header->backend.sigprocCode);
	writeTag(file, &indent, "sigproc_code", content);

	closeTag(file, &indent, "backend");

	//recorded_polarisations
	strcpy(content, header->recordedPol);
	writeTag(file, &indent, "recorded_polarisations", content);

	//observed_polarisations OPTIONAL
	if (strlen(header->observedPol)> 0) {
		strcpy(content, header->observedPol);
		writeTag(file, &indent, "observed_polarisations", content);
	}

	//telescope
	openTag(file, &indent, "telescope");
	strcpy(content, header->telescope.name);
	writeTag(file, &indent, "name", content);
	strcpy(attr[0], "units");
	strcpy(attr[1], "degrees");
	sprintf(content, "%lf", header->telescope.zenithLimit);
	writeTagAttr(file, &indent, "zenith_limit", attr, 1, content);
	sprintf(content, "%lf", header->telescope.longitude);
	writeTagAttr(file, &indent, "longitude", attr, 1, content);
	sprintf(content, "%lf", header->telescope.latitude);
	writeTagAttr(file, &indent, "latitude", attr, 1, content);
	sprintf(content, "%lf", header->telescope.x);
	writeTag(file, &indent, "x", content);
	sprintf(content, "%lf", header->telescope.y);
	writeTag(file, &indent, "y", content);
	sprintf(content, "%lf", header->telescope.z);
	writeTag(file, &indent, "z", content);
	strcpy(content, header->telescope.tempoCode);
	writeTag(file, &indent, "tempo_code", content);
	sprintf(content, "%d", header->telescope.sigprocCode);
	writeTag(file, &indent, "sigproc_code", content);
	strcpy(content, header->telescope.pulsarhunterCode);
	writeTag(file, &indent, "pulsarhunter_code", content);

	closeTag(file, &indent, "telescope");

	//observing_programme OPTIONAL
	if (strlen(header->observingProgramme)> 0) {
		strcpy(content, header->observingProgramme);
		writeTag(file, &indent, "observing_programme", content);
	}

	//observer_name OPTIONAL
	if (strlen(header->observerName)> 0) {
		strcpy(content, header->observerName);
		writeTag(file, &indent, "observer_name", content);
	}

	//observation_type OPTIONAL
	if (strlen(header->observationType)> 0) {
		strcpy(content, header->observationType);
		writeTag(file, &indent, "observation_type", content);
	}

	//observation_configuration OPTIONAL
	if (strlen(header->observationConfiguration)> 0) {
		strcpy(content, header->observationConfiguration);
		writeTag(file, &indent, "observation_configuration", content);
	}

	//comment OPTIONAL
	if (header->comment != NULL) {
		strcpy(content, header->comment);
		writeTag(file, &indent, "comment", content);
	}

	//data OPTIONAL
	for (i = 0; i < header->files_length; i++) {
		openTag(file, &indent, "data");
		//data_uid
		if (header->files[i]->uid!=NULL) {
			strcpy(content, header->files[i]->uid);
			strcpy(attr[0], "type");
			strcpy(attr[1], header->files[i]->uid_alg);
			writeTagAttr(file, &indent, "data_uid", attr, 1, content);
		}
		//data_checksum OPTIONAL
		if (strlen(header->files[i]->checksum)> 0) {
			strcpy(content, header->files[i]->checksum);
			strcpy(attr[0], "type");
			strcpy(attr[1], header->files[i]->checksum_type);
			writeTagAttr(file, &indent, "data_checksum", attr, 1, content);

		}

		//filename OPTIONAL
		if (strlen(header->files[i]->filename)> 0) {
			strcpy(content, header->files[i]->filename);
			writeTag(file, &indent, "filename", content);
		}

		//dataType OPTIONAL
		if (strlen(header->files[i]->filename)> 0) {
			strcpy(content, header->files[i]->dataType);
			writeTag(file, &indent, "data_type", content);
		}

		//endian
		if (header->files[i]->endian == LITTLE)
			strcpy(content, "LITTLE_ENDIAN");
		else if (header->files[i]->endian == BIG)
			strcpy(content, "BIG_ENDIAN");
		else
			strcpy(content, "INDEPENDANT");

		writeTag(file, &indent, "endian", content);

		strcpy(attr[0], "units");
		strcpy(attr[1], "bytes");
		//header_length
		sprintf(content, "%d", header->files[i]->headerLength);
		writeTagAttr(file, &indent, "header_length", attr, 1, content);

		//block_size
		sprintf(content, "%d", header->files[i]->blockLength);
		writeTagAttr(file, &indent, "block_size", attr, 1, content);

		//block_header_length
		sprintf(content, "%d", header->files[i]->blockHeaderLength);
		writeTagAttr(file, &indent, "block_header_length", attr, 1, content);

		//bits_per_sample
		sprintf(content, "%d", header->files[i]->bitsPerSample);
		writeTag(file, &indent, "bits_per_sample", content);

		//channel_interleaved
		if (header->files[i]->isChannelInterleaved) {
			writeTag(file, &indent, "data_order", "TFP");
		} else {
			writeTag(file, &indent, "data_order", "FTP");
		}
		
		//first_sample_is_msb
		if (header->files[i]->firstSampleIsMostSignificantBit) {
			writeTag(file, &indent, "bit_order_first_sample_in", "MSB");
		} else {
			writeTag(file, &indent, "bit_order_first_sample_in", "LSB");
		}
		//signed
		if (header->files[i]->isSigned) {
			writeTag(file, &indent, "signed", "TRUE");
		} else {
			writeTag(file, &indent, "signed", "FALSE");
		}
		if (header->files[i]->blockHeaders_length> 0) {
			sprintf(content, "%d", header->files[i]->blockHeaders_length);
			writeTag(file, &indent, "number_of_blocks", content);

			for (block = 0; block < header->files[i]->blockHeaders_length; block++) {
				strcpy(attr[0], "block_number");
				sprintf(attr[1], "%d", block);
				openTagAttr(file, &indent, "block_header", attr, 1);
				if (header->files[i]->blockHeaders[block].has_sha1_hash) {
					strcpy(content,
							header->files[i]->blockHeaders[block].sha1_hash);
					writeTag(file, &indent, "block_sha1", content);
				}
				closeTag(file, &indent, "block_header");
			}
		}
		closeTag(file, &indent, "data");
	}

	closeTag(file, &indent, "psrxml");
	fprintf(file, "\n");
	fclose(file);
	for (i=0; i<64; i++)
		free(attr[i]);

}
