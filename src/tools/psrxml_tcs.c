#include <stdio.h>
#include <psrxml.h>

void mergeTcs(psrxml* header, char* tcsfilename);

int main(int argc, char** argv){
	psrxml* header;
	char infilename[1024];
	char outfilename[1024];
	char tcsfilename[1024];

	strcpy(infilename,argv[1]);
	strcpy(outfilename,argv[2]);
	strcpy(tcsfilename,argv[3]);

	printf("%s --(%s)-> %s\n",infilename,tcsfilename,outfilename);

	header = (psrxml*)malloc(sizeof(psrxml));

	readPsrXml(header,infilename);

//	mergeTcs(header,tcsfilename);

	writePsrXml(header,outfilename);
	

	return 0;
}

void mergeTcs(psrxml* header, char* tcsfilename){
	FILE* file;

	file = fopen(tcsfilename, "r");

	while(!feof(file)){
		

	}

	fclose(file);
}
