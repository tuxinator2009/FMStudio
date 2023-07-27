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

#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QToolTip>
#include <QWheelEvent>
#include <QWidget>
#include "fmproject.h"
#include "fmsong.h"
#include "globals.h"
#include "patterneditor.h"
#include "virtualpiano.h"

PatternEditor::PatternEditor(QWidget *parent) : QWidget(parent)
{
  audio = nullptr;
  hScrollOffset = 0;
  vScrollOffset = 0;
  noteDuration = 31;
  noteSnap = 32;
  gridSnap = 32;
  gridSize = 128;
  wheelAngleDelta = 0;
  selectedNote = -1;
  playbackPosition = 0;
  ghostNoteVisible = false;
  resizingNote = false;
  movingNote = false;
  autoScroll = true;
}

PatternEditor::~PatternEditor()
{
}

void PatternEditor::setVirtualKeyboard(VirtualPiano *value)
{
  keyboard = value;
}

void PatternEditor::setAudioPlaying(QAudioOutput *value, int tempo)
{
  audio = value;
  playbackTempo = tempo;
  if (audio != nullptr)
    update();
}

void PatternEditor::setHScrollOffset(int value)
{
  hScrollOffset = value;
  update();
}

void PatternEditor::setVScrollOffset(int value)
{
  vScrollOffset = value;
  update();
}

void PatternEditor::setVScrollRange(int min, int max)
{
  vScrollMin = min;
  vScrollMax = max;
}

void PatternEditor::increaseNoteDuration(int amount)
{
  if (255 - noteDuration >= amount)
    noteDuration += amount;
  ghostNote.duration = noteDuration;
  update();
}

void PatternEditor::decreaseNoteDuration(int amount)
{
  if (noteDuration >= amount)
    noteDuration -= amount;
  ghostNote.duration = noteDuration;
  update();
}

void PatternEditor::setNoteSnap(int value)
{
  noteSnap = 1 << value;
  pattern->noteSnap = value;
}

void PatternEditor::setGridSnap(int value)
{
  gridSnap = 1 << value;
  pattern->gridSnap = value;
}

void PatternEditor::setGridSize(int value)
{
  gridSize = (1 << value) * 4;
  update();
  pattern->gridSize = value;
}

void PatternEditor::setNoteDuration(int value)
{
  noteDuration = value;
  if (ghostNoteVisible && !movingNote)
  {
    ghostNote.duration = noteDuration;
    findGhostNoteOverlaps();
  }
  update();
}

void PatternEditor::setSong(FMSong *value)
{
  song = value;
}

void PatternEditor::setPattern(FMSong::Pattern *value)
{
  pattern = value;
  update();
}

void PatternEditor::setMaxDuration(int value)
{
  maxDuration = value;
  update();
}

void PatternEditor::setPlaybackPosition(int value)
{
  playbackPosition = value;
  if (autoScroll)
  {
    if (playbackPosition < hScrollOffset)
      hScrollOffset = playbackPosition;
    else if (playbackPosition - hScrollOffset > width() / 2)
      hScrollOffset = playbackPosition - width() / 2;
  }
  if (audio != nullptr)
    update();
}

void PatternEditor::setAutoScroll(bool value)
{
  autoScroll = value;
}

void PatternEditor::findGhostNoteOverlaps()
{
  overlaps.clear();
  for (auto note2 : pattern->notes)
  {
    int x1, x2;
    //notes are stored according to offset so if note2 is beyond the right edge then all subsequent notes will be too
    if (ghostNote.offset + ghostNote.duration < note2.offset)
      break;
    else if (note2.offset + note2.duration < ghostNote.offset)
      continue;
    if (ghostNote.offset < note2.offset)
      x1 = note2.offset;
    else
      x1 = ghostNote.offset;
    if (ghostNote.offset + ghostNote.duration < note2.offset + note2.duration)
      x2 = ghostNote.offset + ghostNote.duration;
    else
      x2 = note2.offset + note2.duration;
    overlaps += FMSong::Note(x1, ghostNote.midikey, x2 - x1, ghostNote.velocity);
    overlaps += FMSong::Note(x1, note2.midikey, x2 - x1, note2.velocity);
  }
}

