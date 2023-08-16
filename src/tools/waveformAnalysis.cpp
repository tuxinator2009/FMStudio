#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <complex>
#include <initializer_list>
#include <cmath>

const int numSamples = 8192;
const int sampleRate = 8000;
const int numMelFilters = 26;  // Number of Mel filters
const int numCoefficients = 13;  // Number of MFCC coefficients to keep
const int numFrequencies = numSamples / 2 + 1;
const double mseWeight = 1.0;//0.5;
const double fftWeight = 1.0;//0.3;
const double mfccWeight = 1.0;//0.4;
const double dtwWeight = 1.0;//0.8;
const double cosWeight = 1.0;//0.2;
const double avgAmpWeight = 1.0;
const double maxAmpWeight = 1.0;
const double power = 8.0;
const double baseFrequency = 261.6;
const int window = (sampleRate / baseFrequency) * 1.5;
const int windowStep = (sampleRate / baseFrequency) / 2.0;

void fft(std::complex<double>* data, int size)
{
  // Bit-reverse swapping
  int n = size;
  int j = n >> 1;
  for (int i = 1; i < n - 1; i++)
  {
    if (i < j)
      std::swap(data[j], data[i]);
    int k = n >> 1;
    while (j >= k)
    {
      j -= k;
      k >>= 1;
    }
    j += k;
  }
  
  // Cooley-Tukey Decimation in Time FFT
  for (int step = 1; step < n; step <<= 1)
  {
    double theta = -M_PI / step;
    std::complex<double> w(1.0, 0.0);
    std::complex<double> wn(std::cos(theta), std::sin(theta));
    
    for (int m = 0; m < step; m++)
    {
      for (int k = m; k < n; k += step << 1)
      {
        std::complex<double> t = w * data[k + step];
        std::complex<double> u = data[k];
        data[k] = u + t;
        data[k + step] = u - t;
      }
      w *= wn;
    }
  }
}

void normalize(std::complex<double> *data, int size)
{
  double max = 0.0;
  for (int i = 0; i < size; ++i)
  {
    if (std::abs(data[i]) > max)
      max = std::abs(data[i]);
  }
  for (int i = 0; i < size; ++i)
    data[i] /= max;
}

void normalize(double *data, int size)
{
  double max = 0.0;
  for (int i = 0; i < size; ++i)
  {
    if (std::abs(data[i]) > max)
      max = std::abs(data[i]);
  }
  for (int i = 0; i < size; ++i)
    data[i] /= max;
}

double calculateRMSE(std::complex<double> data[2][numSamples], int size)
{
  double sumSquaredDiff = 0.0;
  for (int i = 0; i < size; ++i)
  {
    double diff = std::abs(data[0][i] - data[1][i]);
    sumSquaredDiff += diff * diff;
  }
  double meanSquaredDiff = sumSquaredDiff / size;
  double rmse = std::sqrt(meanSquaredDiff);
  return rmse;
}

void fillMelFilterbank(double melFilterbank[numMelFilters][numFrequencies])
{
  double minMel = 2595 * log10(1 + 0 / 700);  // Mel scale at lowest frequency
  double maxMel = 2595 * log10(1 + (sampleRate / 2) / 700);  // Mel scale at Nyquist frequency
  
  // Create evenly spaced center frequencies on the Mel scale
  double centerFrequencies[numMelFilters];
  for (int i = 0; i < numMelFilters; ++i)
  {
    double mel = minMel + (maxMel - minMel) * (i + 1) / (numMelFilters + 1);
    centerFrequencies[i] = 700 * (pow(10, mel / 2595) - 1);
    for (int j = 0; j < numFrequencies; ++j)
      melFilterbank[i][j] = 0.0;
  }
  
  // Create the Mel filterbank matrix
  for (int i = 0; i < numMelFilters; ++i)
  {
    for (int j = 1; j < numFrequencies; ++j)
    {
      double lowerFreq = (j - 1) * sampleRate / numSamples;
      double centerFreq = centerFrequencies[i];
      double upperFreq = (j + 1) * sampleRate / numSamples;
      if (lowerFreq <= centerFreq && centerFreq <= upperFreq)
      {
        if (centerFreq <= lowerFreq + 1)
          melFilterbank[i][j] = (centerFreq - lowerFreq) / (upperFreq - lowerFreq);
        else if (centerFreq >= upperFreq - 1)
          melFilterbank[i][j] = (upperFreq - centerFreq) / (upperFreq - lowerFreq);
        else
          melFilterbank[i][j] = 1;
      }
    }
  }
}

