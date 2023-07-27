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

#include <QWidget>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include "fmproject.h"
#include "globals.h"
#include "virtualpiano.h"

VirtualPiano::VirtualPiano(QWidget *parent) : QWidget(parent)
{
  activeKey = 0;
}

VirtualPiano::~VirtualPiano()
{
}

int VirtualPiano::getMidikey(int y)
{
  for (auto key : blackKeys)
  {
    if (y >= key.rect.y() && y < key.rect.y() + key.rect.height())
      return key.midikey;
  }
  for (auto key : whiteKeys)
  {
    if (y >= key.rect.y() && y < key.rect.y() + key.rect.height())
      return key.midikey;
  }
  return -1;
}

void VirtualPiano::scrollKeyboard(QWheelEvent *event)
{
  wheelEvent(event);
}

void VirtualPiano::mouseMoveEvent(QMouseEvent *event)
{
  if (event->buttons() & Qt::LeftButton)
  {
    uint8_t midikey = 0;
    for (int i = 0; i < blackKeys.count() && midikey == 0; ++i)
    {
      if (blackKeys[i].rect.contains(event->pos()))
        midikey = blackKeys[i].midikey;
    }
    for (int i = 0; i < whiteKeys.count() && midikey == 0; ++i)
    {
      if (whiteKeys[i].rect.contains(event->pos()))
        midikey = whiteKeys[i].midikey;
    }
    if (midikey != 0 && activeKey != midikey)
    {
      activeKey = midikey;
      update();
      emit notePressed(midikey);
    }
  }
}

void VirtualPiano::mousePressEvent(QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton)
  {
    uint8_t midikey = 0;
    for (int i = 0; i < blackKeys.count() && midikey == 0; ++i)
    {
      if (blackKeys[i].rect.contains(event->pos()))
        midikey = blackKeys[i].midikey;
    }
    for (int i = 0; i < whiteKeys.count() && midikey == 0; ++i)
    {
      if (whiteKeys[i].rect.contains(event->pos()))
        midikey = whiteKeys[i].midikey;
    }
    if (midikey != 0)
    {
      activeKey = midikey;
      update();
      emit notePressed(midikey);
    }
  }
}

void VirtualPiano::mouseReleaseEvent(QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton)
  {
    activeKey = 0;
    update();
    emit noteReleased();
  }
}

void VirtualPiano::paintEvent(QPaintEvent *event)
{
  Q_UNUSED(event);
  QPainter painter;
  int whiteKey = 0;
  int octave = 0;
  if (whiteKeys.isEmpty())
    initializeKeys();
  painter.begin(this);
  painter.setFont(font());
  painter.fillRect(0, 0, width(), height(), QColor(255, 255, 255));
  painter.setPen(QColor(0, 0, 0));
  for (auto key : whiteKeys)
  {
    if (key.midikey == activeKey)
      painter.fillRect(key.rect, QColor(255, 0, 0));
    else if (key.midikey % 6 == 0)
      painter.fillRect(key.rect, QColor(192, 192, 192));
    painter.drawLine(key.rect.left(), key.rect.bottom(), key.rect.right(), key.rect.bottom());
    painter.drawLine(key.rect.right(), key.rect.top(), key.rect.right(), key.rect.bottom());
    if ((Globals::project->getShowKeyNames() == FMProject::ShowKeyNames::CNotes && whiteKey % 7 == 2) || Globals::project->getShowKeyNames() == FMProject::ShowKeyNames::AllNotes)
    {
      QPoint pos;
      char letter = 'A' + (whiteKey % 7);
      if (whiteKey % 7 == 2)
        ++octave;
      if (width() > height()) //horizontal
        pos = QPoint(whiteKey * 16, key.rect.bottom() - 16);
      else //vertical
        pos = QPoint(key.rect.right() - 16, 832 - whiteKey * 16 - 16);
      painter.drawText(pos.x(), pos.y(), 16, 16, Qt::AlignLeft | Qt::AlignVCenter, QString("%1%2").arg(QChar(letter)).arg(octave));
    }
    ++whiteKey;
  }
  for (auto key : blackKeys)
    painter.fillRect(key.rect, QColor((key.midikey == activeKey) ? 255:0, 0, 0));
  painter.end();
}

void VirtualPiano::initializeKeys()
{
  int barHeight = Globals::NOTE_HEIGHT / 2 + 1;
  int midikey = 21;
  if (width() > height()) //horizontal
  {
    int barOffset = (Globals::NOTE_HEIGHT * 2 - barHeight) / 2;
    for (int i = 0; i < 52; ++i)
    {
      Key whiteKey;
      whiteKey.midikey = midikey++;
      whiteKey.rect = QRect(i * Globals::NOTE_HEIGHT, 0, Globals::NOTE_HEIGHT, Globals::NOTE_HEIGHT * 4);
      whiteKeys += whiteKey;
      if (i % 7 != 1 && i % 7 != 4 && i != 51)
      {
        Key blackKey;
        blackKey.midikey = midikey++;
        blackKey.rect = QRect(i * Globals::NOTE_HEIGHT + barOffset, 0, barHeight, Globals::NOTE_HEIGHT * 2);
        blackKeys += blackKey;
      }
    }
  }
  else //vertical
  {
    int barOffset = barHeight / 2;
    for (int i = 0; i < 52; ++i)
    {
      Key whiteKey;
      whiteKey.midikey = midikey++;
      whiteKey.rect = QRect(0, height() - (i + 1) * Globals::NOTE_HEIGHT, Globals::NOTE_HEIGHT * 4, Globals::NOTE_HEIGHT);
      whiteKeys += whiteKey;
      if (i % 7 != 1 && i % 7 != 4 && i != 51)
      {
        Key blackKey;
        blackKey.midikey = midikey++;
        blackKey.rect = QRect(0, height() - (i + 1) * Globals::NOTE_HEIGHT - barOffset, Globals::NOTE_HEIGHT * 2, barHeight);
        blackKeys += blackKey;
      }
    }
  }
}
