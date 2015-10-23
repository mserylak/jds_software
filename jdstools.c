#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "jds.h"

/* FUNCTION EXTRACTING BIT VALUES FROM INT VARIABLE */
unsigned int getBits(int x, int p, int n) {
   return (x >> (p + 1 - n)) & ~(~0 << n); /* read "n" bits at "p" position from the "x" variable, WARNING position should be rightmost bit from interesting range */
}

/* FUNCTION OUTPUTS THE CONTENTS OF THE HEADER */
int dumpheaderjds(struct FHEADER headerjds) {
   printf("dumpheaderjds> name:                   %s (original name of the file in ddmmyy_hhmmss.jds format)\n", headerjds.name);
   printf("dumpheaderjds> time:                   %s", headerjds.time);
   printf("dumpheaderjds> gmtt:                   %s\n", headerjds.gmtt);
   printf("dumpheaderjds> sysn:                   %s (operator name, system name, observatory name etc.)\n", headerjds.sysn);
   printf("dumpheaderjds> SYSTEMTIME.yr:          %hd (year)\n", headerjds.SYSTEMTIME.yr);
   printf("dumpheaderjds> SYSTEMTIME.mn:          %hd (month)\n", headerjds.SYSTEMTIME.mn);
   printf("dumpheaderjds> SYSTEMTIME.dow:         %hd (day of week)\n", headerjds.SYSTEMTIME.dow);
   printf("dumpheaderjds> SYSTEMTIME.day:         %hd (day of month)\n", headerjds.SYSTEMTIME.day);
   printf("dumpheaderjds> SYSTEMTIME.hr:          %hd (hr)\n", headerjds.SYSTEMTIME.hr);
   printf("dumpheaderjds> SYSTEMTIME.min:         %hd (min)\n", headerjds.SYSTEMTIME.min);
   printf("dumpheaderjds> SYSTEMTIME.sec:         %hd (s)\n", headerjds.SYSTEMTIME.sec);
   printf("dumpheaderjds> SYSTEMTIME.msec:        %hd (ms)\n", headerjds.SYSTEMTIME.msec);
   printf("dumpheaderjds> place:                  %s (place of the observation)\n", headerjds.place);
   printf("dumpheaderjds> desc:                   %s (additional field used for informations)\n", headerjds.desc);
   printf("dumpheaderjds> PP.mode bits 2..0:      %u (0:waveform data both streams; 1:waveform data one stream selected by bit 5; 2:spectra for both streams)\n", getBits(headerjds.PP.mode, 2, 3));
   printf("dumpheaderjds>                           (3:stream for A and A*B; 4: stream for B and A*B; 5: stream for A+B and A*B)\n");
   printf("dumpheaderjds> PP.mode bits 4..3:      %d (reserved)\n", getBits(headerjds.PP.mode, 4, 2));
   printf("dumpheaderjds> PP.mode bit 5:          %d (stream select, 0:first stream (A); 1:second stream (B) for operation mode 1)\n", getBits(headerjds.PP.mode, 5, 1));
   printf("dumpheaderjds> PP.mode bit 6:          %d (0:normal operation mode; 1:switching off spectrum averaging and decimation is performed instead)\n", getBits(headerjds.PP.mode, 6, 1));
   printf("dumpheaderjds> PP.mode bit 7:          %d (0:Hanning FFT window; 1:rectangle FFT window)\n", getBits(headerjds.PP.mode, 7, 1));
   printf("dumpheaderjds> PP.mode bit 8:          %d (0:without DC removing; 1:DC removing is switched on)\n", getBits(headerjds.PP.mode, 8, 1));
   printf("dumpheaderjds> PP.mode bit 9:          %d (0:internal CLC 66 MHz; 1:external CLC)\n", getBits(headerjds.PP.mode, 9, 1));
   printf("dumpheaderjds> PP.mode bit 10:         %d (0:stream A is switched on; 1:stream A is switched off)\n", getBits(headerjds.PP.mode, 10, 1));
   printf("dumpheaderjds> PP.mode bit 11:         %d (0:stream B is switched on; 1:stream B is switched off)\n", getBits(headerjds.PP.mode, 11, 1));
   printf("dumpheaderjds> PP.mode bit 12:         %d (0:start with synchronization; 1:start without synchronization (from GPS pulse))\n", getBits(headerjds.PP.mode, 12, 1));
   printf("dumpheaderjds> PP.mode bit 13:         %d (after rising from 0 to 1, the ADC sampling will wait for the first GPS pulse (it works if bit 12 is \"1\"))\n", getBits(headerjds.PP.mode, 13, 1));
   printf("dumpheaderjds> PP.mode bit 14:         %d (0:internal Hanning window; 1:external weighting window)\n", getBits(headerjds.PP.mode, 14, 1));
   printf("dumpheaderjds> PP.mode bits 30..15:    %d (reserved)\n", getBits(headerjds.PP.mode, 30, 16));
   printf("dumpheaderjds> PP.mode bit 31:         %d (DMA interrupt enabling)\n", getBits(headerjds.PP.mode, 31, 1));
   printf("dumpheaderjds> PP.size:                %d (DMA block size, field used only in real-time data transferring from DSP board to PC memory,)\n", headerjds.PP.size);
   printf("dumpheaderjds>                                 (carries no relevant information about data processing)\n");
   printf("dumpheaderjds> PP.prc_mode bits 11..0  %d (number of the averaged spectra minus 1)\n", getBits(headerjds.PP.prc_mode, 11, 12));
   printf("dumpheaderjds> PP.prc_mode bits 18..16 %d (offset for the first spectral line of the spectrum output; can be 0..7, so the first line = value * 1024)\n", getBits(headerjds.PP.prc_mode, 18, 3));
   printf("dumpheaderjds> PP.prc_mode bits 22..19 %d (spectrum width; can be 0..8, so the spectrum width = value * 1024)\n", getBits(headerjds.PP.prc_mode, 22, 4));
   printf("dumpheaderjds> PP.tst_gen:             %d (not used)\n", headerjds.PP.tst_gen);
   printf("dumpheaderjds> PP.clk:                 %d (not used)\n", headerjds.PP.clk);
   printf("dumpheaderjds> PP.fft_size:            %d (not used)\n", headerjds.PP.fft_size);
   printf("dumpheaderjds> DSPP.FFT_Size:          %d (used in DSP - FFT size)\n", headerjds.DSPP.FFT_Size);
   printf("dumpheaderjds> DSPP.MinDSPSize:        %d (used in DSP - minimal internal DSP buffer)\n", headerjds.DSPP.MinDSPSize);
   printf("dumpheaderjds> DSPP.MinDMASize:        %d (used in DSP - minimal DMA size)\n", headerjds.DSPP.MinDMASize);
   printf("dumpheaderjds> DSPP.DMASizeCnt:        %d (used in DSP - counts of MinDMASize)\n", headerjds.DSPP.DMASizeCnt);
   printf("dumpheaderjds> DSPP.DMASize:           %d (used in DSP - DMA buffer size)\n", headerjds.DSPP.DMASize);
   printf("dumpheaderjds> DSPP.CLCfrq:            %.2f (sampling ADC frequency)\n", headerjds.DSPP.CLCfrq);
   printf("dumpheaderjds> DSPP.Synch:             %d (0:with GPS sync; 1:without sync)\n", headerjds.DSPP.Synch);
   printf("dumpheaderjds> DSPP.SSht:              %d (snapshot mode, always 1)\n", headerjds.DSPP.SSht);
   printf("dumpheaderjds> DSPP.Mode:              %d (0:waveform; 1:spectrum; 2:correlation)\n", headerjds.DSPP.Mode);
   printf("dumpheaderjds> DSPP.Wch:               %d (0:str. A waveform; 1:str. B waveform; 2:both)\n", headerjds.DSPP.Wch);
   printf("dumpheaderjds> DSPP.Smd:               %d (0:both str. with spectra; 1:str. with A spectra and A*B; 2:str. B with spectra and A*B; 3:A+B and A*B))\n", headerjds.DSPP.Smd);
   printf("dumpheaderjds> DSPP.Offt:              %d (0:full spectrum, 8192 samples; 1:lower half of spectrum, 4096 samples; 2:top half of spectrum, 4096 samples; 3:tunable)\n", headerjds.DSPP.Offt);
   printf("dumpheaderjds> DSPP.Lb:                %d (low cutoff of the spectrum in samples)\n", headerjds.DSPP.Lb);
   printf("dumpheaderjds> DSPP.Hb:                %d (high cutoff of the spectrum in samples)\n", headerjds.DSPP.Hb);
   printf("dumpheaderjds> DSPP.Wb:                %d (spectrum width in samples)\n", headerjds.DSPP.Wb);
   printf("dumpheaderjds> DSPP.NAvr:              %d (number of averaged spectra)\n", headerjds.DSPP.NAvr);
   printf("dumpheaderjds> DSPP.CAvr:              %d (spectrum averaging, 0:on; 1:off) \n", headerjds.DSPP.CAvr);
   printf("dumpheaderjds> DSPP.Weight:            %d (weighting window, 0:on; 1:off)\n", headerjds.DSPP.Weight);
   printf("dumpheaderjds> DSPP.DCRem:             %d (DC compensation, 0:off; 1:on)\n", headerjds.DSPP.DCRem);
   printf("dumpheaderjds> DSPP.ExtSyn:            %d (external CLC synchronisation, 0:off; 1:on)\n", headerjds.DSPP.ExtSyn);
   printf("dumpheaderjds> DSPP.Ch1:               %d (stream A, 0:on; 1:off)\n", headerjds.DSPP.Ch1);
   printf("dumpheaderjds> DSPP.Ch2:               %d (stream B, 0:on; 1:off)\n", headerjds.DSPP.Ch2);
   printf("dumpheaderjds> DSPP.ExtWin:            %d (external weighting coefficients (0:internal weighting window; 1:external weighting window))\n", headerjds.DSPP.ExtWin);
   printf("dumpheaderjds> DSPP.Clip:              %d (spectrum clipping value K: dB = 2^21*10^K/20)\n", headerjds.DSPP.Clip);
   printf("dumpheaderjds> DSPP.HPF0:              %d (stream A High Pass Filter settings: 0:8MHz; 1:12MHz; 2:16MHz; 3:20MH)\n", headerjds.DSPP.HPF0);
   printf("dumpheaderjds> DSPP.HPF1:              %d (stream B High Pass Filter settings: 0:8MHz; 1:12MHz; 2:16MHz; 3:20MH)\n", headerjds.DSPP.HPF1);
   printf("dumpheaderjds> DSPP.LPF0:              %d (not used)\n", headerjds.DSPP.LPF0);
   printf("dumpheaderjds> DSPP.LPF1:              %d (not used)\n", headerjds.DSPP.LPF1);
   printf("dumpheaderjds> DSPP.ATT0:              %d (stream A attenuation 0..31 dB)\n", headerjds.DSPP.ATT0);
   printf("dumpheaderjds> DSPP.ATT1:              %d (stream B attenuation 0..31 dB)\n", headerjds.DSPP.ATT1);
   printf("dumpheaderjds> DSPP.Soft:              %s (software type name)\n", headerjds.DSPP.Soft);
   printf("dumpheaderjds> DSPP.SVer:              %s (software version)\n", headerjds.DSPP.SVer);
   printf("dumpheaderjds> DSPP.DSPv:              %s (DSP software version)\n", headerjds.DSPP.DSPv);
   /*printf("dumpheaderjds> DSPP. : %d\n", headerjds.DSPP.); template for new information */
   return 0;
}