int PatternEditor::noteAt(int x, int midikey, int w)
{
  for (int i = 0; i < pattern->notes.size(); ++i)
  {
    if (x >= (pattern->notes[i].offset + pattern->notes[i].duration + 1) * 4)
      continue;
    else if (x + w < pattern->notes[i].offset * 4)
      break;
    else if (midikey == pattern->notes[i].midikey)
      return i;
  }
  return -1;
}

void PatternEditor::leaveEvent(QEvent *event)
{
  Q_UNUSED(event);
  ghostNoteVisible = false;
  update();
}

void PatternEditor::mousePressEvent(QMouseEvent *event)
{
  int x = event->pos().x() + hScrollOffset;
  int y = event->pos().y() + vScrollOffset;
  int midikey = keyboard->getMidikey(y);
  lastPos = event->pos();
  if (event->button() == Qt::LeftButton)
  {
    if (event->pos().y() >= height() - 128)
    {
      selectedNote = -1;
      for (int i = 0; i < pattern->notes.length(); ++i)
      {
        int noteX = pattern->notes[i].offset * 4;
        int noteY = height() - pattern->notes[i].velocity;
        if (x >= noteX && x < noteX + 4 && event->pos().y() >= noteY && event->pos().y() < noteY + 4)
        {
          selectedNote = i;
          adjustingVelocity = true;
          break;
        }
      }
    }
    else
    {
      int note = noteAt(x, midikey, ghostNote.duration);
      if (note == -1)
      {
        if ((overlaps.size() == 0 || Globals::project->getOverlapPolicy() != FMProject::OverlapPolicy::Forbidden) && ghostNoteVisible)
        {
          bool foundOne = false;
          for (int i = 0; i < pattern->notes.length(); ++i)
          {
            if (ghostNote.offset < pattern->notes[i].offset)
            {
              pattern->notes.insert(i, ghostNote);
              foundOne = true;
              break;
            }
          }
          if (!foundOne)
            pattern->notes += ghostNote;
          pattern->findOverlaps();
          pattern->getDuration();
          overlaps.clear();
          ghostNoteVisible = false;
          Globals::project->setSaved(false);
          emit patternChanged();
          update();
        }
      }
      else
      {
        selectedNote = noteAt(x, midikey, 1);
        noteDuration = pattern->notes[selectedNote].duration;
        emit noteDurationChanged(noteDuration);
        if (x >= (pattern->notes[selectedNote].offset + pattern->notes[selectedNote].duration + 1) * 4 - 1)
        {
          lastPos = QPoint(x, y);
          resizingNote = true;
          ghostNoteVisible = false;
        }
        else if (x >= pattern->notes[selectedNote].offset * 4 && x < (pattern->notes[selectedNote].offset + pattern->notes[selectedNote].duration + 1) * 4)
        {
          ghostNote = pattern->notes.takeAt(selectedNote);
          movingNote = true;
          lastPos.setX(ghostNote.offset - (x / 4) / gridSnap * gridSnap);
          pattern->findOverlaps();
          findGhostNoteOverlaps();
          ghostNoteVisible = true;
          update();
        }
      }
    }
  }
  else if (event->button() == Qt::RightButton) //Delete notes
  {
    int note = noteAt(x, midikey, 1);
    if (note != -1)
    {
      pattern->notes.removeAt(note);
      pattern->findOverlaps();
      pattern->getDuration();
      Globals::project->setSaved(false);
      emit patternChanged();
      update();
    }
    setCursor(Qt::ArrowCursor);
    ghostNoteVisible = false;
  }
}

