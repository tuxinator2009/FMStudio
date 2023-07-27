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

#ifndef SONGEDITOR_H
#define SONGEDITOR_H

#include <QAudioOutput>
#include <QWidget>
#include "fmsong.h"

class FMSong;

class SongEditor : public QWidget
{
  Q_OBJECT
  public:
    SongEditor(QWidget *parent=nullptr);
    ~SongEditor();
    void setSong(FMSong *value);
    void setAudioPlaying(QAudioOutput *value, int tempo);
  signals:
    void updateSongLength(uint32_t length);
    void playbackFinished();
  public slots:
    void setPattern(int value);
    void setInstrument(int value);
    void setSnapAmount(int value);
    void setPlaybackPosition(uint32_t value);
    void setAutoScroll(bool value);
  private:
    void leaveEvent(QEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);
    static const int noteOffsets[12];
    QAudioOutput *audio;
    FMSong *song;
    FMSong::Section *currentSection;
    QPoint lastPos;
    uint32_t playbackPosition;
    int currentOffset;
    int currentChannel;
    int currentPattern;
    int currentInstrument;
    int oldChannel;
    int xOffset;
    int snap;
    int playbackTempo;
    bool showSection;
    bool movingSection;
    bool autoScroll;
};

#endif //SONGEDITOR_H
