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

#ifndef FMPROJECT_H
#define FMPROJECT_H

#include <QList>
#include "FMSynth/Patch.h"
#include "fmsong.h"

class FMProject
{
  public:
    enum class OverlapPolicy
    {
      Forbidden=0,
      Highlight=1,
      Allow=2
    };
    enum class ShowKeyNames
    {
      None=0,
      CNotes=1,
      AllNotes=2
    };
    FMProject();
    FMProject(QString fileLocation);
    ~FMProject();
    void saveProject(QString fileLocation=QString());
    bool isSaved();
    void setSaved(bool value);
    QString getName();
    void setName(QString value);
    QString getLocation();
    void setLocation(QString value);
    OverlapPolicy getOverlapPolicy();
    void setOverlapPolicy(OverlapPolicy value);
    bool hasOverlaps();
    ShowKeyNames getShowKeyNames();
    void setShowKeyNames(ShowKeyNames value);
    FMSong *getSong(int id);
    void addSong(FMSong *song);
    void deleteSong(int id);
    int numSongs();
    FMSynth::Patch *getInstrument(int id);
    int indexOfInstrument(FMSynth::Patch *instrument);
    void addInstrument(FMSynth::Patch *instrument);
    void deleteInstrument(int id);
    int numInstruments();
  private:
    void fromJson(QJsonObject object);
    QList<FMSong*> songs;
    QList<FMSynth::Patch*> instruments;
    QString name;
    QString location;
    OverlapPolicy overlapPolicy;
    ShowKeyNames showKeyNames;
    bool saved;
};

#endif //FMPROJECT_H