void applyDCT(const double *input, double *output)
{
  for (int i = 1; i < numCoefficients + 1; ++i)
  {
    double sum = 0.0;
    for (int j = 0; j < numMelFilters; ++j)
    {
      double coefficient = input[j];
      double angle = M_PI * (j + 0.5) * i / numMelFilters;
      sum += coefficient * std::cos(angle);
    }
    output[i - 1] = sum;
  }
}

void calculateMFCCs(const std::complex<double> *fftMagnitudes, double *mfccs)
{
  double melFilterbank[numMelFilters][numFrequencies];
  double melEnergies[numMelFilters];
  
  // Define the Mel filterbank coefficients (replace with your own)
  fillMelFilterbank(melFilterbank);
  // Apply the Mel filterbank to FFT magnitudes
  for (int i = 0; i < numMelFilters; ++i)
  {
    melEnergies[i] = 0.0;
    for (int j = 0; j < numFrequencies; ++j)
    {
      melEnergies[i] += melFilterbank[i][j] * std::abs(fftMagnitudes[j]);
    }
  }
  
  // Take the logarithm of Mel energies
  for (int i = 0; i < numMelFilters; ++i)
    melEnergies[i] = std::log(melEnergies[i]);
  
  // Apply Discrete Cosine Transform (DCT) to get MFCC coefficients
  applyDCT(melEnergies, mfccs);
}

double calculateEuclideanDistance(const double *mfccSet1, const double *mfccSet2)
{
  double sumSquaredDifferences = 0.0;
  for (size_t i = 0; i < numCoefficients; ++i)
  {
    double difference = mfccSet1[i] - mfccSet2[i];
    sumSquaredDifferences += difference * difference;
  }
  return std::sqrt(sumSquaredDifferences);
}

/*double calculateDTW(const double *sequence1, const double *sequence2, double *max)
{
  double dtw[numCoefficients][numCoefficients];
  *max = 0.0;
  for (int i = 0; i < numCoefficients; ++i)
  {
    for (int j = 0; j < numCoefficients; ++j)
    {
      double distance = std::abs(sequence1[i] - sequence2[j]);
      dtw[i][j] = distance;
      if (i > 0 || j > 0)
      {
        double l = (i > 0) ? dtw[i - 1][j]:INFINITY;
        double m = (j > 0) ? dtw[i][j - 1]:INFINITY;
        double r = (i > 0 && j > 0) ? dtw[i - 1][j - 1]:INFINITY;
        dtw[i][j] += std::min(l, std::min(m, r));
      }
      if (dtw[i][j] > *max)
        *max = dtw[i][j];
    }
  }
  return 1.0 - dtw[numCoefficients - 1][numCoefficients - 1];
}*/

double euclideanDistance(double mfcc1, double mfcc2)
{
  double diff = mfcc1 - mfcc2;
  return diff * diff;
}

double calculateDTW(const double *sequence1, const double *sequence2, double *max)
{
  // Initialize DTW matrix
  double dtw[numCoefficients+1][numCoefficients+1];
  *max = 0.0;
  for (size_t i = 1; i <= numCoefficients; ++i)
  {
    for (size_t j = 1; j <= numCoefficients; ++j)
    {
      double cost = euclideanDistance(sequence1[i - 1], sequence2[j - 1]);
      dtw[i][j] = cost + std::min(dtw[i - 1][j], std::min(dtw[i][j - 1], dtw[i - 1][j - 1]));
      if (dtw[i][j] > *max)
        *max = dtw[i][j];
    }
  }
  
  // Return the DTW distance between the sequences
  return dtw[numCoefficients][numCoefficients];
}

double dotProduct(const double *a, const double *b)
{
  double sum = 0.0;
  for (size_t i = 0; i < numCoefficients; ++i) {
    sum += a[i] * b[i];
  }
  return sum;
}

double magnitude(const double *v)
{
  double sum = 0.0;
  for (int i = 0; i < numCoefficients; ++i)
    sum += v[i] * v[i];
  return std::sqrt(sum);
}

