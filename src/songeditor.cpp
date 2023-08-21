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

#include <QAudioOutput>
#include <QMouseEvent>
#include <QPainter>
#include <QWidget>
#include "FMSynth/Patch.h"
#include "fmproject.h"
#include "fmsong.h"
#include "globals.h"
#include "songeditor.h"
#include "undo.h"

const int SongEditor::noteOffsets[12] = {0, 0, 1, 2, 2, 3, 3, 4, 5, 5, 6, 6};

SongEditor::SongEditor(QWidget *parent) : QWidget(parent)
{
  audio = nullptr;
  song = nullptr;
  playbackPosition = 0;
  xOffset = 0;
  snap = 128;
  currentOffset = 0;
  currentChannel = 0;
  currentPattern = 0;
  currentInstrument = 0;
  currentSection = nullptr;
  showSection = false;
  movingSection = false;
  autoScroll = true;
}

SongEditor::~SongEditor()
{
}

void SongEditor::setAudioPlaying(QAudioOutput *value, int tempo)
{
  audio = value;
  playbackTempo = tempo;
  if (audio != nullptr)
    update();
}

void SongEditor::setSong(FMSong *value)
{
  song = value;
  update();
}

void SongEditor::setPattern(int value)
{
  currentPattern = value;
  update();
}

void SongEditor::setInstrument(int value)
{
  currentInstrument = value;
  update();
}

void SongEditor::setSnapAmount(int value)
{
  snap = 1 << value;
}

void SongEditor::setPlaybackPosition(uint32_t value)
{
  playbackPosition = value;
  if (autoScroll)
  {
    if ((int)playbackPosition < xOffset)
      xOffset = playbackPosition;
    else if ((int)playbackPosition - xOffset > width() / 2)
      xOffset = playbackPosition - width() / 2;
  }
  if (audio != nullptr)
    update();
}

void SongEditor::setAutoScroll(bool value)
{
  autoScroll = value;
}

void SongEditor::leaveEvent(QEvent *event)
{
  Q_UNUSED(event);
  showSection = false;
  update();
}

void SongEditor::mouseDoubleClickEvent(QMouseEvent *event)
{
  if (event->button() != Qt::LeftButton)
    return;
  int channel = event->pos().y() / 64;
  int x = event->pos().x() + xOffset;
  for (int i = 0; i < song->numSections(channel); ++i)
  {
    FMSong::Section *section = song->getSection(channel, i);
    if (x >= section->offset && x <= section->offset + section->pattern->duration * 128)
    {
      //Undo::ChangeInstrument *undoStep = new Undo::ChangeInstrument;
      //undoStep->channel = channel;
      //undoStep->section = i;
      //undoStep->oldInstrument = section->instrument;
      //song->getUndo()->addUndoStep(Undo::Type::ChangeInstrument, (void*)undoStep);
      section->instrument = Globals::project->getInstrument(currentInstrument);
      song->getPattern(currentPattern)->lastInstrument = section->instrument;
      Globals::project->setSaved(false);
      update();
      break;
    }
  }
}

void SongEditor::mousePressEvent(QMouseEvent *event)
{
  int x = event->pos().x() + xOffset;
  lastPos = event->pos();
  if (event->button() == Qt::LeftButton) //Place new section or grab section
  {
    int channel = event->pos().y() / 64;
    movingSection = false;
    for (int i = 0; i < song->numSections(channel) && !movingSection; ++i)
    {
      FMSong::Section *section = song->getSection(channel, i);
      if (x >= section->offset && x < section->offset + section->pattern->duration * 128)
      {
        currentOffset = (x - section->offset) / snap * snap;
        currentSection = section;
        oldChannel = channel;
        movingSection = true;
      }
    }
    if (!movingSection)
    {
      FMSong::Pattern *pattern = song->getPattern(currentPattern);
      FMSong::Section *section = new FMSong::Section;
      section->offset = currentOffset;
      section->pattern = pattern;
      section->instrument = Globals::project->getInstrument(currentInstrument);
      song->addSection(section, currentChannel);
      pattern->lastInstrument = section->instrument;
      Globals::project->setSaved(false);
      update();
      emit updateSongLength(song->getLength());
    }
  }
  else if (event->button() == Qt::RightButton) //Delete section if possible
  {
    int channel = event->pos().y() / 64;
    for (int i = 0; i < song->numSections(channel); ++i)
    {
      FMSong::Section *section = song->getSection(channel, i);
      if (x >= section->offset && x < section->offset + section->pattern->duration * 128)
      {
        song->deleteSection(channel, i);
        Globals::project->setSaved(false);
        update();
        emit updateSongLength(song->getLength());
        break;
      }
    }
  }
}

