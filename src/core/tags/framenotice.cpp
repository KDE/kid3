/**
 * \file framenotice.cpp
 * Warning about tag frame.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 19 May 2017
 *
 * Copyright (C) 2017  Urs Fleisch
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

#include "framenotice.h"
#include <QCoreApplication>

/**
 * Get translated description of notice.
 * @return description, empty if none.
 */
QString FrameNotice::getDescription() const
{
  static const char* const descriptions[] = {
    "",
    QT_TRANSLATE_NOOP("@default", "Truncated"),
    QT_TRANSLATE_NOOP("@default", "Size is too large"),
    QT_TRANSLATE_NOOP("@default", "Value is too long"),
    QT_TRANSLATE_NOOP("@default", "Must be unique"),
    QT_TRANSLATE_NOOP("@default", "New line is forbidden"),
    QT_TRANSLATE_NOOP("@default", "Carriage return is forbidden"),
    QT_TRANSLATE_NOOP("@default", "Field must be non-empty"),
    QT_TRANSLATE_NOOP("@default", "Must be numeric"),
    QT_TRANSLATE_NOOP("@default", "Must be numeric or number/total"),
    QT_TRANSLATE_NOOP("@default", "Format is DDMM"),
    QT_TRANSLATE_NOOP("@default", "Format is HHMM"),
    QT_TRANSLATE_NOOP("@default", "Format is YYYY"),
    QT_TRANSLATE_NOOP("@default", "Must be ISO 8601 date/time"),
    QT_TRANSLATE_NOOP("@default", "Must be musical key, 3 characters, A-G, b, #, m, o"),
    QT_TRANSLATE_NOOP("@default", "Must be ISO 639-2 language code, 3 lowercase characters"),
  };
  struct not_used { int array_size_check[
      sizeof(descriptions) / sizeof(descriptions[0]) == NumWarnings
      ? 1 : -1 ]; };
  return m_warning < NumWarnings
      ? QCoreApplication::translate("@default", descriptions[m_warning])
      : QString();
}
