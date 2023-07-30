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

#include <QAction>
#include <QAudioFormat>
#include <QAudioOutput>
#include <QFile>
#include <QFileDialog>
#include <QKeyEvent>
#include <QMainWindow>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QScrollBar>
#include <QTextStream>
#include <QTimer>
#include "CHeaderParser/cheaderparser.h"
#include "fmproject.h"
#include "fmsong.h"
#include "globals.h"
#include "instrumenteditor.h"
#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
  QMenu *menu = Globals::loadRecentProjects(this);
  Globals::loadInstruments();
  Globals::project = new FMProject();
  song = Globals::project->getSong(0);
  ignoreEvents = true;
  setupUi(this);
  audio = Globals::createAudioOutput(this);
  source = new FMSource(5);
  leProjectName->setText(Globals::project->getName());
  optInstrument->clear();
  for (int i = 0; i < Globals::project->numInstruments(); ++i)
    optInstrument->addItem(Globals::project->getInstrument(i)->name);
  optInstrument->setCurrentIndex(0);
  for (int i = 0; i < Globals::project->numSongs(); ++i)
    lstSongs->addItem(Globals::project->getSong(i)->getName());
  lstSongs->setCurrentRow(lstSongs->count() - 1);
  if (menu == nullptr)
    btnOpenProject->setPopupMode(QToolButton::DelayedPopup);
  else
    btnOpenProject->setMenu(menu);
  ignoreEvents = false;
  on_optInstrument_currentIndexChanged(0);
  on_lstSongs_currentRowChanged(0);
  connect(optPatternSnap, SIGNAL(currentIndexChanged(int)), wSections, SLOT(setSnapAmount(int)));
  connect(wSections, SIGNAL(playbackFinished()), this, SLOT(playSong()));
  connect(optNoteSnap, SIGNAL(currentIndexChanged(int)), wNotes, SLOT(setNoteSnap(int)));
  connect(optGridSnap, SIGNAL(currentIndexChanged(int)), wNotes, SLOT(setGridSnap(int)));
  connect(optGridSize, SIGNAL(currentIndexChanged(int)), wNotes, SLOT(setGridSize(int)));
  connect(numNoteDuration, SIGNAL(valueChanged(int)), wNotes, SLOT(setNoteDuration(int)));
  connect(scrollArea->verticalScrollBar(), SIGNAL(valueChanged(int)), wNotes, SLOT(setVScrollOffset(int)));
  connect(scrollArea->verticalScrollBar(), SIGNAL(rangeChanged(int, int)), wNotes, SLOT(setVScrollRange(int, int)));
  connect(wNotes, SIGNAL(patternChanged()), wSections, SLOT(update()));
  connect(wNotes, SIGNAL(scrollKeyboard(int)), scrollArea->verticalScrollBar(), SLOT(setValue(int)));
  connect(wNotes, SIGNAL(noteDurationChanged(int)), numNoteDuration, SLOT(setValue(int)));
  connect(wNotes, SIGNAL(playbackFinished()), this, SLOT(playPattern()));
  connect(chkAutoScrollSong, SIGNAL(toggled(bool)), wSections, SLOT(setAutoScroll(bool)));
  connect(chkAutoScrollPattern, SIGNAL(toggled(bool)), wNotes, SLOT(setAutoScroll(bool)));
  wNotes->setVirtualKeyboard(wKeyboard);
  setWindowTitle("FMStudio - Untitled");
  menu = new QMenu(this);
  menu->addAction(aExportCHeader);
  menu->addAction(aExportRawAudio);
  btnExportProject->setMenu(menu);
}

MainWindow::~MainWindow()
{
  delete source;
}

