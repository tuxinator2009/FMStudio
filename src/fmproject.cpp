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

#include <QFile>
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QList>
#include <QMessageBox>
#include <QString>
#include "fmproject.h"
#include "fmsong.h"
#include "globals.h"

FMProject::FMProject()
{
  name = "Untitled";
  location = "";
  overlapPolicy = OverlapPolicy::Forbidden;
  showKeyNames = ShowKeyNames::CNotes;
  for (auto& patch : Globals::patches)
    instruments += new FMSynth::Patch(patch);
  songs += new FMSong();
  saved = true;
}

FMProject::FMProject(QString fileLocation)
{
  QFile file(fileLocation);
  QJsonDocument json;
  QJsonParseError error;
  if (!file.open(QFile::ReadOnly))
  {
    QMessageBox::critical(nullptr, "File Error", QString("Failed to open file: \"%1\"\nReason: %2").arg(fileLocation).arg(file.errorString()));
    saved = false;
    return;
  }
  json = QJsonDocument::fromJson(file.readAll(), &error);
  file.close();
  if (error.error != QJsonParseError::NoError)
  {
    QMessageBox::critical(nullptr, "JSON Error", QString("Failed to parse json file: \"%1\"\nReason: %2").arg(fileLocation).arg(error.errorString()));
    saved = false;
    return;
  }
  fromJson(json.object());
  saved = true;
  location = fileLocation;
}

FMProject::~FMProject()
{
  for (auto instrument : instruments)
    delete instrument;
  for (auto song : songs)
    delete song;
}

void FMProject::saveProject(QString fileLocation)
{
  QString saveLocation;
  if (!fileLocation.isEmpty())
    saveLocation = fileLocation;
  else if (location.isEmpty())
  {
    location = QFileDialog::getSaveFileName(nullptr, "Save Project", location, "FM Studio Project (*.fmx)");
    if (location.isEmpty())
      return;
    saveLocation = location;
  }
  QFile file(saveLocation);
  QJsonDocument json;
  QJsonObject data;
  QJsonArray array;
  data["name"] = name;
  if (overlapPolicy == OverlapPolicy::Forbidden)
    data["overlapPolicy"] = "forbidden";
  else if (overlapPolicy == OverlapPolicy::Highlight)
    data["overlapPolicy"] = "highlight";
  else if (overlapPolicy == OverlapPolicy::Allow)
    data["overlapPolicy"] = "allow";
  if (showKeyNames == ShowKeyNames::None)
    data["showKeyNames"] = "none";
  else if (showKeyNames == ShowKeyNames::CNotes)
    data["showKeyNames"] = "cNotes";
  else if (showKeyNames == ShowKeyNames::AllNotes)
    data["showKeyNames"] = "allNotes";
  for (auto instrument : instruments)
    array += Globals::patchToJson(instrument);
  data["instruments"] = array;
  array = QJsonArray();
  for (auto song : songs)
    array += song->toJson();
  data["songs"] = array;
  if (!file.open(QFile::WriteOnly))
  {
    QMessageBox::critical(nullptr, "File Error", QString("Failed to open file: \"%1\"\nReason: %2").arg(location).arg(file.errorString()));
    return;
  }
  json.setObject(data);
  file.write(json.toJson());
  file.close();
  if (fileLocation != saveLocation)
    saved = true;
}

bool FMProject::isSaved()
{
  return saved;
}

void FMProject::setSaved(bool value)
{
  saved = value;
  if (!saved)
    saveProject(Globals::homePath + "/backup.fmx");
}

QString FMProject::getName()
{
  return name;
}

void FMProject::setName(QString value)
{
  if (name != value)
  {
    saved = false;
    saveProject(Globals::homePath + "/backup.fmx");
  }
  name = value;
}

QString FMProject::getLocation()
{
  return location;
}

void FMProject::setLocation(QString value)
{
  location = value;
}

FMProject::OverlapPolicy FMProject::getOverlapPolicy()
{
  return overlapPolicy;
}

void FMProject::setOverlapPolicy(OverlapPolicy value)
{
  if (overlapPolicy != value)
    saved = false;
  overlapPolicy = value;
}

FMProject::ShowKeyNames FMProject::getShowKeyNames()
{
  return showKeyNames;
}

bool FMProject::hasOverlaps()
{
  for (auto song : songs)
  {
    if (song->hasOverlaps())
      return true;
  }
  return false;
}

void FMProject::setShowKeyNames(ShowKeyNames value)
{
  showKeyNames = value;
}

FMSong *FMProject::getSong(int id)
{
  return songs[id];
}

void FMProject::addSong(FMSong *song)
{
  songs += song;
  saved = false;
  saveProject(Globals::homePath + "/backup.fmx");
}

void FMProject::deleteSong(int id)
{
  delete songs.takeAt(id);
  saved = false;
  saveProject(Globals::homePath + "/backup.fmx");
}

int FMProject::numSongs()
{
  return songs.size();
}

FMSynth::Patch *FMProject::getInstrument(int id)
{
  return instruments[id];
}

int FMProject::indexOfInstrument(FMSynth::Patch *instrument)
{
  return instruments.indexOf(instrument);
}

void FMProject::addInstrument(FMSynth::Patch *instrument)
{
  instruments += instrument;
  saved = false;
  saveProject(Globals::homePath + "/backup.fmx");
}

void FMProject::deleteInstrument(int id)
{
  FMSynth::Patch *instrument = instruments.takeAt(id);
  for (auto song : songs)
    song->deleteInstrument(instruments[id]);
  delete instrument;
  saved = false;
  saveProject(Globals::homePath + "/backup.fmx");
}

int FMProject::numInstruments()
{
  return instruments.size();
}

void FMProject::fromJson(QJsonObject object)
{
  QJsonArray array;
  QString policy = object["overlapPolicy"].toString();
  QString keyNames = object["showKeyNames"].toString();
  name = object["name"].toString();
  if (policy == "forbidden")
    overlapPolicy = OverlapPolicy::Forbidden;
  else if (policy == "highlight")
    overlapPolicy = OverlapPolicy::Highlight;
  else if (policy == "allow")
    overlapPolicy = OverlapPolicy::Allow;
  if (keyNames == "none")
    showKeyNames = ShowKeyNames::None;
  else if (keyNames == "cNotes")
    showKeyNames = ShowKeyNames::CNotes;
  else if (keyNames == "allNotes")
    showKeyNames = ShowKeyNames::AllNotes;
  array = object["instruments"].toArray();
  for (int i = 0; i < array.size(); ++i)
    instruments += Globals::patchFromJson(array[i].toObject());
  array = object["songs"].toArray();
  for (int i = 0; i < array.size(); ++i)
    songs += new FMSong(array[i].toObject(), instruments);
}
