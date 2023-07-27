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

#include <QInputDialog>
#include <QMessageBox>
#include <QPainter>
#include <QPixmap>
#include "CHeaderParser/cheaderparser.h"
#include "cheaderview.h"
#include "fmproject.h"
#include "globals.h"
#include "instrumenteditor.h"
#include "newinstrument.h"

InstrumentEditor::InstrumentEditor(QWidget *parent) : QDialog(parent)
{
  setupUi(this);
  patch = nullptr;
  wPatch->setEnabled(false);
  btnCHeaderData->setEnabled(false);
  btnDeleteInstrument->setEnabled(false);
  audio = Globals::createAudioOutput(this);
  for (int i = 0; i < Globals::project->numInstruments(); ++i)
    lstInstruments->addItem(Globals::project->getInstrument(i)->name);
  lstInstruments->setCurrentRow(0);
  btnDeleteInstrument->setEnabled(Globals::project->numInstruments() > 1);
  source = new FMSource(1);
}

InstrumentEditor::~InstrumentEditor()
{
  delete source;
}

void InstrumentEditor::on_btnNewInstrument_clicked()
{
  NewInstrument *instrument = new NewInstrument(this);
  if (instrument->exec())
  {
    Globals::project->addInstrument(new FMSynth::Patch(instrument->getPatch()));
    lstInstruments->addItem(patch->name);
    lstInstruments->setCurrentRow(lstInstruments->count() - 1);
  }
  instrument->deleteLater();
  btnDeleteInstrument->setEnabled(true);
}

void InstrumentEditor::on_btnCHeaderData_clicked()
{
  CHeaderView *view = new CHeaderView(Globals::patchToCHeader(*patch));
  if (view->exec())
  {
    CHeaderObject data = CHeaderParser::parseCHeader(view->getText()).toObject();
    *patch = Globals::patchFromCHeader(data);
    loadPatchValues();
  }
}

void InstrumentEditor::on_lstInstruments_currentRowChanged(int currentRow)
{
  if (currentRow < 0 || currentRow >= Globals::project->numInstruments())
    return;
  wPatch->setEnabled(true);
  btnCHeaderData->setEnabled(true);
  patch = Globals::project->getInstrument(currentRow);
  loadPatchValues();
  updateWaveformPreview();
}

void InstrumentEditor::on_btnDeleteInstrument_clicked()
{
  if (QMessageBox::question(this, "Delete Instrument", QString("Are you sure you want to remove %1 from the current project?").arg(patch->name)) == QMessageBox::Yes)
  {
    Globals::project->deleteInstrument(lstInstruments->currentRow());
    delete lstInstruments->takeItem(lstInstruments->currentRow());
    if (lstInstruments->count() == 1)
      btnDeleteInstrument->setEnabled(false);
  }
}

void InstrumentEditor::on_leName_textChanged(QString value)
{
  strcpy(patch->name, value.toLocal8Bit().data());
  lstInstruments->currentItem()->setText(value);
}

void InstrumentEditor::on_btnPreviousAlgorithm_clicked()
{
  if (patch->algorithm == 1)
    return;
  --patch->algorithm;
  lblAlgorithm->setPixmap(QPixmap(QString(":/images/algo%1.png").arg(patch->algorithm)));
  updateWaveformPreview();
}

void InstrumentEditor::on_btnNextAlgorithm_clicked()
{
  if (patch->algorithm == 11)
    return;
  ++patch->algorithm;
  lblAlgorithm->setPixmap(QPixmap(QString(":/images/algo%1.png").arg(patch->algorithm)));
  updateWaveformPreview();
}

void InstrumentEditor::on_numVolume_valueChanged(int value)
{
  audio->setVolume(QAudio::convertVolume(value / 100.0, QAudio::LogarithmicVolumeScale, QAudio::LinearVolumeScale));
}

void InstrumentEditor::on_numPatchVolume_valueChanged(int value)
{
  patch->volume = value;
  updateWaveformPreview();
}

void InstrumentEditor::on_numPatchFeedback_valueChanged(int value)
{
  patch->feedback = value + 50;
  updateWaveformPreview();
}

