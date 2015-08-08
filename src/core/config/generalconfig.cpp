/**
 * \file generalconfig.cpp
 * General configuration.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
 *
 * Copyright (C) 2003-2007  Urs Fleisch
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

#include "generalconfig.h"
#include <QStringList>

/**
 * Constructor.
 * Set default configuration.
 *
 * @param grp configuration group
 */
GeneralConfig::GeneralConfig(const QString& grp) : m_group(grp) {}

/**
 * Destructor.
 */
GeneralConfig::~GeneralConfig() {}

/**
 * Convert list of integers to list of strings.
 * @param intList list of integers
 * @return list of strings.
 */
QStringList GeneralConfig::intListToStringList(const QList<int>& intList)
{
  QStringList result;
  foreach (int value, intList) {
    result.append(QString::number(value));
  }
  return result;
}

/**
 * Convert list of strings to list of integers.
 * @param strList list of strings
 * @return list of integers.
 */
QList<int> GeneralConfig::stringListToIntList(const QStringList& strList)
{
  QList<int> result;
  foreach (const QString& value, strList) {
    result.append(value.toInt());
  }
  return result;
}
