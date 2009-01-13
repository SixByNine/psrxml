#include <libxml/globals.h>
#include <libxml/xmlerror.h>
#include <libxml/parser.h>
#include <libxml/parserInternals.h> /* only for xmlNewInputFromFile() */
#include <libxml/tree.h>
#include <libxml/debugXML.h>
#include <libxml/xmlmemory.h>


#include <string.h>
#include <stdio.h>
#include <math.h>

#include "phcx.h"

static xmlSAXHandler phcx_xml_handler;

int phcx_isWhitespace(char c) {
    return (c == ' ' || c == '\t' || c == '\n' || c == '\r');
}

char* phcx_trim(char* str) {
    int i;
    char* end;

    i = 0;

    while (phcx_isWhitespace(*str)) {
        str++;
    }

    end = str;

    while (*end != '\0') {
        end++;
    }

    end--;

    while (end > str && (phcx_isWhitespace(*end))) {
        end--;
    }

    *(end + 1) = '\0';

    return str;
}

void free_phcx(phcx* thePhcx) {

    int i;

    for (i = 0; i < thePhcx->nsections; i++) {
        free(thePhcx->sections[i].name);
        if (thePhcx->sections[i].subints) free(thePhcx->sections[i].subints);
        if (thePhcx->sections[i].subbands) free(thePhcx->sections[i].subbands);
        if (thePhcx->sections[i].pulseProfile) free(thePhcx->sections[i].pulseProfile);
        if (thePhcx->sections[i].snrBlock.block) {
            free(thePhcx->sections[i].snrBlock.block);
            free(thePhcx->sections[i].snrBlock.periodIndex);
            free(thePhcx->sections[i].snrBlock.dmIndex);
            free(thePhcx->sections[i].snrBlock.accnIndex);
            free(thePhcx->sections[i].snrBlock.jerkIndex);
        }
    }
    free(thePhcx->sections);
    free(thePhcx);
}

phcx* read_phcx(char* filename) {

    phcx* retval = (phcx*) malloc(sizeof (phcx));

    retval->nsections = 0;
    retval->sections=NULL;


    phcx_reader_state state;

    state.theContent = retval;
    state.charBuffer = (char*) malloc(sizeof (char) *1024 * 1024);
    if (state.charBuffer == NULL) {
        fprintf(stderr, "Error, could not allocate memory");
        exit(1);
    }
    state.sizeOfCharBuffer = 1024 * 1024;
    state.nchar = 0;

    state.units[0] = '\0';
    state.nBins = -1;
    state.nVals = -1;
    state.nSub = -1;
    state.min = 0.0;
    state.max = 0.0;
    state.format[0] = '\0';
    state.name[0] = '\0';


    xmlSAXUserParseFile(&phcx_xml_handler, &state, filename);

    free(state.charBuffer);

    return retval;
}

