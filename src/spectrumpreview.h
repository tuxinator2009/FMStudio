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

#ifndef SPECTRUMPREVIEW_H
#define SPECTRUMPREVIEW_H

#include <QWidget>
#include <complex>

class SpectrumPreview : public QWidget
{
  Q_OBJECT
  public:
    struct LikenessScore
    {
      double mse;
      double fft;
      double mfcc;
      double dtw;
      double cos;
      double avgAmp;
      double maxAmp;
      double likeness;
    };
    SpectrumPreview(QWidget *parent=nullptr);
    ~SpectrumPreview();
    void setWaveformData(const uint8_t *data, int id);
    void setShowSecondSpectrum(bool value);
    LikenessScore getLikenessRating();
  signals:
    void rangeChanged(int min, int max);
    void hOffsetChanged(int value);
  protected slots:
    void setHOffset(int value);
    void setZoomLevel(int value);
  private:
    void normalizeSpectrum(std::complex<double> *in, double *out);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);
    QPoint lastPos;
    double waveforms[2][8192];
    double spectrum[2][4097];
    int zoom;
    int hOffset;
    bool showSecondSpectrum;
    bool showCursorPoint;
};

#endif //WAVEFORMPREVIEW_H
