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

#ifndef INSTRUMENTEDITOR_H
#define INSTRUMENTEDITOR_H

#include <QAudioOutput>
#include <QDialog>
#include "ui_instrumenteditor.h"
#include "FMSource.h"

class InstrumentEditor : public QDialog, public Ui::InstrumentEditor
{
  Q_OBJECT
  public:
    InstrumentEditor(QWidget *parent=0);
    ~InstrumentEditor();
  protected slots:
    void on_btnNewInstrument_clicked();
    void on_btnCHeaderData_clicked();
    void on_lstInstruments_currentRowChanged(int currentRow);
    void on_btnDeleteInstrument_clicked();
    void on_leName_textChanged(QString value);
    void on_btnPreviousAlgorithm_clicked();
    void on_btnNextAlgorithm_clicked();
    //void on_btnOpenWaveformAnalyzer_clicked();
    void on_numVolume_valueChanged(int value);
    void on_btnOpenSample_clicked();
    void on_btnCloseSample_clicked();
    void on_btnPlaySample_clicked();
    void on_btnHelp_clicked();
    void on_numPatchVolume_valueChanged(int value);
    void on_numPatchFeedback_valueChanged(int value);
    void on_numPatchGlide_valueChanged(int value);
    void on_numPatchAttack_valueChanged(int value);
    void on_numPatchDecay_valueChanged(int value);
    void on_numPatchSustain_valueChanged(int value);
    void on_numPatchRelease_valueChanged(int value);
    void on_numPatchLFOSpeed_valueChanged(int value);
    void on_numPatchLFOAttack_valueChanged(int value);
    void on_numPatchLFODepth_valueChanged(int value);
    void on_numOp1Level_valueChanged(int value);
    void on_btnOp1PitchFixed_toggled(bool on);
    void on_numOp1PitchCoarse_valueChanged(int value);
    void on_numOp1PitchFine_valueChanged(int value);
    void on_numOp1Detune_valueChanged(int value);
    void on_numOp1Attack_valueChanged(int value);
    void on_numOp1Decay_valueChanged(int value);
    void on_numOp1Sustain_valueChanged(int value);
    void on_sldrOp1Loop_valueChanged(int value);
    void on_numOp2Level_valueChanged(int value);
    void on_btnOp2PitchFixed_toggled(bool on);
    void on_numOp2PitchCoarse_valueChanged(int value);
    void on_numOp2PitchFine_valueChanged(int value);
    void on_numOp2Detune_valueChanged(int value);
    void on_numOp2Attack_valueChanged(int value);
    void on_numOp2Decay_valueChanged(int value);
    void on_numOp2Sustain_valueChanged(int value);
    void on_sldrOp2Loop_valueChanged(int value);
    void on_numOp3Level_valueChanged(int value);
    void on_btnOp3PitchFixed_toggled(bool on);
    void on_numOp3PitchCoarse_valueChanged(int value);
    void on_numOp3PitchFine_valueChanged(int value);
    void on_numOp3Detune_valueChanged(int value);
    void on_numOp3Attack_valueChanged(int value);
    void on_numOp3Decay_valueChanged(int value);
    void on_numOp3Sustain_valueChanged(int value);
    void on_sldrOp3Loop_valueChanged(int value);
    void on_numOp4Level_valueChanged(int value);
    void on_btnOp4PitchFixed_toggled(bool on);
    void on_numOp4PitchCoarse_valueChanged(int value);
    void on_numOp4PitchFine_valueChanged(int value);
    void on_numOp4Detune_valueChanged(int value);
    void on_numOp4Attack_valueChanged(int value);
    void on_numOp4Decay_valueChanged(int value);
    void on_numOp4Sustain_valueChanged(int value);
    void on_sldrOp4Loop_valueChanged(int value);
    void on_wKeyboard_notePressed(int midikey);
    void on_wKeyboard_noteReleased();
  private:
    void loadPatchValues();
    void loadSampleFile();
    void updateWaveformPreview();
    void updateLikenessRating();
    QAudioOutput *audio;
    FMSynth::Patch *patch;
    FMSource *source;
    static const char *helpText;
    int waveformNote;
    bool ignoreEvents;
};

#endif // INSTRUMENTEDITOR_H
