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

#ifndef GLOBALS_H
#define GLOBALS_H

#include "FMSynth/Patch.h"

class QAudioOutput;
class QJsonObject;
class QMenu;
class QObject;
class QWidget;
class CHeaderObject;
class FMProject;

namespace Globals
{
  struct RecentProject
  {
    QString name;
    QString location;
    bool operator==(const RecentProject &other) const {return location == other.location;}
  };
  void init();
  void loadSettings();
  void saveSettings();
  void loadInstruments();
  QMenu *loadRecentProjects(QWidget *parent);
  void saveRecentProjects();
  void addRecentProject(QString name, QString location);
  QAudioOutput *createAudioOutput(QWidget *parent=nullptr);
  QString patchToCHeader(const FMSynth::Patch &patch);
  FMSynth::Patch patchFromCHeader(const CHeaderObject &data);
  FMSynth::Patch *patchFromJson(const QJsonObject &json);
  QJsonObject patchToJson(const FMSynth::Patch *patch);
  bool isWhiteKey(int midikey);
  extern QList<RecentProject> recentProjects;
  extern QList<FMSynth::Patch> patches;
  extern QString appPath;
  extern QString homePath;
  extern QString backupProjectLocation;
  extern FMProject *project;
  extern QRect geometry;
  extern int maxVolume;
  extern bool firstTimeAudio;
  extern const char *patchCHeaderTemplate;
  static constexpr int NOTE_HEIGHT = 16;
  constexpr bool whiteKey[12] = {true, false, true, true, false, true, false, true, true, false, true, false};
};

#endif //GLOBALS_H