double calculateCosineDistance(const double *sequence1, const double *sequence2)
{
  double dot = 0.0;
  double mag1 = 0.0;
  double mag2 = 0.0;
  for (int i = 0; i < numCoefficients; ++i)
  {
    dot += sequence1[i] * sequence2[i];
    mag1 += sequence1[i] * sequence1[i];
    mag2 += sequence2[i] * sequence2[i];
  }
  return 1.0 - (dot / (std::sqrt(mag1 * mag2)));
  
  /*double dot = dotProduct(sequence1, sequence2);
  double mag1 = magnitude(sequence1);
  double mag2 = magnitude(sequence2);
  if (mag1 == 0.0 || mag2 == 0.0)
    return -1.0;  // Invalid input
  return 1.0 - (dot / (mag1 * mag2));*/
}

/*double cosine_similarity(double *A, double *B, unsigned int Vector_Length)
{
  double dot = 0.0, denom_a = 0.0, denom_b = 0.0 ;
  for(unsigned int i = 0u; i < Vector_Length; ++i) {
    dot += A[i] * B[i] ;
    denom_a += A[i] * A[i] ;
    denom_b += B[i] * B[i] ;
  }
  return dot / (sqrt(denom_a) * sqrt(denom_b)) ;
}*/

double calculateMSE(const std::complex<double> input[2][numSamples], double *max)
{
  double sum = 0.0;
  *max = 0.0001;
  for (int i = 0; i < numSamples; ++i)
  {
    double distance = std::abs(input[0][i] - input[1][i]);
    if (distance * distance > *max)
      *max = distance * distance;
    sum += distance * distance;
  }
  *max = 1.0;//Try with maximum possible difference between two samples
  sum /= numSamples;
  return std::sqrt(sum);
}

void calculateAmplitudeShiftRating(const std::complex<double> input[2][numSamples], double *avg, double *max)
{
  double avgSum = 0.0;
  double maxSum = 0.0;
  int numWindows = (numSamples - window) / windowStep + 1;
  for (int i = 0; i < numSamples; i += windowStep)
  {
    double sum[2] = {0.0, 0.0};
    double max[2] = {0.0, 0.0};
    if (i + window >= numSamples)
      break;
    for (int j = i; j < i + window && j < numSamples; ++j)
    {
      if (input[0][j].real() > max[0])
        max[0] = std::abs(input[0][j]);
      if (input[1][j].real() > max[1])
        max[1] = std::abs(input[1][j]);
      sum[0] += input[0][j].real();
      sum[1] += input[1][j].real();
    }
    sum[0] /= window;
    sum[1] /= window;
    avgSum += std::abs(sum[1] - sum[0]);
    maxSum += std::abs(max[1] - max[0]);
  }
  avgSum /= numWindows;
  maxSum /= numWindows;
  *avg = avgSum;
  *max = maxSum;
}

// Calculate weighted average given a list of value-weight pairs
double calculateWeightedAverage(std::initializer_list<std::pair<double, double>> valueWeightPairs)
{
  double weightedSum = 0.0;
  double totalWeight = 0.0;
  double min = 1.0 / (valueWeightPairs.begin()->first + 0.0001);
  double max = min;
  double range;
  
  
  //First we get the min and max of the inverted values
  for (const auto& pair : valueWeightPairs)
  {
    min = std::min(min, 1.0 / (pair.first + 0.0001));
    max = std::max(max, 1.0 / (pair.first + 0.0001));
  }
  range = max - min;
  if (range < 0.0001)
    range = 1.0;
  
  //Next we invert the values and scale them from the range of min - max to 0 - 1
  for (const auto& pair : valueWeightPairs)
  {
    double value = 1.0 / (pair.first + 0.0001);
    double result = (value - min) / range;
    //double value = 1.0 - std::min(pair.first, 0.7);
    //double result = std::min(-std::log(value), 1.0);
    weightedSum += result * pair.second;
    totalWeight += pair.second;
  }
  
  if (totalWeight == 0.0)
    return 0.0; // Avoid division by zero
  
  return weightedSum / totalWeight;
}

bool loadFile(const char *location, std::complex<double> *samples)
{
  FILE *file = fopen(location, "rb");
  uint8_t data[numSamples];
  if (!file)
  {
    printf("Error: failed to open first file\n");
    perror("Reason:");
    return false;
  }
  //Initialize all values to silence (0x80)
  for (int i = 0; i < numSamples; ++i)
    data[i] = 0x80;
  //Read at most 8000 samples (remainder will be padded with silence and overage will get truncated)
  fread(data, 1, 8000, file);
  fclose(file);
  //Convert raw unsigned 8-bit samples to doubles
  for (int i = 0; i < numSamples; ++i)
    samples[i] = data[i] / 128.0 - 1.0;
  normalize(samples, numSamples);
  return true;
}

