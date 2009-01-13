#ifndef PSRXML_XML_WRITING_C_
#define PSRXML_XML_WRITING_C_
#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
void closeTag(FILE* file, int* indent, char* tagName);
void emptyTag(FILE* file, int* indent, char* tagName, char** attributes,
		int attributes_length);

void writeTagAttr(FILE* file, int* indent, char* tagName, char** attributes,
		int attributes_length, char* content);
void openTagAttr(FILE* file, int* indent, char* tagName, char** attributes,
		int attributes_length);

void openTag(FILE* file, int* indent, char* tagName);

void writeTag(FILE* file, int* indent, char* tagName, char* content);

#ifdef __cplusplus
}
#endif
#endif /*PSRXML_XML_WRITING_C_*/