void SongEditor::mouseMoveEvent(QMouseEvent *event)
{
  int x = event->pos().x() + xOffset;
  int newOffset = x / snap * snap;
  int newChannel = event->pos().y() / 64;
  bool newShowSection = true;
  for (int i = 0; i < song->numSections(newChannel) && !movingSection; ++i)
  {
    FMSong::Section *section = song->getSection(newChannel, i);
    if (newOffset + song->getPattern(currentPattern)->duration * 128 <= section->offset)
      continue;
    else if (newOffset >= section->offset + section->pattern->duration * 128)
      continue;
    newShowSection = false;
    break;
  }
  if (event->buttons() == Qt::LeftButton && movingSection) //Drag section
  {
    bool canMove = true;
    newOffset -= currentOffset;
    if (newOffset == currentSection->offset && newChannel == currentChannel)
      return;
    for (int i = 0; i < song->numSections(newChannel) && canMove; ++i)
    {
      FMSong::Section *section = song->getSection(newChannel, i);
      if (section == currentSection)
        continue;
      else if (newOffset >= section->offset + section->pattern->duration * 128)
        continue;
      else if (newOffset + currentSection->pattern->duration * 128 <= section->offset) //Sections are sorted by offset meaning all subsequent sections would also be beyond the end of the current section
        break;
      canMove = false;
    }
    if (canMove)
    {
      Globals::project->setSaved(false);
      currentSection->offset = newOffset;
      if (newChannel != oldChannel)
      {
        song->changeChannel(currentSection, oldChannel, newChannel);
        oldChannel = newChannel;
      }
      update();
    }
  }
  else if (event->buttons() == Qt::MiddleButton) //Pan view
  {
    xOffset -= event->pos().x() - lastPos.x();
    if (xOffset < 0)
      xOffset = 0;
    update();
    lastPos = event->pos();
  }
  else if (event->buttons() == Qt::RightButton) //Delete sections
  {
    int channel = event->pos().y() / 64;
    for (int i = 0; i < song->numSections(channel); ++i)
    {
      FMSong::Section *section = song->getSection(channel, i);
      if (x >= section->offset && x < section->offset + section->pattern->duration * 128)
      {
        song->deleteSection(channel, i);
        Globals::project->setSaved(false);
        update();
        emit updateSongLength(song->getLength());
        break;
      }
    }
  }
  else if (newOffset != currentOffset || newChannel != currentChannel || newShowSection != showSection)
  {
    currentOffset = newOffset;
    currentChannel = newChannel;
    showSection = newShowSection;
    update();
  }
}

void SongEditor::mouseReleaseEvent(QMouseEvent *event)
{
  Q_UNUSED(event);
  if (movingSection)
  {
    song->sortChannel(oldChannel);
    emit updateSongLength(song->getLength());
  }
  movingSection = false;
}

