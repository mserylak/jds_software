#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "jds.h"

/* function extracting bit values from integer variable */
unsigned int getBits(int x, int p, int n)
{
  return (x >> (p + 1 - n)) & ~(~0 << n); /* read "n" bits at "p" position from the "x" variable, WARNING: position should be rightmost bit from interesting range */
}

/* function outputs the contents of the header */
int dumpheaderjds(struct FHEADER headerjds)
{
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
  printf("dumpheaderjds> PP.mode bits 2..0:      %u (0:waveform data both streams; 1:waveform data one stream selected by bit 5)\n", getBits(headerjds.PP.mode, 2, 3));
  printf("dumpheaderjds>                           (2:spectra in both streams)\n");
  printf("dumpheaderjds>                           (3:first stream as A (spectrum), second stream as A*B (correlation))\n"); 
  printf("dumpheaderjds>                           (4:first stream as B (spectrum), second stream as A*B (correlation))\n");
  printf("dumpheaderjds>                           (5:first stream as A+B (spectrum), second stream as A*B (correlation))\n");
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
  printf("dumpheaderjds> DSPP.Wch:               %d (0:stream A waveform; 1:stream B waveform; 2:both streams waveform)\n", headerjds.DSPP.Wch);
  printf("dumpheaderjds> DSPP.Smd:               %d (0:both streams with spectra; 1:stream with A spectra and stream B with correlation (A*B))\n", headerjds.DSPP.Smd);
  printf("dumpheaderjds>                           (2:stream B with spectra and stream B with correlation (A*B))\n");
  printf("dumpheaderjds>                           (3:stream A with spectra (A+B) and stream B with correlation (A*B))\n");
  printf("dumpheaderjds> DSPP.Offt:              %d (0:full spectrum, 8192 channels; 1:lower half of spectrum, 4096 channels; 2:top half of spectrum, 4096 channels; 3:tunable)\n", headerjds.DSPP.Offt);
  printf("dumpheaderjds> DSPP.Lb:                %d (low cut off of the spectrum in channels)\n", headerjds.DSPP.Lb);
  printf("dumpheaderjds> DSPP.Hb:                %d (high cut off of the spectrum in channels)\n", headerjds.DSPP.Hb);
  printf("dumpheaderjds> DSPP.Wb:                %d (spectrum width in channels)\n", headerjds.DSPP.Wb);
  printf("dumpheaderjds> DSPP.NAvr:              %d (number of averaged spectra)\n", headerjds.DSPP.NAvr);
  printf("dumpheaderjds> DSPP.CAvr:              %d (spectrum averaging, 0:on; 1:off) \n", headerjds.DSPP.CAvr);
  printf("dumpheaderjds> DSPP.Weight:            %d (weighting window, 0:on; 1:off)\n", headerjds.DSPP.Weight);
  printf("dumpheaderjds> DSPP.DCRem:             %d (DC compensation, 0:off; 1:on)\n", headerjds.DSPP.DCRem);
  printf("dumpheaderjds> DSPP.ExtSyn:            %d (external CLC synchronisation, 0:off; 1:on)\n", headerjds.DSPP.ExtSyn);
  printf("dumpheaderjds> DSPP.Ch1:               %d (stream A, 0:on; 1:off)\n", headerjds.DSPP.Ch1);
  printf("dumpheaderjds> DSPP.Ch2:               %d (stream B, 0:on; 1:off)\n", headerjds.DSPP.Ch2);
  printf("dumpheaderjds> DSPP.ExtWin:            %d (external weighting coefficients (0:internal weighting window; 1:external weighting window))\n", headerjds.DSPP.ExtWin);
  printf("dumpheaderjds> DSPP.Clip:              %d (spectrum clipping value K: dB = 2^21*10^K/20)\n", headerjds.DSPP.Clip);
  printf("dumpheaderjds> DSPP.HPF0:              %d (stream A High Pass Filter settings; 0:8MHz; 1:12MHz; 2:16MHz; 3:20MH)\n", headerjds.DSPP.HPF0);
  printf("dumpheaderjds> DSPP.HPF1:              %d (stream B High Pass Filter settings; 0:8MHz; 1:12MHz; 2:16MHz; 3:20MH)\n", headerjds.DSPP.HPF1);
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

/* function converts the dspz floating point data format to conventional 32-bit floating point data format */
int DSPZ2Float(struct FHEADER *headerjds, unsigned int *rawData, int numberChannels, int correlation, float *dataFloat)
{
  int i, Nc;
  float SNrm, WNrm;
  int expn, sign, expnRe, signRe, expnIm, signIm;
  unsigned int mantissa, mantissaRe, mantissaIm;
  unsigned int stream, streamRe, streamIm;
  i = Nc = 0;
  expn = sign = mantissa = 0;
  expnRe = signRe = mantissaRe = 0;
  expnIm = signIm = mantissaIm = 0;
  SNrm = WNrm = 0.0;
  if (correlation > 4) /* data in standard modes: waveform, spectral or standard correlation */
  {
    if (headerjds->DSPP.Mode == 0) /* data in the waveform mode */
    {
      WNrm = 1.0 / 32768.0; /* normalisation */
      for (i = 0; i < numberChannels; i++)
      {
        dataFloat[i] = rawData[i] * WNrm; /* conversion from DSP float to PC float */
      }
    }
    else if (headerjds->DSPP.Mode == 1) /* data in the spectral mode */
    {
      SNrm = 4.0 * 2.0 * 1024.0 / 4294967296.0 / headerjds->DSPP.NAvr; /* normalisation */
      for (i = 0; i < numberChannels; i++)
      {
        expn = getBits(rawData[i], 4, 5); /* bits 4...0 - exponent */
        mantissa = getBits(rawData[i], 31, 26); /* bits 31...6 - mantissa value */
        dataFloat[i] = (float)mantissa / pow(2.0, expn) / SNrm; /* conversion from DSPZ float to PC float */
      }
    }
    else if (headerjds->DSPP.Mode == 2) /* data in the standard correlation mode */
    {
      SNrm = 4.0 * 2.0 * 1024.0 / 4294967296.0 / headerjds->DSPP.NAvr; /* normalisation */
      for (i = 0; i < numberChannels; i++)
      {
        expn = getBits(rawData[i], 4, 5); /* bits 4...0 - exponent */
        if ((getBits(rawData[i], 5, 1)) == 1) /* bit 5 - the sign of mantissa in standard correlation mode 0 - plus / 1 - minus */
        {
          sign = -1.0;
        }
        else if ((getBits(rawData[i], 5, 1)) == 0)
        {
          sign = 1.0;
        }
        mantissa = getBits(rawData[i], 31, 26); /* bits 31...6 - mantissa absolute value */
        dataFloat[i] = sign * (float)mantissa / pow(2.0, expn) / SNrm; /* conversion from DSP float to PC float */
      }
    }
  }
  else if (correlation == 0 || correlation == 1 || correlation == 2 || correlation == 3) /* extract streams 0 to 3 from the non-standard correlation DSPZ data */
  {
    Nc = 4;
    SNrm = 4.0 * 2.0 * 1024.0 / 4294967296.0 / headerjds->DSPP.NAvr; /* normalisation */
    for (i = 0; i < numberChannels; i++)
    {
      stream = rawData[i * Nc + 3]; /* select samples from 3rd stream */
      expn = getBits(stream, 4, 5); /* bits 4...0 - exponent */
      if ((getBits(stream, 5, 1)) == 1) /* bit 5 - the sign of mantissa in correlation mode 0 - plus / 1 - minus */
      {
        sign = -1.0;
      }
      else if ((getBits(stream, 5, 1)) == 0)
      {
        sign = 1.0;
      }
      mantissa = getBits(stream, 31, 26); /* bits 31...6 - mantissa absolute value */
      dataFloat[i] = sign * (float)mantissa / pow(2.0, expn) / SNrm; /* conversion from DSP float to PC float */
    }
  }
  else if (correlation == 4) /* extracts power from the non-standard correlation DSPZ data (sqrt(A^2 + B^2)) */
  {
    Nc = 4;
    SNrm = 4.0 * 2.0 * 1024.0 / 4294967296.0 / headerjds->DSPP.NAvr; /* normalisation */
    for (i = 0; i < numberChannels; i++)
    {
        streamRe = rawData[i * Nc + 2];
        streamIm = rawData[i * Nc + 3];
        expnRe = getBits(streamRe, 4, 5); /* bits 4...0 - exponent */
        expnIm = getBits(streamIm, 4, 5); /* bits 4...0 - exponent */
        if ((getBits(streamRe, 5, 1)) == 1) /* bit 5 - the sign of mantissa in correlation mode 0 - plus / 1 - minus */
        {
          signRe = -1.0;
        }
        if ((getBits(streamIm, 5, 1)) == 1) /* bit 5 - the sign of mantissa in correlation mode 0 - plus / 1 - minus */
        {
          signIm = -1.0;
        }
        else if ((getBits(streamRe, 5, 1)) == 0)
        {
          signRe = 1.0;
        }
        else if ((getBits(streamIm, 5, 1)) == 0)
        {
          signIm = 1.0;
        }
        mantissaRe = getBits(streamRe, 31, 26); /* bits 31...6 - mantissa absolute value */
        mantissaIm = getBits(streamIm, 31, 26); /* bits 31...6 - mantissa absolute value */
        dataFloat[i] = sqrt(pow((signRe * (float)mantissaRe / pow(2.0, expnRe) / SNrm), 2.0) + pow((signIm * (float)mantissaIm / pow(2.0, expnIm) / SNrm), 2.0)); /* conversion from DSP float to PC float */
    }
  }
  return 0;
}

/* function getting service data from 2 last samples per stream, 4 in total, warning: only for data taken after 2008 */
void DSPZ2Service(struct FHEADER *headerjds, unsigned int *rawData, int numberChannels, int Verbose, int *corruptedSpectrumFlag)
{
  int CntA1_bits26_0, CntB1_bits16_0, CntA2_bit31, CntA2_bit30, CntA2_bits29_0, CntB2_bit31, CntB2_bit30, CntB2_bits29_0;
  *corruptedSpectrumFlag = 0; /* this flag has to be reset every time a new spectrum is read */
  CntA2_bit31 = getBits(rawData[(numberChannels) - 2], 31, 1); /* 1 - CntA2 - bit 31 - overflow flag ADC-A for accumulation time FFT */
  CntA2_bit30 = getBits(rawData[(numberChannels) - 2], 30, 1); /* 2 - CntA2 - bit 30 - CRC flag in stream A */
  CntB2_bit31 = getBits(rawData[(numberChannels) - 1], 31, 1); /* 3 - CntB2 - bit 31 - overflow flag ADC-B for accumulation time FFT */
  CntB2_bit30 = getBits(rawData[(numberChannels) - 1], 30, 1); /* 4 - CntB2 - bit 30 - CRC flag in stream B */
  CntA2_bits29_0 = getBits(rawData[(numberChannels) - 2], 29, 30); /* 5 - CntA2 - bits 29..0 - count of us since start of recording */
  CntB2_bits29_0 = getBits(rawData[(numberChannels) - 1], 29, 30); /* 6 - CntB2 - bits 29..0 - count of the number of issued FFTs since the start of recording (might be reset to 0 if a hardware error) */
  CntA1_bits26_0 = getBits(rawData[(numberChannels) - 4], 26, 27); /* 7 - CntA1 - bits 26..0 - the counter of phase of second (number of cycles of the ADC from the rise of the PPS to first sample in the data block used for current FFT) */
  CntB1_bits16_0 = getBits(rawData[(numberChannels) - 3], 16, 17); /* 8 - CntB1 - bits 16..0 - count seconds in the day */
  if (CntA2_bit31 == 1 || CntA2_bit30 == 1 || CntA2_bits29_0 == 1 || CntB2_bit31 == 1 || CntB2_bit30 == 1 || CntB2_bits29_0 == 1)
  {
    *corruptedSpectrumFlag = 1;
    if (Verbose == 2 || Verbose == 3)
    {
      printf("DSPZ2service> Zapping corrupted spectrum due to ADC-A: %d CRC-A: %d ADC-B: %d CRC-B: %d us: %d FFT: %d phase-of-sec: %d count-sec-day: %d\n", CntA2_bit31, CntA2_bit30, CntB2_bit31, CntB2_bit30, CntA2_bits29_0, CntB2_bits29_0, CntA1_bits26_0, CntB1_bits16_0);
    }
  }
}

/* function reversing array */
void reverseArray(float array[], int size)
{
  int i;
  int m = size >> 1;
  int j = size - 1;
  float temp;
  for (i = 0; i < m; ++i, --j)
  {
    temp = array[i];
    array[i] = array[j];
    array[j] = temp;
  }
}

/* comparison function for qsort and readZapFile */
int compareFunction(const void * a, const void * b)
{
  return ( *(int*)a - *(int*)b );
}

/* function reads the file with list of bad channels */
int readZapFile(char *zapFileName, int *zapChannels, int *counter)
{
  FILE *ZAP_FILE;
  int i, j, c;
  c = 0;
  ZAP_FILE = fopen(zapFileName, "r");
  if (ZAP_FILE == NULL)
  {
    fprintf(stderr, "readZapFile> Cannot open %s\n\n", zapFileName);
    return -1;
  }
  do
  {
    j = fscanf(ZAP_FILE, "%d", &i); /* read in spectral channel number */
    /*if (j == 1) {
      printf("Zapping channel %d\n", i);
    }*/
    zapChannels[c] = i;
    c++;
  }
  while (j == 1);
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

/* calculate moving average (resulting array will have n-m-1 elements) */
void movingAverage(int arraySize, float *inputArray, int runningMeanWindow, float *runningMeanArray)
{
  int i, runningMeanArraySize, counter;
  float arraySum;
  arraySum = 0.0; /* zeroing the variable */
  runningMeanArraySize = arraySize - runningMeanWindow; /* calculating the size of running average size */
  for (counter = 0; counter <= runningMeanArraySize; counter++) /* for each value in running average array... */
  {
    for (i = counter; i < (counter + runningMeanWindow); i++) /* move the window by one bin */
    {
      arraySum += inputArray[i]; /* ... calculate the sum in the running window */
    }
    runningMeanArray[counter] = arraySum / runningMeanWindow; /* storing running average in array */
    /*printf("%d %lf\n", counter, runningMeanValue);*/
    arraySum = 0.0; /* zeroing variable for the next run */
  }
}

/* compute a running median of one array */
void movingMedian(int arraySize, float *inputArray, int movingMedianWindow, float *movingMedianArray)
{
  int i, j, movingMedianArraySize;
  float *sortedArray, *tempArray, *tempMovingMedianArray;
  float tempArraySum;
  tempArraySum = 0.0;
  movingMedianArraySize = arraySize - movingMedianWindow + 1; /* calculate array size after applying moving median */
  sortedArray = (float *) calloc(movingMedianWindow, sizeof(float)); /* allocate memory for sorted array */
  tempMovingMedianArray = (float *) calloc(arraySize, sizeof(float)); /* allocate memory for temporary array */
  tempArray = (float *) calloc(arraySize, sizeof(float)); /* allocate memory for temporary array */
  if (sortedArray == NULL)
  {
    puts("runningMedian> Cannot allocate memory for sortedArray");
    return;
  }
  for (i = 0; i < movingMedianArraySize; i++) /* start calculating moving median */
  {
    for (j = 0; j < movingMedianWindow; j++)
    {
      sortedArray[j] = inputArray[j + i]; /* input the data to array which will be sorted */
    }
    qsort(sortedArray, movingMedianWindow, sizeof(float), compareFunction); /* sort the array with built in qsort C function */
    if (movingMedianWindow % 2) /* calculate the median depending if the window is divisible by 2... */
    {
      tempMovingMedianArray[i] = sortedArray[movingMedianWindow / 2];
    }
    else /* ...or not */
    {
      tempMovingMedianArray[i] = (sortedArray[movingMedianWindow / 2] + sortedArray[(movingMedianWindow - 1) / 2]) * 0.5;
    }
  }
  /* this bit of the code extends the moving median by adding the signal which is lost during its calculation (N - M + 1) */
  for (i = 1; i < (int) movingMedianWindow/2; i++) /* calculate the average value for beginning of the spectrum */
  {
    for (j = 0; j < 2 * i; j++) /* this loop changes the number of values from which the average is calculated */
    {
      tempArraySum += inputArray[i];
    }
    tempArray[i] = tempArraySum / (2 * i + 1);
    tempArraySum = 0.0;
  }
  for (i = 0; i < (int) movingMedianWindow/2; i++) /* transfer the moving median to array holding average values calculated above */
  {
    movingMedianArray[i] = tempArray[i];
  }
  for (i = 0 ; i < arraySize; i++) /* transfer the moving median to array holding average values calculated above */
  {
    movingMedianArray[i + (int) movingMedianWindow/2] = tempMovingMedianArray[i];
  }
  for (i = arraySize - 1 - (int) movingMedianWindow/2; i < arraySize; i++) /* do the same but for the upper part of the spectrum */
  {
    for (j = 2 * i - arraySize + 1; j < arraySize; j++)
    {
      tempArraySum += inputArray[i];
    }
    movingMedianArray[i] = tempArraySum / (2 * arraySize - 2 * i - 1);
    tempArraySum = 0.0; /* zeroing variable */
  }
  for (i = 0 ; i < arraySize; i++) /* check if there are any negative values and set them to 0 */
  {
    if (movingMedianArray[i] < 0.0)
    {
      movingMedianArray[i] = 0.0;
    }
  }
}

/* calculate average value */
float averageValue(unsigned int n, float *x)
{
  unsigned int i;
  float average;
  average = 0.0;
  for (i = 0; i < n; i++)
  {
    average += x[i];
  }
  average /= n;
  return average;
}

/* calculate standard deviation */
float standardDeviation(unsigned int n, float *x)
{
  unsigned int i;
  float average, sigma_sum, average_sigma, sigma;
  average = 0.0;
  sigma_sum = 0.0;
  average_sigma = 0.0;
  for (i = 0; i < n; i++)
  {
    average += x[i];
  }
  average /= n;
  for (i = 0; i < n; i++)
  {
    sigma_sum += ((x[i] - average)*(x[i] - average));
  }
  sigma_sum /= (n - 1);
  sigma = sqrtf(sigma_sum);
  average_sigma = sigma / sqrtf(n);
  return sigma;
}

/* removes the "extension" from myString which is the string to process, extensionSeparator is
the extension separator (only one character), pathSeparator is the path separator (also only
one character, 0 means to ignore), returns an allocated string identical to the original but
with the extension removed. it must be freed when you're finished with it. if you pass in null
or the new string can't be allocated, it returns null */
char *removeExtension (char* myString, char extensionSeparator, char pathSeparator)
{
  char *returnString, *lastExtensionSeparator, *lastPathSeparator;
  if (myString == NULL) /* if myString is not given function returns NULL as address of returnString */
  {
    fprintf(stderr, "removeExtension> Input string not defined.\n");
    return NULL;
  }
  if ((returnString = malloc(strlen(myString) + 1)) == NULL)
  {
    fprintf(stderr, "removeExtension> Cannot allocate memory for input string.\n");
    return NULL; /* if memory for returnString is not allocated function returns NULL as address of returnString */
  }
  strcpy(returnString, myString); /* make a copy of myString into returnString */
  lastExtensionSeparator = strrchr(returnString, extensionSeparator); /* find last occurrence of extensionSeparator in returnString */
  /* *lastPathSeparator = (pathSeparator == 0) ? NULL : strrchr (returnString, sep); ternary operator which can be replaced by below if statements */
  /* result = (a > b) ? x : y; is equivalent to: if (a > b) { result = x; } else { result = y; } this is how ternary operator works */
  if (pathSeparator == 0) /* if there is no pathSeparator... */
  {
    lastPathSeparator = NULL;
  }
  else
  {
    lastPathSeparator = strrchr(returnString, pathSeparator);  /* ...or find last occurrence of pathSeparator in returnString */
  }
  if (lastExtensionSeparator != NULL) /* if it has an extension separator... */
  {
    if (lastPathSeparator != NULL) /* ...and it's before the extension separator... */
    {
      if (lastPathSeparator < lastExtensionSeparator)
      {
        *lastExtensionSeparator = '\0'; /* ...then remove it */
      }
    }
    else
    {
      *lastExtensionSeparator = '\0'; /* has extension separator with no path separator */
    }
  }
  /* return the modified string */
  return returnString;
}

/* convert date to Julian date */
double gregorian2Julian(int gregorianYear, int gregorianMonth, int gregorianDay, double gregorianHour, double gregorianMinute, double gregorianSecond)
{
  double decimalDay, correctionA, correctionB, decimalYear, julianDate, integerYear, integerMonth;
  decimalDay = gregorianDay + gregorianHour / 24.0 + gregorianMinute / 1440.0 + gregorianSecond / 86400.0;
  if (gregorianMonth <= 2)
  {
    integerYear = gregorianYear - 1;
    integerMonth = gregorianMonth + 12;
  }
  else
  {
    integerYear = gregorianYear;
    integerMonth = gregorianMonth;
  }
  decimalYear = integerYear + integerMonth / 100.0 + decimalDay / 10000.0;
  correctionA = 0.0;
  correctionB = 0.0;
  if (decimalYear >= 1582.1015)
  {
    correctionA = floor(integerYear / 100.0);
    correctionB = 2 - correctionA + floor(correctionA / 4.0);
  }
  julianDate = floor(365.25 * integerYear) + floor(30.6001 * (integerMonth + 1)) + decimalDay + 1720994.5 + correctionB;
  return julianDate;
}
