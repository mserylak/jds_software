struct FHEADER headerjds;
int dumpheaderjds(struct FHEADER headerjds);
int DSPZ2Float(struct FHEADER *headerjds, unsigned int *rawdata, int nbsamp, float *dataf);
void DSPZ2Service(struct FHEADER *headerjds, unsigned int *RawData, int NumberSamples, int Verbose, int *CorruptedSpectrumFlag);
unsigned int getBits(int x, int p, int n);
void reverseArray(float array[], int size);
int readZapFile(char *zapFileName, int *zapChannels, int *counter);
void movingAverage(int arraySize, float *inputArray, int runningMeanWindow, float *runningMeanArray);
int compareFunction(const void * a, const void * b);
void movingMedian(int arraySize, float *inputArray, int movingMedianWindow, float *movingMedianArray);
float averageValue(unsigned int n, float *x);
float standardDeviation(unsigned int n, float *x);
char *removeExtension (char* myString, char extensionSeparator, char pathSeparator);
double gregorian2Julian(int gregorianYear, int gregorianMonth, int gregorianDay, double gregorianHour, double gregorianMinute, double gregorianSecond);
