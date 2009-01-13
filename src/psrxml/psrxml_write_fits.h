#ifndef PSRXML_WRITE_FITS_H_
#define PSRXML_WRITE_FITS_H_
#ifdef FITS

void psrxml_write_fits_hdr(psrxml* header, char* header_file,int fileNum, char* data_file);
void psrxml_append_fits_data(void* data, char* data_file, int nbytes);






#endif
#endif /*PSRXML_WRITE_FITS_H_*/