void InstrumentEditor::on_numPatchGlide_valueChanged(int value)
{
  patch->glide = value;
  updateWaveformPreview();
}

void InstrumentEditor::on_numPatchAttack_valueChanged(int value)
{
  patch->attack = value;
  updateWaveformPreview();
}

void InstrumentEditor::on_numPatchDecay_valueChanged(int value)
{
  patch->decay = value;
  updateWaveformPreview();
}

void InstrumentEditor::on_numPatchSustain_valueChanged(int value)
{
  patch->sustain = value;
  updateWaveformPreview();
}

void InstrumentEditor::on_numPatchRelease_valueChanged(int value)
{
  patch->release = value;
  updateWaveformPreview();
}

void InstrumentEditor::on_numPatchLFOSpeed_valueChanged(int value)
{
  patch->lfo.speed = value;
  updateWaveformPreview();
}

void InstrumentEditor::on_numPatchLFOAttack_valueChanged(int value)
{
  patch->lfo.attack = value;
  updateWaveformPreview();
}

void InstrumentEditor::on_numPatchLFODepth_valueChanged(int value)
{
  patch->lfo.pmd = value;
  updateWaveformPreview();
}

void InstrumentEditor::on_numOp1Level_valueChanged(int value)
{
  patch->op[0].level = value;
  updateWaveformPreview();
}

void InstrumentEditor::on_btnOp1Fixed_toggled(bool on)
{
  patch->op[0].pitch.fixed = on;
  if (on)
    btnOp1Fixed->setText("FIXED");
  else
    btnOp1Fixed->setText("RATIO");
  updateWaveformPreview();
}

void InstrumentEditor::on_dialOp1Pitch_valueChanged(int value)
{
  patch->op[0].pitch.coarse = value / 100;
  patch->op[0].pitch.fine = value % 100;
  dblOp1Pitch->setValue(value / 100.0);
  updateWaveformPreview();
}

void InstrumentEditor::on_dblOp1Pitch_valueChanged(double value)
{
  dialOp1Pitch->setValue((int)(value * 100));
  updateWaveformPreview();
}

void InstrumentEditor::on_numOp1Detune_valueChanged(int value)
{
  patch->op[0].detune = value;
  updateWaveformPreview();
}

void InstrumentEditor::on_numOp1Attack_valueChanged(int value)
{
  patch->op[0].attack = value;
  updateWaveformPreview();
}

void InstrumentEditor::on_numOp1Decay_valueChanged(int value)
{
  patch->op[0].decay = value;
  updateWaveformPreview();
}

void InstrumentEditor::on_numOp1Sustain_valueChanged(int value)
{
  patch->op[0].sustain = value;
  updateWaveformPreview();
}

void InstrumentEditor::on_sldrOp1Loop_valueChanged(int value)
{
  patch->op[0].loop = value == 1;
  updateWaveformPreview();
}

void InstrumentEditor::on_numOp2Level_valueChanged(int value)
{
  patch->op[1].level = value;
  updateWaveformPreview();
}

void InstrumentEditor::on_btnOp2Fixed_toggled(bool on)
{
  patch->op[1].pitch.fixed = on;
  if (on)
    btnOp2Fixed->setText("FIXED");
  else
    btnOp2Fixed->setText("RATIO");
  updateWaveformPreview();
}

void InstrumentEditor::on_dialOp2Pitch_valueChanged(int value)
{
  patch->op[1].pitch.coarse = value / 100;
  patch->op[1].pitch.fine = value % 100;
  dblOp2Pitch->setValue(value / 100.0);
  updateWaveformPreview();
}

void InstrumentEditor::on_dblOp2Pitch_valueChanged(double value)
{
  dialOp2Pitch->setValue((int)(value * 100));
  updateWaveformPreview();
}

void InstrumentEditor::on_numOp2Detune_valueChanged(int value)
{
  patch->op[1].detune = value;
  updateWaveformPreview();
}

void InstrumentEditor::on_numOp2Attack_valueChanged(int value)
{
  patch->op[1].attack = value;
  updateWaveformPreview();
}

