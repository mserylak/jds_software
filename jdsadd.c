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
#include "jdstools.h" /* definitios of header extraction and data conversion functions */

int main(int argc, char *argv[], char *arge[])
{
  int i, argument;
  int spectrum_counter, max_spectra_number, spectrum_number, spectrum_width;
  int spectrum_counter2, max_spectra_number2, spectrum_number2, spectrum_width2;
  int number_channels, first_spec_chan_num, last_spec_chan_num;
  int number_channels2, first_spec_chan_num2, last_spec_chan_num2;
  int nbytes, nbytes2write, nbits, bps, byte;
  int jd, jm, jy, jhr, jmn;
  int jd2, jm2, jy2, jhr2, jmn2;
  int Verbose;
  int Nfmin[4], Nf[4], Nc;
  int Nfmin2[4], Nf2[4], Nc2;
  char stday[32], stmonth[32];
  char jds_out_filename[256], jds_filename[256], jds_filename2[256];
  char stday2[32], stmonth2[32];
  float sampling_time, freq_first, freq_last, channel_bw;
  float sampling_time2, freq_first2, freq_last2, channel_bw2;
  double start_MJD, jsc, file_size;
  double start_MJD2, jsc2, file_size2;
  FILE *JDS_IN, *JDS_IN2, *JDS_OUT;
  int *raw_data; /* allocate pointer to read raw data in file */
  int *raw_data2; /* allocate pointer to read raw data in file */
  struct FHEADER headerjds; /* global declaration of headerjds as variable of FHEADER type */
  struct FHEADER headerjds2; /* global declaration of headerjds as variable of FHEADER type */

  /* PRESET OPTIONS & DEFAULT VALUES */
  spectrum_counter = 0;
  spectrum_counter2 = 0;
  spectrum_number = 0;
  spectrum_number2 = 0;
  first_spec_chan_num = 0; /* first spectral channel number */
  first_spec_chan_num2 = 0; /* first spectral channel number */
  last_spec_chan_num = 0; /* last spectral channel number */
  last_spec_chan_num2 = 0; /* last spectral channel number */
  bps = 4; /* bytes per sample */
  byte = 8; /* bytes per bit */
  max_spectra_number = 1e06;
  max_spectra_number2 = 1e06;
  nbits = 32;
  Verbose = 0; /* verbose mode */

  /* AVAILABLE HELP */
  if (argc < 2 || (strcmp(argv[1], "-h") == 0)) /* in case there is a -h switch */
  {
    puts("\nProgram adds two UTR-2 data format files.");
    puts("\nUsage: jdsadd [parameters] <filename>");
    puts("\nAvailable parameters are:\n");
    printf("-v         Verbose mode\n\n");
    printf("-h         Display this useful help page\n\n");
    return -1;
  }

  /* READING COMMAND LINE ARGUMENTS */
  if (argc > 2)
  {
    for (argument = 1; argument < argc - 1; argument++)
    {
      if (strcmp(argv[argument], "-v") == 0)
      {
        Verbose = 1;
      } //else {
        //fprintf(stderr, "Unknown option: %s\n", argv[argument]);
        //return -1;
      //}
    }
  }

  /* OPENING FILES */
  strcpy(jds_filename, argv[argc-2]);
  strcpy(jds_filename2, argv[argc-1]);
  if ((JDS_IN = fopen64(jds_filename, "r")) == NULL)
  {
    printf("%s> Unable to open file %s\n", argv[0], jds_filename);
    return -1;
  }
  if ((JDS_IN2 = fopen64(jds_filename2, "r")) == NULL) {
    printf("%s> Unable to open file %s\n", argv[0], jds_filename2);
    return -1;
  }
  printf("%s> Files %s and %s are open and ready to be read.\n", argv[0], jds_filename, jds_filename2);

  /* READING HEADERS INFORMATION AND CALCULATING PARAMETERS FOR ADDING FILES */
  fread(&headerjds, sizeof(headerjds), 1, JDS_IN); /* binary read header of jds file and reading name, time, gmtt, sysn, syst, place, desc, PP, DSPP values */
  fread(&headerjds2, sizeof(headerjds2), 1, JDS_IN2);
  printf("%s> Reading header of file %s\n", argv[0], jds_filename);
  if (Verbose == 1)
  {
    printf("%s> Header size: %ld bytes\n", argv[0], sizeof(headerjds));
    printf("%s> UPP size: %ld bytes\n", argv[0], sizeof(struct UPP));
    printf("%s> UDSPP size: %ld bytes\n", argv[0], sizeof(struct UDSPP));
    dumpheaderjds(headerjds); /* output all header information */
  }
  printf("%s> Reading header of file %s\n", argv[0], jds_filename2);
  if (Verbose == 1)
  {
    printf("%s> Header size: %ld bytes\n", argv[0], sizeof(headerjds2));
    printf("%s> UPP size: %ld bytes\n", argv[0], sizeof(struct UPP));
    printf("%s> UDSPP size: %ld bytes\n", argv[0], sizeof(struct UDSPP));
    dumpheaderjds(headerjds2); /* output all header information */
  }
  sscanf(headerjds.gmtt, "%s %s", stday, stmonth); /* get the date of observation */
  sscanf(headerjds2.gmtt, "%s %s", stday2, stmonth2);
  jy = headerjds.SYSTEMTIME.yr; /* year */
  jy2 = headerjds2.SYSTEMTIME.yr;
  jm = headerjds.SYSTEMTIME.mn; /* month */
  jm2 = headerjds2.SYSTEMTIME.mn;
  jd = headerjds.SYSTEMTIME.day; /* day */
  jd2 = headerjds2.SYSTEMTIME.day;
  jhr = headerjds.SYSTEMTIME.hr; /* hour */
  jhr2 = headerjds2.SYSTEMTIME.hr;
  jmn = headerjds.SYSTEMTIME.min; /* minute */
  jmn2 = headerjds2.SYSTEMTIME.min;
  jsc = headerjds.SYSTEMTIME.sec + (headerjds.SYSTEMTIME.msec * 0.001); /* seconds + milliseconds */
  jsc2 = headerjds2.SYSTEMTIME.sec + (headerjds2.SYSTEMTIME.msec * 0.001);
  start_MJD = gregorian2Julian(jy, jm, jd, (double)jhr, (double)jmn, jsc) - 2400000.5; /* convert the date to JD and subtract 2400000.5 to get MJD */
  start_MJD2 = gregorian2Julian(jy2, jm2, jd2, (double)jhr2, (double)jmn2, jsc2) - 2400000.5;
  printf("%s> Observation recorded in file %s started on %04d/%02d/%02d at %02d:%02d:%02.3f\n", argv[0], jds_filename, jy, jm, jd, jhr, jmn, jsc);
  printf("%s> Time stamp of first sample (MJD) in file %s: %.9lf\n", argv[0], jds_filename, start_MJD);
  printf("%s> Observation recorded in file %s started on %04d/%02d/%02d at %02d:%02d:%02.3f\n", argv[0], jds_filename2, jy2, jm2, jd2, jhr2, jmn2, jsc2);
  printf("%s> Time stamp of first sample (MJD) in file %s: %.9lf\n", argv[0], jds_filename2, start_MJD2);

  spectrum_width = headerjds.DSPP.Wb; /* original spectral width */
  spectrum_width2 = headerjds2.DSPP.Wb;
  first_spec_chan_num = 0; /* assigning value to variable defining first spectral channel number */
  first_spec_chan_num2 = 0;
  last_spec_chan_num = spectrum_width; /* assigning value to variable defining last spectral channel number */
  last_spec_chan_num2 = spectrum_width2;

  sampling_time = 8192.0 / headerjds.DSPP.CLCfrq * headerjds.DSPP.NAvr; /* calculation of time resolution: full band spectrum / sampling ADC frequency * number of averaged spectra */
  sampling_time2 = 8192.0 / headerjds2.DSPP.CLCfrq * headerjds2.DSPP.NAvr; /* calculation of time resolution: full band spectrum / sampling ADC frequency * number of averaged spectra */
  printf("%s> File %s has sampling time: %f s\n", argv[0], jds_filename, sampling_time);
  printf("%s> File %s has sampling time: %f s\n", argv[0], jds_filename2, sampling_time2);

  Nfmin[0] = 0; /* matrix storing low bound number of samples */
  Nfmin2[0] = 0; /* matrix storing low bound number of samples */
  Nfmin[1] = 0; /* matrix storing low bound number of samples */
  Nfmin2[1] = 0; /* matrix storing low bound number of samples */
  Nfmin[2] = 4096; /* matrix storing low bound number of samples */
  Nfmin2[2] = 4096; /* matrix storing low bound number of samples */
  Nfmin[3] = headerjds.DSPP.Lb; /* matrix storing low bound number of samples if it is in tunable mode */
  Nfmin2[3] = headerjds2.DSPP.Lb; /* matrix storing low bound number of samples if it is in tunable mode */

  Nf[0] = 8192; /* not sure what it is */
  Nf2[0] = 8192; /* not sure what it is */
  Nf[1] = 4096; /* not sure what it is */
  Nf2[1] = 4096; /* not sure what it is */
  Nf[2] = 4096, /* not sure what it is */
  Nf2[2] = 4096, /* not sure what it is */
  Nf[3] = headerjds.DSPP.Wb; /* spectrum width in samples */
  Nf2[3] = headerjds2.DSPP.Wb; /* spectrum width in samples */

  Nc = 2 - headerjds.DSPP.Ch1 - headerjds.DSPP.Ch2; /* checking if file has one or two streams, if Nc = 1 then only one stream present, if 2 then both streams present */
  Nc2 = 2 - headerjds2.DSPP.Ch1 - headerjds2.DSPP.Ch2; /* checking if file has one or two streams, if Nc = 1 then only one stream present, if 2 then both streams present */
  printf("%s> File %s has %d streams.\n", argv[0], jds_filename, Nc);
  printf("%s> File %s has %d streams.\n", argv[0], jds_filename2, Nc2);

  rewind(JDS_IN); /* rewinding file to be sure that file will be read from the begining */
  fseek(JDS_IN, 0L, SEEK_END); /* seeking end of file */
  file_size = ftell(JDS_IN); /* if you use ftell, then you must open the file in binary mode. If you open it in text mode, ftell only returns a "cookie" that is only usable by fseek */
  rewind(JDS_IN); /* rewinding file to be sure that file will be read from the begining */
  max_spectra_number = (file_size - sizeof(headerjds)) / bps / Nf[headerjds.DSPP.Offt] / Nc; /* calculating number of spectra (time samples) in the file */
  fseek(JDS_IN, sizeof(headerjds), SEEK_SET); /* fseek to position after estimating file size */
  rewind(JDS_IN2); /* rewinding file to be sure that file will be read from the begining */
  fseek(JDS_IN2, 0L, SEEK_END); /* seeking end of file */
  file_size2 = ftell(JDS_IN2); /* if you use ftell, then you must open the file in binary mode. If you open it in text mode, ftell only returns a "cookie" that is only usable by fseek */
  rewind(JDS_IN2); /* rewinding file to be sure that file will be read from the begining */
  max_spectra_number2 = (file_size2 - sizeof(headerjds2)) / bps / Nf2[headerjds2.DSPP.Offt] / Nc2; /* calculating number of spectra (time samples) in the file */
  fseek(JDS_IN2, sizeof(headerjds2), SEEK_SET); /* fseek to position after estimating file size */

  channel_bw = headerjds.DSPP.CLCfrq / 16384.0; /* channel bandwidth: sampling ADC frequency * amount of samples per stream */
  channel_bw2 = headerjds2.DSPP.CLCfrq / 16384.0; /* channel bandwidth: sampling ADC frequency * amount of samples per stream */

  number_channels = spectrum_width; /* use full spectrum width */
  number_channels2 = spectrum_width2; /* use full spectrum width */
  printf("%s> Number of channels in file %s: %d.\n", argv[0], jds_filename, number_channels);
  printf("%s> Number of channels in file %s: %d.\n", argv[0], jds_filename2, number_channels2);

  freq_first = Nfmin[headerjds.DSPP.Offt] * channel_bw / 1.0e6; /* lowest observed frequency */
  freq_first2 = Nfmin2[headerjds2.DSPP.Offt] * channel_bw2 / 1.0e6; /* lowest observed frequency */
  freq_last = freq_first + Nf[headerjds.DSPP.Offt] * channel_bw / 1.0e6; /* highest observed frequency */
  freq_last2 = freq_first2 + Nf2[headerjds2.DSPP.Offt] * channel_bw2 / 1.0e6; /* highest observed frequency */
  printf("%s> In file %s frequency of first channel: %f MHz; frequency of last channel: %f MHz.\n", argv[0], jds_filename, freq_first, freq_last);
  printf("%s> In file %s frequency of first channel: %f MHz; frequency of last channel: %f MHz.\n", argv[0], jds_filename2, freq_first2, freq_last2);

  nbytes = spectrum_width * nbits / byte;
  nbytes2write = nbytes;

  /*printf("%s> \n", argv[0], );*/
  if (strrchr(jds_filename, '/') != NULL)
  {
    sprintf(jds_out_filename, "%s.new", (strrchr(jds_filename, '/') + sizeof(char)));
  }
  else
  {
    sprintf(jds_out_filename, "%s.new", jds_filename);
  }
  printf("%s> Creating file %s\n", argv[0], jds_out_filename);
  if ((JDS_OUT = fopen(jds_out_filename, "wb")) == NULL)
  {
    printf("%s> Unable to open file %s\n", argv[0], jds_out_filename);
    return -1;
  }

  /* ALLOCATING MEMORY FOR DATA ARRAYS */
  printf("%s> Allocating data arrays.\n", argv[0]);

  raw_data = (int *) calloc(bps * headerjds.DSPP.Wb * Nc, sizeof(float));  /* allocate memory to read raw data in file */
  raw_data2 = (int *) calloc(bps * headerjds2.DSPP.Wb * Nc2, sizeof(float)); /* allocate memory to read raw data in second file */

  fwrite(&headerjds, sizeof(headerjds), 1, JDS_OUT);
  spectrum_counter = 0;
  for (i = 0; i < max_spectra_number; i++)
  {
    if (fread(raw_data, bps * headerjds.DSPP.Wb * Nc, 1, JDS_IN) > 0)
    {
      printf("%s> Just read raw data record #%06d\r", argv[0], spectrum_counter); /* spectrum counter is initialized at the begining of the program */
    }
    /* WRITING IN THE OUTPUT FILE (ONE SPECTRUM AT A TIME) */
    fwrite(raw_data, bps * headerjds.DSPP.Wb * Nc, 1, JDS_OUT);
    spectrum_counter++;
  }
  spectrum_counter2 = 0;
  for (i = 0; i < max_spectra_number2; i++)
  {
    if (fread(raw_data2, bps * headerjds2.DSPP.Wb * Nc2, 1, JDS_IN2) > 0)
    {
      printf("%s> Just read raw data record #%06d\r", argv[0], spectrum_counter + spectrum_counter2); /* spectrum counter is initialized at the begining of the program */
    }
    /* WRITING IN THE OUTPUT FILE (ONE SPECTRUM AT A TIME) */
    fwrite(raw_data2, bps * headerjds2.DSPP.Wb * Nc2, 1, JDS_OUT);
    spectrum_counter2++;
  }
  printf("\n");

  printf("%s> Joined %d spectra from file %s and %s.\n", argv[0], spectrum_counter + spectrum_counter2, jds_filename, jds_filename2);
  printf("%s> Closing file %s\n", argv[0], jds_out_filename);
  fclose(JDS_OUT);
  return 0;
}
