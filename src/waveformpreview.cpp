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

#include <QBitmap>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QWidget>
#include "waveformpreview.h"

WaveformPreview::WaveformPreview(QWidget *parent) : QWidget(parent)
{
  zoom = 0;
  hOffset = 0;
  for (int i = 0; i < 8000; ++i)
  {
    waveform[0][i] = 0;
    waveform[1][i] = 0;
  }
  showSecondWaveform = false;
}

WaveformPreview::~WaveformPreview()
{
}

void WaveformPreview::setWaveformData(const uint8_t *data, int id)
{
  for (int i = 0; i < 8000; ++i)
    waveform[id][i] = data[i];
  if (id == 0 || showSecondWaveform)
    update();
}

const uint8_t *WaveformPreview::getWaveformData(int id)
{
  return waveform[id];
}

void WaveformPreview::setShowSecondWaveform(bool value)
{
  showSecondWaveform = value;
  update();
}

void WaveformPreview::setHOffset(int value)
{
  int scaledWidth = 7600 * zoom / 100 + 400;
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

void WaveformPreview::setZoomLevel(int value)
{
  int scaledWidth = 7600 * value / 100 + 400;
  zoom = value;
  emit rangeChanged(0, scaledWidth - width());
  if (hOffset + width() > scaledWidth)
  {
    hOffset = scaledWidth - width();
    emit hOffsetChanged(hOffset);
  }
  update();
}

void WaveformPreview::mousePressEvent(QMouseEvent *event)
{
  setCursor(Qt::ClosedHandCursor);
  lastPos = event->pos();
}

void WaveformPreview::mouseMoveEvent(QMouseEvent *event)
{
  if (event->buttons() == Qt::NoButton)
    return;
  int scaledWidth = 7600 * zoom / 100 + 400;
  hOffset -= event->pos().x() - lastPos.x();
  if (hOffset < 0)
    hOffset = 0;
  if (hOffset + width() > scaledWidth)
    hOffset = scaledWidth - width();
  emit hOffsetChanged(hOffset);
  lastPos = event->pos();
  update();
}

void WaveformPreview::mouseReleaseEvent(QMouseEvent *event)
{
  Q_UNUSED(event);
  setCursor(Qt::ArrowCursor);
}

void WaveformPreview::paintEvent(QPaintEvent *event)
{
  Q_UNUSED(event);
  QPainter painter;
  painter.begin(this);
  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.fillRect(0, 0, width(), height(), QColor(192, 192, 192));
  painter.setPen(QColor(64, 64, 64));
  painter.drawLine(0, height() / 2, width(), height() / 2);
  painter.scale((7600.0 * zoom / 100.0 + 400.0) / 8000.0, 0.5);
  painter.translate(-hOffset, 0);
  painter.setPen(QColor(0, 0, 255));
  for (int i = 0; i < 7999; ++i)
    painter.drawLine(i, 255 - waveform[0][i], i + 1, 255 - waveform[0][i + 1]);
  if (showSecondWaveform)
  {
    painter.setPen(QColor(255, 0, 0));
    painter.setOpacity(0.75);
    for (int i = 0; i < 7999; ++i)
      painter.drawLine(i, 255 - waveform[1][i], i + 1, 255 - waveform[1][i + 1]);
  }
  painter.end();
}
