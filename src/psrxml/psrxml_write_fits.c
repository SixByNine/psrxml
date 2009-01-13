#ifdef FITS
#include <qfits.h>
#include "psrxml.h"
#include "psrxml_write_fits.h"
#include <stdio.h>

void psrxml_write_fits_hdr(psrxml* header, char* header_file,int fileNum, char* data_file){
	
	qfits_header* fits_header;
	FILE* outfile;
	char str[80];
	int npix;
	
	fits_header = qfits_header_new();
	qfits_header_append(fits_header,"SIMPLE","T","Fits Format",NULL);
	

	
	qfits_header_append(fits_header,"BITPIX","8","Bits per sample",NULL);
	qfits_header_append(fits_header,"NAXIS","1","number of data axes",NULL);
	npix = (header->numberOfSamples * header->files[fileNum]->bitsPerSample) /8;
	sprintf(str,"%d",npix);
	qfits_header_append(fits_header,"NAXIS1",str,"length of data axis 1",NULL);

	qfits_header_append(fits_header,"SRC_NAME",header->sourceName,"Source Name",NULL);
	
	sprintf(str,"%f",header->startCoordinate.ra);
	qfits_header_append(fits_header,"RA",str,"Right Ascention",NULL);
	sprintf(str,"%f",header->startCoordinate.dec);
	qfits_header_append(fits_header,"DEC",str,"Declination",NULL);

	// end the header!
	qfits_header_append(fits_header,"END",NULL,NULL,NULL);

	
	outfile = fopen(data_file,"w");
	qfits_header_dump(fits_header,outfile);
	fclose(outfile);

	qfits_zeropad(data_file);
	
	
	strcpy(header->files[fileNum]->filename,data_file);
	header->files[fileNum]->headerLength = 2880;
	header->files[fileNum]->blockHeaderLength = 0;
	header->files[fileNum]->blockLength = header->numberOfChannels;

	
	writePsrXml(header,header_file);
}

void psrxml_append_fits_data(void* data, char* data_file, int nbytes){
	FILE* outfile;
	
	outfile = fopen(data_file,"a");
	fwrite(data,1,nbytes,outfile);
	fclose(outfile);
	
	
}



#endif
