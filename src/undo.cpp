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

#include "fmsong.h"
#include "undo.h"

Undo::Undo()
{
}

Undo::~Undo()
{
  for (auto step : undoSteps)
  {
    if (step.type == Type::AddSection)
      delete (AddSection*)step.data;
    else if (step.type == Type::DeleteSection)
      delete (DeleteSection*)step.data;
    else if (step.type == Type::MoveSection)
      delete (MoveSection*)step.data;
    else if (step.type == Type::ChangeInstrument)
      delete (ChangeInstrument*)step.data;
    else if (step.type == Type::AddPattern)
      delete (AddPattern*)step.data;
    else if (step.type == Type::DeletePattern)
      delete (DeletePattern*)step.data;
    else if (step.type == Type::AddNote)
      delete (AddNote*)step.data;
    else if (step.type == Type::DeleteNote)
      delete (DeleteNote*)step.data;
    else if (step.type == Type::MoveNote)
      delete (MoveNote*)step.data;
    else if (step.type == Type::ResizeNote)
      delete (ResizeNote*)step.data;
  }
  for (auto step : redoSteps)
  {
    if (step.type == Type::AddSection)
      delete (AddSection*)step.data;
    else if (step.type == Type::DeleteSection)
      delete (DeleteSection*)step.data;
    else if (step.type == Type::MoveSection)
      delete (MoveSection*)step.data;
    else if (step.type == Type::ChangeInstrument)
      delete (ChangeInstrument*)step.data;
    else if (step.type == Type::AddPattern)
      delete (AddPattern*)step.data;
    else if (step.type == Type::DeletePattern)
      delete (DeletePattern*)step.data;
    else if (step.type == Type::AddNote)
      delete (AddNote*)step.data;
    else if (step.type == Type::DeleteNote)
      delete (DeleteNote*)step.data;
    else if (step.type == Type::MoveNote)
      delete (MoveNote*)step.data;
    else if (step.type == Type::ResizeNote)
      delete (ResizeNote*)step.data;
  }
}

void Undo::addUndoStep(Type type, void *data)
{
  undoSteps += Step(type, data);
  redoSteps.clear();
}

void Undo::addRedoStep(Type type, void *data)
{
  redoSteps += Step(type, data);
}

Undo::Step Undo::undo()
{
  return undoSteps.takeLast();
}

Undo::Step Undo::redo()
{
  return redoSteps.takeLast();
}

bool Undo::undoAvailable()
{
  return undoSteps.size() > 0;
}

bool Undo::redoAvailable()
{
  return redoSteps.size() > 0;
}