void InstrumentEditor::on_numOp2Decay_valueChanged(int value)
{
  patch->op[1].decay = value;
  updateWaveformPreview();
}

void InstrumentEditor::on_numOp2Sustain_valueChanged(int value)
{
  patch->op[1].sustain = value;
  updateWaveformPreview();
}

void InstrumentEditor::on_sldrOp2Loop_valueChanged(int value)
{
  patch->op[1].loop = value == 1;
  updateWaveformPreview();
}

void InstrumentEditor::on_numOp3Level_valueChanged(int value)
{
  patch->op[2].level = value;
  updateWaveformPreview();
}

void InstrumentEditor::on_btnOp3Fixed_toggled(bool on)
{
  patch->op[2].pitch.fixed = on;
  if (on)
    btnOp3Fixed->setText("FIXED");
  else
    btnOp3Fixed->setText("RATIO");
  updateWaveformPreview();
}

void InstrumentEditor::on_dialOp3Pitch_valueChanged(int value)
{
  patch->op[2].pitch.coarse = value / 100;
  patch->op[2].pitch.fine = value % 100;
  dblOp3Pitch->setValue(value / 100.0);
  updateWaveformPreview();
}

void InstrumentEditor::on_dblOp3Pitch_valueChanged(double value)
{
  dialOp3Pitch->setValue((int)(value * 100));
  updateWaveformPreview();
}

void InstrumentEditor::on_numOp3Detune_valueChanged(int value)
{
  patch->op[2].detune = value;
  updateWaveformPreview();
}

void InstrumentEditor::on_numOp3Attack_valueChanged(int value)
{
  patch->op[2].attack = value;
  updateWaveformPreview();
}

void InstrumentEditor::on_numOp3Decay_valueChanged(int value)
{
  patch->op[2].decay = value;
  updateWaveformPreview();
}

void InstrumentEditor::on_numOp3Sustain_valueChanged(int value)
{
  patch->op[2].sustain = value;
  updateWaveformPreview();
}

void InstrumentEditor::on_sldrOp3Loop_valueChanged(int value)
{
  patch->op[2].loop = value == 1;
  updateWaveformPreview();
}

void InstrumentEditor::on_numOp4Level_valueChanged(int value)
{
  patch->op[3].level = value;
  updateWaveformPreview();
}

void InstrumentEditor::on_btnOp4Fixed_toggled(bool on)
{
  patch->op[3].pitch.fixed = on;
  if (on)
    btnOp4Fixed->setText("FIXED");
  else
    btnOp4Fixed->setText("RATIO");
  updateWaveformPreview();
}

void InstrumentEditor::on_dialOp4Pitch_valueChanged(int value)
{
  patch->op[3].pitch.coarse = value / 100;
  patch->op[3].pitch.fine = value % 100;
  dblOp4Pitch->setValue(value / 100.0);
  updateWaveformPreview();
}

void InstrumentEditor::on_dblOp4Pitch_valueChanged(double value)
{
  dialOp4Pitch->setValue((int)(value * 100));
  updateWaveformPreview();
}

void InstrumentEditor::on_numOp4Detune_valueChanged(int value)
{
  patch->op[3].detune = value;
  updateWaveformPreview();
}

void InstrumentEditor::on_numOp4Attack_valueChanged(int value)
{
  patch->op[3].attack = value;
  updateWaveformPreview();
}

void InstrumentEditor::on_numOp4Decay_valueChanged(int value)
{
  patch->op[3].decay = value;
  updateWaveformPreview();
}

void InstrumentEditor::on_numOp4Sustain_valueChanged(int value)
{
  patch->op[3].sustain = value;
  updateWaveformPreview();
}

void InstrumentEditor::on_sldrOp4Loop_valueChanged(int value)
{
  patch->op[3].loop = value == 1;
  updateWaveformPreview();
}

void InstrumentEditor::on_wKeyboard_notePressed(int midikey)
{
  source->noteOn(0, *patch, midikey, -1);
  if (audio->state() != QAudio::ActiveState || audio->state() != QAudio::IdleState)
    audio->start(source);
}

