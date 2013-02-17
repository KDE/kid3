/**
 * \file batchimportprofile.cpp
 * Profile containing a name list for source for batch import.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 2 Jan 2013
 *
 * Copyright (C) 2013  Urs Fleisch
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

#include "batchimportprofile.h"
#include <QStringList>

/**
 * Constructor.
 */
BatchImportProfile::BatchImportProfile()
{
}

/**
 * Destructor.
 */
BatchImportProfile::~BatchImportProfile()
{
}

/**
 * Restore batch import sources from serialized string.
 * @param str string representation of import sources
 */
void BatchImportProfile::setSourcesFromString(const QString& str)
{
  m_sources.clear();
  if (!str.isEmpty()) {
    QStringList srcStrs = str.split(QLatin1Char(';'));
    foreach (const QString& srcStr, srcStrs) {
      QStringList propStrs = srcStr.split(QLatin1Char(':'));
      Source src;
      if (propStrs.size() > 0)
        src.setName(propStrs.at(0));
      if (propStrs.size() > 1)
        src.setRequiredAccuracy(propStrs.at(1).toInt());
      if (propStrs.size() > 2) {
        const QString& enableStr = propStrs.at(2);
        src.enableStandardTags(enableStr.contains(QLatin1Char('S')));
        src.enableAdditionalTags(enableStr.contains(QLatin1Char('A')));
        src.enableCoverArt(enableStr.contains(QLatin1Char('C')));
      }
      m_sources.append(src);
    }
  }
}

/**
 * Serialize batch import sources as a string.
 * @return string representation of import sources.
 */
QString BatchImportProfile::getSourcesAsString() const
{
  QStringList strs;
  foreach (const Source& src, m_sources) {
    QString enableStr;
    if (src.standardTagsEnabled())   enableStr += QLatin1Char('S');
    if (src.additionalTagsEnabled()) enableStr += QLatin1Char('A');
    if (src.coverArtEnabled())       enableStr += QLatin1Char('C');
    strs.append(src.getName() + QLatin1Char(':') +
                QString::number(src.getRequiredAccuracy()) + QLatin1Char(':') +
                enableStr);
  }
  return strs.join(QLatin1String(";"));
}
