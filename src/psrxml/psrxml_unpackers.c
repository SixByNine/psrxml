#include <psrxml.h>

enum endianness endian() {
    int i = 1;
    char *p = (char *) & i;

    if (p[0] == 1)
        return LITTLE;
    else
        return BIG;
}

void swap_bytes(unsigned int* in, char swapem) {
    if (swapem) {
        *in = (*in & 0x000000FF << 24) | (*in & 0x0000FF00 << 16) | (*in & 0x00FF0000 << 8) | (*in & 0xFF000000);
    }
}

void unpackDataChunk_32bit(unsigned char* raw, float* outdata, int nbits,
        int nchans, unsigned int nsamps, enum endianness end, char isSigned,
        char isChannelInterleaved, int nsampStart, int nsampEnd,
        char swapChannels) {
    int i, channel, sample;
    int in, out, outchan;
    char swapem = 0;
    float* farr = (float*) raw;

    if(end!=INDEPENDANT){
	    swapem = endian() != end;
    }
    if (!isChannelInterleaved) {
        for (channel = 0; channel < nchans; channel++) {
            if (swapChannels) {
                outchan = nchans - channel - 1;
            } else {
                outchan = channel;
            }
            in = channel*nsamps;
            out = outchan*nsamps;
            for (sample = nsampStart; sample < nsampEnd; sample++) {
                outdata[out + sample] = farr[in + sample];
                swap_bytes((unsigned int*) (&outdata[out + sample]), swapem);
            }
        }
    } else {
        farr += nsampStart * nchans;
        
        for (sample = nsampStart; sample < nsampEnd; sample++) {
            for (channel = 0; channel < nchans; channel++) {

                if (swapChannels) {
                    out = (nchans - channel - 1) * nsamps + sample;
                    outdata[out]
                            = *farr;
                } else {
                    out = (channel) * nsamps + sample;
                    outdata[out]
                            = *farr;
                }
                swap_bytes((unsigned int*) (&outdata[out]), swapem);
                farr++;

            }
        }
    }

}

/*
 * Unpacks into an array that increments fastest by bin, then by channel.
 * 
 */
void unpackDataChunk_1to8bit(unsigned char* raw, float* outdata, int nbits,
        int nchans, unsigned int nsamps, char fsInMSB, char isSigned,
        char isChannelInterleaved, int nsampStart, int nsampEnd,
        char swapChannels) {
    int sample;
    int channel;
    int counter, i;
    //	int nbytes;
    int chaninc, sampinc;
    int outchan;
    float ***lookupTable;

    unsigned char byte;
    if (!checkLookup(nbits, fsInMSB, isSigned)) {
        makeLookup(nbits, fsInMSB, isSigned);
    }
    lookupTable = getLookupTable();

    if (!isChannelInterleaved) {

        //		fprintf(stderr,"Non Channel-Interleaved data not yet supported in unpackDataChunk!\n");
        //		exit(5);
        sampinc = 8 / nbits;

        for (channel = 0; channel < nchans; channel++) {
            counter = (channel * nsamps + nsampStart) / sampinc;
            if (swapChannels)
                outchan = nchans - channel - 1;
            else
                outchan = channel;
            for (sample = nsampStart; sample < nsampEnd; sample += sampinc) {

                byte = raw[counter];
                //				printf("%d\n",byte);
                for (i = 0; i < sampinc; i++) {
                    // a bit complicated, but this is arranging the data nicely
                    outdata[(outchan) * nsamps + sample + i]
                            = lookupTable[nbits][i][byte];
                }
                counter++;
            }
        }
        return;
    }

    chaninc = 8 / nbits;

    //nbytes = nchans*nsamps*nbits /8;


    counter = nsampStart * nchans / chaninc;
    for (sample = nsampStart; sample < nsampEnd; sample++) {
        for (channel = 0; channel < nchans; channel += chaninc) {

            byte = raw[counter];
            //				printf("%d\n",byte);
            for (i = 0; i < chaninc; i++) {
                // a bit complicated, but this is arranging the data nicely
                if (swapChannels)
                    outdata[(nchans - channel - i - 1) * nsamps + sample]
                        = lookupTable[nbits][i][byte];
                else
                    outdata[(channel + i) * nsamps + sample]
                        = lookupTable[nbits][i][byte];

                //					printf("%02x %d %f\n",(unsigned int)byte,i,lookupTable[nbits][i][(unsigned int)byte]);
            }

            counter++;
        }
    }

}

void unpackDataChunk(unsigned char* raw, float* outData, psrxml* header,
        int fileNum, int nSamps, int nsampStart, int nsampEnd, char swapChannels) {
    switch (header->files[fileNum]->bitsPerSample) {
        case 1:
        case 2:
        case 4:
        case 8:
            unpackDataChunk_1to8bit(raw, outData,
                    header->files[fileNum]->bitsPerSample,
                    header->numberOfChannels, nSamps,
                    header->files[fileNum]->firstSampleIsMostSignificantBit,
                    header->files[fileNum]->isSigned,
                    header->files[fileNum]->isChannelInterleaved, nsampStart,
                    nsampEnd, swapChannels);
            break;
        case 32:
            unpackDataChunk_32bit(raw, outData,
                    header->files[fileNum]->bitsPerSample,
                    header->numberOfChannels, nSamps,
                    header->files[fileNum]->endian,
                    header->files[fileNum]->isSigned,
                    header->files[fileNum]->isChannelInterleaved, nsampStart,
                    nsampEnd, swapChannels);
            break;
        default:
            fprintf(stderr, "\n%d bit data not yet supported in unpackDataChunk!\n", header->files[fileNum]->bitsPerSample);
            exit(5);
            return;
    }
    zapDataChunk(outData, header, fileNum, nSamps);

}

/*
 * Splits indata (nchans x nbins) by nsubs subbands.
 * out needs dimentions (nsubs x (nchans/nsubs))
 */
void unpackToChannels(float* indata, float** out, int nchans, int nsamp) {
    int i;
    for (i = 0; i < nchans; i++) {
        out[i] = (indata + i * nsamp);
    }
}
