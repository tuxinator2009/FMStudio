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

#ifndef FMSOURCE_H
#define FMSOURCE_H

#include <QIODevice>
#include "FMSynth/Voice.h"
#include "fmsong.h"

class FMSource : public QIODevice
{
  public:
    FMSource(int numChannels);
    ~FMSource();
    void setTempo(uint32_t tempo);
    int getBaseTempo();
    void playSong(FMSong *song);
    void stopSong();
    void playPattern(int channel, const QList<FMSong::Note> &notes, const FMSynth::Patch &patch);
    void stopPattern(int channel);
    void noteOn(int channel, const FMSynth::Patch &patch, uint8_t note, int duration, uint8_t velocity=127);
    bool atEnd() const override;
  public slots:
    void noteOff(int channel);
    inline uint32_t samples(int duration) {return (_tempo * (duration + 1)) / 32;}
  protected:
    qint64 readData(char *data, qint64 maxSize) override;
    qint64 writeData(const char *data, qint64 maxSize) override;
  private:
    struct Voice
    {
      Voice() {}
      Voice(const Voice&) = delete;
      Voice& operator=(const Voice&) = delete;
      FMSynth::Voice<8000> synth;
      uint32_t samples;
    };
    struct Channel
    {
      QList<Voice*> voices;
      QList<FMSong::Note> notes;
      FMSynth::Patch patch;
      uint32_t offset;
      int section;
    };
    inline uint8_t mix(uint8_t a, uint8_t b)
    {
      int32_t v = (int32_t)a + (int32_t)b - 127;
      if(v < 0) return 0;
      if(v > 0xFF) return 0xFF;
      return v;
    }
    Channel *_channels;
    FMSong *_song;
    uint32_t _tempo;
    uint32_t _sample;
    int _numChannels;
    int baseTempo;
};

#endif //FMSOURCE_H
