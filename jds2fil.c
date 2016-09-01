/* ADD WRITING OF SAMPLES TO FILE IN unisgned int FORMAT FOR OTHER PROGRAMS */

#define _FILE_OFFSET_BITS 64 /* making sure that programs can handle large files */
#define _USE_LARGEFILE 1 /* ditto */
#define _USE_LARGEFILE 1 /* ditto */
#define _LARGEFILE64_SOURCE 1 /* ditto */
#define _LARGEFILE_SOURCE 1 /* ditto */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "jds.h" /* jds header definition*/
#include "jdstools.h" /* definitions of header extraction and data conversion functions */

/* INITIALIZATION OF FUNCTIONS ... */
void send_string(char *string); /* functions used to write "filterbank" PRESTO data files */
void send_float(char *name, float floating_point); /* ditto */
void send_double(char *name, double double_precision); /* ditto */
void send_long(char *name, long integer); /* ditto */
void send_int(char *name, int integer); /* ditto */
void send_coords(double raj, double dej, double az, double za); /* ditto */

/* ... AND GLOBAL VARIABLES */
unsigned int *rawData; /* allocate pointer to array to read raw data from file */
int *zapChannelsArr; /* allocate pointer to array storing bad channels to zap */
float *block; /* allocate pointer to array for bytes to be actually written to file */
float *dataFloat; /* allocate pointer to array for data after first step of conversion to floats */
float *dataReady; /* allocate pointer to array for data after streams A & B subtraction */
float *bandpassMedianSmoothed; /* allocate pointer to array for running mean spectrum */
float *bandpassArray; /* allocate pointer to array for bandpass */
FILE *FILTERBANK_OUT; /* pointer to output file */

