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

#ifndef NOTESPINBOX_H
#define NOTESPINBOX_H

#include <QSpinBox>

class NoteSpinBox : public QSpinBox
{
  Q_OBJECT
  public:
    NoteSpinBox(QWidget *parent=nullptr) : QSpinBox(parent) {}
    ~NoteSpinBox() {}
  protected:
    QString textFromValue(int value) const
    {
      int numerator = value + 1;
      int denominator = 128;
      int gcf = numerator;
      while (numerator % gcf != 0 || denominator % gcf != 0)
        --gcf;
      return QString("%1/%2").arg(numerator / gcf).arg(denominator / gcf);
    }
    int valueFromText(const QString &text) const
    {
      int numerator = text.section('/', 0, 0).toInt();
      int denominator = text.section('/', 1, 1).toInt();
      int factor = 128 / denominator;
      return numerator * factor - 1;
    }
};

#endif //NOTESPINBOX_H
