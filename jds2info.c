#define _FILE_OFFSET_BITS 64 /* making sure that programs can handle large files */
#define _USE_LARGEFILE 1 /* ditto */
#define _USE_LARGEFILE 1 /* ditto */
#define _LARGEFILE64_SOURCE 1 /* ditto */
#define _LARGEFILE_SOURCE 1 /* ditto */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "jds.h" /* jds header definition*/
#include "jdstools.h" /* definitions of header extraction and data conversion functions */

/* main body of the program */
int main(int argc, char *argv[], char *arge[])
{
  int argument, argument_counter, spectrum_counter, i, spectrum_width, info_counter;
  int jd, jm, jy, jhr, jmn, unique_spectrum_id, last_unique_spectrum_id, do_loop;
  int Nfmin[4], Nf[4], Nc, bps;
  int Put_Info, Verbose, Change_Tag_File, Correlation;
  int *raw_data;
  char stday[32], stmonth[32], stmode[8];
  char jds_filename[1024], string[1024], info[1024][1024];
  char jds_timetag_file[1024] = "jds.timetag";
  float fminf, fmaxf;
  float sampling_time, channel_bw;
  double start_MJD, jsc;
  FILE *JDS_IN, *TAG_OUT; /* pointers to jds file (JDS_IN) and to output file (TAG_OUT) where time tags are stored */

  /* pre-set options & default values */
  argument_counter = 1; /* counts argument values, argument_counter = 0 is name of the program */
  info_counter = 0;
  bps = 4;
  do_loop = 1; /* entry condition for reading the command line arguments */
  Put_Info = 1;
  Change_Tag_File = 1;
  Verbose = 0;

  /* available help */
  if (argc < 2 || (strcmp(argv[1], "-h") == 0)) /* in case there is a -h switch */
  {
    puts("\nUsage: jds2info [parameters] <file name> ... <file name>\n");
    puts("Available parameters are:\n");
    printf("-tag   Specify filename where tags will be written (default: %s)\n", jds_timetag_file);
    printf("-v     Verbose mode\n");
    printf("-c     Data contains 4 streams (default: false)\n");
    printf("-h     Display this useful help page\n\n");
    return -1;
  }

  /* reading command line arguments */
  while (do_loop == 1) /* work as long as the do_loop is 1 */
  {
    do_loop = 0; /* exit condition if both flags are found */
    if (strcmp(argv[argument_counter], "-tag") == 0)
    {
      strcpy(jds_timetag_file, argv[argument_counter+1]);
      argument_counter+=2; /* increasing by 2 because -tag <file name> */
      do_loop = 1;
    }
    if (strcmp(argv[argument_counter], "-v") == 0)
    {
      Verbose = 1;
      argument_counter++;
      do_loop = 1;
    }
    else if (strcmp(argv[argument_counter], "-c") == 0)
    {
      Correlation = 1;
      argument_counter++;
      do_loop = 1;
    }
  }

  /* find jds.timetag file, if found use it */
  if ((TAG_OUT = fopen(jds_timetag_file, "r")) == NULL)
  {
    printf("%s> Unable to open file %s or this file does not exists!\n", argv[0], jds_timetag_file);
    info_counter = 0;
  } 
  else
  {
    info_counter = 0;
    while ((fgets(string, 256, TAG_OUT)) && !feof(TAG_OUT))
    {
      strcpy(info[info_counter], string);
      info_counter++;
    }
    fclose(TAG_OUT);
  }
  if (info_counter == 0)
  {
    printf("%s> Found no info in file %s\n", argv[0], jds_timetag_file);
  }
  else
  {
    printf("%s> Found %d info in file %s\n", argv[0], info_counter, jds_timetag_file);
  }
  if (Verbose == 1)
  {
    for (i = 0; i < info_counter; i++)
    {
      printf("%s> [%d] = %s", argv[0], i, info[i]);
    }
  }

  /* loop over all the files */
  for (argument = argument_counter; argument < argc; argument++)
  {
    strcpy(jds_filename, argv[argument]); /* get the name of the file */
    printf("%s>\n%s> File number %04d out of %04d %s\n", argv[0], argv[0], argument-argument_counter+1, argc-argument_counter, jds_filename);

    /* search if this file is already logged in the tag file */
    Put_Info = 1; /* set the flag at the beginning of the loop */
    for (i = 0; i < info_counter; i++)
    {
      if (strstr(info[i], jds_filename) != NULL) /* search for the existing entry in the tag file */
      {
        printf("%s> Information about file %s already exists in line %d\n", argv[0], jds_filename, i+1);
        printf("%s>\n", argv[0]);
        printf("%s> %s", argv[0], info[i]);
        Put_Info = 0; /* do not open the file, go to the end of the loop */
      }
    }

    /* getting data from the file */
    if (Put_Info == 1)
    {
      /* open jds file */
      if ((JDS_IN = fopen64(jds_filename, "r")) == NULL)
      {
        printf("%s> Unable to open data file %s\n", argv[0], jds_filename);
        return -1;
      }
      else
      {
        printf("%s> File is open and ready to be read.\n", argv[0]);
      }

      /* read the header */
      fread(&headerjds, sizeof(headerjds), 1, JDS_IN);
      if (Verbose == 1)
      {
        dumpheaderjds(headerjds);
      }
      sprintf(stmode, "SPEC");
      if ((headerjds.DSPP.Mode) == 0)
      {
        sprintf(stmode,"WAVE");
      }
      printf("%s> Reading data file.\n", argv[0]);
      sscanf(headerjds.gmtt, "%s %s", stday, stmonth); /* get the date of observation */
      jy = headerjds.SYSTEMTIME.yr; /* year */
      jm = headerjds.SYSTEMTIME.mn; /* month */
      jd = headerjds.SYSTEMTIME.day; /* day */
      jhr = headerjds.SYSTEMTIME.hr; /* hour */
      jmn = headerjds.SYSTEMTIME.min; /* minute */
      jsc = headerjds.SYSTEMTIME.sec + (headerjds.SYSTEMTIME.msec * 0.001); /* seconds + milliseconds */
      start_MJD = gregorian2Julian(jy, jm, jd, (double)jhr, (double)jmn, jsc) - 2400000.5; /* convert the date to JD and subtract 2400000.5 to get MJD */
      printf("%s> Observation started on %04d/%02d/%02d at %02d:%02d:%02.3f\n", argv[0], jy, jm, jd, jhr, jmn, jsc);
      printf("%s> Time stamp of first sample (MJD): %.9lf\n", argv[0], start_MJD);
      spectrum_width = headerjds.DSPP.Wb; /* original spectral width */
      sampling_time = 8192.0 / headerjds.DSPP.CLCfrq * headerjds.DSPP.NAvr; /* calculation of time resolution: full band spectrum / sampling ADC frequency * number of averaged spectra */
      printf("%s> Sampling time: %f s\n", argv[0], sampling_time);
      Nfmin[0] = 0; /* matrix storing low bound number of samples */
      Nfmin[1] = 0; /* matrix storing low bound number of samples */
      Nfmin[2] = 4096; /* matrix storing low bound number of samples */
      Nfmin[3] = headerjds.DSPP.Lb; /* matrix storing low bound number of samples if it is in tunable mode */
      Nf[0] = 8192; /* matrix storing number of samples: full spectrum, 8192 samples */
      Nf[1] = 4096; /* matrix storing number of samples: lower half of spectrum, 4096 samples */
      Nf[2] = 4096, /* matrix storing number of samples: top half of spectrum, 4096 samples; 3:tunable */
      Nf[3] = headerjds.DSPP.Wb; /* matrix storing number of samples: tunable mode */
      if (Correlation == 1)
      {
        Nc = 4; /* data contains 4 data stream (stream A, stream B, real A*B, imaginary A*B) */
      }
      else
      {
        Nc = 2 - headerjds.DSPP.Ch1 - headerjds.DSPP.Ch2; /* checking if file has one or two streams, if Nc = 1 then only one stream present, if 2 then both stream present */
      }
      printf("%s> File has %d streams.\n", argv[0], Nc);
      channel_bw = headerjds.DSPP.CLCfrq / 16384.0;
      fminf = Nfmin[headerjds.DSPP.Offt] * channel_bw / 1.0e6;
      fmaxf = fminf + Nf[headerjds.DSPP.Offt] * channel_bw / 1.0e6;
      printf("%s> %d channels with bandwidth: %f kHz.\n", argv[0], spectrum_width, channel_bw/1e3);
      printf("%s> Frequency of first channel: %f MHz; frequency of last channel: %f MHz.\n", argv[0], fminf, fmaxf);

      /* allocate data array (8192 is the maximum allowed number of points in the spectra) and read the first spectra */
      raw_data = (int *)malloc(bps * 8192 * Nc);
      fread(raw_data, bps * headerjds.DSPP.Wb * Nc, 1, JDS_IN);

      /* determine the current spectra index (the first one) */
      unique_spectrum_id = raw_data[headerjds.DSPP.Wb * Nc - 1] & 0x7FFFFFFF;
      spectrum_counter = 0;

      /* determine the index of the following spectra */
      while (fread(raw_data, bps * headerjds.DSPP.Wb * Nc, 1, JDS_IN) > 0)
      {
        last_unique_spectrum_id = raw_data[headerjds.DSPP.Wb * Nc - 1] & 0x7FFFFFFF;
        spectrum_counter++;
        printf("%s> Sample: %06d Unique Spectrum ID: %010d (%010d)\r", argv[0], spectrum_counter, last_unique_spectrum_id, raw_data[headerjds.DSPP.Wb * Nc - 1]);
      }
      fclose(JDS_IN);

      /* write out the last spectra index */
      printf("%s> Sample: %06d Unique Spectrum ID: %010d (%010d)\n", argv[0], spectrum_counter, last_unique_spectrum_id, raw_data[headerjds.DSPP.Wb * Nc - 1]);
      sprintf(info[info_counter], "%s  %04d %02d %02d %02d %02d %02f  %d %d %s %s\n", jds_filename, jy, jm, jd, jhr, jmn, jsc, unique_spectrum_id, last_unique_spectrum_id, headerjds.desc, stmode);
      printf("%s> Added file %s information to line %d\n", argv[0], jds_filename, info_counter+1);
      info_counter++;
      Change_Tag_File = 1; /* this flag enables writing informations to tag file */
      /*printf("%s> info_counter= %d\n",argv[0],info_counter);*/
    }
  }
  printf("%s>\n", argv[0]);

  if (Change_Tag_File == 1) /* enables write out the information */
  {
    /* write the current info file */
    if ((TAG_OUT = fopen(jds_timetag_file, "w")) == NULL)
    {
      printf("%s> Unable to open file %s\n", argv[0], jds_timetag_file);
      return -1;
    }
    else
    {
      for (i = 0; i < info_counter; i++)
      {
        fputs(info[i], TAG_OUT);
      }
      fclose(TAG_OUT);
      printf("%s>\n%s> Wrote information about %d files into file %s\n", argv[0], argv[0], info_counter, jds_timetag_file);
    }
  }
  else
  {
    printf("%s> No need to change the file %s\n", argv[0], jds_timetag_file);
  }
  return 0;
}
