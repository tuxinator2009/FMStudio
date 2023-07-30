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

#include <QAudioFormat>
#include <QAudioOutput>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QMap>
#include <QMenu>
#include <QMessageBox>
#include <QRegularExpression>
#include <QDir>
#include <QFile>
#include <QString>
#include <QTextStream>
#include "CHeaderParser/cheaderparser.h"
#include "fmproject.h"
#include "globals.h"

QList<Globals::RecentProject> Globals::recentProjects;
QList<FMSynth::Patch> Globals::patches;
QString Globals::appPath;
FMProject *Globals::project;
const char *Globals::patchCHeaderTemplate = 
  "#pragma once\n"
  "\n"
  "FMSynth::Patch patch_%1 =\n"
  "{\n"
  "  .name=\"%2\",\n"
  "  .algorithm=%3, .volume=%4, .feedback=%5, .glide=%6, .attack=%7, .decay=%8, .sustain=%9, .release=%10,\n"
  "  .lfo={.speed=%11, .attack=%12, .pmd=%13},\n"
  "  .op=\n"
  "  {\n"
  "    {.level=%14, .pitch={.fixed=%15, .coarse=%16, .fine=%17}, .detune=%18, .attack=%19, .decay=%20, .sustain=%21, .loop=%22},\n"
  "    {.level=%23, .pitch={.fixed=%24, .coarse=%25, .fine=%26}, .detune=%27, .attack=%28, .decay=%29, .sustain=%30, .loop=%31},\n"
  "    {.level=%32, .pitch={.fixed=%33, .coarse=%34, .fine=%35}, .detune=%36, .attack=%37, .decay=%38, .sustain=%39, .loop=%40},\n"
  "    {.level=%41, .pitch={.fixed=%42, .coarse=%43, .fine=%44}, .detune=%45, .attack=%46, .decay=%47, .sustain=%48, .loop=%49}\n"
  "  }\n"
  "};\n";
bool Globals::firstTimeAudio = true;

void Globals::loadInstruments()
{
#ifdef Q_OS_MACOS
  QDir dir(QCoreApplication::applicationDirPath());
  while (!dir.dirName().endsWith(".app", Qt::CaseInsensitive))
    dir.cdUp();
  appPath = dir.absolutePath();
  dir.cd("Contents");
  dir.cd("Resources");
  dir.cd("instruments");
#else
  QDir dir(QString("%1/instruments").arg(QCoreApplication::applicationDirPath()));
  appPath = QCoreApplication::applicationDirPath();
#endif
  printf("Looking for instruments in: \"%s\"\n", dir.absolutePath().toLocal8Bit().data());
  QStringList files = dir.entryList(QStringList() << "*.h", QDir::Files, QDir::Name);
  for (auto filename : files)
  {
    QFile file(dir.absoluteFilePath(filename));
    QTextStream stream(&file);
    file.open(QFile::ReadOnly|QFile::Text);
    patches += patchFromCHeader(CHeaderParser::parseCHeader(stream.readAll()).toObject());
    file.close();
  }
  if (files.size() == 0)
  {
    FMSynth::Patch patch;
    patch.name[0] = 'B';
    patch.name[1] = 'L';
    patch.name[2] = 'A';
    patch.name[3] = 'N';
    patch.name[4] = 'K';
    patch.name[5] = '\0';
    patch.algorithm = 1;
    patch.volume = 100;
    patch.feedback = 0;
    patch.glide = 0;
    patch.attack = 0;
    patch.decay = 0;
    patch.sustain = 0;
    patch.release = 0;
    patch.lfo.speed = 0;
    patch.lfo.attack = 0;
    patch.lfo.pmd = 0;
    for (int i = 0; i < 4; ++i)
    {
      patch.op[i].level = 0;
      patch.op[i].pitch.fixed = false;
      patch.op[i].pitch.coarse = 0;
      patch.op[i].pitch.fine = 0;
      patch.op[i].detune = 0;
      patch.op[i].attack = 0;
      patch.op[i].decay = 0;
      patch.op[i].sustain = 0;
      patch.op[i].loop = false;
    }
    patches += patch;
  }
}

