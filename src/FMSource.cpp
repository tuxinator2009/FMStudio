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

#include <QIODevice>
#include "FMSynth/Voice.h"
#include "fmproject.h"
#include "fmsong.h"
#include "FMSource.h"
#include "globals.h"

FMSource::FMSource(int numChannels)
{
  open(QIODevice::ReadOnly);
  _numChannels = numChannels;
  _channels = new Channel[numChannels];
  _song = nullptr;
}

FMSource::~FMSource()
{
  close();
  delete[] _channels;
  _song = nullptr;
}

void FMSource::setTempo(uint32_t tempo)
{
  _tempo = 8 * 60000 / tempo;
  baseTempo = tempo;
}

int FMSource::getBaseTempo()
{
  return baseTempo;
}

void FMSource::playSong(FMSong *song)
{
  _song = song;
  for (int i = 0; i < 4; ++i)
    _channels[i].section = 0;
  _sample = 0;
}

void FMSource::stopSong()
{
  for (int i = 0; i < _numChannels; ++i)
  {
    Channel &channel = _channels[i];
    for (auto voice : channel.voices)
    {
      voice->synth.noteOff();
      voice->samples = 0;
    }
    channel.notes.clear();
    channel.section = -1;
  }
  _song = nullptr;
}

void FMSource::playPattern(int channel, const QList<FMSong::Note> &notes, const FMSynth::Patch &patch)
{
  _channels[channel].notes = notes;
  _channels[channel].patch = patch;
  _channels[channel].offset = 0;
  _sample = 0;
}

void FMSource::stopPattern(int channel)
{
  _channels[channel].notes.clear();
  noteOff(channel);
}

void FMSource::noteOn(int channel, const FMSynth::Patch &patch, uint8_t note, int duration, uint8_t velocity)
{
  bool foundOne = false;
  for (auto voice : _channels[channel].voices)
  {
    if (voice->samples == 0)
    {
      voice->synth.noteOn(patch, note, velocity);
      voice->samples = samples(duration);
      foundOne = true;
      break;
    }
  }
  if (!foundOne)
  {
    Voice *voice = new Voice;
    voice->synth.noteOn(patch, note, velocity);
    voice->samples = samples(duration);
    _channels[channel].voices += voice;
  }
}

bool FMSource::atEnd() const
{
  for (int i = 0; i < _numChannels; ++i)
  {
    Channel &channel = _channels[i];
    for (auto voice : channel.voices)
    {
      if (!voice->synth.released())
        return false;
      else if (voice->samples > 0)
        return false;
    }
    if (channel.notes.size() > 0)
      return false;
    if (_song != nullptr && i < 4)
    {
      if (channel.section < _song->numSections(i))
        return false;
    }
  }
  return true;
}

void FMSource::noteOff(int channel)
{
  for (auto voice : _channels[channel].voices)
  {
    voice->synth.noteOff();
    voice->samples = 0;
  }
}

qint64 FMSource::readData(char *data, qint64 maxSize)
{
  qint64 bufferSize = (maxSize > 512) ? 512:maxSize;
  for (qint64 i = 0; i < bufferSize; ++i)
  {
    uint8_t byte = 128;
    bool first = true;
    for (int i = 0; i < _numChannels; ++i)
    {
      Channel &channel = _channels[i];
      if (_song != nullptr && i < 4)
      {
        if (channel.section < _song->numSections(i))
        {
          FMSong::Section *section = _song->getSection(i, channel.section);
          if (_sample == samples(section->offset - 1))
          {
            channel.notes = section->pattern->notes;
            channel.patch = *section->instrument;
            channel.offset = _sample;
            ++channel.section;
          }
        }
      }
      while (channel.notes.size() > 0 && _sample == samples(channel.notes[0].offset - 1) + channel.offset)
      {
        FMSong::Note note = channel.notes.takeFirst();
        noteOn(i, channel.patch, note.midikey, note.duration, note.velocity);
      }
      for (auto voice : channel.voices)
      {
        if (!voice->synth.released())
        {
          if (voice->samples > 0)
          {
            --voice->samples;
            if (voice->samples == 0)
              voice->synth.noteOff();
          }
          if (first)
            byte = voice->synth.update();
          else
            byte = mix(byte, voice->synth.update());
          first = false;
        }
      }
    }
    *data++ = (char)byte;
    ++_sample;
  }
  return bufferSize;
}

qint64 FMSource::writeData(const char *data, qint64 maxSize)
{
  Q_UNUSED(data);
  Q_UNUSED(maxSize);
  return 0;
}
