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

#include <QComboBox>
#include <QLineEdit>
#include "globals.h"
#include "newinstrument.h"

NewInstrument::NewInstrument(QWidget *parent) : QDialog(parent)
{
  setupUi(this);
  for (int i = 0; i < Globals::patches.size(); ++i)
    optBase->addItem(Globals::patches[i].name);
  leName->setFocus();
}

NewInstrument::~NewInstrument()
{
}

FMSynth::Patch NewInstrument::getPatch()
{
  FMSynth::Patch patch = Globals::patches[optBase->currentIndex()];
  strcpy(patch.name, leName->text().toLocal8Bit().data());
  return patch;
}