QMenu *Globals::loadRecentProjects(QWidget *parent)
{
  QFile file(QString("%1/.fmstudio.txt").arg(QDir::homePath()));
  QTextStream stream(&file);
  QMenu *menu = new QMenu(parent);
  if (file.open(QFile::ReadOnly | QFile::Text))
  {
    while (!stream.atEnd())
    {
      RecentProject project;
      project.name = stream.readLine();
      project.location = stream.readLine();
      recentProjects += project;
      menu->addAction(project.name)->setData(project.location);
    }
  }
  file.close();
  if (recentProjects.size() == 0)
  {
    delete menu;
    return nullptr;
  }
  return menu;
}

void Globals::saveRecentProjects()
{
  QFile file(QString("%1/.fmstudio.txt").arg(QDir::homePath()));
  QTextStream stream(&file);
  file.open(QFile::WriteOnly | QFile::Text);
  while (recentProjects.size() > 10)
    recentProjects.removeLast();
  for (auto project : recentProjects)
    stream << project.name << "\n" << project.location << "\n";
  file.close();
}

void Globals::addRecentProject(QString name, QString location)
{
  RecentProject project;
  project.name = name;
  project.location = location;
  recentProjects.removeAll(project);
  recentProjects.push_front(project);
}

QAudioOutput *Globals::createAudioOutput(QWidget *parent)
{
  QAudioFormat audioFormat;
  QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
  audioFormat.setChannelCount(1);
  audioFormat.setCodec("audio/pcm");
  audioFormat.setSampleType(QAudioFormat::UnSignedInt);
  audioFormat.setSampleRate(8000);
  audioFormat.setSampleSize(8);
  if (!info.isFormatSupported(audioFormat) && firstTimeAudio)
    QMessageBox::critical(parent, "Audio Error", "Raw audio format not supported by backend, cannot play audio.");
  firstTimeAudio = false;
  return new QAudioOutput(audioFormat, parent);
}

QString Globals::patchToCHeader(const FMSynth::Patch &patch)
{
  QString text = patchCHeaderTemplate;
  QString varName = patch.name;
  varName.remove(QChar(' '));
  varName.replace(QRegularExpression("[^a-zA-Z0-9_]"), "_");
  text = text.arg(varName);
  text = text.arg(patch.name);
  text = text.arg(patch.algorithm).arg(patch.volume).arg(patch.feedback).arg(patch.glide).arg(patch.attack).arg(patch.decay).arg(patch.sustain).arg(patch.release);
  text = text.arg(patch.lfo.speed).arg(patch.lfo.attack).arg(patch.lfo.pmd);
  for (int i = 0; i < 4; ++i)
  {
    text = text.arg(patch.op[i].level);
    text = text.arg(patch.op[i].pitch.fixed ? "true":"false").arg(patch.op[i].pitch.coarse).arg(patch.op[i].pitch.fine);
    text = text.arg(patch.op[i].detune).arg(patch.op[i].attack).arg(patch.op[i].decay).arg(patch.op[i].sustain).arg(patch.op[i].loop ? "true":"false");
  }
  return text;
}

