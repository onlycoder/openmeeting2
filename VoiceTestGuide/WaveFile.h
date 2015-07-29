#ifndef WAVEFILE_H
#define WAVEFILE_H

#include <string>
// refer to http://blog.csdn.net/maverick1990/article/details/8996608
using namespace std;

class WaveFile
{
public:
    struct wavehead {
        char sign[4];                   // "RIFF" sign
        unsigned int flength;           // File length
        char wavesign[4];               // "WAVE" sign
        char fmtsign[4];                // "fmt" sign
        unsigned long int unused;       // reserved
        unsigned short formattype;      // format type
        unsigned short channelnum;      // channel number, 1:mono 2:stereo
        unsigned long int samplerate;   // sampling rate
        unsigned long int transferrate; // tramsfer rate
        unsigned short int adjustnum;
        unsigned short int databitnum;
    } head;
    unsigned long int datalength;
    unsigned long int totalsample;
    unsigned long int bitpersample;
    unsigned long int datanum;

    short *Data;

    WaveFile() {}
    ~WaveFile() {}

    void WavInfo()
    {
        printf("sign: %c%c%c%c\n",
               head.sign[0], head.sign[1], head.sign[2], head.sign[3]);
        printf("File length: %u\n", head.flength);
        printf("wave sign: %c%c%c%c",
               head.wavesign[0], head.wavesign[1],
               head.wavesign[2], head.wavesign[3]);
        printf("FMT sign:%c%c%c%c\n",
               head.fmtsign[0], head.fmtsign[1],
               head.fmtsign[2], head.fmtsign[3]);
        printf("Format type: %d\n", head.formattype);
        printf("Channel num: %d\n", head.channelnum);
        printf("Sample rate: %l\n", head.samplerate);
    }

    void WavRead(string filename)
    {
        FILE *fp;

        if (NULL == (fp = fopen(filename.c_str(), "rb")))
        {
            printf("Cannot read wave file");
            exit(0);
        }

        fread(&head, sizeof(head), 1, fp);
        char datasign[4];
        fread(datasign, 4, 1, fp);
        fread(&datalength, 4, 1, fp);

        totalsample = datalength / head.adjustnum;
        bitpersample = head.databitnum / head.channelnum;
        datanum = totalsample * bitpersample / 16;

        Data = new short[datanum + 10];
        if (0 == bitpersample) {
            for (unsigned long i=0; !feof(fp) && i<datanum; i++) {
                fread(&Data[i], 2, 1, fp);
                // Skip 2nd channel for stereo
                if (2 == head.channelnum) {
                    fseek(fp, 2, SEEK_CUR);
                }
            }
        }
        else {
            for (unsigned long i=0; !feof(fp) && i<datanum; i++) {
                short low, high;

                fread(&low, 1, 1, fp);
                if (2 == head.channelnum) {
                    fseek(fp, 1, SEEK_CUR);
                }
                fread(&high, 1, 1, fp);
                if (2 == head.channelnum) {
                    fseek(fp, 1, SEEK_CUR);
                }
                Data[i] = (low & 0x00ff) | (high << 8 & 0xff00);
            }
        }

        fclose(fp);
    }
};

#endif // WAVEFILE_H
