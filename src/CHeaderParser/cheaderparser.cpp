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

#include <QRegularExpression>
#include "cheaderarray.h"
#include "cheaderobject.h"
#include "cheaderparser.h"
#include "cheadervalue.h"

const QRegularExpression CHeaderParser::regexInt = QRegularExpression("^[0-9]+$");
const QRegularExpression CHeaderParser::regexDouble = QRegularExpression("^[0-9]+\\.[0-9]+$");
const QRegularExpression CHeaderParser::regexBool = QRegularExpression("^(true)|(false)$");
const QRegularExpression CHeaderParser::regexVariable = QRegularExpression("\\.([a-zA-Z_][a-zA-Z0-9_]{0,})=");

CHeaderValue CHeaderParser::parseCHeader(QString cheader)
{
  int start = cheader.indexOf('{');
  int end = cheader.lastIndexOf('}');
  return parseBlock(cheader.mid(start + 1, end - start - 1)); //parse everything between the two braces
}

bool CHeaderParser::isBlockArray(QString block)
{
  int level = 0;
  for (int i = 0; i < block.length(); ++i)
  {
    if (block[i] == '{')
      ++level;
    else if (block[i] == '}')
      --level;
    else if (block[i] == '=' && level == 0)
      return false;
  }
  return true;
}

QString CHeaderParser::getValue(QString block, int *index)
{
  int start = *index;
  int level = 0;
  bool inString = false;
  for (int i = *index; i < block.length(); ++i)
  {
    if (block[i] == '{' && !inString)
      ++level;
    else if (block[i] == '}' && !inString)
      --level;
    else if (block[i] == '"')
      inString = !inString;
    else if (block[i] == ',' && level == 0)
    {
      *index = i + 1;
      return block.mid(start, i - start);
    }
    else if (level == -1)
    {
      *index = i;
      return block.mid(start, i - start);
    }
  }
  *index = block.length();
  return block.mid(start);
}

CHeaderValue CHeaderParser::parseBlock(QString block)
{
  if (isBlockArray(block))
    return parseArray(block);
  return parseObject(block);
}

CHeaderValue CHeaderParser::parseArray(QString block)
{
  CHeaderArray array;
  int index = 0;
  while (index < block.length())
  {
    QString value = getValue(block, &index);
    QString condensed = value.simplified().remove(' ');
    if (value.isEmpty())
      exit(0);
    if (condensed[0] == '{')
    {
      int start = value.indexOf('{');
      int end = value.lastIndexOf('}');
      array.addValue(parseBlock(value.mid(start + 1, end - start - 1)));
    }
    if (condensed[0] == '"')
    {
      int start = value.indexOf("\"");
      int end = value.lastIndexOf("\"");
      array.addValue(value.mid(start + 1, end - start - 1));
    }
    else if (condensed.contains(regexDouble))
      array.addValue(condensed.toDouble());
    else if (condensed.contains(regexInt))
      array.addValue(condensed.toInt());
    else if (condensed.contains(regexBool))
      array.addValue(condensed == "true");
  }
  return CHeaderValue(array);
}

CHeaderValue CHeaderParser::parseObject(QString block)
{
  CHeaderObject object;
  QRegularExpressionMatch match;
  int index = 0;
  while ((index = block.indexOf(regexVariable, index, &match)) != -1)
  {
    index = match.capturedEnd();
    QString key = match.captured(1);
    QString value = getValue(block, &index);
    QString condensed = value.simplified().remove(' ');
    if (condensed[0] == '{')
    {
      int start = value.indexOf('{');
      int end = value.lastIndexOf('}');
      object.insert(key, parseBlock(value.mid(start + 1, end - start - 1)));
    }
    else if (condensed[0] == '"')
    {
      int start = value.indexOf("\"");
      int end = value.lastIndexOf("\"");
      object.insert(key, value.mid(start + 1, end - start - 1));
    }
    else if (condensed.contains(regexDouble))
      object.insert(key, condensed.toDouble());
    else if (condensed.contains(regexInt))
      object.insert(key, condensed.toInt());
    else if (condensed.contains(regexBool))
      object.insert(key, condensed == "true");
  }
  return CHeaderValue(object);
}