FMSynth::Patch Globals::patchFromCHeader(const CHeaderObject &data)
{
  CHeaderArray array;
  CHeaderObject obj;
  CHeaderValue value;
  FMSynth::Patch patch;
  strncpy(patch.name, data["name"].toString(), 15);
  patch.algorithm = data["algorithm"].toInt();
  patch.volume = data["volume"].toInt();
  patch.feedback = data["feedback"].toInt();
  patch.glide = data["glide"].toInt();
  patch.attack = data["attack"].toInt();
  patch.decay = data["decay"].toInt();
  patch.sustain = data["sustain"].toInt();
  patch.release = data["release"].toInt();
  obj = data["lfo"].toObject();
  patch.lfo.speed = obj["speed"].toInt();
  patch.lfo.attack = obj["attack"].toInt();
  patch.lfo.pmd = obj["pmd"].toInt();
  array = data["op"].toArray();
  for (int i = 0; i < 4; ++i)
  {
    CHeaderObject op = array[i].toObject();
    patch.op[i].level = op["level"].toInt();
    obj = op["pitch"].toObject();
    patch.op[i].pitch.fixed = obj["fixed"].toBool();
    patch.op[i].pitch.coarse = obj["coarse"].toInt();
    patch.op[i].pitch.fine = obj["fine"].toInt();
    patch.op[i].detune = op["detune"].toInt();
    patch.op[i].attack = op["attack"].toInt();
    patch.op[i].decay = op["decay"].toInt();
    patch.op[i].sustain = op["sustain"].toInt();
    patch.op[i].loop = op["loop"].toBool();
  }
  return patch;
}

FMSynth::Patch *Globals::patchFromJson(const QJsonObject &json)
{
  QJsonObject obj;
  QJsonArray array;
  FMSynth::Patch *patch = new FMSynth::Patch;
  strncpy(patch->name, json["name"].toString().toLatin1().data(), 15);
  patch->algorithm = json["algorithm"].toInt();
  patch->volume = json["volume"].toInt();
  patch->feedback = json["feedback"].toInt();
  patch->glide = json["glide"].toInt();
  patch->attack = json["attack"].toInt();
  patch->decay = json["decay"].toInt();
  patch->sustain = json["sustain"].toInt();
  patch->release = json["release"].toInt();
  obj = json["lfo"].toObject();
  patch->lfo.speed = obj["speed"].toInt();
  patch->lfo.attack = obj["attack"].toInt();
  patch->lfo.pmd = obj["pmd"].toInt();
  array = json["op"].toArray();
  for (int i = 0; i < 4; ++i)
  {
    QJsonObject op = array[i].toObject();
    patch->op[i].level = op["level"].toInt();
    obj = op["pitch"].toObject();
    patch->op[i].pitch.fixed = obj["fixed"].toBool();
    patch->op[i].pitch.coarse = obj["coarse"].toInt();
    patch->op[i].pitch.fine = obj["fine"].toInt();
    patch->op[i].detune = op["detune"].toInt();
    patch->op[i].attack = op["attack"].toInt();
    patch->op[i].decay = op["decay"].toInt();
    patch->op[i].sustain = op["sustain"].toInt();
    patch->op[i].loop = op["loop"].toBool();
  }
  return patch;
}

QJsonObject Globals::patchToJson(const FMSynth::Patch *patch)
{
  QJsonObject json;
  QJsonObject obj;
  QJsonArray array;
  json["name"] = patch->name;
  json["algorithm"] = patch->algorithm;
  json["volume"] = patch->volume;
  json["feedback"] = patch->feedback;
  json["glide"] = patch->glide;
  json["attack"] = patch->attack;
  json["decay"] = patch->decay;
  json["sustain"] = patch->sustain;
  json["release"] = patch->release;
  obj["speed"] = patch->lfo.speed;
  obj["attack"] = patch->lfo.attack;
  obj["pmd"] = patch->lfo.pmd;
  json["lfo"] = obj;
  for (int i = 0; i < 4; ++i)
  {
    QJsonObject op;
    op["level"] = patch->op[i].level;
    obj = QJsonObject();
    obj["fixed"] = patch->op[i].pitch.fixed;
    obj["coarse"] = patch->op[i].pitch.coarse;
    obj["fine"] = patch->op[i].pitch.fine;
    op["pitch"] = obj;
    op["detune"] = patch->op[i].detune;
    op["attack"] = patch->op[i].attack;
    op["decay"] = patch->op[i].decay;
    op["sustain"] = patch->op[i].sustain;
    op["loop"] = patch->op[i].loop;
    array += op;
  }
  json["op"] = array;
  return json;
}
