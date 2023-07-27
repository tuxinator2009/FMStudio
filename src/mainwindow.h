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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAudioOutput>
#include "ui_mainwindow.h"
#include "fmsong.h"
#include "FMSource.h"

class MainWindow : public QMainWindow, public Ui::MainWindow
{
  Q_OBJECT
  public:
    MainWindow(QWidget *parent=0);
    ~MainWindow();
  protected slots:
    void on_btnNewProject_clicked();
    void on_btnOpenProject_clicked();
    void on_btnOpenProject_triggered(QAction *action);
    void on_btnSaveProject_clicked();
    void on_aExportCHeader_triggered();
    void on_aExportRawAudio_triggered();
    void on_leProjectName_textChanged(QString text);
    void on_optInstrument_currentIndexChanged(int index);
    void on_btnEditInstruments_clicked();
    void on_optOverlapPolicy_currentIndexChanged(int index);
    void on_optShowKeyNames_currentIndexChanged(int index);
    void on_numVolume_valueChanged(int value);
    void on_btnNewSong_clicked();
    void on_lstSongs_currentRowChanged(int row);
    void on_btnDeleteSong_clicked();
    void on_leSongName_textChanged(QString text);
    void on_numSongTempo_valueChanged(int value);
    void on_btnPlaySong_clicked();
    void on_wSections_updateSongLength(uint32_t length);
    void on_btnNewPattern_clicked();
    void on_lstPatterns_currentRowChanged(int currentRow);
    void on_btnDeletePattern_clicked();
    void on_lePatternName_textChanged(QString text);
    void on_btnMinusNoteDuration_clicked();
    void on_btnPlusNoteDuration_clicked();
    void on_btnPlayPattern_clicked();
    void on_wKeyboard_notePressed(int midikey);
    void on_wKeyboard_noteReleased();
    void playSong(bool force=false);
    void playPattern(bool force=false);
  private:
    void closeEvent(QCloseEvent *event);
    QAudioOutput *audio;
    FMSong *song;
    FMSong::Pattern *pattern;
    FMSynth::Patch *patch;
    FMSource *source;
    bool ignoreEvents;
};

#endif // MAINWINDOW_H
