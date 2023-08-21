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

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include "CHeaderParser/cheaderparser.h"
#include "fmproject.h"
#include "fmsong.h"
#include "globals.h"
#include "undo.h"

FMSong::FMSong()
{
  Pattern *pattern = new Pattern;
  name = "Untitled";
  tempo = 120;
  pattern->name = "Pattern 1";
  pattern->lastInstrument = nullptr;
  pattern->duration = 1;
  pattern->noteSnap = 5;
  pattern->gridSnap = 5;
  pattern->gridSize = 5;
  patterns += pattern;
  undo = new Undo();
}

FMSong::FMSong(QJsonObject json, const QList<FMSynth::Patch*> &instruments)
{
  QJsonObject obj;
  QJsonArray array;
  name = json["name"].toString();
  tempo = json["tempo"].toInt();
  array = json["patterns"].toArray();
  for (int i = 0; i < array.size(); ++i)
  {
    QJsonArray notes;
    Pattern *pattern = new Pattern;
    obj = array[i].toObject();
    pattern->name = obj["name"].toString();
    if (obj.contains("lastInstrument"))
      pattern->lastInstrument = instruments[obj["lastInstrument"].toInt()];
    else 
      pattern->lastInstrument = nullptr;
    pattern->noteSnap = obj["noteSnap"].toInt();
    pattern->gridSnap = obj["gridSnap"].toInt();
    pattern->gridSize = obj["gridSize"].toInt();
    pattern->duration = 0;
    notes = obj["notes"].toArray();
    for (int j = 0; j < notes.size(); ++j)
    {
      QJsonArray noteArray = notes[j].toArray();
      Note note;
      note.offset = noteArray[0].toInt();
      note.midikey = noteArray[1].toInt();
      note.duration = noteArray[2].toInt();
      note.velocity = noteArray[3].toInt();
      pattern->notes += note;
      if (pattern->duration < note.offset + note.duration + 1)
        pattern->duration = note.offset + note.duration + 1;
    }
    if (pattern->duration == 0)
      pattern->duration = 1;
    else
      pattern->duration = pattern->duration / 128 + ((pattern->duration % 128 == 0) ? 0:1);
    pattern->findOverlaps();
    patterns += pattern;
  }
  array = json["sections"].toArray();
  for (int i = 0; i < array.size(); ++i)
  {
    QJsonArray channel = array[i].toArray();
    for (int j = 0; j < channel.size(); ++j)
    {
      QJsonArray sectionArray = channel[j].toArray();
      Section *section = new Section;
      section->offset = sectionArray[0].toInt();
      section->pattern = patterns[sectionArray[1].toInt()];
      section->instrument = instruments[sectionArray[2].toInt()];
      //section->pattern = sectionArray[1].toInt();
      //section->instrument = sectionArray[2].toInt();
      sections[i] += section;
    }
  }
  undo = new Undo();
}

FMSong::~FMSong()
{
  for (auto pattern : patterns)
    delete pattern;
  for (int i = 0; i < 4; ++i)
  {
    for (auto section : sections[i])
      delete section;
  }
  delete undo;
}

QJsonObject FMSong::toJson()
{
  QJsonObject json;
  QJsonObject obj;
  QJsonArray array;
  json["name"] = name;
  json["tempo"] = tempo;
  for (auto pattern : patterns)
  {
    QJsonArray notesArray;
    obj = QJsonObject();
    obj["name"] = pattern->name;
    if (pattern->lastInstrument != nullptr)
      obj["lastInstrument"] = Globals::project->indexOfInstrument(pattern->lastInstrument);
    obj["noteSnap"] = pattern->noteSnap;
    obj["gridSnap"] = pattern->gridSnap;
    obj["gridSize"] = pattern->gridSize;
    for (auto note : pattern->notes)
    {
      QJsonArray noteArray;
      noteArray += note.offset;
      noteArray += note.midikey;
      noteArray += note.duration;
      noteArray += note.velocity;
      notesArray += noteArray;
    }
    obj["notes"] = notesArray;
    array += obj;
  }
  json["patterns"] = array;
  array = QJsonArray();
  for (int i = 0; i < 4; ++i)
  {
    QJsonArray channelArray;
    for (auto section : sections[i])
    {
      QJsonArray sectionArray;
      sectionArray += section->offset;
      sectionArray += patterns.indexOf(section->pattern);
      sectionArray += Globals::project->indexOfInstrument(section->instrument);
      //sectionArray += section->pattern;
      //sectionArray += section->instrument;
      channelArray += sectionArray;
    }
    array += channelArray;
  }
  json["sections"] = array;
  return json;
}