/* FUNCTION CONVERTS THE DSP FLOATING POINT FORMAT TO CONVENTIONAL FLOATING POINT FORMAT */
int DSPZ2Float(struct FHEADER *headerjds, unsigned int *rawdata, int nbsamp, float *dataf) {
   int i;
   float SNrm, WNrm;
   int expn, sign;
   unsigned int mantissa;
   i = expn = sign = mantissa = 0;
   SNrm = WNrm = 0.0;
   if (headerjds->DSPP.Mode == 0) { /* data in the waveform mode */
      WNrm = 1.0 / 32768.0; /* norming */
      for (i = 0; i < nbsamp; i++) {
         dataf[i] = rawdata[i] * WNrm; /* conversion from DSP float to PC float */
      }
   } else if (headerjds->DSPP.Mode == 1) { /* data in the spectral mode */
      SNrm = 4.0 * 2.0 * 1024.0 / 4294967296.0 / headerjds->DSPP.NAvr; /* norming */
      for (i = 0; i < nbsamp; i++) {
         expn = getBits(rawdata[i], 4, 5); /* bits 4...0 - exponent */
         mantissa = getBits(rawdata[i], 31, 26); /* bits 31...6 - mantissa value */
         dataf[i] = (float)mantissa / pow(2.0, expn) / SNrm; /* conversion from DSP float to PC float */
      }
   } else if (headerjds->DSPP.Mode == 2) { /* data in the correlation mode */
      SNrm = 4 * 2 * 1024.0 / 4294967296.0 / headerjds->DSPP.NAvr; /* norming */
      for (i = 0; i < nbsamp; i++) {
         expn = getBits(rawdata[i], 4, 5); /* bits 4...0 - exponent */
         if ((getBits(rawdata[i], 5, 1)) == 1) { /* bit 5 - the sign of mantissa in correlation mode 0 - plus / 1 - minus */
            sign = -1.0;
         } else if ((getBits(rawdata[i], 5, 1)) == 0) {
            sign = 1.0;
         }
         mantissa = getBits(rawdata[i], 31, 26); /* bits 31...6 - mantissa absolute value */
         dataf[i] = sign * (float)mantissa / pow(2.0, expn) / SNrm; /* conversion from DSP float to PC float */
      }
   }
   return 0;
}

