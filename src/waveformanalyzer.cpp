/**********************************************************************************
 * MIT License                                                                    *
 *                                                                                *
 * Copyright (c) 2023 Justin (tuxinator2009) Davis                                *
 *                                                                                *
 * Permission is hereby granted, free of charge, to any person obtaining a copy   *
 * of this software and associated documentation files (the "Software"), to deal  *
 * in the Software without restriction, including without limitation the rights   *
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell      *
 * copies of the Software, and to permit persons to whom the Software is          *
 * furnished to do so, subject to the following conditions:                       *
 *                                                                                *
 * The above copyright notice and this permission notice shall be included in all *
 * copies or substantial portions of the Software.                                *
 *                                                                                *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR     *
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,       *
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE    *
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER         *
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,  *
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE  *
 * SOFTWARE.                                                                      *
 **********************************************************************************/

#include <cstdlib>
#include <cstdint>
#include <complex>
#include <initializer_list>
#include <cmath>
#include "waveformanalyzer.h"

void WaveformAnalyzer::fft(std::complex<double> data[numSamples])
{
  // Bit-reverse swapping
  int j = numSamples >> 1;
  for (int i = 1; i < numSamples - 1; i++)
  {
    if (i < j)
      std::swap(data[j], data[i]);
    int k = numSamples >> 1;
    while (j >= k)
    {
      j -= k;
      k >>= 1;
    }
    j += k;
  }
  
  // Cooley-Tukey Decimation in Time FFT
  for (int step = 1; step < numSamples; step <<= 1)
  {
    double theta = -M_PI / step;
    std::complex<double> w(1.0, 0.0);
    std::complex<double> wn(std::cos(theta), std::sin(theta));
    
    for (int m = 0; m < step; m++)
    {
      for (int k = m; k < numSamples; k += step << 1)
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

void WaveformAnalyzer::normalize(std::complex<double> *data, int size)
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

void WaveformAnalyzer::normalize(double *data, int size)
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

double WaveformAnalyzer::calculateRMSE(double data[2][numFrequencies])
{
  double sumSquaredDiff = 0.0;
  for (int i = 0; i < numFrequencies; ++i)
  {
    double diff = std::abs(data[0][i] - data[1][i]);
    sumSquaredDiff += diff * diff;
  }
  double meanSquaredDiff = sumSquaredDiff / numFrequencies;
  double rmse = std::sqrt(meanSquaredDiff);
  return rmse;
}

void WaveformAnalyzer::fillMelFilterbank(double melFilterbank[numMelFilters][numFrequencies])
{
  double minMel = 2595 * log10(1 + 0 / 700);  // Mel scale at lowest frequency
  double maxMel = 2595 * log10(1 + (sampleRate / 2) / 700);  // Mel scale at Nyquist frequency
  
  // Create evenly spaced center frequencies on the Mel scale
  double centerFrequencies[numMelFilters];
  for (int i = 0; i < numMelFilters; ++i)
  {
    double mel = minMel + (maxMel - minMel) * (i + 1) / (numMelFilters + 1);
    centerFrequencies[i] = 700 * (pow(10, mel / 2595) - 1);
  }
  
  // Create the Mel filterbank matrix
  for (int i = 0; i < numMelFilters; ++i)
  {
    for (int j = 0; j < numFrequencies; ++j)
      melFilterbank[i][j] = 0.0;
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

void WaveformAnalyzer::applyDCT(const double *input, double *output)
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

void WaveformAnalyzer::calculateMFCCs(const double fftMagnitudes[numFrequencies], double mfccs[numCoefficients])
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

double WaveformAnalyzer::calculateEuclideanDistance(const double *mfccSet1, const double *mfccSet2)
{
  double sumSquaredDifferences = 0.0;
  for (size_t i = 0; i < numCoefficients; ++i)
    sumSquaredDifferences += euclideanDistance(mfccSet1[i], mfccSet2[i]);
  return std::sqrt(sumSquaredDifferences) / (2.0 * std::sqrt(numCoefficients));
}

double WaveformAnalyzer::calculateDTW(const double mfccSet1[numCoefficients], const double mfccSet2[numCoefficients])
{
  // Initialize DTW matrix
  double dtw[numCoefficients + 1][numCoefficients + 1];
  double max = 0.0;
  for (size_t i = 1; i <= numCoefficients; ++i)
  {
    for (size_t j = 1; j <= numCoefficients; ++j)
    {
      double cost = euclideanDistance(mfccSet1[i - 1], mfccSet2[j - 1]);
      dtw[i][j] = cost + std::min(dtw[i - 1][j], std::min(dtw[i][j - 1], dtw[i - 1][j - 1]));
      if (dtw[i][j] > max)
        max = dtw[i][j];
    }
  }
  
  // Return the DTW distance between the sequences
  return dtw[numCoefficients][numCoefficients] / max;
}

double WaveformAnalyzer::dotProduct(const double *a, const double *b)
{
  double sum = 0.0;
  for (size_t i = 0; i < numCoefficients; ++i) {
    sum += a[i] * b[i];
  }
  return sum;
}

double WaveformAnalyzer::magnitude(const double *v)
{
  double sum = 0.0;
  for (int i = 0; i < numCoefficients; ++i)
    sum += v[i] * v[i];
  return std::sqrt(sum);
}

double WaveformAnalyzer::calculateCosineDistance(const double mfccSet1[numCoefficients], const double mfccSet2[numCoefficients])
{
  double dot = dotProduct(mfccSet1, mfccSet2);
  double mag1 = magnitude(mfccSet1);
  double mag2 = magnitude(mfccSet2);
  if (mag1 == 0.0 || mag2 == 0.0)
    return -1.0;  // Invalid input
  return (1.0 - (dot / std::sqrt(mag1 * mag2))) / 2.0;
}

double WaveformAnalyzer::calculateMSE(double input[2][numSamples])
{
  double sum = 0.0;
  for (int i = 0; i < numSamples; ++i)
  {
    double distance = std::abs(input[0][i] - input[1][i]);
    sum += distance * distance;
  }
  sum /= numSamples;
  return std::sqrt(sum);
}

void WaveformAnalyzer::calculateAmplitudeShiftRating(double samples[2][numSamples], double *avg, double *max)
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
      if (samples[0][j] > max[0])
        max[0] = std::abs(samples[0][j]);
      if (samples[1][j] > max[1])
        max[1] = std::abs(samples[1][j]);
      sum[0] += samples[0][j];
      sum[1] += samples[1][j];
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

double WaveformAnalyzer::calculateLikenessRating(double mse, double fft, double mfcc, double dtw, double cos, double avgAmp, double maxAmp)
{
  double sum = 0.0;
  sum += applyThreshold(mse, mseMin, mseMax) * mseWeight;
  sum += applyThreshold(fft, fftMin, fftMax) * fftWeight;
  sum += applyThreshold(mfcc, mfccMin, mfccMax) * mfccWeight;
  sum += applyThreshold(dtw, dtwMin, dtwMax) * dtwWeight;
  sum += applyThreshold(cos, cosMin, cosMax) * cosWeight;
  sum += applyThreshold(avgAmp, avgAmpMin, avgAmpMax) * avgAmpWeight;
  sum += applyThreshold(maxAmp, maxAmpMin, maxAmpMax) * maxAmpWeight;
  sum /= mseWeight + fftWeight + mfccWeight + dtwWeight + cosWeight + avgAmpWeight + maxAmpWeight;
  return 100.0 * (1.0 - sum);
}
