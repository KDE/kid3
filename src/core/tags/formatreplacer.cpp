/**
 * \file formatreplacer.cpp
 * Replaces format codes in a string.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 06 Jul 2008
 *
 * Copyright (C) 2008-2024  Urs Fleisch
 *
 * This file is part of Kid3.
 *
 * Kid3 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Kid3 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "formatreplacer.h"
#include <QUrl>
#include "saferename.h"

/**
 * Constructor.
 *
 * @param str string with format codes
 */
FormatReplacer::FormatReplacer(const QString& str) : m_str(str) {}

/**
 * Destructor.
 */
FormatReplacer::~FormatReplacer()
{
}

/**
 * Replace escaped characters.
 * Replaces the escaped characters ("\n", "\t", "\r", "\\", "\a", "\b",
 * "\f", "\v") with the corresponding characters.
 */
void FormatReplacer::replaceEscapedChars()
{
  if (!m_str.isEmpty()) {
    constexpr int numEscCodes = 8;
    constexpr QChar escCode[numEscCodes] = {
      QLatin1Char('n'), QLatin1Char('t'), QLatin1Char('r'), QLatin1Char('\\'),
      QLatin1Char('a'), QLatin1Char('b'), QLatin1Char('f'), QLatin1Char('v')};
    constexpr char escChar[numEscCodes] = {
      '\n', '\t', '\r', '\\', '\a', '\b', '\f', '\v'};

    for (int pos = 0; pos < m_str.length();) {
      pos = m_str.indexOf(QLatin1Char('\\'), pos);
      if (pos == -1) break;
      ++pos;
      for (int k = 0;; ++k) {
        if (k >= numEscCodes) {
          // invalid code at pos
          ++pos;
          break;
        }
        if (m_str[pos] == escCode[k]) {
          // code found, replace it
          m_str.replace(pos - 1, 2, QLatin1Char(escChar[k]));
          break;
        }
      }
    }
  }
}

/**
 * Replace percent codes.
 *
 * @param flags flags: FSF_SupportUrlEncode to support modifier u
 *              (with code c "%uc") to URL encode,
 *              FSF_ReplaceSeparators to replace directory separators
 *              ('/', '\\', ':') in tags,
 *              FSF_SupportHtmlEscape to support modifier h
 *              (with code c "%hc") to replace HTML metacharacters
 *              ('<', '>', '&', '"', ''', non-ascii) in tags.
 */
void FormatReplacer::replacePercentCodes(unsigned flags)
{
  if (!m_str.isEmpty()) {
    for (int pos = 0; pos < m_str.length();) {
      pos = m_str.indexOf(QLatin1Char('%'), pos);
      if (pos == -1) break;

      int codePos = pos + 1;
      int codeLen = 0;
      QString prefix, postfix;
      bool urlEncode = false;
      bool htmlEscape = false;
      QString repl;
      if ((flags & FSF_SupportUrlEncode) && m_str[codePos] == QLatin1Char('u')) {
        ++codePos;
        urlEncode = true;
      }
      if ((flags & FSF_SupportHtmlEscape) && m_str[codePos] == QLatin1Char('h')) {
        ++codePos;
        htmlEscape = true;
      }
      if (m_str[codePos] == QLatin1Char('{')) {
        if (int closingBracePos = m_str.indexOf(QLatin1Char('}'), codePos + 1);
            closingBracePos > codePos + 1) {
          QString longCode =
            m_str.mid(codePos + 1, closingBracePos - codePos - 1).toLower();
          if (longCode.startsWith(QLatin1Char('"'))) {
            if (int prefixEnd = longCode.indexOf(QLatin1Char('"'), 1);
                prefixEnd != -1 && prefixEnd < longCode.length() - 2) {
              prefix = longCode.mid(1, prefixEnd - 1);
              longCode.remove(0, prefixEnd + 1);
            }
          }
          if (longCode.endsWith(QLatin1Char('"'))) {
            if (int postfixStart = longCode.lastIndexOf(QLatin1Char('"'), -2);
                postfixStart > 1) {
              postfix = longCode.mid(postfixStart + 1,
                                     longCode.length() - postfixStart - 2);
              longCode.truncate(postfixStart);
            }
          }
          repl = getReplacement(longCode);
          codeLen = closingBracePos - pos + 1;
        }
      } else {
        repl = getReplacement(QString(m_str[codePos]));
        codeLen = codePos - pos + 1;
      }

      if (codeLen > 0) {
        if (flags & FSF_ReplaceSeparators) {
#ifdef Q_OS_WIN32
          static constexpr char illegalChars[] = "<>:\"|?*\\/";
#else
          // ':' and '\' are included in the set of illegal characters to
          // keep the old behavior when no string replacement is enabled.
          static constexpr char illegalChars[] = ":\\/";
#endif
          Utils::replaceIllegalFileNameCharacters(repl, QLatin1String("-"),
                                                  illegalChars);
        }
        if (urlEncode) {
          repl = QString::fromLatin1(QUrl::toPercentEncoding(repl));
        }
        if (htmlEscape) {
          repl = escapeHtml(repl);
        }
        if (!repl.isEmpty()) {
          if (!prefix.isEmpty()) {
            repl = prefix + repl;
          }
          if (!postfix.isEmpty()) {
            repl += postfix;
          }
        }
        if (!repl.isNull() || codeLen > 2) {
          m_str.replace(pos, codeLen, repl);
          pos += repl.length();
        } else {
          ++pos;
        }
      } else {
        ++pos;
      }
    }
  }
}

/**
 * Converts the plain text string @a plain to a HTML string with
 * HTML metacharacters replaced by HTML entities.
 * @param plain plain text
 * @return html text with HTML entities.
 */
QString FormatReplacer::escapeHtml(const QString& plain)
{
  QString rich;
  rich.reserve(static_cast<int>(plain.length() * 1.1));
  for (int i = 0; i < plain.length(); ++i) {
    if (ushort ch = plain.at(i).unicode(); ch == '<')
      rich += QLatin1String("&lt;");
    else if (ch == '>')
      rich += QLatin1String("&gt;");
    else if (ch == '&')
      rich += QLatin1String("&amp;");
    else if (ch == '"')
      rich += QLatin1String("&quot;");
    else if (ch == '\'')
      rich += QLatin1String("&apos;");
    else if (ch >= 128)
      rich += QString(QLatin1String("&#%1;")).arg(ch);
    else
      rich += plain.at(i);
  }
  return rich;
}