static void phcx_startElement(void *vs, const xmlChar *name, const xmlChar **atts) {
    int i;
    phcx_reader_state* state;
    state = (phcx_reader_state*) vs;

    /* Read the attributes! */
    if ((char*) atts != NULL) {
        for (i = 0; ((char*) atts[i] != NULL); i++) {
            if (strcmp((char*) atts[i], "units") == 0) strcpy(state->units, (char*) atts[++i]);
            if (strcmp((char*) atts[i], "nVals") == 0) sscanf((char*) atts[++i], "%d", &state->nVals);
            if (strcmp((char*) atts[i], "nBins") == 0) sscanf((char*) atts[++i], "%d", &state->nBins);
            if (strcmp((char*) atts[i], "nSub") == 0) sscanf((char*) atts[++i], "%d", &state->nSub);
            if (strcmp((char*) atts[i], "format") == 0) strcpy(state->format, (char*) atts[++i]);
            if (strcmp((char*) atts[i], "name") == 0) strcpy(state->name, (char*) atts[++i]);
            if (strcmp((char*) atts[i], "min") == 0) sscanf((char*) atts[++i], "%lf", &state->min);
            if (strcmp((char*) atts[i], "max") == 0) sscanf((char*) atts[++i], "%lf", &state->max);
        }
    }

    /* Are we starting a new section? */
    if (strcmp((char*) name, "Section") == 0) {
        if (state->theContent->nsections == 0) {
            state->theContent->sections = malloc(sizeof (phcx_section));
            state->theContent->nsections = 1;
        } else {
            state->theContent->nsections++;
            state->theContent->sections = realloc(state->theContent->sections, sizeof (phcx_section) * state->theContent->nsections);
        }
        state->currentSection = &state->theContent->sections[state->theContent->nsections - 1];
        state->currentSection->name = malloc(sizeof (char) * strlen(state->name));
        strcpy(state->currentSection->name, state->name);
	state->currentSection->snrBlock.block = NULL;
	state->currentSection->subints=NULL;
	state->currentSection->subbands=NULL;
	state->currentSection->pulseProfile=NULL;
	state->currentSection->nbins=0;
	state->currentSection->nsubints=0;
	state->currentSection->nsubbands=0;
    }

    if (strcmp((char*) name, "Profile") == 0) {
        state->currentSection->nbins = state->nBins;
    }
    if (strcmp((char*) name, "SubIntegrations") == 0) {
        state->currentSection->nbins = state->nBins;
        state->currentSection->nsubints = state->nSub;
    }
    if (strcmp((char*) name, "SubBands") == 0) {
        state->currentSection->nbins = state->nBins;
        state->currentSection->nsubbands = state->nSub;
    }


    /* reset the char buffer */
    state->nchar = 0;
}

