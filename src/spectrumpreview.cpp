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

#include <QMouseEvent>
#include <QPainter>
#include <QToolTip>
#include <QWidget>
#include <complex>
#include <cmath>
#include "spectrumpreview.h"
#include "waveformanalyzer.h"

SpectrumPreview::SpectrumPreview(QWidget *parent) : QWidget(parent)
{
  for (int i = 0; i < 8192; ++i)
  {
    waveforms[0][i] = 0.0;
    waveforms[1][i] = 0.0;
  }
  for (int i = 0; i < 4097; ++i)
  {
    spectrum[0][i] = 0.0;
    spectrum[1][i] = 0.0;
  }
  zoom = 0;
  hOffset = 0;
  showSecondSpectrum = false;
  showCursorPoint = false;
}

SpectrumPreview::~SpectrumPreview()
{
}

void SpectrumPreview::setWaveformData(const uint8_t *data, int id)
{
  std::complex<double> input[WaveformAnalyzer::numSamples];
  for (int i = 0; i < WaveformAnalyzer::sampleRate; ++i)
    waveforms[id][i] = data[i] / 128.0 - 1.0;
  for (int i = WaveformAnalyzer::sampleRate; i < WaveformAnalyzer::numSamples; ++i)
    waveforms[id][i] = 0.0;
  WaveformAnalyzer::normalize(waveforms[id], WaveformAnalyzer::numSamples);
  for (int i = 0; i < WaveformAnalyzer::numSamples; ++i)
    input[i] = waveforms[id][i];
  WaveformAnalyzer::fft(input);
  normalizeSpectrum(input, spectrum[id]);
  if (id == 0 || showSecondSpectrum)
    update();
}

void SpectrumPreview::setShowSecondSpectrum(bool value)
{
  showSecondSpectrum = value;
  update();
  if (!showSecondSpectrum)
    return;
}

SpectrumPreview::LikenessScore SpectrumPreview::getLikenessRating()
{
  LikenessScore score;
  double mfccs[2][WaveformAnalyzer::numCoefficients];
  score.mse = WaveformAnalyzer::calculateMSE(waveforms);
  score.fft = WaveformAnalyzer::calculateRMSE(spectrum);
  WaveformAnalyzer::calculateMFCCs(spectrum[0], mfccs[0]);
  WaveformAnalyzer::calculateMFCCs(spectrum[1], mfccs[1]);
  score.mfcc = WaveformAnalyzer::calculateEuclideanDistance(mfccs[0], mfccs[1]);
  score.dtw = WaveformAnalyzer::calculateDTW(mfccs[0], mfccs[1]);
  score.cos = WaveformAnalyzer::calculateCosineDistance(mfccs[0], mfccs[1]);
  WaveformAnalyzer::calculateAmplitudeShiftRating(waveforms, &score.avgAmp, &score.maxAmp);
  score.likeness = WaveformAnalyzer::calculateLikenessRating(score.mse, score.fft, score.mfcc, score.dtw, score.cos, score.avgAmp, score.maxAmp);
  return score;
}

void SpectrumPreview::setHOffset(int value)
{
  int scaledWidth = (4097 - width()) * zoom / 100 + width();
  if (value < 0)
    value = 0;
  if (value + width() > scaledWidth)
    value = scaledWidth - width();
  if (value == hOffset)
    return;
  hOffset = value;
  emit hOffsetChanged(hOffset);
  update();
}

void SpectrumPreview::setZoomLevel(int value)
{
  int scaledWidth = (4097 - width()) * value / 100 + width();
  zoom = value;
  emit rangeChanged(0, scaledWidth - width());
  if (hOffset + width() > scaledWidth)
  {
    hOffset = scaledWidth - width();
    emit hOffsetChanged(hOffset);
  }
  update();
}

void SpectrumPreview::normalizeSpectrum(std::complex<double> *in, double *out)
{
  double max = 0.0;
  for (int i = 0; i < 4097; ++i)
  {
    out[i] = std::abs(in[i]);
    if (out[i] > max)
      max = out[i];
  }
  for (int i = 0; i < 4097; ++i)
    out[i] /= max;
}