void SongEditor::paintEvent(QPaintEvent *event) //FIXME: render song sections
{
  Q_UNUSED(event);
  QPainter painter;
  int maxX = xOffset + width();
  painter.begin(this);
  painter.setFont(font());
  painter.fillRect(0, 0, width(), height(), QColor(61, 56, 70));
  for (int i = 0; i < 4; ++i)
    painter.fillRect(0, i * 64 + 63, width(), 1, QColor(0, 0, 0));
  for (int x = xOffset / 32 * 32; x < maxX; x += 32)
  {
    if ((x / 32) % 4 == 3)
      painter.fillRect(x - xOffset + 31, 0, 1, height(), QColor(128, 128, 128));
    else
      painter.fillRect(x - xOffset + 31, 0, 1, height(), QColor(0, 0, 0));
  }
  if (song == nullptr)
    return;
  for (int channel = 0; channel < 4; ++channel)
  {
    for (int i = 0; i < song->numSections(channel); ++i)
    {
      FMSong::Section *section = song->getSection(channel, i);
      if (section->offset + section->pattern->duration * 128 < xOffset)
        continue;
      else if (section->offset >= maxX) //sections are sorted by their offset so all subsequent sections will also be offscreen
        break;
      for (auto note : section->pattern->notes)
      {
        int octave = (note.midikey - 21) / 12;
        int semitone = (note.midikey - 21) % 12;
        int noteX = section->offset + note.offset - xOffset;
        int noteY = 62 - octave * 7 - noteOffsets[semitone] + channel * 64;
        if (noteX + note.duration + 1 < 0)
          continue;
        else if (noteX >= width()) //notes are sorted by their offset so all subsequent notes will also be offscreen
          break;
        painter.setPen(QColor(0, 255, 0));
        painter.drawLine(noteX, noteY, noteX + note.duration, noteY);
        painter.setPen(QColor(0, 128, 0));
        painter.drawPoint(noteX + note.duration, noteY);
      }
      if (Globals::project->getOverlapPolicy() == FMProject::OverlapPolicy::Highlight)
      {
        painter.setPen(QColor(255, 0, 0));
        painter.setOpacity(0.75);
        for (auto note : section->pattern->overlaps)
        {
          int octave = (note.midikey - 21) / 12;
          int semitone = (note.midikey - 21) % 12;
          int noteX = section->offset + note.offset - xOffset;
          int noteY = 62 - octave * 7 - noteOffsets[semitone] + channel * 64;
          if (noteX + note.duration + 1 < 0)
            continue;
          else if (noteX >= width()) //notes are sorted by their offset so all subsequent notes will also be offscreen
            break;
          painter.drawLine(noteX, noteY, noteX + note.duration, noteY);
        }
      }
      painter.setOpacity(1.0);
      painter.setPen(QColor(255, 255, 255));
      painter.setBrush(Qt::NoBrush);
      painter.drawRect(section->offset - xOffset, channel * 64, section->pattern->duration * 128 - 1, 63);
      painter.fillRect(section->offset - xOffset, channel * 64, section->pattern->duration * 128, 11, QColor(255, 255, 255));
      painter.setPen(QColor(0, 0, 0));
      painter.drawText(section->offset - xOffset + 1, channel * 64 + 1, section->pattern->duration * 128 - 2, 9, Qt::AlignLeft | Qt::AlignVCenter, QString("%1 - %2").arg(section->instrument->name).arg(section->pattern->name));
    }
  }
  if (showSection)
  {
    FMSong::Pattern *pattern = song->getPattern(currentPattern);
    FMSynth::Patch *instrument = Globals::project->getInstrument(currentInstrument);
    painter.setOpacity(0.5);
    for (auto note : pattern->notes)
    {
      int octave = (note.midikey - 21) / 12;
      int semitone = (note.midikey - 21) % 12;
      int noteX = currentOffset + note.offset - xOffset;
      int noteY = 62 - octave * 7 - noteOffsets[semitone] + currentChannel * 64;
      if (noteX + note.duration + 1 >= 0 && noteX < width())
      {
        painter.setPen(QColor(0, 255, 0));
        painter.drawLine(noteX, noteY, noteX + note.duration, noteY);
        painter.setPen(QColor(0, 128, 0));
        painter.drawPoint(noteX + note.duration, noteY);
      }
    }
    if (Globals::project->getOverlapPolicy() == FMProject::OverlapPolicy::Highlight)
    {
      painter.setOpacity(0.5);
      painter.setPen(QColor(255, 0, 0));
      for (auto note : pattern->overlaps)
      {
        int octave = (note.midikey - 21) / 12;
        int semitone = (note.midikey - 21) % 12;
        int noteX = currentOffset + note.offset - xOffset;
        int noteY = 62 - octave * 7 - noteOffsets[semitone] + currentChannel * 64;
        if (noteX + note.duration + 1 >= 0 && noteX < width())
          painter.drawLine(noteX, noteY, noteX + note.duration, noteY);
      }
    }
    painter.setPen(QColor(255, 255, 255));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(currentOffset - xOffset, currentChannel * 64, pattern->duration * 128 - 1, 63);
    painter.fillRect(currentOffset - xOffset, currentChannel * 64, pattern->duration * 128, 11, QColor(255, 255, 255));
    painter.setPen(QColor(0, 0, 0));
    painter.drawText(currentOffset - xOffset + 1, currentChannel * 64 + 1, pattern->duration * 128 - 2, 9, Qt::AlignLeft | Qt::AlignVCenter, QString("%1 - %2").arg(instrument->name).arg(pattern->name));
    painter.setOpacity(1.0);
  }
  if (audio != nullptr)
  {
    uint32_t msecs = audio->elapsedUSecs() / 1000;
    setPlaybackPosition(msecs * playbackTempo * 32 / 60000);
    if (playbackPosition >= song->getDuration())
      emit playbackFinished();
    painter.fillRect(playbackPosition - xOffset, 0, 1, height(), QColor(0, 0, 255));
  }
  painter.end();
}