void MainWindow::on_btnNewProject_clicked()
{
  if (!Globals::project->isSaved())
  {
    QMessageBox::StandardButton confirm = QMessageBox::question(this, "Save Project", "Save changes to current project?", QMessageBox::Save|QMessageBox::Discard|QMessageBox::Cancel, QMessageBox::Cancel);
    if (confirm == QMessageBox::Save)
    {
      Globals::project->saveProject();
      if (!Globals::project->isSaved())
        return;
    }
    else if (confirm == QMessageBox::Cancel)
      return;
  }
  wSections->setEnabled(false);
  wNotes->setEnabled(false);
  delete Globals::project;
  Globals::project = new FMProject();
  song = Globals::project->getSong(0);
  ignoreEvents = true;
  leProjectName->setText(Globals::project->getName());
  optInstrument->clear();
  for (int i = 0; i < Globals::project->numInstruments(); ++i)
    optInstrument->addItem(Globals::project->getInstrument(i)->name);
  optInstrument->setCurrentIndex(0);
  optOverlapPolicy->setCurrentIndex((int)Globals::project->getOverlapPolicy());
  lstSongs->clear();
  for (int i = 0; i < Globals::project->numSongs(); ++i)
    lstSongs->addItem(Globals::project->getSong(i)->getName());
  lstSongs->setCurrentRow(0);
  btnDeleteSong->setEnabled(lstSongs->count() > 1);
  ignoreEvents = false;
  wSections->setEnabled(true);
  wNotes->setEnabled(true);
  on_optInstrument_currentIndexChanged(0);
  on_lstSongs_currentRowChanged(0);
  setWindowTitle("FMStudio - Untitled");
}

void MainWindow::on_btnOpenProject_clicked()
{
  if (!Globals::project->isSaved())
  {
    QMessageBox::StandardButton confirm = QMessageBox::question(this, "Save Project", "Save changes to current project?", QMessageBox::Save|QMessageBox::Discard|QMessageBox::Cancel, QMessageBox::Cancel);
    if (confirm == QMessageBox::Save)
    {
      Globals::project->saveProject();
      if (!Globals::project->isSaved())
        return;
    }
    else if (confirm == QMessageBox::Cancel)
      return;
  }
  QString location = QFileDialog::getOpenFileName(this, "Open Project", Globals::appPath, "FM Studio Project (*.fmx)");
  QFileInfo info(location);
  if (!location.isEmpty())
  {
    FMProject *project = new FMProject(location);
    if (project->isSaved())
    {
      delete Globals::project;
      Globals::project = project;
      ignoreEvents = true;
      leProjectName->setText(Globals::project->getName());
      optInstrument->clear();
      for (int i = 0; i < project->numInstruments(); ++i)
        optInstrument->addItem(project->getInstrument(i)->name);
      optInstrument->setCurrentIndex(0);
      optOverlapPolicy->setCurrentIndex((int)project->getOverlapPolicy());
      lstSongs->clear();
      for (int i = 0; i < Globals::project->numSongs(); ++i)
        lstSongs->addItem(Globals::project->getSong(i)->getName());
      lstSongs->setCurrentRow(0);
      btnDeleteSong->setEnabled(lstSongs->count() > 1);
      ignoreEvents = false;
      on_optInstrument_currentIndexChanged(0);
      on_lstSongs_currentRowChanged(0);
      Globals::addRecentProject(info.fileName(), location);
      setWindowTitle(QString("FMStudio - %1").arg(info.fileName()));
    }
  }
}

void MainWindow::on_btnOpenProject_triggered(QAction *action)
{
  if (!Globals::project->isSaved())
  {
    QMessageBox::StandardButton confirm = QMessageBox::question(this, "Save Project", "Save changes to current project?", QMessageBox::Save|QMessageBox::Discard|QMessageBox::Cancel, QMessageBox::Cancel);
    if (confirm == QMessageBox::Save)
    {
      Globals::project->saveProject();
      if (!Globals::project->isSaved())
        return;
    }
    else if (confirm == QMessageBox::Cancel)
      return;
  }
  QString name = action->text();
  QString location = action->data().toString();
  if (!location.isEmpty())
  {
    FMProject *oldProject = Globals::project;
    Globals::project = new FMProject(location);
    if (Globals::project->isSaved())
    {
      delete oldProject;
      ignoreEvents = true;
      leProjectName->setText(Globals::project->getName());
      optInstrument->clear();
      for (int i = 0; i < Globals::project->numInstruments(); ++i)
        optInstrument->addItem(Globals::project->getInstrument(i)->name);
      optInstrument->setCurrentIndex(0);
      optOverlapPolicy->setCurrentIndex((int)Globals::project->getOverlapPolicy());
      lstSongs->clear();
      for (int i = 0; i < Globals::project->numSongs(); ++i)
        lstSongs->addItem(Globals::project->getSong(i)->getName());
      lstSongs->setCurrentRow(0);
      btnDeleteSong->setEnabled(lstSongs->count() > 1);
      ignoreEvents = false;
      on_optInstrument_currentIndexChanged(0);
      on_lstSongs_currentRowChanged(0);
      Globals::addRecentProject(name, location);
      setWindowTitle(QString("FMStudio - %1").arg(name));
    }
    else
    {
      delete Globals::project;
      Globals::project = oldProject;
    }
  }
}

