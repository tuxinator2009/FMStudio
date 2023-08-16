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

#ifndef WAVEFORMANALYZER_H
#define WAVEFORMANALYZER_H

#include <cstdlib>
#include <cstdint>
#include <complex>
#include <initializer_list>
#include <cmath>

namespace WaveformAnalyzer
{
  constexpr int numSamples = 8192;
  constexpr int sampleRate = 8000;
  constexpr int numMelFilters = 26;  // Number of Mel filters
  constexpr int numCoefficients = 13;  // Number of MFCC coefficients to keep
  constexpr int numFrequencies = numSamples / 2 + 1;//4097
  constexpr double mseMin = 0.20;
  constexpr double mseMax = 0.5;
  constexpr double mseWeight = 7.0;
  constexpr double fftMin = 0.09;
  constexpr double fftMax = 0.15;
  constexpr double fftWeight = 5.0;
  constexpr double mfccMin = 0.2;
  constexpr double mfccMax = 0.5;
  constexpr double mfccWeight = 2.0;
  constexpr double dtwMin = 0.15;
  constexpr double dtwMax = 0.30;
  constexpr double dtwWeight = 8.0;
  constexpr double cosMin = 0.1;
  constexpr double cosMax = 0.5;
  constexpr double cosWeight = 4.0;
  constexpr double avgAmpMin = 0.03;
  constexpr double avgAmpMax = 0.1;
  constexpr double avgAmpWeight = 9.0;
  constexpr double maxAmpMin = 0.1;
  constexpr double maxAmpMax = 0.4;
  constexpr double maxAmpWeight = 10.0;
  constexpr double baseFrequency = 261.6;
  constexpr int window = (sampleRate / baseFrequency) * 1.5;
  constexpr int windowStep = (sampleRate / baseFrequency) / 2.0;
  void fft(std::complex<double> data[numSamples]);
  void normalize(std::complex<double> *data, int size);
  void normalize(double *data, int size);
  double calculateRMSE(double data[2][numFrequencies]);
  void fillMelFilterbank(double melFilterbank[numMelFilters][numFrequencies]);
  void applyDCT(const double *input, double *output);
  void calculateMFCCs(const double fftMagnitudes[numFrequencies], double mfccs[numCoefficients]);
  double calculateEuclideanDistance(const double mfccSet1[numCoefficients], const double mfccSet2[numCoefficients]);
  double calculateDTW(const double mfccSet1[numCoefficients], const double mfccSet2[numCoefficients]);
  double dotProduct(const double *a, const double *b);
  double magnitude(const double *v);
  double calculateCosineDistance(const double mfccSet1[numCoefficients], const double mfccSet2[numCoefficients]);
  double calculateMSE(double samples[2][numSamples]);
  void calculateAmplitudeShiftRating(double samples[2][numSamples], double *avg, double *max);
  double calculateLikenessRating(double mse, double fft, double mfcc, double dtw, double cos, double avgAmp, double maxAmp);
  inline double euclideanDistance(double value1, double value2) {return (value2 - value1) * (value2 - value1);}
  inline double applyThreshold(double value, double min, double max) {return std::max(std::min((value - min) / (max - min), 1.0), 0.0);}
};

#endif //WAVEFORMANALYZER_H
