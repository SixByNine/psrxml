#include "psrxml_xml_writing.h"
#include "phcx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void maxmin(float** arr, int xlen, int ylen, double *max, double *min) {
    int x, y;
    double max_v = -1e200;
    double min_v = 1e200;
    for (x = 0; x < xlen; x++) {
        for (y = 0; y < ylen; y++) {
            if (arr[x][y] < min_v)min_v = arr[x][y];
            if (arr[x][y] > max_v)max_v = arr[x][y];
        }
    }
    *min = min_v;
    *max = max_v;
}

void hexprint(float** arr, int xlen, int ylen, double max, double min, FILE* file) {
    double v;
    int x, y;
    int i, count;
    count = 0;
    for (x = 0; x < xlen; x++) {
        for (y = 0; y < ylen; y++) {
            v = (arr[x][y] - min) / (max - min);
            i = (int) (v * 255 + 0.5);
            fprintf(file, "%02X", i);
            count++;
            if (count == 40) {
                fprintf(file, "\n");
                count = 0;
            }
        }
    }


}

void write_phcx(char* fileName, phcx* cand) {
    FILE* file;
    int indent;
    int i, block, s;
    int sb, si, bin;
    int p, d, a, j;
    int count;
    char *attr[64];
    char content[1024];

    double max, min;

    for (i = 0; i < 64; i++)
        attr[i] = (char*) malloc(1024);

    indent = 0;

    file = fopen(fileName, "w");
    fprintf(file, "<?xml version='1.0'?>\n");
    openTag(file, &indent, "phcf"); // <phcf>
    openTag(file, &indent, "head"); // <head>
    writeTag(file, &indent, "SourceID", cand->header.sourceID);
    writeTag(file, &indent, "Telescope", cand->header.telescope);

    openTag(file, &indent, "Coordinate"); // <Coordinate>
    strcpy(attr[0], "units");
    strcpy(attr[1], "degrees");
    sprintf(content, "%lf", cand->header.ra);
    writeTagAttr(file, &indent, "RA", attr, 1, content);
    sprintf(content, "%lf", cand->header.dec);
    writeTagAttr(file, &indent, "Dec", attr, 1, content);
    writeTag(file, &indent, "Epoch", "J2000");
    closeTag(file, &indent, "Coordinate"); //</Coordinate>

    strcpy(attr[0], "units");
    strcpy(attr[1], "MHz");
    sprintf(content, "%lf", cand->header.centreFreq);
    writeTagAttr(file, &indent, "CentreFreq", attr, 1, content);
    sprintf(content, "%lf", cand->header.bandwidth);
    writeTagAttr(file, &indent, "BandWidth", attr, 1, content);
    sprintf(content, "%lf", cand->header.mjdStart);
    writeTag(file, &indent, "MjdStart", content);
    strcpy(attr[0], "units");
    strcpy(attr[1], "seconds");
    sprintf(content, "%lf", cand->header.observationLength);
    writeTagAttr(file, &indent, "ObservationLength", attr, 1, content);
    closeTag(file, &indent, "head"); //</head>


    for (s = 0; s < cand->nsections; s++) {
        phcx_section* section = cand->sections + s;
        strcpy(attr[0], "name");
        strcpy(attr[1], section->name);
        openTagAttr(file, &indent, "Section", attr, 1);
        openTag(file, &indent, "BestValues");
        strcpy(attr[0], "units");
        strcpy(attr[1], "seconds");
        sprintf(content, "%16.14lf", section->bestTopoPeriod);
        writeTagAttr(file, &indent, "TopoPeriod", attr, 1, content);
        sprintf(content, "%16.14lf", section->bestBaryPeriod);
        writeTagAttr(file, &indent, "BaryPeriod", attr, 1, content);
        sprintf(content, "%lf", section->bestDm);
        writeTag(file, &indent, "Dm", content);
        strcpy(attr[1], "m/s/s");
        sprintf(content, "%lf", section->bestAccn);
        writeTagAttr(file, &indent, "Accn", attr, 1, content);
        strcpy(attr[1], "m/s/s/s");
        sprintf(content, "%lf", section->bestJerk);
        writeTagAttr(file, &indent, "Jerk", attr, 1, content);

        sprintf(content, "%lf", section->bestSnr);
        writeTag(file, &indent, "Snr", content);

        sprintf(content, "%lf", section->bestWidth);
        writeTag(file, &indent, "Width", content);

        closeTag(file, &indent, "BestValues");
        sprintf(content, "%lf", section->tsamp);
        writeTag(file, &indent, "SampleRate", content);

        if (section->subints != NULL) {
            maxmin(section->subints, section->nsubints, section->nbins, &max, &min);
            strcpy(attr[0], "nBins");
            sprintf(attr[1], "%d", section->nbins);
            strcpy(attr[2], "nSub");
            sprintf(attr[3], "%d", section->nsubints);
            strcpy(attr[4], "format");
            strcpy(attr[5], "02X");
            strcpy(attr[6], "min");
            sprintf(attr[7], "%lf", min);
            strcpy(attr[8], "max");
            sprintf(attr[9], "%lf", max);


            openTagAttr(file, &indent, "SubIntegrations", attr, 5);
            hexprint(section->subints, section->nsubints, section->nbins, max, min, file);
            closeTag(file, &indent, "SubIntegrations");
        }

        if (section->subbands != NULL) {
            maxmin(section->subbands, section->nsubbands, section->nbins, &max, &min);
            strcpy(attr[0], "nBins");
            sprintf(attr[1], "%d", section->nbins);
            strcpy(attr[2], "nSub");
            sprintf(attr[3], "%d", section->nsubbands);
            strcpy(attr[4], "format");
            strcpy(attr[5], "02X");
            strcpy(attr[6], "min");
            sprintf(attr[7], "%lf", min);
            strcpy(attr[8], "max");
            sprintf(attr[9], "%lf", max);
        


        openTagAttr(file, &indent, "SubBands", attr, 5);
        hexprint(section->subbands, section->nsubbands, section->nbins, max, min, file);
        closeTag(file, &indent, "SubBands");
        }
        if (section->pulseProfile != NULL) {

            maxmin(&(section->pulseProfile), 1, section->nbins, &max, &min);
            strcpy(attr[0], "nBins");
            sprintf(attr[1], "%d", section->nbins);
            strcpy(attr[2], "format");
            strcpy(attr[3], "02X");
            strcpy(attr[4], "min");
            sprintf(attr[5], "%lf", min);
            strcpy(attr[6], "max");
            sprintf(attr[7], "%lf", max);


            openTagAttr(file, &indent, "Profile", attr, 4);
            hexprint(&(section->pulseProfile), 1, section->nbins, max, min, file);
            closeTag(file, &indent, "Profile");
        }
        if (section->snrBlock.block != NULL) {
            openTag(file, &indent, "SnrBlock");
            strcpy(attr[0], "nVals");
            sprintf(attr[1], "%d", section->snrBlock.nperiod);
            strcpy(attr[2], "format");
            strcpy(attr[3], "16.12f");
            openTagAttr(file, &indent, "PeriodIndex", attr, 2);
            for (i = 0; i < section->snrBlock.nperiod; i++) {
                fprintf(file, "%1.12f\n", section->snrBlock.periodIndex[i]);
            }
            closeTag(file, &indent, "PeriodIndex");

            strcpy(attr[0], "nVals");
            sprintf(attr[1], "%d", section->snrBlock.ndm);
            strcpy(attr[2], "format");
            strcpy(attr[3], "6.4f");
            openTagAttr(file, &indent, "DmIndex", attr, 2);
            for (i = 0; i < section->snrBlock.ndm; i++) {
                fprintf(file, "%6.4f\n", section->snrBlock.dmIndex[i]);
            }
            closeTag(file, &indent, "DmIndex");

            strcpy(attr[0], "nVals");
            sprintf(attr[1], "%d", section->snrBlock.naccn);
            strcpy(attr[2], "format");
            strcpy(attr[3], "6.4f");
            openTagAttr(file, &indent, "AccnIndex", attr, 2);
            for (i = 0; i < section->snrBlock.naccn; i++) {
                fprintf(file, "%6.4f\n", section->snrBlock.accnIndex[i]);
            }
            closeTag(file, &indent, "AccnIndex");

            strcpy(attr[0], "nVals");
            sprintf(attr[1], "%d", section->snrBlock.njerk);
            strcpy(attr[2], "format");
            strcpy(attr[3], "6.4f");
            openTagAttr(file, &indent, "JerkIndex", attr, 2);
            for (i = 0; i < section->snrBlock.njerk; i++) {
                fprintf(file, "%6.4f\n", section->snrBlock.jerkIndex[i]);
            }
            closeTag(file, &indent, "JerkIndex");

            max = -1e200;
            min = 1e200;
            for (d = 0; d < section->snrBlock.ndm; d++) {
                for (p = 0; p < section->snrBlock.nperiod; p++) {
                    for (a = 0; a < section->snrBlock.naccn; a++) {
                        for (j = 0; j < section->snrBlock.njerk; j++) {
                            if (section->snrBlock.block[d][p][a][j] > max)max = section->snrBlock.block[d][p][a][j];
                            if (section->snrBlock.block[d][p][a][j] < min)min = section->snrBlock.block[d][p][a][j];

                        }
                    }
                }
            }

            strcpy(attr[0], "format");
            sprintf(attr[1], "02X");
            strcpy(attr[2], "min");
            sprintf(attr[3], "%lf", min);
            strcpy(attr[4], "max");
            sprintf(attr[5], "%lf", max);
            openTagAttr(file, &indent, "DataBlock", attr, 3);
            count = 0;
            for (d = 0; d < section->snrBlock.ndm; d++) {
                for (p = 0; p < section->snrBlock.nperiod; p++) {
                    for (a = 0; a < section->snrBlock.naccn; a++) {
                        for (j = 0; j < section->snrBlock.njerk; j++) {
                            i = (int) (255*(section->snrBlock.block[d][p][a][j] - min) / (max - min) + 0.5);
                            fprintf(file, "%02X", i);
                            count++;
                            if (count == 40) {
                                fprintf(file, "\n");
                                count = 0;
                            }
                        }
                    }
                }
            }

            closeTag(file, &indent, "DataBlock");


            closeTag(file, &indent, "SnrBlock");
        }
        closeTag(file, &indent, "Section");




    }
    closeTag(file, &indent, "phcf");

    fclose(file);

}