void MainWindow::on_btnSaveProject_clicked()
{
  Globals::project->saveProject();
}

void MainWindow::on_aExportCHeader_triggered()
{
}

void MainWindow::on_aExportRawAudio_triggered()
{
  QFileInfo info(Globals::project->getLocation());
  if (!info.exists())
  {
    QMessageBox::critical(this, "Can't Export", "The project needs to be saved to a file before it can be exported.");
    return;
  }
  setEnabled(false);
  QCoreApplication::processEvents();
  for (int i = 0; i < Globals::project->numSongs(); ++i)
  {
    FMSong *s = Globals::project->getSong(i);
    QFile file(info.absolutePath() + "/" + Globals::project->getName() + " - " + s->getName() + ".raw");
    if (!file.open(QFile::WriteOnly))
    {
      QMessageBox::critical(this, "Export Failed", QString("Failed to export %1 to %2\nReason: %3").arg(s->getName()).arg(file.fileName()).arg(file.errorString()));
      return;
    }
    source->setTempo(s->getTempo());
    source->playSong(s);
    while (!source->atEnd())
      file.write(source->read(512));
    file.close();
  }
  setEnabled(true);
}

void MainWindow::on_leProjectName_textChanged(QString text)
{
  Globals::project->setName(text);
}

void MainWindow::on_optInstrument_currentIndexChanged(int index)
{
  if (ignoreEvents)
    return;
  if (index < 0 || index >= Globals::project->numInstruments())
    optInstrument->setCurrentIndex(0);
  patch = Globals::project->getInstrument(optInstrument->currentIndex());
  wSections->setInstrument(optInstrument->currentIndex());
}

void MainWindow::on_btnEditInstruments_clicked()
{
  if (ignoreEvents)
    return;
  InstrumentEditor *editor = new InstrumentEditor;
  int currentInstrument = optInstrument->currentIndex();
  editor->exec();
  editor->deleteLater();
  ignoreEvents = true;
  optInstrument->clear();
  for (int i = 0; i < Globals::project->numInstruments(); ++i)
    optInstrument->addItem(Globals::project->getInstrument(i)->name);
  optInstrument->setCurrentIndex(currentInstrument);
  patch = Globals::project->getInstrument(optInstrument->currentIndex());
  wSections->update();
  ignoreEvents = false;
}

void MainWindow::on_optOverlapPolicy_currentIndexChanged(int index)
{
  if (ignoreEvents)
    return;
  FMProject::OverlapPolicy policy = (FMProject::OverlapPolicy)index;
  if (policy == FMProject::OverlapPolicy::Forbidden)
  {
    QStringList overlaps;
    for (int i = 0; i < Globals::project->numSongs(); ++i)
    {
      if (song->hasOverlaps())
        overlaps += song->getName();
    }
    if (overlaps.size() > 0)
    {
      QMessageBox::information(this, "Overlapping Notes", QString("The following songs have overlapping notes, the overlap policy will be set to 'Highlight' to help show them:  %1\n").arg(overlaps.join("\n  ")));
      optOverlapPolicy->setCurrentIndex((int)FMProject::OverlapPolicy::Highlight);
      return;
    }
  }
  Globals::project->setOverlapPolicy(policy);
  wSections->update();
  wNotes->update();
}

void MainWindow::on_optShowKeyNames_currentIndexChanged(int index)
{
  if (ignoreEvents)
    return;
  Globals::project->setShowKeyNames((FMProject::ShowKeyNames)index);
  wKeyboard->update();
}

void MainWindow::on_numVolume_valueChanged(int value)
{
  audio->setVolume(QAudio::convertVolume(value / 100.0, QAudio::LogarithmicVolumeScale, QAudio::LinearVolumeScale));
}

