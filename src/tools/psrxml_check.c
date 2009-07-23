#include <stdio.h>
#include <psrxml.h>


int main(int argc, char** argv){
	psrxml* header;
	char infilename[1024];
	int read;
	char *darr;
	int chk;
	strcpy(infilename,argv[1]);

	header = (psrxml*)malloc(sizeof(psrxml));

	printf("Reading xml file %s\n",infilename);
	readPsrXml(header,infilename);

	
	printf("Checking data file %s\n",header->file[0]->fileName);

	printf("Block size=%d bytes\n",header->file[0]->blockLength);
	darr=malloc(header->file[0]->blockLength);

	read =1;
	while(read > 0){
		read = readPsrXmlNextDataBlockIntoExistingArray(header->file[0],darr);
		chk=psrxml_checkHash(dataFile, buffer, dataFile->currentBlockNumber);
		switch(chk){
			case 0:
				printf("BAD BLOCK: %d Hash mismatch\n",dataFile->currentBlockNumber);
				break;
			case 1:
				break;
			case 2:
				printf("Warning: no checksum for block %d \n",dataFile->currentBlockNumber);
				break;
		}
	}
	printf("Done\nThere were %d bad blocks.\n");
	free(darr);
	return 0;
}

