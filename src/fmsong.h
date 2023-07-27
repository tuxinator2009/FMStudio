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

#ifndef FMSONG_H
#define FMSONG_H

#include <QList>
#include <QString>
#include "FMSynth/Patch.h"

class QJsonObject;
class CHeaderArray;
class CHeaderObject;
class Undo;

class FMSong
{
  public:
    struct Note
    {
      Note()
      {
        offset = 0;
        midikey = 0;
        duration = 0;
        velocity = 0;
      }
      Note(int o, int m, int d, int v)
      {
        offset = o;
        midikey = m;
        duration = d;
        velocity = v;
      }
      int offset;
      int midikey;
      int duration;
      int velocity;
    };
    struct Pattern
    {
      void findOverlaps()
      {
        overlaps.clear();
        for (int i = 0; i < notes.size() - 1; ++i)
        {
          Note note1 = notes[i];
          for (int j = i + 1; j < notes.size(); ++j)
          {
            Note note2 = notes[j];
            int x1, x2;
            //notes are stored according to offset so if note2 is beyond the right edge then all subsequent notes will be too
            if (note1.offset + note1.duration < note2.offset)
              break;
            if (note1.offset < note2.offset)
              x1 = note2.offset;
            else
              x1 = note1.offset;
            if (note1.offset + note1.duration < note2.offset + note2.duration)
              x2 = note1.offset + note1.duration;
            else
              x2 = note2.offset + note2.duration;
            overlaps += Note(x1, note1.midikey, x2 - x1, note1.velocity);
            overlaps += Note(x1, note2.midikey, x2 - x1, note2.velocity);
          }
        }
      }
      void sortNotes()
      {
        std::sort(notes.begin(), notes.end(), [](const Note &a, const Note &b)
        {
          if (a.offset < b.offset)
            return true;
          return a.midikey < b.midikey;
        });
      }
      int getDuration()
      {
        duration = 0;
        for (auto note : notes)
        {
          if (note.offset + note.duration + 1 > duration)
            duration = note.offset + note.duration + 1;
        }
        if (duration == 0)
          duration = 1;
        else
          duration = duration / 128 + ((duration % 128 == 0) ? 0:1);
        return duration;
      }
      QList<Note> notes;
      QList<Note> overlaps;
      QString name;
      int duration;
      int noteSnap;
      int gridSnap;
      int gridSize;
    };
    struct Section
    {
      int offset;
      Pattern *pattern;
      FMSynth::Patch *instrument;
    };
    FMSong();
    FMSong(QJsonObject json, const QList<FMSynth::Patch*> &instruments);
    ~FMSong();
    QJsonObject toJson();
    void exportSong(QString file);
    Pattern *getPattern(int id);
    void addPattern(Pattern *pattern);
    void deletePattern(int id);
    int numPatterns();
    int getPatternMaxDuration(int id);
    bool hasOverlaps();
    Section *getSection(int channel, int id);
    void addSection(Section *section, int channel);
    void deleteSection(int channel, int id);
    int numSections(int channel);
    void changeChannel(Section *section, int oldChannel, int newChannel);
    void sortChannel(int channel);
    QString getName();
    void setName(QString value);
    int getTempo();
    void setTempo(int value);
    uint32_t getDuration();
    uint32_t getLength();
    Undo *getUndo();
    void deleteInstrument(FMSynth::Patch *instrument);
  private:
    Undo *undo;
    QList<Pattern*> patterns;
    QList<Section*> sections[4];
    QString name;
    int tempo;
};

#endif //FMSONG_H