void MainWindow::on_btnNewSong_clicked()
{
  song = new FMSong();
  song->setName(QString("Song %1").arg(Globals::project->numSongs() + 1));
  Globals::project->addSong(song);
  lstSongs->addItem(song->getName());
  lstSongs->setCurrentRow(lstSongs->count() - 1);
  btnDeleteSong->setEnabled(true);
}

void MainWindow::on_lstSongs_currentRowChanged(int row)
{
  if (ignoreEvents)
    return;
  uint32_t length;
  ignoreEvents = true;
  song = Globals::project->getSong(row);
  leSongName->setText(song->getName());
  numSongTempo->setValue(song->getTempo());
  lstPatterns->clear();
  for (int i = 0; i < song->numPatterns(); ++i)
    lstPatterns->addItem(song->getPattern(i)->name);
  ignoreEvents = false;
  btnDeletePattern->setEnabled(lstPatterns->count() > 1);
  lstPatterns->setCurrentRow(0);
  length = song->getLength();
  lblSongDuration->setText(QString("%1:%2").arg(length / 60).arg(length % 60, 2, 10, QChar('0')));
  wSections->setSong(song);
}

void MainWindow::on_btnDeleteSong_clicked()
{
  if (QMessageBox::question(this, "Delete Song?", QString("Are you sure you want to permanently delete %1?").arg(song->getName())) == QMessageBox::Yes)
  {
    Globals::project->deleteSong(lstSongs->currentRow());
    delete lstSongs->takeItem(lstSongs->currentRow());
    if (lstSongs->count() == 1)
      btnDeleteSong->setEnabled(false);
  }
}

void MainWindow::on_leSongName_textChanged(QString text)
{
  if (ignoreEvents)
    return;
  Globals::project->setName(text);
  lstSongs->currentItem()->setText(text);
  song->setName(text);
}

void MainWindow::on_numSongTempo_valueChanged(int value)
{
  if (ignoreEvents)
    return;
  song->setTempo(value);
  Globals::project->setSaved(false);
}

void MainWindow::on_btnPlaySong_clicked()
{
  if (btnPlaySong->text() == "Play")
  {
    wSongs->setEnabled(false);
    wPatternEditor->setEnabled(false);
    btnPlaySong->setText("Stop");
    playSong(true);
  }
  else
  {
    wSongs->setEnabled(true);
    wPatternEditor->setEnabled(true);
    btnPlaySong->setText("Play");
    source->stopSong();
    audio->stop();
    wSections->setPlaybackPosition(0);
    wSections->setAudioPlaying(nullptr, 0);
  }
}

void MainWindow::on_wSections_updateSongLength(uint32_t length)
{
  lblSongDuration->setText(QString("%1:%2").arg(length / 60).arg(length % 60, 2, 10, QChar('0')));
  wNotes->setMaxDuration(song->getPatternMaxDuration(lstPatterns->currentRow()));
}

void MainWindow::on_btnNewPattern_clicked()
{
  FMSong::Pattern *newPattern = new FMSong::Pattern;
  newPattern->name = QString("Pattern %1").arg(song->numPatterns() + 1);
  newPattern->noteSnap = optNoteSnap->currentIndex();
  newPattern->gridSnap = optGridSnap->currentIndex();
  newPattern->gridSize = optGridSize->currentIndex();
  newPattern->duration = 1;
  song->addPattern(newPattern);
  lstPatterns->addItem(newPattern->name);
  lstPatterns->setCurrentRow(lstPatterns->count() - 1);
  Globals::project->setSaved(false);
  btnDeletePattern->setEnabled(false);
}

void MainWindow::on_lstPatterns_currentRowChanged(int currentRow)
{
  if (ignoreEvents)
    return;
  ignoreEvents = true;
  pattern = song->getPattern(currentRow);
  lePatternName->setText(pattern->name);
  wSections->setPattern(currentRow);
  wNotes->setPattern(pattern);
  wNotes->setMaxDuration(song->getPatternMaxDuration(currentRow));
  optNoteSnap->setCurrentIndex(pattern->noteSnap);
  optGridSnap->setCurrentIndex(pattern->gridSnap);
  optGridSize->setCurrentIndex(pattern->gridSize);
  ignoreEvents = false;
}