void printFileName(const char *location)
{
  int start = 0;
  while (location[start] != '\0')
    ++start;
  while (start > 0)
  {
    if (location[start] == '/')
      break;
    --start;
  }
  ++start;
  printf("%s", location + start);
}

int main(int argc, char *argv[])
{
  std::complex<double> samples[2][numSamples];
  double mfccs[2][numCoefficients];
  double dtw;
  double maxDTW;
  double cosDistance;
  double maxCosDistance = 2.0;
  double rmse;
  double distance;
  double mse;
  double maxMSE;
  double fftLikeness = 0.0;
  double mfccLikeness = 0.0;
  double dtwLikeness = 0.0;
  double cosLikeness = 0.0;
  double mseLikeness = 0.0;
  double avgAmpLikeness = 0.0;
  double maxAmpLikeness = 0.0;
  double averageLikeness = 0.0;
  bool batchMode = false;
  if (argc != 3 && argc != 4)
  {
    printf("usage: %s file1.raw file2.raw\n", argv[0]);
    return -1;
  }
  if (argc == 4)
    batchMode = true;
  if (!loadFile(argv[1], samples[0]))
    return -1;
  if (!loadFile(argv[2], samples[1]))
    return -1;
  
  // Generate your 8-bit unsigned audio samples here and convert to complex numbers
  mse = calculateMSE(samples, &maxMSE);
  calculateAmplitudeShiftRating(samples, &avgAmpLikeness, &maxAmpLikeness);
  mseLikeness = mse / maxMSE;
  
  fft(samples[0], numSamples);
  fft(samples[1], numSamples);
  
  normalize(samples[0], numFrequencies);
  normalize(samples[1], numFrequencies);
  
  calculateMFCCs(samples[0], mfccs[0]);
  calculateMFCCs(samples[1], mfccs[1]);
  
  normalize(mfccs[0], numCoefficients);
  normalize(mfccs[1], numCoefficients);
  
  rmse = calculateRMSE(samples, numFrequencies);
  fftLikeness = rmse;
  
  distance = calculateEuclideanDistance(mfccs[0], mfccs[1]);
  mfccLikeness = distance / (2 * std::sqrt(numCoefficients));
  
  dtw = calculateDTW(mfccs[0], mfccs[1], &maxDTW);
  dtwLikeness = dtw / maxDTW;
  
  cosDistance = calculateCosineDistance(mfccs[0], mfccs[1]);
  cosLikeness = cosDistance / maxCosDistance;
  
  averageLikeness = 1.0 - calculateWeightedAverage({
    {mseLikeness, mseWeight},
    {fftLikeness, fftWeight},
    {mfccLikeness, mfccWeight},
    {dtwLikeness, dtwWeight},
    {cosLikeness, cosWeight},
    {avgAmpLikeness, avgAmpWeight},
    {maxAmpLikeness, maxAmpWeight}
  });
  
  if (!batchMode)
  {
    printf("Likeness Scores: %6.2f%%\n", averageLikeness * 100.0);
    printf("  MSE:  %6.5f (%3.2f)\n", mseLikeness, mseWeight);
    printf("  FTT:  %6.5f (%3.2f)\n", fftLikeness, fftWeight);
    printf("  MFCC: %6.5f (%3.2f)\n", mfccLikeness, mfccWeight);
    printf("  DTW:  %6.5f (%3.2f) / %f\n", dtwLikeness, dtwWeight, maxDTW);
    printf("  COS:  %6.5f (%3.2f)\n", cosLikeness, cosWeight);
    printf("  AVG:  %6.2f (%3.2f)\n", avgAmpLikeness, avgAmpWeight);
    printf("  MAX:  %6.2f (%3.2f)\n", maxAmpLikeness, maxAmpWeight);
  }
  else
  {
    printFileName(argv[1]);
    printf(",");
    printFileName(argv[2]);
    printf(",%f,%f,%f,%f,%f,%f,%f,%f\n", mseLikeness, fftLikeness, mfccLikeness, dtwLikeness, cosLikeness, avgAmpLikeness, maxAmpLikeness, maxDTW);
  }
  //printf("Likeness Score: mse=%d%% ftt=%d%% mfcc=%d%% dtw=%d%% cosine=%d%% avg=%d%%\n", mseLikeness, fftLikeness, mfccLikeness, dtwLikeness, cosineLikeness, averageLikeness);
}