void InstrumentEditor::on_wKeyboard_noteReleased()
{
  source->noteOff(0);
}

void InstrumentEditor::loadPatchValues()
{
  leName->setText(patch->name);
  lblAlgorithm->setPixmap(QPixmap(QString(":/images/algo%1.png").arg(patch->algorithm)));
  numPatchVolume->setValue(patch->volume);
  numPatchFeedback->setValue(patch->feedback - 50);
  numPatchGlide->setValue(patch->glide);
  numPatchAttack->setValue(patch->attack);
  numPatchDecay->setValue(patch->decay);
  numPatchSustain->setValue(patch->sustain);
  numPatchRelease->setValue(patch->release);
  numPatchLFOSpeed->setValue(patch->lfo.speed);
  numPatchLFOAttack->setValue(patch->lfo.attack);
  numPatchLFODepth->setValue(patch->lfo.pmd);
  numOp1Level->setValue(patch->op[0].level);
  btnOp1Fixed->setChecked(patch->op[0].pitch.fixed);
  dblOp1Pitch->setValue(patch->op[0].pitch.coarse + patch->op[0].pitch.fine / 100.0);
  numOp1Detune->setValue(patch->op[0].detune);
  numOp1Attack->setValue(patch->op[0].attack);
  numOp1Decay->setValue(patch->op[0].decay);
  numOp1Sustain->setValue(patch->op[0].sustain);
  sldrOp1Loop->setValue(patch->op[0].loop ? 1:0);
  numOp2Level->setValue(patch->op[1].level);
  btnOp2Fixed->setChecked(patch->op[1].pitch.fixed);
  dblOp2Pitch->setValue(patch->op[1].pitch.coarse + patch->op[1].pitch.fine / 100.0);
  numOp2Detune->setValue(patch->op[1].detune);
  numOp2Attack->setValue(patch->op[1].attack);
  numOp2Decay->setValue(patch->op[1].decay);
  numOp2Sustain->setValue(patch->op[1].sustain);
  sldrOp2Loop->setValue(patch->op[1].loop ? 1:0);
  numOp3Level->setValue(patch->op[2].level);
  btnOp3Fixed->setChecked(patch->op[2].pitch.fixed);
  dblOp3Pitch->setValue(patch->op[2].pitch.coarse + patch->op[2].pitch.fine / 100.0);
  numOp3Detune->setValue(patch->op[2].detune);
  numOp3Attack->setValue(patch->op[2].attack);
  numOp3Decay->setValue(patch->op[2].decay);
  numOp3Sustain->setValue(patch->op[2].sustain);
  sldrOp3Loop->setValue(patch->op[2].loop ? 1:0);
  numOp4Level->setValue(patch->op[3].level);
  btnOp4Fixed->setChecked(patch->op[3].pitch.fixed);
  dblOp4Pitch->setValue(patch->op[3].pitch.coarse + patch->op[3].pitch.fine / 100.0);
  numOp4Detune->setValue(patch->op[3].detune);
  numOp4Attack->setValue(patch->op[3].attack);
  numOp4Decay->setValue(patch->op[3].decay);
  numOp4Sustain->setValue(patch->op[3].sustain);
  sldrOp4Loop->setValue(patch->op[3].loop ? 1:0);
}

void InstrumentEditor::updateWaveformPreview()
{
  QPixmap pixmap(8000, 256);
  QPainter painter;
  FMSynth::Voice<8000> voice;
  uint8_t previous;
  voice.noteOn(*patch, 60, 127);
  painter.begin(&pixmap);
  painter.fillRect(0, 0, 8000, 256, QColor(192, 192, 192));
  painter.setPen(QColor(0, 0, 255));
  previous = voice.update();
  for (int i = 1; i < 8000; ++i)
  {
    uint8_t next = voice.update();
    if (i == 4000)
      voice.noteOff();
    painter.drawLine(i - 1, previous, i, next);
    previous = next;
  }
  painter.end();
  lblWaveform->setPixmap(pixmap);
}