void MainWindow::on_btnDeletePattern_clicked()
{
  if (QMessageBox::question(this, "Delete Pattern?", QString("Are you sure you want to permanently delete %1?").arg(pattern->name)) == QMessageBox::Yes)
  {
    song->deletePattern(lstPatterns->currentRow());
    delete lstPatterns->takeItem(lstPatterns->currentRow());
    Globals::project->setSaved(false);
    if (lstPatterns->count() == 1)
      btnDeletePattern->setEnabled(false);
    wSections->update();
  }
}

void MainWindow::on_lePatternName_textChanged(QString text)
{
  if (ignoreEvents)
    return;
  pattern->name = text;
  lstPatterns->currentItem()->setText(text);
  Globals::project->setSaved(false);
}

void MainWindow::on_btnMinusNoteDuration_clicked()
{
  int value = numNoteDuration->value();
  int shift = 1 << optNoteSnap->currentIndex();
  if (value - shift > 0)
    numNoteDuration->setValue(value - shift);
}

void MainWindow::on_btnPlusNoteDuration_clicked()
{
  int value = numNoteDuration->value();
  int shift = 1 << optNoteSnap->currentIndex();
  if (value + shift < 256)
    numNoteDuration->setValue(value + shift);
}

void MainWindow::on_btnPlayPattern_clicked()
{
  if (btnPlayPattern->text() == "Play")
  {
    wPatterns->setEnabled(false);
    btnPlayPattern->setText("Stop");
    playPattern(true);
  }
  else
  {
    source->stopPattern(0);
    wPatterns->setEnabled(true);
    btnPlayPattern->setText("Play");
    audio->stop();
    wNotes->setPlaybackPosition(0);
    wNotes->setAudioPlaying(nullptr, 0);
  }
}

void MainWindow::on_wKeyboard_notePressed(int midikey)
{
  source->noteOn(4, *patch, midikey, -1);
  if (audio->state() != QAudio::ActiveState || audio->state() != QAudio::IdleState)
    audio->start(source);
}

void MainWindow::on_wKeyboard_noteReleased()
{
  source->noteOff(4);
}

void MainWindow::playSong(bool force)
{
  if (btnPlaySong->text() == "Play")
    return;
  if (chkLoopSong->isChecked() || force)
  {
    source->setTempo(numSongTempo->value());
    source->playSong(song);
    wSections->setPlaybackPosition(0);
    audio->start(source);
    audio->setNotifyInterval((numSongTempo->value() * 1000)/(128 * 60));
    wSections->setAudioPlaying(audio, numSongTempo->value());
  }
  else
  {
    audio->stop();
    wSongs->setEnabled(true);
    wPatternEditor->setEnabled(true);
    btnPlaySong->setText("Play");
    source->stopSong();
    wSections->setAudioPlaying(nullptr, 0);
    wSections->setPlaybackPosition(0);
  }
}

void MainWindow::playPattern(bool force)
{
  if (btnPlayPattern->text() == "Play")
    return;
  if (chkLoopPattern->isChecked() || force)
  {
    source->setTempo(numSongTempo->value());
    source->playPattern(0, pattern->notes, *patch);
    wNotes->setPlaybackPosition(0);
    audio->start(source);
    audio->setNotifyInterval((numSongTempo->value() * 1000)/(512 * 60));
    wNotes->setAudioPlaying(audio, numSongTempo->value());
  }
  else
  {
    audio->stop();
    wPatterns->setEnabled(true);
    btnPlayPattern->setText("Play");
    wNotes->setAudioPlaying(nullptr, 0);
    wNotes->setPlaybackPosition(0);
  }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
  if (!Globals::project->isSaved())
  {
    QMessageBox::StandardButton confirm = QMessageBox::question(this, "Save Project", "Save changes to current project?", QMessageBox::Save|QMessageBox::Discard|QMessageBox::Cancel, QMessageBox::Cancel);
    if (confirm == QMessageBox::Save)
    {
      Globals::project->saveProject();
      if (!Globals::project->isSaved())
      {
        event->ignore();
        return;
      }
    }
    else if (confirm == QMessageBox::Cancel)
    {
      event->ignore();
      return;
    }
  }
  Globals::saveRecentProjects();
  event->accept();
}
