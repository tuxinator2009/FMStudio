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

#ifndef CHEADERVALUE_H
#define CHEADERVALUE_H

#include "cheaderarray.h"
#include "cheaderobject.h"

class CHeaderValue
{
  public:
    enum class Type
    {
      Null,
      String,
      Int,
      Double,
      Bool,
      Array,
      Object
    };
    friend class CHeaderParser;
    CHeaderValue();
    CHeaderValue(const CHeaderValue &other);
    ~CHeaderValue();
    const char *toString() const;
    int toInt() const;
    double toDouble() const;
    bool toBool() const;
    const CHeaderArray &toArray() const;
    const CHeaderObject &toObject() const;
    void print(int indent=0, QString name="") const;
    CHeaderValue &operator=(const CHeaderValue &other);
  private:
    CHeaderValue(QString value);
    CHeaderValue(int value);
    CHeaderValue(double value);
    CHeaderValue(bool value);
    CHeaderValue(const CHeaderArray &value);
    CHeaderValue(const CHeaderObject &value);
    union
    {
      char *stringValue;
      int intValue;
      double doubleValue;
      bool boolValue;
      CHeaderArray *arrayValue;
      CHeaderObject *objectValue;
    } value;
    Type type;
};

#endif //CHEADERVALUE_H