void SpectrumPreview::mousePressEvent(QMouseEvent *event)
{
  int scaledWidth = (4097 - width()) * zoom / 100 + width();
  lastPos = event->pos();
  if (event->button() == Qt::LeftButton)
  {
    QString text;
    int index = (event->pos().x() + hOffset) * 4097 / scaledWidth;
    double freq = index * 8000.0 / 8192.0;
    text = QString("%1Hz: %2").arg(freq, 0, 'f', 2).arg(spectrum[0][index], 0, 'f', 3);
    if (showSecondSpectrum)
      text += QString(" - %1").arg(spectrum[1][index], 0, 'f', 3);
    QToolTip::showText(mapToGlobal(event->pos()), text, this);
    showCursorPoint = true;
    update();
  }
  else if (event->button() == Qt::MiddleButton)
    setCursor(Qt::ClosedHandCursor);
}

void SpectrumPreview::mouseMoveEvent(QMouseEvent *event)
{
  int scaledWidth = (4097 - width()) * zoom / 100 + width();
  if (event->buttons() & Qt::LeftButton)
  {
    QString text;
    int index = (event->pos().x() + hOffset) * 4097 / scaledWidth;
    double freq = index * 8000.0 / 8192.0;
    text = QString("%1Hz: %2").arg(freq, 0, 'f', 2).arg(spectrum[0][index], 0, 'f', 3);
    if (showSecondSpectrum)
      text += QString(" - %1").arg(spectrum[1][index], 0, 'f', 3);
    QToolTip::showText(mapToGlobal(event->pos()), text, this);
    lastPos = event->pos();
    update();
  }
  else if (event->buttons() & Qt::MiddleButton)
  {
    hOffset -= event->pos().x() - lastPos.x();
    if (hOffset < 0)
      hOffset = 0;
    if (hOffset + width() > scaledWidth)
      hOffset = scaledWidth - width();
    emit hOffsetChanged(hOffset);
    update();
    lastPos = event->pos();
  }
}

void SpectrumPreview::mouseReleaseEvent(QMouseEvent *event)
{
  Q_UNUSED(event);
  setCursor(Qt::ArrowCursor);
  showCursorPoint = false;
  update();
}

void SpectrumPreview::paintEvent(QPaintEvent *event)
{
  Q_UNUSED(event);
  QPainter painter;
  int scaledWidth = (4097 - width()) * zoom / 100 + width();
  painter.begin(this);
  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.fillRect(0, 0, width(), height(), QColor(192, 192, 192));
  painter.scale(scaledWidth / 4097.0, 1.0);
  painter.translate(-hOffset, 0);
  painter.setPen(QColor(0, 0, 255));
  for (int i = 0; i < 4097; ++i)
    painter.drawLine(QLineF(i, height(), i, height() * (1.0 - spectrum[0][i])));
  if (showSecondSpectrum)
  {
    painter.setPen(QColor(255, 0, 0));
    painter.setOpacity(0.75);
    for (int i = 0; i < 4097; ++i)
      painter.drawLine(QLineF(i, height(), i, height() * (1.0 - spectrum[1][i])));
  }
  if (showCursorPoint)
  {
    int index = (lastPos.x() + hOffset) * 4097 / scaledWidth;
    painter.setPen(QPen(QColor(0, 0, 255), 2.0));
    painter.setBrush(Qt::NoBrush);
    painter.setOpacity(1.0);
    painter.drawEllipse(QRectF(index - 3, height() * (1.0 - spectrum[0][index]) - 3, 7, 7));
    painter.setPen(QPen(QColor(255, 0, 0), 2.0));
    if (showSecondSpectrum)
      painter.drawEllipse(QRectF(index - 3, height() * (1.0 - spectrum[1][index]) - 3, 7, 7));
  }
  painter.end();
}