/* FUNCTION REVERSING ARRAY */
void reverseArray(float array[], int size) {
   int i;
   int m = size >> 1;
   int j = size - 1;
   float temp;
   for (i = 0; i < m; ++i, --j) {
      temp = array[i];
      array[i] = array[j];
      array[j] = temp;
   }
}

/* COMPARISON FUNCTION FOR qsort AND readZapFile */
int compareFunction(const void * a, const void * b) {
   return ( *(int*)a - *(int*)b );
}

/* FUNCTION READS THE FILE WITH LIST OF BAD CHANNELS */
int readZapFile(char *zapFileName, int *zapChannels, int *counter) {
   FILE *ZAP_FILE;
   int i, j, c;
   c = 0;
   ZAP_FILE = fopen(zapFileName, "r");
   if (ZAP_FILE == NULL) {
      fprintf(stderr, "readZapFile> Cannot open %s\n\n", zapFileName);
      return -1;
   }
   do {
      j = fscanf(ZAP_FILE, "%d", &i); /* read in spectral channel number */
      /*if (j == 1) {
         printf("Zapping channel %d\n", i);
      }*/
      zapChannels[c] = i;
      c++;
   } while (j == 1);
   c -= 1;
   *counter = c;
   fclose(ZAP_FILE);
   /*for (i = 0; i < c; i++) {
      printf("%d\n", zapChannels[i]);
   }*/
   qsort(zapChannels, c, sizeof(int), compareFunction);
   /*for (i = 0; i < c; i++) {
      printf("%d\n", zapChannels[i]);
   }*/
   return 1;
}

