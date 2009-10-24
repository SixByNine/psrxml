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
	unsigned long long int sampread;
	int bad,good,unk;
	strcpy(infilesname,argv[1]);

	header = (psrxml*)malloc(sizeof(psrxml));

	printf("Reading xml files %s\n",infilesname);
	readPsrXml(header,infilesname);

	
	printf("Checking data files %s\n",header->files[0]->filename);

	printf("Block size=%d bytes\n",header->files[0]->blockLength);

	darr=malloc(header->files[0]->blockLength);

	readPsrXMLPrepDataFile(header->files[0],header->files[0]->filename);

	read =1;
	sampread=0;
	bad=good=unk=0;
	while(!feof(header->files[0]->file) && sampread < header->numberOfSamples){
		read = readPsrXmlNextDataBlockIntoExistingArray(header->files[0],darr);
		if(read < 1)break;
		sampread += (read * 8)/(header->files[0]->bitsPerSample * header->numberOfChannels);
		chk=psrxml_checkHash(header->files[0], darr,read, header->files[0]->currentBlockNumber-1);
		fprintf(stderr,"\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
		fprintf(stderr,"\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
		fprintf(stderr,"Checking data in block %04d Read %04.1f%%",header->files[0]->currentBlockNumber-1,100*(float)sampread/(float)header->numberOfSamples);
		fflush(stderr);

		switch(chk){
			case 0:
				printf("\nBAD BLOCK: %d Hash mismatch\n",header->files[0]->currentBlockNumber-1);
				bad++;
				break;
			case 1:
				good++;
				break;
			case 2:
				unk++;
				printf("\nWarning: no checksum for block %d \n",header->files[0]->currentBlockNumber-1);
				break;
		}
	}
	printf("\nDone\n");
	if(sampread != header->numberOfSamples){
		printf("WARNING: Header clamed %lld samples, we found %lld\n",header->numberOfSamples,sampread);
	}

	if(unk)printf("No checksum info for %d blocks\n",unk);
	else printf("Checksum info was found for all blocks\n");
	printf("There were %d good blocks\n",good);

	if(bad){
		printf("\n\n========\nWARNING: THERE WERE %d BAD BLOCKS\n=======\n",bad);
	} else {
		printf("There were no bad blocks\n");
	}
	free(darr);
	return 0;
}

