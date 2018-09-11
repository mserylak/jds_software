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
#include "jds_header.h" /* JDS header definition*/
#include "jds_tools.h" /* definitions of header extraction and data conversion functions */

/* initialization of global variables */
unsigned int *rawData; /* allocate pointer to array to read raw data from file */
int *zapChannelsArr; /* allocate pointer to array storing bad channels to zap */
float *block; /* allocate pointer to array for bytes to be actually written to file */
float *dataFloat; /* allocate pointer to array for data after first step of conversion to floats */
float *dataReady; /* allocate pointer to array for data after streams A & B subtraction */
float *bandpassMedianSmoothed; /* allocate pointer to array for running mean spectrum */
float *bandpassArray; /* allocate pointer to array for bandpass */
FILE *FILTERBANK_OUT; /* pointer to output file */

/* main body of the program */
int main(int argc, char *argv[])
{
  int i, j, argument, k;
  int spectrumCounter, maxSpectraNumber, spectrumWidth;
  int nbytes, nbits, bps, byte;
  int jd, jm, jy, jhr, jmn;
  int telescopeID;
  int freqRanges, clipInput;
  int firstSpecChanNum, lastSpecChanNum, numberChannels, specChanNum;
  int zapChannelsCounter, CorruptedFlag, movingMeanWindow, streamOperation, correlationStreamOperation;
  int NoCRC, CorrelationDSPZ, HeaderLess, InvertChannels, ZapChannels, ClipSpectrum, Frequency, OutFilename, Verbose;
  int Nfmin[4], Nf[4], Nc;
  char stDay[32], stMonth[32], RAtxt[16], DecTxt[16];
  char psrName[64], jdsFilename[256], zapFilename[256], outFilename[256];
  float freqFirst, freqLast, channelBW, freqLow, freqHigh, clipValue;
  float averageBandpassMedianSmoothed, sigmaBandpassMedianSmoothed;
  double samplingTime, RA, Dec, RAh, RAm, RAs, DecD, DecM, DecS;
  double startMJD, jsc, fileSize;
  char *filterbankFilename;
  FILE *JDS_IN;

  /* pre-set flags & default values */
  bps = 4; /* bytes per sample */
  byte = 8; /* bytes per bit */
  nbits = 32; /* number of bits in the output file */
  telescopeID = 15; /* ID of the telescope, using SIGPYPROC convention */
  strcpy(psrName, "B0950+08"); /* name of the source */
  RA = 95309.3097; /* right ascension of the source */
  Dec = 075535.75; /* declination of the source */
  spectrumCounter = 0; /* counter of the spectra being processed */
  maxSpectraNumber = 0; /* number of spectra in the file, based on the file size */
  clipValue = 10.0;
  movingMeanWindow = 256;
  averageBandpassMedianSmoothed = 0.0;
  sigmaBandpassMedianSmoothed = 0.0;
  streamOperation = 0; /* stream handling, 0 -> A-B, 1 -> A+B, 2 -> A, 3 -> B, 4 -> B-A */
  correlationStreamOperation = 4; /* correlation stream handling, 0 -> A, 1 -> B, 2 -> Re(A*B), 3 -> Im(A*B), 4 -> power(A*B) */
  HeaderLess = 0; /* do not write header */
  CorruptedFlag = 0; /* flag signalling corrupted spectrum (ADC or CRC error) */
  NoCRC = 0; /* flag activating removal of spectra with ADC overflows and CRC errors */
  Frequency = 0; /* flag activating frequency range */
  InvertChannels = 1; /* flag activating inverting the channels order */
  ZapChannels = 0; /* flag activating channel zapping */
  ClipSpectrum = 0; /* flag activating spectrum clipping */
  OutFilename = 0; /* flag for giving a new output file name */
  Verbose = 0; /* flag activating verbose mode, 1 - normal verbose, 2 - very verbose, 3 - even more verbose */

  /* available help */
  if (argc < 2 || (strcmp(argv[1], "-h") == 0)) /* in case there is a -h switch */
  {
    printf("\nProgram converts an UTR-2 data format file to 32-bit filterbank format file.\n");
    printf("\nUsage: jds2fil [parameters] <filename>\n");
    printf("\nAvailable parameters are:\n");
    printf("-psrname     Name of the source to be written in .fil file (default: %s)\n", psrName);
    printf("-ra          Right Ascension (J2000) of the source in hh:mm:ss.ssss (default: 09:53:09.3097)\n");
    printf("-dec         Declination (J2000) of the source in dd:mm:ss.ssss (default: +07:55:35.75)\n");
    printf("-tel         Telescope SIGPROC identifier (UTR2: 15, etc.; default: %d)\n", telescopeID);
    printf("-freq        Lowest and highest frequency to keep (default: all)\n");
    printf("-headerless  Write out data without any header info (default: with header)\n");
    printf("-noinv       Turn off inverting order of frequency channels (default: invert)\n");
    printf("-zap         File with channel numbers to be zapped (input in column, numbers will be sorted upon opening)\n");
    printf("-clip        Clip channels using median smoothed difference (default: %.1f sigma, smoothing window of %d spectra)\n", clipValue, movingMeanWindow);
    printf("-stream      Stream operation (A-B: 0, A+B: 1, A: 2, B: 3, B-A: 4; disabled when -corr is used; default: %d).\n", streamOperation);
    printf("-nocrc       Disable automatic removal of spectra with ADC overflow and CRC errors (n.b. use when converting pre-Aug 2008 data, default: remove).\n");
    printf("-corr        Process raw data in non-standard DSPZ correlation mode (A: 0; B: 1; Re(A*B): 2; Im(A*B): 3; power(A*B): 4; default: %d).\n", correlationStreamOperation);
    printf("-o           Give new name of output file (default: add .fil to original file name)\n");
    printf("-v           Verbose mode\n");
    printf("-vv          Very verbose mode\n");
    printf("-vvv         Even more verbose mode (huge output)\n");
    printf("-h           Display this useful help page\n\n");
    return -1;
  }

  /* reading command line arguments */
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
        if (strncmp (DecTxt, "-", 1) == 0)
        {
          Dec = (DecD * 10000.0) + (DecM * -100.0) + (-1.0 * DecS);
        }
        else
        {
          Dec = (DecD * 10000.0) + (DecM * 100.0) + DecS;
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
          fprintf(stderr, "%s> No output file name specified.\n", argv[0]);
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
      else if (strcmp(argv[argument], "-corr") == 0)
      {
        printf("%s> WARNING! Special correlation mode selected. Some of options for data conversion might not work as advertised.\n", argv[0]);
        correlationStreamOperation = atoi(argv[argument+1]);
        if (correlationStreamOperation < 0 || correlationStreamOperation > 4)
        {
          fprintf(stderr, "%s> Error parsing -corr option.\n", argv[0]);
          return -1;
        }
        CorrelationDSPZ = 1;
        argument++;
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

  /* open file */
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

  /* reading header information and calculating parameters for the conversion */
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
  if (CorrelationDSPZ == 1)
  {
    Nc = 4; /* data contains non-standard 4 data stream (stream A, stream B, real A*B, imaginary A*B) */
  }
  else
  {
    Nc = 2 - headerjds.DSPP.Ch1 - headerjds.DSPP.Ch2; /* checking if file has one or two streams, if Nc = 1 then only one stream present, if 2 then both streams present */
  }
  printf("%s> File has %d data streams.\n", argv[0], Nc);
  rewind(JDS_IN); /* rewinding file to be sure that file will be read from the beginning */
  fseek(JDS_IN, 0L, SEEK_END); /* seeking end of file */
  fileSize = ftell(JDS_IN); /* if you use ftell, then you must open the file in binary mode. If you open it in text mode, ftell only returns a "cookie" that is only usable by fseek */
  rewind(JDS_IN); /* rewinding file to be sure that file will be read from the beginning */
  maxSpectraNumber = (fileSize - sizeof(headerjds)) / bps / Nf[headerjds.DSPP.Offt] / Nc; /* calculating number of spectra (time samples) in the file */
  fseek(JDS_IN, sizeof(headerjds), SEEK_SET); /* fseek to position just after header after estimating file size */
  channelBW = headerjds.DSPP.CLCfrq / 16384.0; /* channel bandwidth: sampling ADC frequency * amount of samples per stream */
  printf("%s> Number of frequency channels: %d.\n", argv[0], spectrumWidth);
  printf("%s> Frequency channel bandwidth (kHz) : %f\n", argv[0], channelBW / 1e3);
  freqFirst = Nfmin[headerjds.DSPP.Offt] * channelBW / 1.0e6; /* lowest observed frequency */
  freqLast = freqFirst + Nf[headerjds.DSPP.Offt] * channelBW / 1.0e6; /* highest observed frequency */
  printf("%s> Frequency of first channel: %f MHz; frequency of last channel: %f MHz.\n", argv[0], freqFirst, freqLast);
  nbytes = spectrumWidth * nbits / byte; /* calculate number of bytes per spectrum... */

  /* selecting frequency range and changing frequency range */
  if (Frequency == 1) /* select frequency range... */
  {
    printf("%s> Changing frequency range.\n", argv[0]);
    if (freqLow >= freqLast || freqLow <= freqFirst) /* in case of erroneously given low frequency... */
    {
      printf("%s> Lower frequency range exceeds allowed ranges! Resetting to default value of %f MHz.\n", argv[0], freqFirst);
      freqLow = freqFirst; /* ...reset to lowest value */
    }
    if (freqHigh >= freqLast || freqHigh <= freqFirst) /* in case of erroneously given high frequency... */
    {
      printf("%s> Higher frequency range exceeds allowed ranges! Resetting to default value of %f MHz.\n", argv[0], freqLast);
      freqHigh = freqLast; /* ...reset to lowest value */
    }
    firstSpecChanNum = (freqLow - freqFirst) / (channelBW / 1.0e6); /* calculating number of first spectral channel to use */
    lastSpecChanNum = (freqHigh / (channelBW / 1.0e6)) - (freqFirst / (channelBW / 1.0e6)) + 1; /* calculating number of first spectral channel to use (+1 is because of C array starting from 0 */
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

  if (streamOperation == 0 && Nc == 2)
  {
    printf("%s> Subtracting stream A from stream B (A-B) for standard DSPZ data.\n", argv[0]);
  }
  else if (streamOperation == 1 && Nc == 2)
  {
    printf("%s> Adding stream A to stream B (A+B) for standard DSPZ data.\n", argv[0]);
  }
  else if (streamOperation == 2 && Nc == 2)
  {
    printf("%s> Passing only stream A for standard DSPZ data.\n", argv[0]);
  }
  else if (streamOperation == 3 && Nc == 2)
  {
    printf("%s> Passing only stream B for standard DSPZ data.\n", argv[0]);
  }
  else if (streamOperation == 4 && Nc == 2)
  {
    printf("%s> Subtracting stream B from stream A (B-A) for standard DSPZ data.\n", argv[0]);
  }

  if (CorrelationDSPZ == 1 && correlationStreamOperation == 0)
  {
    printf("%s> Extracting power of stream 0 from the non-standard correlation DSPZ data.\n", argv[0]);
  }
  else if (CorrelationDSPZ == 1 && correlationStreamOperation == 1)
  {
    printf("%s> Extracting power of stream 1 from the non-standard correlation DSPZ data.\n", argv[0]);
  }
  else if (CorrelationDSPZ == 1 && correlationStreamOperation == 2)
  {
    printf("%s> Extracting the real part of cross-correlation from the non-standard correlation DSPZ data.\n", argv[0]);
  }
  else if (CorrelationDSPZ == 1 && correlationStreamOperation == 3)
  {
    printf("%s> Extracting the imaginary part of the complex cross-correlation from the non-standard correlation DSPZ data.\n", argv[0]);
  }
  else if (CorrelationDSPZ == 1 && correlationStreamOperation == 4)
  {
    printf("%s> Extracting power (sqrt(A^2 + B^2)) from the non-standard correlation DSPZ data.\n", argv[0]);
  }

  /* creating output file */
  if (OutFilename == 1)
  {
    if ((filterbankFilename = malloc(strlen(outFilename) + 1)) == NULL)
    {
      fprintf(stderr, "%s> Cannot allocate memory for output file name\n", argv[0]);
      return -1;
    }
    else
    {
      sprintf(filterbankFilename, "%s", outFilename);
    }
  }
  else
  {
    filterbankFilename = removeExtension(jdsFilename, '.', '/'); /* remove file name extension using external function */
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

  /* sending header information to .fil file header */
  if (HeaderLess == 0)
  {
    send_string("HEADER_START");
    send_string("rawdatafile"); /* setting... */
    send_string(filterbankFilename); /* ... the name of the newly created data file */
    send_string("source_name"); /* setting... */
    send_string(psrName); /* ... the name of the source being observed by the telescope */
    send_int("machine_id", 15); /* ID of data taking machine, FAKE:0, PSPM:1, WAPP:2, AOFTM:3, BPP:4, OOTY:5, SCAMP:6, SPIGOT:7, PULSAR2000:8, ARTEMIS:10, BG/P:11, DSPZ:15 (not yet added) */
    printf("%s> Writing telescope ID: %d\n", argv[0], telescopeID);
    send_int("telescope_id", telescopeID);
    printf("%s> Writing source RA: %.7lf and Dec: %.7lf \n", argv[0], RA, Dec);
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
    send_int("nifs", 1); /* number of separate IF channels */
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

  /* allocating memory for data arrays */
  printf("%s> Allocating data arrays.\n", argv[0]);
  rawData = (unsigned int *) calloc(bps * spectrumWidth * Nc, sizeof(float)); /* allocate memory for array to read raw data in file */
  dataFloat = (float *) calloc(bps * spectrumWidth * Nc, sizeof(float)); /* allocate memory for array for data after first step of conversion to floats */
  dataReady = (float *) calloc(bps * spectrumWidth * Nc, sizeof(float)); /* allocate memory for array for data after streams A & B subtraction */
  block = (float *) calloc(nbytes, sizeof(float)); /* allocate memory for array for bytes to be actually written to file */
  printf("%s> Reading raw data.\n", argv[0]);

  /* reading file with channels to zap */
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

  /* prepare bandpass for median smoothed spectrum clipping */
  if (ClipSpectrum == 1)
  {
    bandpassArray = (float *) calloc(bps * spectrumWidth * Nc, sizeof(float)); /* allocate memory for array for storing bandpass */
    bandpassMedianSmoothed = (float *) calloc(bps * spectrumWidth, sizeof(float)); /* allocate memory for array for storing median smoothed bandpass */
    printf("%s> Preparing median smoothed bandpass with smoothing window of %d frequency channels.\n", argv[0], movingMeanWindow);
    printf("%s> Clipping spectrum channels with intensity greater than %.1f sigma of running spectrum.\n", argv[0], clipValue);
    spectrumCounter = 0;

    /* reading and conversion from dsp to conventional float raw data */
    for (j = 0; j < maxSpectraNumber; j++) /* iterating per single spectrum */
    {
      if (fread(rawData, bps * spectrumWidth * Nc, 1, JDS_IN) > 0)
      {
        printf("%s> Preparing bandpass... (reading spectrum %7d)\r", argv[0], spectrumCounter); /* spectrum counter is initialized at the beginning of the for loop */
      }
      if (CorrelationDSPZ == 1)
      {
        DSPZ2Float(&headerjds, rawData, spectrumWidth * Nc, correlationStreamOperation, dataFloat); /* function converting raw data to floats */
      }
      else
      {
        DSPZ2Float(&headerjds, rawData, spectrumWidth * Nc, 99, dataFloat); /* function converting raw data to floats */
      }

      /* getting service data from 2 last samples per channel, 4 in total, and zapping bad spectra */
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

      /* adding/subtracting stream A & B and removing diagnostic samples */
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

    /* preparing bandpass */
    for (i = 0; i < spectrumWidth; i++)
    {
      bandpassArray[i] /= maxSpectraNumber;
      if (Verbose == 3)
      {
        printf("%d %f\n", i, bandpassArray[i]);
      }
    }

    /* preparing moving median smoothed bandpass */
    movingMedian(spectrumWidth, bandpassArray, movingMeanWindow, bandpassMedianSmoothed);
    for (i = 0; i < spectrumWidth; i++)
    {
      if (Verbose == 3)
      {
        printf("%d %f\n", i, bandpassMedianSmoothed[i]);
      }
    }

    /* calculating average and sigma of the median smoothed bandpass */
    averageBandpassMedianSmoothed = averageValue(spectrumWidth, bandpassMedianSmoothed);
    sigmaBandpassMedianSmoothed = standardDeviation(spectrumWidth, bandpassMedianSmoothed);
    if (Verbose == 3)
    {
      printf("average of bandpassMedianSmoothed: %f\n", averageBandpassMedianSmoothed);
      printf("sigma of bandpassMedianSmoothed: %f\n", sigmaBandpassMedianSmoothed);
    }
  }

  /* loop reading, converting, performing operations and final writing of data */
  rewind(JDS_IN); /* rewinding file to be sure that file will be read from the beginning */
  fseek(JDS_IN, sizeof(headerjds), SEEK_SET); /* fseek to position after header */
  spectrumCounter = 0;
  for (j = 0; j < maxSpectraNumber; j++) /* iterating per single spectrum */
  {
    /* reading and conversion from dsp to conventional float raw data */
    if (fread(rawData, bps * spectrumWidth * Nc, 1, JDS_IN) > 0)
    {
      printf("%s> Reading spectrum %7d\r", argv[0], spectrumCounter); /* spectrum counter is initialized at the beginning of the for loop */
    }
    if (CorrelationDSPZ == 1)
    {
      DSPZ2Float(&headerjds, rawData, spectrumWidth * Nc, correlationStreamOperation, dataFloat); /* function converting raw data to floats */
    }
    else
    {
      DSPZ2Float(&headerjds, rawData, spectrumWidth * Nc, 99, dataFloat); /* function converting raw data to floats */
    }
    /* getting service data from 2 last samples per channel, 4 in total, and zapping bad spectra */
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

    /* adding/subtracting stream A & B and removing diagnostic samples */
    for (i = 0; i < spectrumWidth * Nc; i++)
    {
      if (Nc == 2) /* for more than one stream... */
      {
        if (streamOperation == 0) /* ... perform different operations... */
        {
          dataReady[i] = dataFloat[2 * i] - dataFloat[2 * i + 1]; /* ... subtract stream A from stream B */
        }
        else if (streamOperation == 1)
        {
          dataReady[i] = dataFloat[2 * i] + dataFloat[2 * i + 1]; /* ... add stream A to stream B */
        }
        else if (streamOperation == 2)
        {
          dataReady[i] = dataFloat[2 * i]; /* ... pass only stream A */
        }
        else if (streamOperation == 3)
        {
          dataReady[i] = dataFloat[2 * i + 1]; /* ... pass only stream B */
        }
        else if (streamOperation == 4)
        {
          dataReady[i] = dataFloat[2 * i + 1] - dataFloat[2 * i]; /* ... subtract stream B from stream A */
        }
      }
      else if (Nc == 4 || Nc == 1)
      {
        dataReady[i] = dataFloat[i]; /* extract data stream 3: real part of A*B */
      }

      /* removing values for last, diagnostic samples for data taken before 2008 set to 1 else use 2 */
      if (i >= spectrumWidth - 2)
      {
        dataReady[i] = 0.0;
      }
      /*printf("%d %d %f %d %f \n", j, i/Nc, dataReady[i], i, dataFloat[i]);*/
    }

    /* clean the spectra from impulsive RFI and negative values */
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

    /* zap bad channels */
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

    /* transfer data in block array */
    for (i = 0; i < spectrumWidth; i++)
    {
      block[i] = dataReady[i];
      if (Verbose == 3)
      {
        printf("%d %d %f\n", j, i, block[i]);
      }
    }

    /* reversing block array */
    if (InvertChannels == 1)
    {
      reverseArray(block, spectrumWidth); /* if we want to transfer channels inverted */
    }

    /* writing in the output file (one spectrum at a time) */
    fwrite(&block[specChanNum], nbytes, 1, FILTERBANK_OUT);
    spectrumCounter++;

    /* zeroing arrays */
    for (i = 0; i < spectrumWidth * Nc; i++)
    {
      dataFloat[i] = 0.0;
      dataReady[i] = 0.0;
      block[i] = 0.0;
    }
  }

  /* number of channels per spectrum and number of spectra */
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