/* FUNCTION GETTING SERVICE DATA FROM 2 LAST SAMPLES PER STREAM, 4 IN TOTAL, ONLY FOR DATA TAKEN AFTER 2008 */
void DSPZ2Service(struct FHEADER *headerjds, unsigned int *RawData, int NumberSamples, int Verbose, int *CorruptedSpectrumFlag) {
   int CntA1_bits26_0, CntB1_bits16_0, CntA2_bit31, CntA2_bit30, CntA2_bits29_0, CntB2_bit31, CntB2_bit30, CntB2_bits29_0;
   *CorruptedSpectrumFlag = 0; /* this flag has the be reset every time a new spectrum is read */
   CntA2_bit31 = getBits(RawData[(NumberSamples)-2], 31, 1);     /* 1 - CntA2 - bit 31 - overflow flag ADC-A for accumulation time FFT */
   CntA2_bit30 = getBits(RawData[(NumberSamples)-2], 30, 1);     /* 2 - CntA2 - bit 30 - CRC flag in stream A */
   CntB2_bit31 = getBits(RawData[(NumberSamples)-1], 31, 1);     /* 3 - CntB2 - bit 31 - overflow flag ADC-B for accumulation time FFT */
   CntB2_bit30 = getBits(RawData[(NumberSamples)-1], 30, 1);     /* 4 - CntB2 - bit 30 - CRC flag in stream B */
   CntA2_bits29_0 = getBits(RawData[(NumberSamples)-2], 29, 30); /* 5 - CntA2 - bits 29..0 - count of us since start of recording */
   CntB2_bits29_0 = getBits(RawData[(NumberSamples)-1], 29, 30); /* 6 - CntB2 - bits 29..0 - count of the number of issued FFTs since the start of recording (might be reset to 0 if a hardware error) */
   CntA1_bits26_0 = getBits(RawData[(NumberSamples)-4], 26, 27); /* 7 - CntA1 - bits 26..0 - the counter of phase of second (number of cycles of the ADC from the rise of the PPS to first sample in the data block used for current FFT) */
   CntB1_bits16_0 = getBits(RawData[(NumberSamples)-3], 16, 17); /* 8 - CntB1 - bits 16..0 - count seconds in the day */
   if (CntA2_bit31 == 1 || CntA2_bit30 == 1 || CntA2_bits29_0 == 1 || CntB2_bit31 == 1 || CntB2_bit30 == 1 || CntB2_bits29_0 == 1) {
      *CorruptedSpectrumFlag = 1;
      if (Verbose == 2 || Verbose == 3) {
         printf("DSPZ2service> Zapping corrupted spectrum due to ADC-A: %d CRC-A: %d ADC-B: %d CRC-B: %d us: %d FFT: %d phase-of-sec: %d count-sec-day: %d\n", CntA2_bit31, CntA2_bit30, CntB2_bit31, CntB2_bit30, CntA2_bits29_0, CntB2_bits29_0, CntA1_bits26_0, CntB1_bits16_0);
      }
   }
}

