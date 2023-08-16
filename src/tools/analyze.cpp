#include <cstdio>
#include <cstdlib>

int main(int argc, char *argv[])
{
  if (argc < 4)
  {
    printf("usage: %s waveformAnalyzer file1.raw file2.raw [file3.raw] [...]\n", argv[0]);
    return -1;
  }
  printf("file1,file2,MSE,FFT,MFCC,DTW,COS,AVG,MAX,MAX DTW\n");
  fflush(stdout);
  for (int i = 2; i < argc; ++i)
  {
    char cmd[512];
    sprintf(cmd, "%s \"%s\" \"%s\" batch", argv[1], argv[i], argv[i]);
    system(cmd);
  }
  for (int i = 2; i < argc - 1; ++i)
  {
    for (int j = i + 1; j < argc; ++j)
    {
      char cmd[512];
      sprintf(cmd, "%s \"%s\" \"%s\" batch", argv[1], argv[i], argv[j]);
      system(cmd);
    }
  }
  return 0;
}
