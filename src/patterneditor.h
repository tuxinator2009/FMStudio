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

#ifndef PATTERNEDITOR_H
#define PATTERNEDITOR_H

#include <QAudioOutput>
#include <QWidget>
#include "fmsong.h"

class VirtualPiano;

class PatternEditor : public QWidget
{
  Q_OBJECT
  public:
    PatternEditor(QWidget *parent=nullptr);
    ~PatternEditor();
    void setVirtualKeyboard(VirtualPiano *value);
    void setAudioPlaying(QAudioOutput *value, int tempo);
  signals:
    void patternChanged();
    void scrollKeyboard(int value);
    void noteDurationChanged(int value);
    void playbackFinished();
  public slots:
    void setHScrollOffset(int value);
    void setVScrollOffset(int value);
    void setVScrollRange(int min, int max);
    void increaseNoteDuration(int amount);
    void decreaseNoteDuration(int amount);
    void setNoteSnap(int value);
    void setGridSnap(int value);
    void setGridSize(int value);
    void setNoteDuration(int value);
    void setSong(FMSong *value);
    void setPattern(FMSong::Pattern *value);
    void setMaxDuration(int value);
    void setPlaybackPosition(int value);
    void setAutoScroll(bool value);
  private:
    void findGhostNoteOverlaps();
    int noteAt(int x, int midikey, int w);
    void leaveEvent(QEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);
    void wheelEvent(QWheelEvent *event);
    QAudioOutput *audio;
    VirtualPiano *keyboard;
    FMSong *song;
    FMSong::Pattern *pattern;
    FMSong::Note ghostNote;
    QList<FMSong::Note> overlaps;
    QPoint lastPos;
    int hScrollOffset;
    int vScrollOffset;
    int vScrollMin;
    int vScrollMax;
    int maxDuration;
    int noteDuration;
    int noteSnap;
    int gridSnap;
    int gridSize;
    int wheelAngleDelta;
    int selectedNote;
    int playbackPosition;
    int playbackTempo;
    bool ghostNoteVisible;
    bool resizingNote;
    bool movingNote;
    bool adjustingVelocity;
    bool autoScroll;
};

#endif //PATTERNEDITOR_H
