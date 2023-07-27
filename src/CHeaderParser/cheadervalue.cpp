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

#include "cheaderarray.h"
#include "cheaderobject.h"
#include "cheadervalue.h"

CHeaderValue::CHeaderValue()
{
  type = Type::Null;
}

CHeaderValue::CHeaderValue(const CHeaderValue &other)
{
  if (other.type == Type::String)
  {
    int len = strlen(other.value.stringValue);
    this->value.stringValue = new char[len + 1];
    for (int i = 0; i < len; ++i)
      this->value.stringValue[i] = other.value.stringValue[i];
    this->value.stringValue[len] = '\0';
  }
  else if (other.type == Type::Int)
    this->value.intValue = other.value.intValue;
  else if (other.type == Type::Double)
    this->value.doubleValue = other.value.doubleValue;
  else if (other.type == Type::Bool)
    this->value.boolValue = other.value.boolValue;
  else if (other.type == Type::Array)
    this->value.arrayValue = new CHeaderArray(*other.value.arrayValue);
  else if (other.type == Type::Object)
    this->value.objectValue = new CHeaderObject(*other.value.objectValue);
  this->type = other.type;
}

CHeaderValue::~CHeaderValue()
{
  if (type == Type::String)
    delete[] value.stringValue;
  else if (type == Type::Array)
    delete value.arrayValue;
  else if (type == Type::Object)
    delete value.objectValue;
}

const char *CHeaderValue::toString() const
{
  return value.stringValue;
}

int CHeaderValue::toInt() const
{
  return value.intValue;
}

double CHeaderValue::toDouble() const
{
  return value.doubleValue;
}

bool CHeaderValue::toBool() const
{
  return value.boolValue;
}

const CHeaderArray &CHeaderValue::toArray() const
{
  return *value.arrayValue;
}

const CHeaderObject &CHeaderValue::toObject() const
{
  return *value.objectValue;
}

void CHeaderValue::print(int indent, QString name) const
{
  for (int i = 0; i < indent; ++i)
    printf("  ");
  if (!name.isEmpty())
    printf("%s=", name.toLocal8Bit().data());
  if (type == Type::String)
    printf("\"%s\"\n", toString());
  else if (type == Type::Int)
    printf("%d\n", toInt());
  else if (type == Type::Double)
    printf("%f\n", toDouble());
  else if (type == Type::Bool)
    printf("%s\n", toBool() ? "true":"false");
  else if (type == Type::Array)
  {
    printf("Array\n");
    toArray().print(indent + 1);
  }
  else if (type == Type::Object)
  {
    printf("Object\n");
    toObject().print(indent + 1);
  }
}

CHeaderValue &CHeaderValue::operator=(const CHeaderValue &other)
{
  if (other.type == Type::String)
  {
    int len = strlen(other.value.stringValue);
    this->value.stringValue = new char[len + 1];
    for (int i = 0; other.value.stringValue[i] != '\0'; ++i)
      this->value.stringValue[i] = other.value.stringValue[i];
    this->value.stringValue[len] = '\0';
  }
  else if (other.type == Type::Int)
    this->value.intValue = other.value.intValue;
  else if (other.type == Type::Double)
    this->value.doubleValue = other.value.doubleValue;
  else if (other.type == Type::Bool)
    this->value.boolValue = other.value.boolValue;
  else if (other.type == Type::Array)
    this->value.arrayValue = new CHeaderArray(*other.value.arrayValue);
  else if (other.type == Type::Object)
    this->value.objectValue = new CHeaderObject(*other.value.objectValue);
  this->type = other.type;
  return *this;
}

CHeaderValue::CHeaderValue(QString value)
{
  char *s = new char[value.length() + 1];
  for (int i = 0; i < value.length(); ++i)
    s[i] = value[i].toLatin1();
  s[value.length()] = '\0';
  this->value.stringValue = s;
  this->type = Type::String;
}

CHeaderValue::CHeaderValue(int value)
{
  this->value.intValue = value;
  this->type = Type::Int;
}

CHeaderValue::CHeaderValue(double value)
{
  this->value.doubleValue = value;
  this->type = Type::Double;
}

CHeaderValue::CHeaderValue(bool value)
{
  this->value.boolValue = value;
  this->type = Type::Bool;
}

CHeaderValue::CHeaderValue(const CHeaderArray &value)
{
  this->value.arrayValue = new CHeaderArray(value);
  this->type = Type::Array;
}

CHeaderValue::CHeaderValue(const CHeaderObject &value)
{
  this->value.objectValue = new CHeaderObject(value);
  this->type = Type::Object;
}
