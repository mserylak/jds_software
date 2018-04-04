struct UPP /* structure contains all the informations about data processing, PP has 16 bytes */
{
  unsigned int mode; /* mode of operation */
  unsigned int size; /* size of DMA data block */
  unsigned int prc_mode; /* processing parameters */
  unsigned int tst_gen; /* not used */
  unsigned int clk; /* not used */
  unsigned int fft_size; /* used for data transferring */
  unsigned int junk[10]; /* comment out this line, if you want to read old data created before August 2008 */
  /* unsigned int junk[26]; uncomment this line, if you want to read old data created before August 2008 */
};

struct UDSPP /* structure contains extended informations about about data processing and data acquisition mode, DSPP has 112 bytes */
{
  int FFT_Size; /* used in DSP - FFT size*/
  int MinDSPSize; /* used in DSP - minimal internal DSP buffer */
  int MinDMASize; /* used in DSP - minimal DMA size */
  int DMASizeCnt; /* used in DSP - counts of MinDMASize */
  int DMASize; /* used in DSP - DMA buffer size */
  float CLCfrq; /* sampling ADC frequency */
  int Synch; /* GPS synchronisation (0 - off, 1 - on) */
  int SSht; /* snapshot mode (should be always 1) */
  int Mode; /* waveform/spectrum/correlation mode (0/1/2) */
  int Wch; /* waveform stream transfer to DSP's  (0 - stream A, 1 - stream B, 2 - both streams) */
  int Smd; /* spectrum stream transfer to DSP's (0 - streams A & B, 1 - stream A & A*B, 2 - stream B & A*B, 3 - A+B & A*B) */
  int Offt; /* fixed part: high or low/tunable spectrum (0 - full 8192, 1 - low 4096, 2 - high 4096, 3 - tunable) */
  int Lb; /* low boundary of tunable spectrum */
  int Hb; /* high boundary of tunable spectrum (subtract 1 to get exact value) */
  int Wb; /* width of tunable spectrum */
  int NAvr; /* number of averaged spectra */
  int CAvr; /* averaging/decimation of spectra (0 - on, 1 - off and decimation instead) */
  int Weight; /* weighting window (0 - on, 1 - off) */
  int DCRem; /* DC compensation (0 - on, 1 - off) */
  int ExtSyn; /* external CLC synchronization (0 - on, 1 - off) */
  int Ch1; /* stream A on/off (0 - on, 1 - off) */
  int Ch2; /* stream B on/off (0 - on, 1 - off) */
  int ExtWin; /* external weighting coefficients (0 - internal weighting window, 1 - external weighting window) */
  int Clip; /* spectrum clipping value K: dB = 2^21*10^K/20 */
  int HPF0; /* stream 1 high pass filter settings */
  int HPF1; /* stream 2 high pass filter settings */
  int LPF0; /* not used */
  int LPF1; /* not used */
  int ATT0; /* stream attenuation 0..31 dB */
  int ATT1; /* stream attenuation 0..31 dB */
  char Soft[16];
  char SVer[16];
  char DSPv[32];
  int junk[66]; /* comment out this line, if you want to read old data created before August 2008 */
  /* int junk[50]; uncomment this line, if you want to read old data created before August 2008 */
};

struct syst /* structure contains system time, it must be short int because IDL is using 2-byte integers to store the information */
{
  short int yr; /* year */
  short int mn; /* month */
  short int dow; /* day of week, this should not exceed 7 ;) */
  short int day; /* day of month */
  short int hr; /* hour */
  short int min; /* minute */
  short int sec; /* second */
  short int msec; /* millisecond */
  short int junk[8];
};

struct FHEADER /* structure contains file header, 1024 bytes size with following fields */
{
  char name[32]; /* original name of the file in the format ddmmyy_hhmmss.jds */
  char time[32]; /* time of the file creation, set by UNIX function call ctime */
  char gmtt[32]; /* as in the previous field but contains the GMT time */
  char sysn[32]; /* operator name (system name, observatory name etc. */
  struct syst SYSTEMTIME; /* binary data of the SYSTEMTIME structure, contains the system time of file creation down to millisecond level */
  char place[96]; /* place of the observation */
  char desc[256]; /* additional field used for informations concerning observations */
  struct UPP PP; /* data processing and service parameters, global declaration of PP as variable of UPP type */
  struct UDSPP DSPP; /* extended version of the previous field, global declaration of DSPP as variable of UDSP type */
};
