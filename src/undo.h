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

#ifndef UNDO_H
#define UNDO_H

#include "fmsong.h"

class Undo
{
  public:
    enum class Type
    {
      AddSection,
      DeleteSection,
      MoveSection,
      ChangeInstrument,
      AddPattern,
      DeletePattern,
      AddNote,
      DeleteNote,
      MoveNote,
      ResizeNote
    };
    struct AddSection
    {
      int section;
    };
    struct DeleteSection
    {
      FMSong::Section section;
      int channel;
    };
    struct MoveSection
    {
      int section;
      int oldChannel;
      int oldOffset;
    };
    struct ChangeInstrument
    {
      int channel;
      int section;
      int oldInstrument;
    };
    struct AddPattern
    {
      int pattern;
    };
    struct DeletePattern
    {
      FMSong::Pattern pattern;
      QList<DeleteSection> sections;
    };
    struct AddNote
    {
      int pattern;
      int note;
    };
    struct DeleteNote
    {
      FMSong::Note note;
      int pattern;
    };
    struct MoveNote
    {
      int pattern;
      int note;
      int oldOffset;
      int oldMidikey;
    };
    struct ResizeNote
    {
      int pattern;
      int note;
      int oldDuration;
    };
    struct Step
    {
      Step(Type t, void *d)
      {
        type = t;
        data = d;
      }
      Type type;
      void *data;
    };
    Undo();
    ~Undo();
    void addUndoStep(Type type, void *data);
    void addRedoStep(Type type, void *data);
    Step undo();
    Step redo();
    bool undoAvailable();
    bool redoAvailable();
  private:
    QList<Step> undoSteps;
    QList<Step> redoSteps;
};

#endif //UNDO