void PatternEditor::mouseMoveEvent(QMouseEvent *event)
{
  int x = event->pos().x() + hScrollOffset;
  int y = event->pos().y() + vScrollOffset;
  int midikey = keyboard->getMidikey(y);
  if (event->buttons() == Qt::LeftButton) //Drag and/or Resize note
  {
    if (resizingNote)
    {
      int oldDuration = pattern->notes[selectedNote].duration;
      int diff = ((x - lastPos.x()) / 4) / noteSnap * noteSnap;
      noteDuration = pattern->notes[selectedNote].duration + diff;
      while (noteDuration < 0)
        noteDuration += noteSnap;
      while (noteDuration > 255)
        noteDuration -= noteSnap;
      noteDuration = pattern->notes[selectedNote].duration;
      emit noteDurationChanged(noteDuration);
      pattern->notes[selectedNote].duration = noteDuration;
      pattern->findOverlaps();
      pattern->getDuration();
      if (pattern->overlaps.size() > 0 && Globals::project->getOverlapPolicy() == FMProject::OverlapPolicy::Forbidden)
        pattern->notes[selectedNote].duration = oldDuration;
      else if (pattern->getDuration() > maxDuration * 128)
        pattern->notes[selectedNote].duration = oldDuration;
      if (pattern->notes[selectedNote].duration != oldDuration)
      {
        Globals::project->setSaved(false);
        lastPos = QPoint(x, y);
        update();
      }
    }
    else if (movingNote)
    {
      FMSong::Note oldNote = ghostNote;
      int note;
      ghostNote.offset = (x / 4) / gridSnap * gridSnap + lastPos.x();
      ghostNote.midikey = midikey;
      findGhostNoteOverlaps();
      note = noteAt(ghostNote.offset * 4, ghostNote.midikey, noteDuration * 4);
      if (note != -1)
        ghostNote = oldNote;
      else if (overlaps.size() > 0 && Globals::project->getOverlapPolicy() == FMProject::OverlapPolicy::Forbidden)
        ghostNote = oldNote;
      else if (ghostNote.offset + ghostNote.duration + 1 > maxDuration * 128)
        ghostNote = oldNote;
      if (ghostNote.offset != oldNote.offset || ghostNote.midikey != oldNote.midikey)
        update();
    }
    else if (adjustingVelocity)
    {
      pattern->notes[selectedNote].velocity = height() - event->pos().y();
      if (pattern->notes[selectedNote].velocity < 0)
        pattern->notes[selectedNote].velocity = 0;
      else if (pattern->notes[selectedNote].velocity > 127)
        pattern->notes[selectedNote].velocity = 127;
      QToolTip::showText(mapToGlobal(QPoint(event->pos().x() + 16, height() - pattern->notes[selectedNote].velocity)), QString::number(pattern->notes[selectedNote].velocity), this);
      update();
    }
  }
  else if (event->buttons() == Qt::MiddleButton) //Pan area
  {
    QPoint delta = event->pos() - lastPos;
    hScrollOffset -= delta.x();
    vScrollOffset -= delta.y();
    if (hScrollOffset < 0)
      hScrollOffset = 0;
    if (vScrollOffset < vScrollMin)
      vScrollOffset = vScrollMin;
    else if (vScrollOffset > vScrollMax)
      vScrollOffset = vScrollMax;
    emit scrollKeyboard(vScrollOffset);
    update();
    ghostNoteVisible = false;
    lastPos = event->pos();
  }
  else if (event->buttons() == Qt::RightButton) //Delete notes
  {
    int note = noteAt(x, midikey, 1);
    if (note != -1)
    {
      pattern->notes.removeAt(note);
      pattern->findOverlaps();
      pattern->getDuration();
      Globals::project->setSaved(false);
      emit patternChanged();
      update();
    }
    ghostNoteVisible = false;
  }
  else if (event->buttons() == Qt::NoButton || movingNote) //Show note placement
  {
    if (event->pos().y() < height() - 128)
    {
      int note;
      ghostNote.offset = (x / 4) / gridSnap * gridSnap;
      ghostNote.midikey = midikey;
      ghostNote.duration = noteDuration;
      ghostNote.velocity = 127;
      note = noteAt(ghostNote.offset * 4, ghostNote.midikey, noteDuration * 4);
      ghostNoteVisible = note == -1;
      if (ghostNote.offset + ghostNote.duration + 1 > maxDuration * 128)
        ghostNoteVisible = false;
      if (note == -1)
      {
        setCursor(Qt::ArrowCursor);
        findGhostNoteOverlaps();
      }
      else
      {
        note = noteAt(x, midikey, 1);
        if (note != -1)
        {
          if (x >= (pattern->notes[note].offset + pattern->notes[note].duration + 1) * 4 - 1)
            setCursor(Qt::SizeHorCursor);
          else if (x > pattern->notes[note].offset * 4 && x < (pattern->notes[note].offset + pattern->notes[note].duration + 1) * 4)
            setCursor(Qt::OpenHandCursor);
        }
        else
          setCursor(Qt::ArrowCursor);
      }
      update();
    }
    else
    {
      ghostNoteVisible = false;
      update();
    }
  }
}