void FMSong::exportSong(QString file)
{
  //TODO
}

FMSong::Pattern *FMSong::getPattern(int id)
{
  return patterns[id];
}

void FMSong::addPattern(Pattern *pattern)
{
  patterns += pattern;
}

void FMSong::deletePattern(int id)
{
  Pattern *pattern = patterns.takeAt(id);
  for (int channel = 0; channel < 4; ++channel)
  {
    for (int i = sections[channel].size() - 1; i >= 0; --i)
    {
      if (sections[channel][i]->pattern == pattern)
        delete sections[channel].takeAt(i);
    }
  }
  delete pattern;
}

int FMSong::numPatterns()
{
  return patterns.count();
}

int FMSong::getPatternMaxDuration(int id)
{
  int maxDuration = 1 << 16;
  for (int channel = 0; channel < 4; ++channel)
  {
    for (int i = 0; i < sections[channel].size() - 1; ++i)
    {
      Section *section = sections[channel][i];
      int maxSectionDuration;
      if (section->pattern != patterns[id])
        continue;
      maxSectionDuration = sections[channel][i + 1]->offset - section->offset;
      if (maxSectionDuration < maxDuration)
        maxDuration = maxSectionDuration;
    }
  }
  return maxDuration / 128;
}

bool FMSong::hasOverlaps()
{
  for (auto pattern : patterns)
  {
    if (pattern->overlaps.size() > 0)
      return true;
  }
  return false;
}

FMSong::Section *FMSong::getSection(int channel, int id)
{
  return sections[channel][id];
}

void FMSong::addSection(Section *section, int channel)
{
  for (int i = 0; i < sections[channel].size(); ++i)
  {
    if (sections[channel][i]->offset > section->offset)
    {
      sections[channel].insert(i, section);
      return;
    }
  }
  sections[channel] += section;
}

void FMSong::deleteSection(int channel, int id)
{
  sections[channel].removeAt(id);
}

int FMSong::numSections(int channel)
{
  return sections[channel].count();
}

void FMSong::changeChannel(Section *section, int oldChannel, int newChannel)
{
  sections[oldChannel].removeAll(section);
  addSection(section, newChannel);
}

void FMSong::sortChannel(int channel)
{
  std::sort(sections[channel].begin(), sections[channel].end(), [](Section *a, Section *b){return a->offset < b->offset;});
}

QString FMSong::getName()
{
  return name;
}

void FMSong::setName(QString value)
{
  name = value;
}

int FMSong::getTempo()
{
  return tempo;
}

void FMSong::setTempo(int value)
{
  tempo = value;
}

uint32_t FMSong::getDuration()
{
  uint32_t maxDuration = 0;
  for (int i = 0; i < 4; ++i)
  {
    if (sections[i].size() == 0)
      continue;
    uint32_t duration = sections[i].last()->offset + sections[i].last()->pattern->duration * 128;
    if (duration > maxDuration)
      maxDuration = duration;
  }
  return maxDuration;
}

uint32_t FMSong::getLength()
{
  return ((getDuration() * 60) / 32) / tempo;
}

Undo *FMSong::getUndo()
{
  return undo;
}

void FMSong::deleteInstrument(FMSynth::Patch *instrument)
{
  FMSynth::Patch *newInstrument = Globals::project->getInstrument(0);
  for (auto pattern : patterns)
  {
    if (pattern->lastInstrument == instrument)
      pattern->lastInstrument = nullptr;
  }
  for (int channel = 0; channel < 4; ++channel)
  {
    for (auto section : sections[channel])
    {
      if (section->instrument == instrument)
      section->instrument = newInstrument;
    }
  }
}