/* CALCULATE MOVING AVERAGE (RESULTING ARRAY WILL HAVE N-M-1 ELEMENTS) */
void movingAverage(int arraySize, float *inputArray, int runningMeanWindow, float *runningMeanArray) {
   int i, runningMeanArraySize, counter;
   float arraySum;
   arraySum = 0.0; /* zeroing the variable */
   runningMeanArraySize = arraySize - runningMeanWindow; /* calculating the size of running average size */
   for (counter = 0; counter <= runningMeanArraySize; counter++) { /* for each value in running average array... */
      for (i = counter; i < (counter + runningMeanWindow); i++) { /* move the window by one bin */
         arraySum += inputArray[i]; /* ... calculate the sum in the running window */
      }
      runningMeanArray[counter] = arraySum / runningMeanWindow; /* storing running average in array */
      /*printf("%d %lf\n", counter, runningMeanValue);*/
      arraySum = 0.0; /* zeroing variable for the next run */
   }
}

/* COMPUTE A RUNNING MEDIAN OF ONE ARRAY */
void movingMedian(int arraySize, float *inputArray, int movingMedianWindow, float *movingMedianArray) {
   int i, j, movingMedianArraySize;
   float *sortedArray, *tempArray, *tempMovingMedianArray;
   float tempArraySum;
   tempArraySum = 0.0;
   movingMedianArraySize = arraySize - movingMedianWindow + 1; /* calculate array size after applying moving median */
   sortedArray = (float *) calloc(movingMedianWindow, sizeof(float)); /* allocate memory for sorted array */
   tempMovingMedianArray = (float *) calloc(arraySize, sizeof(float)); /* allocate memory for temporary array */
   tempArray = (float *) calloc(arraySize, sizeof(float)); /* allocate memory for temporary array */
   if (sortedArray == NULL) {
      puts("runningMedian> Cannot allocate memory for sortedArray");
      return;
   }
   for (i = 0; i < movingMedianArraySize; i++) { /* start calculating moving median */
      for (j = 0; j < movingMedianWindow; j++) {
         sortedArray[j] = inputArray[j + i]; /* input the data to array which will be sorted */
      }
      qsort(sortedArray, movingMedianWindow, sizeof(float), compareFunction); /* sort the array with built in qsort C function */
      if (movingMedianWindow % 2) { /* calculate the median depending if the window is divisible by 2... */
         tempMovingMedianArray[i] = sortedArray[movingMedianWindow / 2];
      } else { /* ...or not */
         tempMovingMedianArray[i] = (sortedArray[movingMedianWindow / 2] + sortedArray[(movingMedianWindow - 1) / 2]) * 0.5;
      }
   }
   /* this bit of the code extends the moving median by adding the signal which is lost during its calculation (N - M + 1) */
   for (i = 1; i < (int) movingMedianWindow/2; i++) {  /* calculate the average value for begining of the spectrum */
      for (j = 0; j < 2 * i; j++) { /* this loop changes the number of values from which the average is calculated */
         tempArraySum += inputArray[i];
      }
      tempArray[i] = tempArraySum / (2 * i + 1);
      tempArraySum = 0.0;
   }
   for (i = 0; i < (int) movingMedianWindow/2; i++) { /* transfer the moving median to array holding average values calculated above */
      movingMedianArray[i] = tempArray[i];
   }
   for (i = 0 ; i < arraySize; i++) { /* transfer the moving median to array holding average values calculated above */
      movingMedianArray[i + (int) movingMedianWindow/2] = tempMovingMedianArray[i];
   }
   for (i = arraySize - 1 - (int) movingMedianWindow/2; i < arraySize; i++) { /* do the same but for the upper part of the spectrum */
      for (j = 2 * i - arraySize + 1; j < arraySize; j++) {
         tempArraySum += inputArray[i];
      }
      movingMedianArray[i] = tempArraySum / (2 * arraySize - 2 * i - 1);
      tempArraySum = 0.0; /* zeroing variable */
   }
   for (i = 0 ; i < arraySize; i++) { /* check if there are any negative values and set them to 0 */
      if (movingMedianArray[i] < 0.0) {
         movingMedianArray[i] = 0.0;
      }
   }
}