/* MAIN BODY OF THE PROGRAM */
int main(int argc, char *argv[])
{
   int i, j, argument, k;
   int spectrumCounter, maxSpectraNumber, spectrumWidth;
   int nbytes, nbits, bps, byte;
   int jd, jm, jy, jhr, jmn;
   int telescopeID;
   int freqRanges, clipInput;
   int firstSpecChanNum, lastSpecChanNum, numberChannels, specChanNum;
   int zapChannelsCounter, CorruptedFlag, movingMeanWindow, streamOperation;
   int NoCRC, HeaderLess, InvertChannels, ZapChannels, ClipSpectrum, Frequency, OutFilename, Verbose;
   int Nfmin[4], Nf[4], Nc;
   char stDay[32], stMonth[32], RAtxt[16], DecTxt[16];
   char psrName[64], jdsFilename[256], zapFilename[256], outFilename[256];
   float freqFirst, freqLast, channelBW, freqLow, freqHigh, clipValue;
   float averageBandpassMedianSmoothed, sigmaBandpassMedianSmoothed;
   double samplingTime, RA, Dec, RAh, RAm, RAs, DecD, DecM, DecS;
   double startMJD, jsc, fileSize;
   char *filterbankFilename;
   FILE *JDS_IN;
   /* PRESET FLAGS & DEFAULT VALUES */
   bps = 4; /* bytes per sample */
   byte = 8; /* bytes per bit */
   nbits = 32; /* number of bits in the output file */
   telescopeID = 15; /* ID of the telescope, using TEMPO or PRESTO convention */
   strcpy(psrName, "B0031-07"); /* name of the source */
   RA = 003408.8703; /* right ascention of the source */
   Dec = -072153.409; /* declination of the source */
   spectrumCounter = 0; /* counter of the spectra being processed */
   maxSpectraNumber = 0; /* number of spectra in the file, based on the file size */
   clipValue = 10.0;
   movingMeanWindow = 256;
   averageBandpassMedianSmoothed = 0.0;
   sigmaBandpassMedianSmoothed = 0.0;
   streamOperation = 0; /* stream handling, 0 -> A-B, 1 -> A+B, 2 -> A, 3 -> B, 4 -> B-A */
   HeaderLess = 0; /* do not write header */
   CorruptedFlag = 0; /* flag signalling corrupted spectrum (ADC or CRC error) */
   NoCRC = 0; /* flag activating removal of spectra with ADC overflows and CRC errors */
   Frequency = 0; /* flag activating freqeuncy range */
   InvertChannels = 1; /* flag activating inverting the channels order */
   ZapChannels = 0; /* flag activating channel zapping */
   ClipSpectrum = 0; /* flag activating spectrum clipping */
   OutFilename = 0; /* flag for giving a new output file name */
   Verbose = 0; /* flag activating verbose mode, 1 - normal verbose, 2 - very verbose, 3 - even more verbose */
   /* AVAILABLE HELP */
   if (argc < 2 || (strcmp(argv[1], "-h") == 0)) /* in case there is a -h switch */
   {
      printf("\nProgram converts an UTR-2 data format file to 32-bit filterbank format file.\n");
      printf("\nUsage: jds2rawfil [parameters] <filename>\n");
      printf("\nAvailable parameters are:\n");
      printf("-psrname     Name of the source to be written in .fil file (default: %s)\n", psrName);
      printf("-ra          Right Ascention (J2000) of the source in hh:mm:ss.ssss (default: 00:34:08.8703)\n");
      printf("-dec         Declination (J2000) of the source in dd:mm:ss.ssss (default: -07:21:53.409)\n");
      printf("-tel         Telescope SIGPROC identifier (UTR2: 15, etc.; default: %d)\n", telescopeID);
      printf("-freq        Lowest and highest frequency to keep (default: all)\n");
      printf("-headerless  Write out data without any header info (default: with header)\n");
      printf("-noinv       Turn off inverting order of frequency channels (important for PRESTO; default: invert)\n");
      printf("-zap         File with channel numbers to be zapped (input in column, numbers will be sorted upon opening)\n");
      printf("-clip        Clip channels using median smoothed difference (default: %.1f sigma, smoothing window of %d spectra)\n", clipValue, movingMeanWindow);
      printf("-stream      Stream operation (A-B: 0, A+B: 1, A: 2, B: 3, B-A: 4; default: %d).\n", streamOperation);
      printf("-nocrc       Disable automatic removal of spectra with ADC overflow and CRC errors (use for pre-Aug 2008 data, default: remove).\n");
      printf("-o           Give new name of output file (default: add .fil to original filename trunk)\n");
      printf("-v           Verbose mode\n");
      printf("-vv          Very verbose mode\n");
      printf("-vvv         Even more verbose mode (really HUGE output)\n");
      printf("-h           Display this useful help page\n\n");
      return -1;
   }
   /* READING COMMAND LINE ARGUMENTS */
   if (argc > 2)
   {
      for (argument = 1; argument < argc - 1; argument++)
      {
         if (strcmp(argv[argument], "-psrname") == 0)
         {
            strcpy(psrName, argv[argument+1]);
            argument++;
         }
         else if (strcmp(argv[argument], "-ra") == 0)
         {
            strcpy(RAtxt, argv[argument+1]);
            sscanf(RAtxt, "%lf:%lf:%lf", &RAh, &RAm, &RAs);
            RA = (RAh * 10000.0) + (RAm * 100.0) + RAs;
            if (RA < 0.0 || RA > 240000.0)
            {
               fprintf(stderr, "%s> RA of %f degrees does not make sense.\n", argv[0], RA);
               return -1;
            }
            argument++;
         }
         else if (strcmp(argv[argument], "-dec") == 0)
         {
            strcpy(DecTxt, argv[argument+1]);
            sscanf(DecTxt, "%lf:%lf:%lf", &DecD, &DecM, &DecS);
            if (strncmp (DecTxt,"-",1) == 0)
            {
               Dec = (DecD * 10000.0) + (DecM * -100.0) + (-1.0 * DecS);
            }
            else
            {
               Dec = (DecD * -10000.0) + (DecM * -100.0) + ( DecS * -1.0);
            }
            if (Dec < -900000.0 || Dec >= 900000.0)
            {
               fprintf(stderr, "%s> Dec value does not make sense.\n", argv[0]);
               return -1;
            }
            argument++;
         }
         else if (strcmp(argv[argument], "-tel") == 0)
         {
            telescopeID = atoi(argv[argument+1]);
            argument++;
         }
         else if (strcmp(argv[argument], "-freq") == 0)
         {
            freqRanges = sscanf(argv[argument+1], "%f %f", &freqLow, &freqHigh);
            if (freqRanges != 2 || freqRanges > 2)
            {
               fprintf(stderr, "%s> Error parsing -freq option.\n", argv[0]);
               return -1;
            }
            if (freqLow >= freqHigh || freqHigh <= freqLow || freqLow <= 0.0 || freqHigh <= 0.0)
            {
               fprintf(stderr, "%s> Error passing frequency ranges.\n", argv[0]);
               return -1;
            }
            Frequency = 1;
            argument++;
         }
         else if(strcmp(argv[argument], "-zap") == 0)
         {
            strcpy(zapFilename, argv[argument+1]);
            ZapChannels = 1;
            argument++;
         }
         else if (strcmp(argv[argument], "-clip") == 0)
         {
            clipInput = sscanf(argv[argument+1], "%f %d", &clipValue, &movingMeanWindow);
            if (clipInput != 2 || clipInput > 2)
            {
               fprintf(stderr, "%s> Error parsing -clip option.\n", argv[0]);
               return -1;
            }
            ClipSpectrum = 1;
            argument++;
         }
         else if (strcmp(argv[argument], "-stream") == 0)
         {
            streamOperation = atoi(argv[argument+1]);
            if (streamOperation < 0 || streamOperation > 4)
            {
               fprintf(stderr, "%s> Error parsing -stream option.\n", argv[0]);
               return -1;
            }
            argument++;
         }
         else if(strcmp(argv[argument], "-o") == 0)
         {
            strcpy(outFilename, argv[argument+1]);
            if (outFilename == NULL)
            {
               fprintf(stderr, "%s> No output filename specified.\n", argv[0]);
               return -1;
            }
            OutFilename = 1;
            argument++;
         }
         else if (strcmp(argv[argument], "-noinv") == 0)
         {
            InvertChannels = 0;
         }
         else if (strcmp(argv[argument], "-headerless") == 0)
         {
            HeaderLess = 1;
         }
         else if (strcmp(argv[argument], "-nocrc") == 0)
         {
            NoCRC = 1;
         }
         else if (strcmp(argv[argument], "-v") == 0)
         {
            Verbose = 1;
         }
         else if (strcmp(argv[argument], "-vv") == 0)
         {
            Verbose = 2;
         }
         else if (strcmp(argv[argument], "-vvv") == 0)
         {
            Verbose = 3;
         }
         else
         {
            fprintf(stderr, "%s> Unknown option: %s\n", argv[0], argv[argument]);
            return -1;
         }
      }
   }
   strcpy(jdsFilename, argv[argc-1]);
   if ((JDS_IN = fopen64(jdsFilename, "r")) == NULL)
   {
      fprintf(stderr, "%s> Unable to open file %s\n", argv[0], jdsFilename);
      return -1;
   }
   else
   {
      printf("%s> File %s is open and ready to be read.\n", argv[0], jdsFilename);
   }
   /* READING HEADER INFORMATION AND CALCULATING PARAMETERS FOR THE CONVERSION */
   fread(&headerjds, sizeof(headerjds), 1, JDS_IN); /* binary read header of jds file and reading name, time, gmtt, sysn, syst, place, desc, PP, DSPP values */
   printf("%s> Reading file header.\n", argv[0]);
   if (Verbose == 2 || Verbose == 3)
   {
      printf("%s> Header size: %ld bytes\n", argv[0], sizeof(headerjds));
      printf("%s> UPP size: %ld bytes\n", argv[0], sizeof(struct UPP));
      printf("%s> UDSPP size: %ld bytes\n", argv[0], sizeof(struct UDSPP));
      dumpheaderjds(headerjds); /* output all header information */
   }
   sscanf(headerjds.gmtt, "%s %s", stDay, stMonth); /* get the date of observation */
   jy = headerjds.SYSTEMTIME.yr; /* year */
   jm = headerjds.SYSTEMTIME.mn; /* month */
   jd = headerjds.SYSTEMTIME.day; /* day */
   jhr = headerjds.SYSTEMTIME.hr; /* hour */
   jmn = headerjds.SYSTEMTIME.min; /* minute */
   jsc = headerjds.SYSTEMTIME.sec + (headerjds.SYSTEMTIME.msec * 0.001); /* seconds + milliseconds */
   startMJD = gregorian2Julian(jy, jm, jd, (double) jhr, (double) jmn, jsc) - 2400000.5; /* convert the date to JD and subtract 2400000.5 to get MJD */
   printf("%s> Observation started on %04d/%02d/%02d at %02d:%02d:%02.3lf\n", argv[0], jy, jm, jd, jhr, jmn, jsc);
   printf("%s> Time stamp of first sample (MJD): %.9lf\n", argv[0], startMJD);
   spectrumWidth = headerjds.DSPP.Wb; /* original spectral width */
   samplingTime = 8192.0 / headerjds.DSPP.CLCfrq * headerjds.DSPP.NAvr; /* calculation of time resolution: full band spectrum / sampling ADC frequency * number of averaged spectra */
   printf("%s> Sampling time: %f s\n", argv[0], samplingTime);
   Nfmin[0] = 0; /* matrix element storing low bound number of samples for Nf[0] - full spectrum, 8192 samples */
   Nfmin[1] = 0; /* matrix element storing low bound number of samples for Nf[1] - lower half of spectrum, 4096 samples*/
   Nfmin[2] = 4096; /* matrix element storing low bound number of samples for Nf[2] - top half of spectrum, 4096 samples */
   Nfmin[3] = headerjds.DSPP.Lb; /* matrix element storing low bound number of samples for Nf[3] - tunable mode */
   Nf[0] = 8192; /* matrix element storing number of samples: full spectrum, 8192 samples */
   Nf[1] = 4096; /* matrix element storing number of samples: lower half of spectrum, 4096 samples */
   Nf[2] = 4096, /* matrix element storing number of samples: top half of spectrum, 4096 samples */
   Nf[3] = headerjds.DSPP.Wb; /* matrix storing number of samples: tunable mode */
   Nc = 2 - headerjds.DSPP.Ch1 - headerjds.DSPP.Ch2; /* checking if file has one or two channels, if Nc = 1 then only one channel present, if 2 then both channels present */
   printf("%s> File has %d channels.\n", argv[0], Nc);
   rewind(JDS_IN); /* rewinding file to be sure that file will be read from the begining */
   fseek(JDS_IN, 0L, SEEK_END); /* seeking end of file */
   fileSize = ftell(JDS_IN); /* if you use ftell, then you must open the file in binary mode. If you open it in text mode, ftell only returns a "cookie" that is only usable by fseek */
   rewind(JDS_IN); /* rewinding file to be sure that file will be read from the begining */
   maxSpectraNumber = (fileSize - sizeof(headerjds)) / bps / Nf[headerjds.DSPP.Offt] / Nc; /* calculating number of spectra (time samples) in the file */
   fseek(JDS_IN, sizeof(headerjds), SEEK_SET); /* fseek to position after estimating file size */
   channelBW = headerjds.DSPP.CLCfrq / 16384.0; /* channel bandwidth: sampling ADC frequency * amount of samples per stream */
   printf("%s> Number of channels: %d.\n", argv[0], spectrumWidth);
   printf("%s> Channel bandwidth (kHz) : %f\n", argv[0], channelBW/1e3);
   freqFirst = Nfmin[headerjds.DSPP.Offt] * channelBW / 1.0e6; /* lowest observed frequency */
   freqLast = freqFirst + Nf[headerjds.DSPP.Offt] * channelBW / 1.0e6; /* highest observed frequency */
   printf("%s> Frequency of first spectral channel: %f MHz; frequency of last spectral channel: %f MHz.\n", argv[0], freqFirst, freqLast);
   nbytes = spectrumWidth * nbits / byte; /* calculate number of bytes per spectrum... */
   /* SELECTING FREQUENCY RANGE AND CHANGING FREQUENCY RANGE */
   if (Frequency == 1) /* select frequency range... */
   {
      printf("%s> Changing frequency range.\n", argv[0]);
      if (freqLow >= freqLast || freqLow <= freqFirst) /* in case of erroneously given low freqeuncy... */
      {
         printf("%s> Lower frequency range exceeds allowed ranges! Reseting to default value of %f MHz.\n", argv[0], freqFirst);
         freqLow = freqFirst; /* ...reset to lowest value */
      }
      if (freqHigh >= freqLast || freqHigh <= freqFirst) /* in case of erroneously given high freqeuncy... */
      {
         printf("%s> Higher frequency range exceeds allowed ranges! Reseting to default value of %f MHz.\n", argv[0], freqLast);
         freqHigh = freqLast; /* ...reset to lowest value */
      }
      firstSpecChanNum = (freqLow - freqFirst) / (channelBW / 1.0e6); /* calculating number of first spectral channel to use */
      lastSpecChanNum = (freqHigh / (channelBW / 1.0e6)) - (freqFirst / (channelBW / 1.0e6))+1; /* calculating number of first spectral channel to use (+1 is bacause of C array starting from 0 */
      numberChannels = lastSpecChanNum - firstSpecChanNum;
      nbytes = numberChannels * nbits / byte; /* ...or replace the default value of bytes per spectrum with new one */
      freqLow = freqFirst + firstSpecChanNum * channelBW / 1.0e6;
      freqHigh = freqFirst + firstSpecChanNum * channelBW / 1.0e6 + fabsf((lastSpecChanNum - firstSpecChanNum) * channelBW / 1.0e6);
      printf("%s> Changed number of frequency channels to %d\n", argv[0], numberChannels);
      if (InvertChannels == 0)
      {
         printf("%s> New frequency of first channel: %f MHz; new frequency of last channel: %f MHz\n", argv[0], freqLow, freqHigh);
      }
      else
      {
         printf("%s> New frequency of first channel: %f MHz; new frequency of last channel: %f MHz\n", argv[0], freqHigh, freqLow);
      }
   }
   else /* ... or keep input values */
   {
      freqLow = freqFirst;
      freqHigh = freqLast;
      firstSpecChanNum = 0;
      lastSpecChanNum = spectrumWidth;
      nbytes = spectrumWidth * nbits / byte; /* calculate number of bytes per spectrum... */
      numberChannels = spectrumWidth;
   }
   if (InvertChannels == 1)
   {
      printf("%s> Inverting order of frequency channels.\n", argv[0]);
   }

   if (streamOperation == 0)
   {
      printf("%s> Subtracting stream A from stream B (A-B).\n", argv[0]);
   }
   else if (streamOperation == 1)
   {
      printf("%s> Adding stream A to stream B (A+B).\n", argv[0]);
   }
   else if (streamOperation == 2)
   {
      printf("%s> Passing only stream A.\n", argv[0]);
   }
   else if (streamOperation == 3)
   {
      printf("%s> Passing only stream B.\n", argv[0]);
   }
   else if (streamOperation == 4)
   {
      printf("%s> Subtracting stream B from stream A (B-A).\n", argv[0]);
   }
   /* CREATING OUTPUT FILE */
   if (OutFilename == 1)
   {
      if ((filterbankFilename = malloc(strlen(outFilename) + 1)) == NULL)
      {
         fprintf(stderr, "%s> Cannot allocate memory for output filename\n", argv[0]);
         return -1; /* if memory for returnString is not allocated function returns NULL as address of returnString */
      }
      else
      {
         sprintf(filterbankFilename, "%s", outFilename);
      }
   }
   else
   {
      filterbankFilename = removeExtension(jdsFilename, '.', '/'); /* remove extension using external function */
      if (HeaderLess == 0)
      {
         strcat(filterbankFilename, ".fil"); /* add new extension */
      }
      else if (HeaderLess == 1)
      {
         strcat(filterbankFilename, ".dat"); /* add new extension */
      }
   }
   printf("%s> Creating file %s\n", argv[0], filterbankFilename);
   if ((FILTERBANK_OUT = fopen(filterbankFilename, "w")) == NULL)
   {
      fprintf(stderr, "%s> Unable to open file %s\n", argv[0], filterbankFilename);
      return -1;
   }
   /* SENDING HEADER INFORMATION TO .fil FILE HEADER */
   if (HeaderLess == 0)
   {
      send_string("HEADER_START");
      send_string("rawdatafile"); /* setting... */
      send_string(filterbankFilename); /* ... the name of the original data file */
      send_string("source_name"); /* setting... */
      send_string(psrName); /* ... the name of the source being observed by the telescope */
      send_int("machine_id", 12); /* ID of datataking machine, FAKE:0, PSPM:1, WAPP:2, AOFTM:3, BPP:4, OOTY:5, SCAMP:6, SPIGOT:7, PULSAR2000:8, ARTEMIS:10, BG/P:11, DSPZ:12 (not yet added) */
      printf("%s> Using telescope ID: %d\n", argv[0], telescopeID);
      send_int("telescope_id", telescopeID);
      printf("%s> Using source RA: %.7lf and Dec: %.7lf \n", argv[0], RA, Dec);
      send_coords(RA, Dec, 0.0, 0.0); /* RA, Dec, Az, ZA */
      send_int("data_type", 1); /* data category, 1 - filterbank */
      if (InvertChannels == 0)
      {
         send_double("fch1", (double)(freqLow)); /* centre frequency (MHz) of first filterbank channel */
         send_double("foff", (double)(channelBW * 1.0e-6)); /* filterbank channel bandwidth (MHz) */
         specChanNum = firstSpecChanNum; /* number of spectral channel from which data should be written to new file */
      }
      else
      {
         send_double("fch1", (double)(freqHigh));  /* invert freq. order */
         send_double("foff", (double)(-channelBW * 1.0e-6));  /* invert freq. order */
         specChanNum = spectrumWidth - lastSpecChanNum; /* as above but with inverted order of channels */
      }
      send_int("nchans", numberChannels); /* number of filterbank channels */
      send_int("nbits", nbits); /* number of bits per time sample */
      send_double("tstart", startMJD); /* time stamp (MJD) of first sample */
      send_double("tsamp", samplingTime); /* time interval between samples (s) */
      send_int("nifs", 1); /* number of seperate IF channels */
      send_string("HEADER_END");
   }
   else if (HeaderLess == 1)
   {
      if (InvertChannels == 0)
      {
         specChanNum = firstSpecChanNum; /* number of spectral channel from which data should be written to new file */
      }
      else
      {
         specChanNum = spectrumWidth - lastSpecChanNum; /* as above but with inverted order of channels */
      }
   }
   /* ALLOCATING MEMORY FOR DATA ARRAYS */
   printf("%s> Allocating data arrays.\n", argv[0]);
   rawData = (unsigned int *) calloc(bps * spectrumWidth * Nc, sizeof(float)); /* allocate memory for array to read raw data in file */
   dataFloat = (float *) calloc(bps * spectrumWidth * Nc, sizeof(float)); /* allocate memory for array for data after first step of conversion to floats */
   dataReady = (float *) calloc(bps * spectrumWidth * Nc, sizeof(float)); /* allocate memory for array for data after streams A & B subtraction */
   block = (float *) calloc(nbytes, sizeof(float)); /* allocate memory for array for bytes to be actually written to file */
   printf("%s> Reading raw data.\n", argv[0]);
   /* READING FILE WITH CHANNELS TO ZAP */
   if (ZapChannels == 1)
   {
      zapChannelsArr = (int *) calloc(spectrumWidth, sizeof(int)); /* allocate memory to read channels zap file, it is always smaller than spectrumWidth */
      readZapFile(zapFilename, zapChannelsArr, &zapChannelsCounter);
      printf("%s> Opened file %s with list of bad channels.\n", argv[0], zapFilename);
      printf("%s> Zapping %d bad channels: ", argv[0], zapChannelsCounter);
      for (i = 0; i < zapChannelsCounter; i++)
      {
         printf("%d ", zapChannelsArr[i]);
      }
      printf("\n");
      /*printf("%s> cont %d\n", argv[0], zapChannelsCounter);*/
   }
   /* PREPARING BANDPASS */
   if (ClipSpectrum == 1)
   {
      bandpassArray = (float *) calloc(bps * spectrumWidth * Nc, sizeof(float)); /* allocate memory for array for storing bandpass */
      bandpassMedianSmoothed = (float *) calloc(bps * spectrumWidth, sizeof(float)); /* allocate memory for array for storing median smoothed bandpass */
      printf("%s> Preparing median smoothed bandpass with smoothing window of %d channels.\n", argv[0], movingMeanWindow);
      printf("%s> Clipping spectrum channels with intensity greater than %.1f sigma of running spectrum.\n", argv[0], clipValue);
      spectrumCounter = 0;
      /* READING AND CONVERSION FROM DSP TO CONVENTIONAL FLOAT RAW DATA */
      for (j = 0; j < maxSpectraNumber; j++) /* iterating per single spectrum */
      {
         if (fread(rawData, bps * spectrumWidth * Nc, 1, JDS_IN) > 0)
         {
            printf("%s> Preparing bandpass... (reading spectrum %7d)\r", argv[0], spectrumCounter); /* spectrum counter is initialized at the begining of the for loop */
         }
         DSPZ2Float(&headerjds, rawData, spectrumWidth * Nc, dataFloat); /* function converting raw data to floats */
         /* GETTING SERVICE DATA FROM 2 LAST SAMPLES PER CHANNEL, 4 IN TOTAL, AND ZAPPING BAD SPECTRA */
         if (NoCRC == 0)
         {
            DSPZ2Service(&headerjds, rawData, spectrumWidth * Nc, Verbose, &CorruptedFlag); /* function getting service data from spectra and flag if spectrum is corrupted */
            if (CorruptedFlag == 1) /* if there is bad spectrum remove it from the data */
            {
               for (i = 0; i < spectrumWidth * Nc; i++)
               {
                  dataFloat[i] = 0.0; /* removing affected spectrum entirely (maybe replace it with previous or running mean?) */
               }
            }
         }
         /* ADDING/SUBTRACKING STREAM A & B AND REMOVING DIAGNOSTIC SAMPLES */
         for (i = 0; i < spectrumWidth * Nc; i++)
         {
            if (Nc == 2) /* for more than one stream... */
            {
               if (streamOperation == 0) /* ... perform different operations... */
               {
                  dataReady[i] = dataFloat[2*i] - dataFloat[2*i+1]; /* ... subtract stream A from stream B */
               }
               else if (streamOperation == 1)
               {
                  dataReady[i] = dataFloat[2*i] + dataFloat[2*i+1]; /* ... add stream A to stream B */
               }
               else if (streamOperation == 2)
               {
                  dataReady[i] = dataFloat[2*i]; /* ... pass only stream A */
               }
               else if (streamOperation == 3)
               {
                  dataReady[i] = dataFloat[2*i+1]; /* ... pass only stream B */
               }
               else if (streamOperation == 4)
               {
                  dataReady[i] = dataFloat[2*i+1] - dataFloat[2*i]; /* ... subtract stream B from stream A */
               }
            }
            else
            {
               dataReady[i] = dataFloat[i]; /* if only one stream present pass it without doing anything */
            }
            if (i >= spectrumWidth - 2) /* removing values for last, diagnostic samples FOR DATA TAKEN BEFORE 2008 SET TO 1 ELSE USE 2 */
            {
               dataReady[i] = 0.0;
            }
            /*printf("%d %d %f %d %f \n", j, i/Nc, dataReady[i], i, dataFloat[i]);*/
            bandpassArray[i] += dataReady[i];
         }
         spectrumCounter++;
      }
      /* PREPARING BANDPASS */
      for (i = 0; i < spectrumWidth; i++)
      {
         bandpassArray[i] /= maxSpectraNumber;
         if (Verbose == 3)
         {
            printf("%d %f\n", i, bandpassArray[i]);
         }
      }
      /* PREPARING MOVING MEDIAN SMOOTHED BANDPASS */
      movingMedian(spectrumWidth, bandpassArray, movingMeanWindow, bandpassMedianSmoothed);
      for (i = 0; i < spectrumWidth; i++)
      {
         if (Verbose == 3)
         {
            printf("%d %f\n", i, bandpassMedianSmoothed[i]);
         }
      }
      /* CALCULATING AVERAGE AND SIGMA OF THE MEDIAN SMOOTHED BANDPASS */
      averageBandpassMedianSmoothed = averageValue(spectrumWidth, bandpassMedianSmoothed);
      sigmaBandpassMedianSmoothed = standardDeviation(spectrumWidth, bandpassMedianSmoothed);
      if (Verbose == 3)
      {
         printf("average of bandpassMedianSmoothed: %f\n", averageBandpassMedianSmoothed);
         printf("sigma of bandpassMedianSmoothed: %f\n", sigmaBandpassMedianSmoothed);
      }
   }
   /* FOR LOOP READING, CONVERTING, PERFORMING OPERATIONS AND FINAL WRITING OF DATA */
   rewind(JDS_IN); /* rewinding file to be sure that file will be read from the begining */
   fseek(JDS_IN, sizeof(headerjds), SEEK_SET); /* fseek to position after header */
   spectrumCounter = 0;
   for (j = 0; j < maxSpectraNumber; j++) /* iterating per single spectrum */
   {
      /* READING AND CONVERSION FROM DSP TO CONVENTIONAL FLOAT RAW DATA */
      if (fread(rawData, bps * spectrumWidth * Nc, 1, JDS_IN) > 0)
      {
         printf("%s> Reading spectrum %7d\r", argv[0], spectrumCounter); /* spectrum counter is initialized at the begining of the for loop */
      }
      DSPZ2Float(&headerjds, rawData, spectrumWidth * Nc, dataFloat); /* function converting raw data to floats */
      /* GETTING SERVICE DATA FROM 2 LAST SAMPLES PER CHANNEL, 4 IN TOTAL, AND ZAPPING BAD SPECTRA */
      if (NoCRC == 0)
      {
         DSPZ2Service(&headerjds, rawData, spectrumWidth * Nc, Verbose, &CorruptedFlag); /* function getting service data from spectra and flag if spectrum is corrupted */
         if (CorruptedFlag == 1) /* if there is bad spectrum remove it from the data */
         {
            if (Verbose == 1 || Verbose == 2 || Verbose == 3)
            {
               printf("%s> Zapping corrupted spectrum number %d\n", argv[0], j);
            }
            for (i = 0; i < spectrumWidth * Nc; i++)
            {
               if (Verbose == 3)
               {
                  printf("%d %d %f\n", j, i, dataFloat[i]);
               }
               dataFloat[i] = 0.0; /* removing affected spectrum entirely (maybe replace it with previous or running mean?) */
            }
         }
      }
      /* ADDING/SUBTRACKING STREAM A & B AND REMOVING DIAGNOSTIC SAMPLES */
      for (i = 0; i < spectrumWidth * Nc; i++)
      {
         if (Nc == 2) /* for more than one stream... */
         {
            if (streamOperation == 0) /* ... perform different operations... */
            {
               dataReady[i] = dataFloat[2*i] - dataFloat[2*i+1]; /* ... subtract stream A from stream B */
            }
            else if (streamOperation == 1)
            {
               dataReady[i] = dataFloat[2*i] + dataFloat[2*i+1]; /* ... add stream A to stream B */
            }
            else if (streamOperation == 2)
            {
               dataReady[i] = dataFloat[2*i]; /* ... pass only stream A */
            }
            else if (streamOperation == 3)
            {
               dataReady[i] = dataFloat[2*i+1]; /* ... pass only stream B */
            }
            else if (streamOperation == 4)
            {
               dataReady[i] = dataFloat[2*i+1] - dataFloat[2*i]; /* ... subtract stream B from stream A */
            }
         }
         else
         {
            dataReady[i] = dataFloat[i]; /* if only one stream present pass it without doing anything */
         }
         if (i >= spectrumWidth - 2) /* removing values for last, diagnostic samples FOR DATA TAKEN BEFORE 2008 SET TO 1 ELSE USE 2 */
         {
            dataReady[i] = 0.0;
         }
         /*printf("%d %d %f %d %f \n", j, i/Nc, dataReady[i], i, dataFloat[i]);*/
      }
      /* CLEAN THE SPECTRA FROM IMPULSIVE RFI AND NEGATIVE VALUES*/
      if (ClipSpectrum == 1)
      {
         for (i = 0; i < spectrumWidth; i++) /* for every spectrum... */
         {
            if (dataReady[i] >= clipValue * sigmaBandpassMedianSmoothed)
            {
               dataReady[i] = clipValue * sigmaBandpassMedianSmoothed;
            }
            else if (dataReady[i] < 0.0)
            {
               dataReady[i] = 0.0;
            }
         }
      }
      /* ZAP BAD CHANNELS */
      if (ZapChannels == 1)
      {
         for (i = 0; i < spectrumWidth; i++) /* for every spectrum... */
         {
            for (k = 0; k < zapChannelsCounter; k++) /* ... go through channels list to be zapped... */
            {
               if (zapChannelsArr[k] == i) /* ... and if you find some... */
               {
                  /*printf("BAD CHANNEL %d %f %d\n", i, dataReady[i], zap_channels[k]);*/
                  if (InvertChannels == 1) /* (if channels are to be reversed) */
                  {
                     dataReady[spectrumWidth - i] = 0.0;  /* ... zap them! */
                  }
                  else
                  {
                     dataReady[i] = 0.0; /* ... zap them! */
                  }
               }
            }
         }
      }
      /* TRANSFER DATA IN block ARRAY */
      for (i = 0; i < spectrumWidth; i++)
      {
         block[i] = dataReady[i];
         if (Verbose == 3)
         {
            printf("%d %d %f\n", j, i, block[i]);
         }
      }
      /* REVERSING block ARRAY */
      if (InvertChannels == 1)
      {
         reverseArray(block, spectrumWidth); /* if we want to transfer channels inversed */
      }
      /* WRITING IN THE OUTPUT FILE (ONE SPECTRUM AT A TIME) */
      fwrite(&block[specChanNum], nbytes, 1, FILTERBANK_OUT);
      spectrumCounter++;
      /* ZEROING ARRAYS */
      for (i = 0; i < spectrumWidth * Nc; i++)
      {
         dataFloat[i] = 0.0;
         dataReady[i] = 0.0;
         block[i] = 0.0;
      }
   }
   /* NUMBER OF CHANNELS PER SPECTRUM AND NUMBER OF SPECTRA */
   if (Nc == 2)
   {
      printf("%s> Found %d twin spectra of %d channels each.\n", argv[0], spectrumCounter, spectrumWidth);
   }
   else
   {
      printf("%s> Found %d single spectra of %d channels each.\n", argv[0], spectrumCounter, spectrumWidth);
   }
   printf("%s> Closing file %s\n", argv[0], filterbankFilename);
   fclose(FILTERBANK_OUT);
   return 0;
}

void send_string(char *string)
{
  int len;
  len = strlen(string);
  fwrite(&len, sizeof(int), 1, FILTERBANK_OUT);
  fwrite(string, sizeof(char), len, FILTERBANK_OUT);
}

void send_float(char *name, float floating_point)
{
  send_string(name);
  fwrite(&floating_point, sizeof(float), 1, FILTERBANK_OUT);
}

void send_double (char *name, double double_precision)
{
  send_string(name);
  fwrite(&double_precision, sizeof(double), 1, FILTERBANK_OUT);
}

void send_long(char *name, long integer)
{
  send_string(name);
  fwrite(&integer, sizeof(long), 1, FILTERBANK_OUT);
}

void send_int(char *name, int integer)
{
  send_string(name);
  fwrite(&integer, sizeof(int), 1, FILTERBANK_OUT);
}

void send_coords(double raj, double dej, double az, double za)
{
  if ((raj != 0.0) || (raj != -1.0)) send_double("src_raj", raj);
  if ((dej != 0.0) || (dej != -1.0)) send_double("src_dej", dej);
  if ((az != 0.0)  || (az != -1.0))  send_double("az_start", az);
  if ((za != 0.0)  || (za != -1.0))  send_double("za_start", za);
}
