#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <psrxml.h>


int main(int argc, char** argv){
	psrxml* header;
	char infilesname[1024];
	int read;
	char *darr;
	int chk;
	strcpy(infilesname,argv[1]);

	header = (psrxml*)malloc(sizeof(psrxml));

	printf("Reading xml files %s\n",infilesname);
	readPsrXml(header,infilesname);

	
	printf("Checking data files %s\n",header->files[0]->filename);

	printf("Block size=%d bytes\n",header->files[0]->blockLength);
	darr=malloc(header->files[0]->blockLength);

	read =1;
	while(read > 0){
		read = readPsrXmlNextDataBlockIntoExistingArray(header->files[0],darr);
		chk=psrxml_checkHash(header->files[0], darr, header->files[0]->currentBlockNumber);
		switch(chk){
			case 0:
				printf("BAD BLOCK: %d Hash mismatch\n",header->files[0]->currentBlockNumber);
				break;
			case 1:
				break;
			case 2:
				printf("Warning: no checksum for block %d \n",header->files[0]->currentBlockNumber);
				break;
		}
	}
	printf("Done\nThere were %d bad blocks.\n");
	free(darr);
	return 0;
}