/* CALCULATE AVERAGE VALUE */
float averageValue(unsigned int n, float *x) {
   unsigned int i;
   float av;
   av = 0.0;
   for (i = 0; i < n; i++) {
      av += x[i];
   }
   av /= n;
   return av;
}

/* CALCULATE STANDARD DEVIATION */
float standardDeviation(unsigned int n, float *x) {
   unsigned int i;
   float av, sigma_sum, av_sigma, sigma;
   av = 0.0;
   sigma_sum = 0.0;
   av_sigma = 0.0;
   for (i = 0; i < n; i++) {
      av += x[i];
   }
   av /= n;
   for (i = 0; i < n; i++) {
      sigma_sum += ((x[i] - av)*(x[i] - av));
   }
   sigma_sum /= n;
   sigma = sqrtf(sigma_sum);
   av_sigma = sigma / sqrtf(n);
   return sigma;
}

/* REMOVES THE "EXTENSION" FROM myString wHICH IS THE STRING TO PROCESS, extensionSeparator IS
THE EXTENSION SEPARATOR (ONLY ONE CHARACTER), pathSeparator IS THE PATH SEPARATOR (ALSO ONLY
ONE CHARACTER, 0 MEANS TO IGNORE), RETURNS AN ALLOCATED STRING IDENTICAL TO THE ORIGINAL BUT
WITH THE EXTENSION REMOVED. IT MUST BE FREED WHEN YOU'RE FINISHED WITH IT. IF YOU PASS IN NULL
OR THE NEW STRING CAN'T BE ALLOCATED, IT RETURNS NULL */
char *removeExtension (char* myString, char extensionSeparator, char pathSeparator) {
   char *returnString, *lastExtensionSeparator, *lastPathSeparator;
   if (myString == NULL) { /* if myString is not given function returns NULL as address of returnString */
      fprintf(stderr, "removeExtension> Input string not defined.\n");
      return NULL;
   }
   if ((returnString = malloc(strlen(myString) + 1)) == NULL) {
      fprintf(stderr, "removeExtension> Cannot allocate memory for input string.\n");
      return NULL; /* if memory for returnString is not allocated function returns NULL as address of returnString */
   }
   strcpy(returnString, myString); /* make a copy of myString into returnString */
   lastExtensionSeparator = strrchr(returnString, extensionSeparator); /* find last occurrence of extensionSeparator in returnString */
   /*lastPathSeparator = (pathSeparator == 0) ? NULL : strrchr (returnString, sep); ternary operator which can be replaced by below if statements */
   /* result = (a > b) ? x : y; is equivalent to: if (a > b) { result = x; } else { result = y; } this is how ternary operator works */
   if (pathSeparator == 0) { /* if there is no pathSeparator... */
      lastPathSeparator = NULL;
   } else {
      lastPathSeparator = strrchr(returnString, pathSeparator);  /* ...or find last occurrence of pathSeparator in returnString */
   }
   if (lastExtensionSeparator != NULL) { /* if it has an extension separator... */
      if (lastPathSeparator != NULL) { /* ...and it's before the extenstion separator... */
         if (lastPathSeparator < lastExtensionSeparator) {
            *lastExtensionSeparator = '\0'; /* ...then remove it */
         }
      } else {
         *lastExtensionSeparator = '\0'; /* has extension separator with no path separator */
      }
   }
   /* return the modified string */
   return returnString;
}

/* CONVERT DATE TO JULIAN DATE */
double gregjd(int jy, int jm, int jd, double rh, double rm, double rs) {
   double rj,ra,rb,rg,tjd,y,m;
   rj=jd+rh/24.0+rm/1440.0+rs/86400.0;
   if(jm<=2) {
      y=jy-1;
      m=jm+12;
   } else {
      y=jy;
      m=jm;
   }
   rg=y+m/100+rj/10000;
   ra=0.0;
   rb=0.0;
   if (rg>=1582.1015) {
      ra=floor(y/100.0);
      rb=2-ra+floor(ra/4.0);
   }
   tjd=floor(365.25*y) + floor(30.6001*(m+1)) + rj +1720994.5 + rb;
   return tjd;
}