void PatternEditor::mouseReleaseEvent(QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton && movingNote)
  {
    bool foundOne = false;
    for (int i = 0; i < pattern->notes.length(); ++i)
    {
      if (ghostNote.offset < pattern->notes[i].offset)
      {
        pattern->notes.insert(i, ghostNote);
        foundOne = true;
        break;
      }
    }
    if (!foundOne)
      pattern->notes += ghostNote;
    pattern->findOverlaps();
    pattern->getDuration();
    overlaps.clear();
    ghostNoteVisible = false;
    Globals::project->setSaved(false);
    emit patternChanged();
    update();
  }
  setCursor(Qt::ArrowCursor);
  resizingNote = false;
  movingNote = false;
  adjustingVelocity = false;
}

void PatternEditor::paintEvent(QPaintEvent *event)
{
  Q_UNUSED(event);
  QPainter painter;
  painter.begin(this);
  painter.fillRect(0, 0, width(), height(), QColor(61, 56, 70));
  for (int x = hScrollOffset / gridSize * gridSize; x < hScrollOffset + width(); x += gridSize)
  {
    if ((x / gridSize) % (512 / gridSize) == 0)
      painter.fillRect(x - hScrollOffset, 0, 1, height() - 128, QColor(128, 128, 128));
    else
      painter.fillRect(x - hScrollOffset, 0, 1, height() - 128, QColor(0, 0, 0));
  }
  for (int i = 0; i < 52; ++i)
  {
    if (i * 16 + 15 - vScrollOffset < height() - 128)
      painter.fillRect(0, i * 16 + 15 - vScrollOffset, width(), 1, QColor(0, 0, 0));
  }
  for (int y = 0; y < 128; y += 16)
    painter.fillRect(0, height() - y - 1, width(), 1, QColor(0, 0, 0));
  painter.fillRect(0, height() - 128, width(), 1, QColor(128, 128, 128));
  painter.fillRect(maxDuration * 512 - hScrollOffset, 0, 1, height() - 128, QColor(255, 0, 0));
  for (auto note : pattern->notes)
  {
    int duration = note.duration + 1;
    int octave = note.midikey / 12 - 1;
    int semitone = note.midikey % 12;
    int noteX = note.offset * 4 - hScrollOffset;
    int noteY = octave * Globals::NOTE_HEIGHT * 7 + semitone * (Globals::NOTE_HEIGHT / 2);
    if (noteX + duration * 4 < 0) //note is not in view
      continue;
    if (noteX >= width()) //notes are sorted by offset so all subsequent notes will also be beyond the widget's view
      break;
    if (semitone > 4)
      noteY += Globals::NOTE_HEIGHT / 2;
    noteY = (56 * Globals::NOTE_HEIGHT) - noteY - vScrollOffset;
    painter.setClipRect(0, 0, width(), height() - 128);
    painter.fillRect(noteX, noteY, duration * 4, Globals::NOTE_HEIGHT - 1, QColor(0, 128, 0));
    painter.fillRect(noteX, noteY, duration * 4 - 1, Globals::NOTE_HEIGHT - 2, QColor(0, 255, 0));
    painter.fillRect(noteX + 1, noteY + 1, duration * 4 - 2, Globals::NOTE_HEIGHT - 3, QColor(0, 192, 0));
    painter.setClipRect(0, height() - 128, width(), 128);
    painter.fillRect(noteX, height() - note.velocity, 1, note.velocity, QColor(0, 192, 0));
    painter.fillRect(noteX, height() - note.velocity, duration * 3, 1, QColor(0, 192, 0));
    painter.fillRect(noteX, height() - note.velocity, 4, 4, QColor(0, 255, 0));
  }
  painter.setClipRect(0, 0, width(), height() - 128);
  if (Globals::project->getOverlapPolicy() == FMProject::OverlapPolicy::Highlight)
  {
    painter.setOpacity(0.75);
    for (auto note : pattern->overlaps)
    {
      int duration = note.duration + 1;
      int octave = note.midikey / 12 - 1;
      int semitone = note.midikey % 12;
      int noteX = note.offset * 4 - hScrollOffset;
      int noteY = octave * Globals::NOTE_HEIGHT * 7 + semitone * (Globals::NOTE_HEIGHT / 2);
      if (noteX + duration * 4 < 0) //note is not in view
        continue;
      if (noteX >= width()) //notes are sorted by offset so all subsequent notes will also be beyond the widget's view
        break;
      if (semitone > 4)
        noteY += Globals::NOTE_HEIGHT / 2;
      noteY = (56 * Globals::NOTE_HEIGHT) - noteY - vScrollOffset;
      painter.fillRect(noteX, noteY, duration * 4, Globals::NOTE_HEIGHT - 1, QColor(255, 0, 0));
    }
    painter.setOpacity(1.0);
  }
  if (ghostNoteVisible)
  {
    int duration = ghostNote.duration + 1;
    int octave = ghostNote.midikey / 12 - 1;
    int semitone = ghostNote.midikey % 12;
    int noteX = ghostNote.offset * 4 - hScrollOffset;
    int noteY = octave * Globals::NOTE_HEIGHT * 7 + semitone * (Globals::NOTE_HEIGHT / 2);
    if (semitone > 4)
      noteY += Globals::NOTE_HEIGHT / 2;
    noteY = (56 * Globals::NOTE_HEIGHT) - noteY - vScrollOffset;
    painter.setOpacity(0.5);
    painter.fillRect(noteX, noteY, duration * 4, Globals::NOTE_HEIGHT - 1, QColor(0, 128, 0));
    painter.fillRect(noteX, noteY, duration * 4 - 1, Globals::NOTE_HEIGHT - 2, QColor(0, 255, 0));
    painter.fillRect(noteX + 1, noteY + 1, duration * 4 - 2, Globals::NOTE_HEIGHT - 3, QColor(0, 192, 0));
    if (Globals::project->getOverlapPolicy() != FMProject::OverlapPolicy::Allow)
    {
      painter.setOpacity(0.75);
      for (auto note : overlaps)
      {
        int duration = note.duration + 1;
        int octave = note.midikey / 12 - 1;
        int semitone = note.midikey % 12;
        int noteX = note.offset * 4 - hScrollOffset;
        int noteY = octave * Globals::NOTE_HEIGHT * 7 + semitone * (Globals::NOTE_HEIGHT / 2);
        if (noteX + duration * 4 < 0) //note is not in view
          continue;
        if (noteX >= width()) //notes are sorted by offset so all subsequent notes will also be beyond the widget's view
          break;
        if (semitone > 4)
          noteY += Globals::NOTE_HEIGHT / 2;
        noteY = (56 * Globals::NOTE_HEIGHT) - noteY - vScrollOffset;
        painter.fillRect(noteX, noteY, duration * 4, Globals::NOTE_HEIGHT - 1, QColor(255, 0, 0));
      }
      painter.setOpacity(1.0);
    }
  }
  if (audio != nullptr)
  {
    uint32_t msecs = audio->elapsedUSecs() / 1000;
    setPlaybackPosition(msecs * playbackTempo * 128 / 60000);
    if (playbackPosition >= pattern->duration * 512)
      emit playbackFinished();
  }
  painter.fillRect(playbackPosition - hScrollOffset, 0, 1, height(), QColor(0, 0, 255));
  painter.end();
}

void PatternEditor::wheelEvent(QWheelEvent *event)
{
  event->accept();
  wheelAngleDelta += event->angleDelta().y();
  vScrollOffset -= wheelAngleDelta / 5;
  wheelAngleDelta %= 5;
  if (vScrollOffset < 0)
    vScrollOffset = 0;
  else if (vScrollOffset + height() - 128 > Globals::NOTE_HEIGHT * 52)
    vScrollOffset = Globals::NOTE_HEIGHT * 52 - (height() - 128);
  emit scrollKeyboard(vScrollOffset);
}