static void phcx_endElement(void *vs, const xmlChar *name) {
    char* content;
    char charStr[2];
    double factor;
    phcx_SNRBlock *snrBlock;
    int i, j, k, l, m, n, posn, intV, ii;
    char c;
    double nmax;
    int digitsPerSamp;
    phcx_reader_state* state;
    state = (phcx_reader_state*) vs;

    digitsPerSamp = 2;
    nmax = pow(16, digitsPerSamp);

    /* NUL Terminate the string! */
    state->charBuffer[(state->nchar)] = '\0';


    content = phcx_trim(state->charBuffer);


    /* What tag are we ending? */
    /* The header elements */
    if (strcmp((char*) name, "Telescope") == 0) {
        strcpy(state->theContent->header.telescope, content);
    }
    if (strcmp((char*) name, "SourceID") == 0) {
        strcpy(state->theContent->header.sourceID, content);
    }
    if (strcmp((char*) name, "RA") == 0) {
        if (strcmp(state->units, "degrees") != 0) {
            fprintf(stderr, "Unknown units '%s' for RA in phcx file. Pretending it's degrees\n", state->units);
        }
        sscanf(content, "%lf", &state->theContent->header.ra);
    }

    if (strcmp((char*) name, "Dec") == 0) {
        if (strcmp(state->units, "degrees") != 0) {
            fprintf(stderr, "Unknown units '%s' for Dec in phcx file. Pretending it's degrees\n", state->units);
        }
        sscanf(content, "%lf", &state->theContent->header.dec);
    }
    if (strcmp((char*) name, "CentreFreq") == 0) {
        factor = 1;
        if (strcmp(state->units, "MHz") == 0) {
        } else if (strcmp(state->units, "GHz") == 0) {
            factor = 1000.0;
        } else {
            fprintf(stderr, "Unknown units '%s' for CentreFreq in phcx file. Pretending it's MHz\n", state->units);
        }
        sscanf(content, "%lf", &state->theContent->header.centreFreq);
        state->theContent->header.centreFreq *= factor;
    }

    if (strcmp((char*) name, "BandWidth") == 0) {
        factor = 1;
        if (strcmp(state->units, "MHz") == 0) {
        } else if (strcmp(state->units, "GHz") == 0) {
            factor = 1000.0;
        } else {
            fprintf(stderr, "Unknown units '%s' for BandWidth in phcx file. Pretending it's MHz\n", state->units);
        }
        sscanf(content, "%lf", &state->theContent->header.bandwidth);
        state->theContent->header.bandwidth *= factor;
    }
    if (strcmp((char*) name, "MjdStart") == 0) {
        sscanf(content, "%lf", &state->theContent->header.mjdStart);
    }
    if (strcmp((char*) name, "ObservationLength") == 0) {
        factor = 1;
        if (strcmp(state->units, "seconds") == 0) {
        } else if (strcmp(state->units, "minutes") == 0) {
            factor = 60.0;
        } else if (strcmp(state->units, "hours") == 0) {
            factor = 3600.0;
        } else {
            fprintf(stderr, "Unknown units '%s' for ObservationLength in phcx file. Pretending it's Seconds\n", state->units);
        }
        sscanf(content, "%lf", &state->theContent->header.observationLength);
        state->theContent->header.observationLength *= factor;
    }



    /* Section elements */
    if (strcmp((char*) name, "TopoPeriod") == 0) {
        factor = 1;
        if (strcmp(state->units, "seconds") == 0) {
        } else if (strcmp(state->units, "miliseconds") == 0) {
            factor = 1e-3;
        } else {
            fprintf(stderr, "Unknown units '%s' for TopoPeriod in phcx file. Pretending it's Seconds\n", state->units);
        }
        sscanf(content, "%lf", &state->currentSection->bestTopoPeriod);
        state->currentSection->bestTopoPeriod *= factor;
    }

    if (strcmp((char*) name, "BaryPeriod") == 0) {
        factor = 1;
        if (strcmp(state->units, "seconds") == 0) {
        } else if (strcmp(state->units, "miliseconds") == 0) {
            factor = 1e-3;
        } else {
            fprintf(stderr, "Unknown units '%s' for BaryPeriod in phcx file. Pretending it's Seconds\n", state->units);
        }
        sscanf(content, "%lf", &state->currentSection->bestBaryPeriod);
        state->currentSection->bestBaryPeriod *= factor;
    }

    if (strcmp((char*) name, "Dm") == 0) {
        sscanf(content, "%lf", &state->currentSection->bestDm);
    }
    if (strcmp((char*) name, "Accn") == 0) {
        factor = 1;
        if (strcmp(state->units, "m/s/s") == 0) {
        } else if (strcmp(state->units, "cm/s/s") == 0) {
            factor = 1e2;
        } else {
            fprintf(stderr, "Unknown units '%s' for Accn in phcx file. Pretending it's m/s/s\n", state->units);
        }
        sscanf(content, "%lf", &state->currentSection->bestAccn);
        state->currentSection->bestAccn *= factor;
    }
    if (strcmp((char*) name, "Jerk") == 0) {
        factor = 1;
        if (strcmp(state->units, "m/s/s/s") == 0) {
        } else if (strcmp(state->units, "cm/s/s/s") == 0) {
            factor = 1e2;
        } else {
            fprintf(stderr, "Unknown units '%s' for Jerk in phcx file. Pretending it's m/s/s/s\n", state->units);
        }
        sscanf(content, "%lf", &state->currentSection->bestJerk);
        state->currentSection->bestJerk *= factor;
    }

    if (strcmp((char*) name, "Snr") == 0) {
        sscanf(content, "%lf", &state->currentSection->bestSnr);
    }
    if (strcmp((char*) name, "Width") == 0) {
        sscanf(content, "%lf", &state->currentSection->bestWidth);
    }


    if (strcmp((char*) name, "SampleRate") == 0) {
        sscanf(content, "%lf", &state->currentSection->tsamp);
    }

    /* Now the plots */
    if (strcmp((char*) name, "Profile") == 0) {
        if (state->nBins > 0) {
            state->currentSection->pulseProfile = malloc(sizeof (float) * state->nBins);

            k = 0;
            for (j = 0; j < state->nBins; j++) {
                intV = 0;
                posn = (int) nmax;
                for (n = digitsPerSamp - 1; n >= 0; n--) {
                    posn /= 16;
                    c = content[k++];
                    while (phcx_isWhitespace(c)) {
                        c = content[k++];
                    }
                    charStr[0] = c;
                    charStr[1] = '\0';
                    sscanf(charStr, "%X", &ii);
                    //printf("'%X' '%s' '%c'\n",i,charStr,c);
                    intV += ii*posn;
                }
                state->currentSection->pulseProfile[j] = (((float) intV) / (nmax - 1)) * (state->max - state->min) + state->min;
            }
        } else {
            fprintf(stderr, "Reading profile but number of bins has not been set!\n");
        }
    }

    if (strcmp((char*) name, "SubIntegrations") == 0) {
        state->currentSection->subints = malloc(sizeof (float*) * state->nSub);
        for (i = 0; i < state->nSub; i++) {
            state->currentSection->subints[i] = malloc(sizeof (float) * state->nBins);
        }


        k = 0;

        for (i = 0; i < state->nSub; i++) {
            for (j = 0; j < state->nBins; j++) {
                intV = 0;
                posn = (int) nmax;
                for (n = digitsPerSamp - 1; n >= 0; n--) {
                    posn /= 16;
                    c = content[k++];
                    while (phcx_isWhitespace(c)) {
                        c = content[k++];
                    }
                    charStr[0] = c;
                    charStr[1] = '\0';
                    sscanf(charStr, "%X", &ii);
                    intV += ii*posn;
                }
                state->currentSection->subints[i][j] = (((float) intV) / (nmax - 1)) * (state->max - state->min) + state->min;
            }
        }
    }
    if (strcmp((char*) name, "SubBands") == 0) {
        state->currentSection->subbands = malloc(sizeof (float*) * state->nSub);
        for (i = 0; i < state->nSub; i++) {
            state->currentSection->subbands[i] = malloc(sizeof (float) * state->nBins);
        }


        k = 0;

        for (i = 0; i < state->nSub; i++) {
            for (j = 0; j < state->nBins; j++) {
                intV = 0;
                posn = (int) nmax;
                for (n = digitsPerSamp - 1; n >= 0; n--) {
                    posn /= 16;
                    c = content[k++];
                    while (phcx_isWhitespace(c)) {
                        c = content[k++];
                    }
                    charStr[0] = c;
                    charStr[1] = '\0';
                    sscanf(charStr, "%X", &ii);
                    intV += ii*posn;
                }
                state->currentSection->subbands[i][j] = (((float) intV) / (nmax - 1)) * (state->max - state->min) + state->min;
            }
        }
    }


    if (strcmp((char*) name, "PeriodIndex") == 0) {
        state->currentSection->snrBlock.periodIndex = malloc(sizeof (double) * state->nVals);
        state->currentSection->snrBlock.nperiod = state->nVals;
        for (j = 0; j < state->nVals; j++) {

            sscanf(content, "%lf", state->currentSection->snrBlock.periodIndex + j);
            while (!phcx_isWhitespace(*content)) {
                content++;
                if (*content == '\0') {
                    content--;
                    break;
                }
            }
            content++;


        }
    }

    if (strcmp((char*) name, "DmIndex") == 0) {
        state->currentSection->snrBlock.dmIndex = malloc(sizeof (double) * state->nVals);
        state->currentSection->snrBlock.ndm = state->nVals;
        for (j = 0; j < state->nVals; j++) {
            sscanf(content, "%lf", state->currentSection->snrBlock.dmIndex + j);
            while (!phcx_isWhitespace(*content)) {
                content++;
                if (*content == '\0') {
                    content--;
                    break;
                }
            }
            content++;
        }
    }


    if (strcmp((char*) name, "AccnIndex") == 0) {
        state->currentSection->snrBlock.accnIndex = malloc(sizeof (double) * state->nVals);
        state->currentSection->snrBlock.naccn = state->nVals;

        for (j = 0; j < state->nVals; j++) {
            sscanf(content, "%lf", state->currentSection->snrBlock.accnIndex + j);

            while (!phcx_isWhitespace(*content)) {
                content++;
                if (*content == '\0') {
                    content--;
                    break;
                }
            }
            content++;
        }
    }

    if (strcmp((char*) name, "JerkIndex") == 0) {
        state->currentSection->snrBlock.jerkIndex = malloc(sizeof (double) * state->nVals);
        state->currentSection->snrBlock.njerk = state->nVals;
        for (j = 0; j < state->nVals; j++) {
            sscanf(content, "%lf", state->currentSection->snrBlock.jerkIndex + j);
            while (!phcx_isWhitespace(*content)) {
                content++;
                if (*content == '\0') {
                    content--;
                    break;
                }
            }
            content++;
        }
    }

    /* This is a big ndm * nperiod * nacc * njerk array */
    if (strcmp((char*) name, "DataBlock") == 0) {

        snrBlock = &state->currentSection->snrBlock;

        snrBlock->block = malloc(sizeof (double***) * snrBlock->ndm);

        k = 0;

        for (i = 0; i < snrBlock->ndm; i++) {
            snrBlock->block[i] = malloc(sizeof (double**) * snrBlock->nperiod);
            for (j = 0; j < snrBlock->nperiod; j++) {
                snrBlock->block[i][j] = malloc(sizeof (double*) * snrBlock->naccn);
                for (m = 0; m < snrBlock->naccn; m++) {
                    snrBlock->block[i][j][m] = malloc(sizeof (double) * snrBlock->njerk);
                    for (l = 0; l < snrBlock->njerk; l++) {
                        intV = 0;
                        posn = (int) nmax;
                        for (n = digitsPerSamp - 1; n >= 0; n--) {
                            posn /= 16;
                            c = content[k++];
                            while (phcx_isWhitespace(c)) {
                                c = content[k++];
                            }
                            charStr[0] = c;
                            charStr[1] = '\0';
                            sscanf(charStr, "%X", &ii);
                            intV += ii*posn;
                        }
                        //printf("'%X' '%s' '%c'\n",i,charStr,c);
                        //                                        intV += ii*posn;
                        //                                                                        }
                        //
                        snrBlock->block[i][j][m][l] = (((double) intV) / (nmax - 1)) * (state->max - state->min) + state->min;
                    }
                }
            }
        }

    }




    /* End of function */
    state->nchar = 0;
}

