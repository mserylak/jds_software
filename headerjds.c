#define _FILE_OFFSET_BITS 64 /* making sure that programs can handle large (> 2 GB) files */
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

/* INITIALIZATION OF STRUCTURES... */
struct FHEADER headerjds; /* global declaration of headerjds as variable of FHEADER type, globalna deklaracja zmiennej strukturalnej headerjds o budowie szablonu FHEADER */

/* MAIN BODY OF THE PROGRAM */
int main(int argc, char *argv[], char *arge[])
{
  int spectrum_width, first_spec_chan_num, last_spec_chan_num, bps, argument;
  int jd, jm, jy, jhr, jmn, All;
  int Nc, Nfmin[4], Nf[4];
  float sampling_time, channel_bw, freq_first_chan, freq_last_chan, observation_time, jsc;
  double start_MJD, file_size, max_spectra_number;
  char jds_filename[256], stday[32], stmonth[32];
  FILE *JDS_IN; /* pointer to input file */
  bps = 4; /* bytes per sample, equals to 32 bits */
  All = 0;
  if (argc < 2 || (strcmp(argv[1], "-h") == 0)) /* in case there is a -h switch */
  {
    puts("\nUsage: headerjds <filename>\n");
    puts("The program will write out header content of a file in UTR-2 data format.");
    puts("Available parameters are:\n");
    puts("-a   Write out all raw header parameters.");
    puts("-h   Display this useful help page.\n\n");
    return -1;
  }
  if (argc > 2)
  {
    for (argument = 1; argument < argc - 1; argument++)
    {
      if (strcmp(argv[argument], "-a") == 0)
      {
        All = 1;
      }
      else
      {
        fprintf(stderr, "Unknown option: %s\n", argv[argument]);
        return -1;
      }
    }
  }
  strcpy(jds_filename, argv[argc-1]);
  if ((JDS_IN = fopen(jds_filename, "r")) == NULL)
  {
    printf("%s> Unable to open file %s\n", argv[0], jds_filename);
    return -1;
  }
  fread(&headerjds, sizeof(headerjds), 1, JDS_IN);
  if (All == 1)
  {
    printf("%s> Reading entire header of file %s\n", argv[0], jds_filename);
    dumpheaderjds(headerjds);
  }
  else
  {
    printf("%s> Reading header of file %s\n", argv[0], jds_filename);
  }
  sscanf(headerjds.gmtt, "%s %s", stday, stmonth);
  jy = headerjds.SYSTEMTIME.yr;
  jm = headerjds.SYSTEMTIME.mn;
  jd = headerjds.SYSTEMTIME.day;
  jhr = headerjds.SYSTEMTIME.hr;
  jmn = headerjds.SYSTEMTIME.min;
  jsc = headerjds.SYSTEMTIME.sec + (headerjds.SYSTEMTIME.msec * 0.001); /* passing seconds with milliseconds */
  start_MJD = gregorian2Julian(jy, jm, jd, (double)jhr, (double)jmn, jsc) - 2400000.5; /* convert the date to Modified Julian Day */
  spectrum_width = headerjds.DSPP.Wb; /* spectrum width in samples */
  first_spec_chan_num = 0; /* lower channel number */
  last_spec_chan_num = spectrum_width; /* upper channel number */
  sampling_time = 8192.0 / headerjds.DSPP.CLCfrq * headerjds.DSPP.NAvr; /* calculation of time resolution: full band spectrum / sampling ADC frequency * number of averaged spectra */
  Nfmin[0] = 0; /* matrix storing low bound number of samples */
  Nfmin[1] = 0; /* matrix storing low bound number of samples */
  Nfmin[2] = 4096; /* matrix storing low bound number of samples */
  Nfmin[3] = headerjds.DSPP.Lb; /* matrix storing low bound number of samples if it is in tunable mode */
  Nf[0] = 8192; /* matrix storing number of samples: full spectrum, 8192 samples */
  Nf[1] = 4096; /* matrix storing number of samples: lower half of spectrum, 4096 samples */
  Nf[2] = 4096, /* matrix storing number of samples: top half of spectrum, 4096 samples; 3:tunable */
  Nf[3] = headerjds.DSPP.Wb; /* matrix storing number of samples: tunable mode */
  Nc = 2 - headerjds.DSPP.Ch1 - headerjds.DSPP.Ch2; /* checking if file has one or two streams, if Nc = 1 then only one stream present, if 2 then both stream present */
  rewind(JDS_IN); /* rewinding file to be sure that file will be read from the begining */
  fseek(JDS_IN, 0L, SEEK_END); /* seeking end of file */
  file_size = ftell(JDS_IN); /* if you use ftell, then you must open the file in binary mode. If you open it in text mode, ftell only returns a "cookie" that is only usable by fseek */
  rewind(JDS_IN); /* rewinding file to be sure that file will be read from the begining */
  max_spectra_number = (file_size - sizeof(headerjds)) / bps / Nf[headerjds.DSPP.Offt] / Nc; /* calculating number of spectra (time samples) in the file */
  channel_bw = headerjds.DSPP.CLCfrq / 16384.0; /* channel bandwidth */
  freq_first_chan = Nfmin[headerjds.DSPP.Offt] * channel_bw / 1.0e6; /* frequency of first channel */
  freq_last_chan = freq_first_chan + Nf[headerjds.DSPP.Offt] * channel_bw / 1.0e6; /* frequency of last channel */
  observation_time = sampling_time * max_spectra_number; /* calculation of observation duration in seconds */
  printf("%s> Header size (bytes)                   : %lu\n", argv[0], sizeof(headerjds));
  printf("%s> Time stamp of first sample (MJD)      : %.9lf\n", argv[0], start_MJD);
  printf("%s> Gregorian date (YYYY/MM/DD)           : %04d/%02d/%02d\n", argv[0], jy, jm, jd);
  printf("%s> Time stamp of first sample (hh:mm:ss) : %02d:%02d:%.3f\n", argv[0], jhr, jmn, jsc);
  printf("%s> Sampling time (seconds)               : %f\n", argv[0], sampling_time);
  printf("%s> Number of samples                     : %d\n", argv[0], (int)max_spectra_number);
  if (observation_time > 60 && observation_time < 3600)
  {
    observation_time /= 60;
    printf("%s> Observation length (minutes)          : %.1f\n", argv[0], observation_time);
  }
  else if (observation_time > 3600)
  {
    observation_time /= 3600;
    printf("%s> Observation length (hours)            : %.1f\n", argv[0], observation_time);
  }
  else
  {
    printf("%s> Observation length (seconds)          : %.1f\n", argv[0], observation_time);
  }
  printf("%s> Number of spectral channels           : %d\n", argv[0], spectrum_width);
  printf("%s> Frequency of first channel (MHz)      : %f\n", argv[0], freq_first_chan);
  printf("%s> Channel bandwidth (kHz)               : %f\n", argv[0], channel_bw/1e3);
  printf("%s> Frequency of last channel (MHz)       : %f\n", argv[0], freq_last_chan);
  fclose(JDS_IN);
  return 0;
}