static void phcx_characters(void *vs, const xmlChar *ch, int len) {
    phcx_reader_state* state;
    int i;

    state = (phcx_reader_state*) vs;


    if (len + state->nchar + 1 > state->sizeOfCharBuffer) {
        printf("Expand %d\n", state->sizeOfCharBuffer);

        /* We will overflow our buffer! */

        state->charBuffer = realloc(state->charBuffer, sizeof (char) *(state->sizeOfCharBuffer + len * 2));
        state->sizeOfCharBuffer = state->sizeOfCharBuffer + len * 2;

    }



    //printf("%d %d %d\n",len,state->nchar,state->sizeOfCharBuffer);
    for (i = 0; i < len; i++) {
        state->charBuffer[state->nchar + i] = ((char*) ch)[i];
    }
    state->nchar += len;
}

static void phcx_endDocument(void* vs) {
}

/*
 *  * empty SAX block
 *     
 */
static xmlSAXHandler phcx_xml_handler = {
    NULL, /* internalSubset */
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
    NULL, /* startDocument */
    phcx_endDocument, /* endDocument */
    phcx_startElement, /* startElement */
    phcx_endElement, /* endElement */
    NULL, /* reference */
    phcx_characters, /* characters */
    NULL, /* ignorableWhitespace */
    NULL, /* processingInstruction */
    NULL, /* comment */
    NULL, /* xmlParserWarning */
    NULL, /* xmlParserError */
    NULL, /* xmlParserError */
    NULL, /* getParameterEntity */
    NULL, /* cdataBlock; */
    NULL, /* externalSubset; */
    1,
    NULL,
    NULL, /* startElementNs */
    NULL, /* endElementNs */
    NULL /* xmlStructuredErrorFunc */
};

