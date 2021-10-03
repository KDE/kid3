/**
 * \file batchimportconfig.cpp
 * Configuration for batch import.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 3 Jan 2013
 *
 * Copyright (C) 2013-2018  Urs Fleisch
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

#include "batchimportconfig.h"
#include <QString>
#include "batchimportprofile.h"
#include "isettings.h"
#include "config.h"

int BatchImportConfig::s_index = -1;

/**
 * Constructor.
 */
BatchImportConfig::BatchImportConfig()
  : StoredConfig<BatchImportConfig>(QLatin1String("BatchImport")),
    m_importDest(Frame::TagV2), m_profileIdx(0)
{
  /**
   * Preset profile expressions.
   */
  m_profileNames <<
    QLatin1String("All") <<
    QLatin1String("MusicBrainz") <<
    QLatin1String("Discogs") <<
    QLatin1String("Cover Art") <<
    QLatin1String("Custom Profile");
  m_profileSources <<
    QLatin1String("MusicBrainz Release:75:SAC;Discogs:75:SAC;Amazon:75:SAC;"
                  "gnudb.org:75:S") <<
    QLatin1String("MusicBrainz Release:75:SAC") <<
    QLatin1String("Discogs:75:SAC") <<
    QLatin1String("Amazon:75:C;Discogs:75:C;MusicBrainz Release:75:C") <<
    QLatin1String("");
}

/**
 * Persist configuration.
 *
 * @param config configuration
 */
void BatchImportConfig::writeToConfig(ISettings* config) const
{
  config->beginGroup(m_group);
  config->setValue(QLatin1String("ImportDestination"), QVariant(m_importDest));
  config->setValue(QLatin1String("ProfileNames"), QVariant(m_profileNames));
  config->setValue(QLatin1String("ProfileSources"), QVariant(m_profileSources));
  config->setValue(QLatin1String("ProfileIdx"), QVariant(m_profileIdx));
  config->endGroup();
  config->beginGroup(m_group, true);
  config->setValue(QLatin1String("WindowGeometry"), QVariant(m_windowGeometry));
  config->endGroup();
}

/**
 * Read persisted configuration.
 *
 * @param config configuration
 */
void BatchImportConfig::readFromConfig(ISettings* config)
{
  QStringList names, sources;
  config->beginGroup(m_group);
  m_importDest = Frame::tagVersionCast(
        config->value(QLatin1String("ImportDestination"), m_importDest).toInt());
  names = config->value(QLatin1String("ProfileNames"),
                        m_profileNames).toStringList();
  sources = config->value(QLatin1String("ProfileSources"),
                          m_profileSources).toStringList();
  m_profileIdx = config->value(QLatin1String("ProfileIdx"), m_profileIdx).toInt();
  config->endGroup();
  config->beginGroup(m_group, true);
  m_windowGeometry = config->value(QLatin1String("WindowGeometry"),
                                   m_windowGeometry).toByteArray();
  config->endGroup();

  // KConfig seems to strip empty entries from the end of the string lists,
  // so we have to append them again.
  const int numNames = names.size();
  while (sources.size() < numNames)
    sources.append(QLatin1String(""));
  /* Use defaults if no configuration found */
  for (auto namesIt = names.constBegin(), sourcesIt = sources.constBegin();
       namesIt != names.constEnd() && sourcesIt != sources.constEnd();
       ++namesIt, ++sourcesIt) {
    int idx = m_profileNames.indexOf(*namesIt);
    if (idx >= 0) {
      m_profileSources[idx] = *sourcesIt;
    } else if (!(*namesIt).isEmpty()) {
      m_profileNames.append(*namesIt);
      m_profileSources.append(*sourcesIt);
    }
  }

  if (m_profileIdx >= static_cast<int>(m_profileNames.size()))
    m_profileIdx = 0;
}

/**
 * Get a batch import profile.
 *
 * @param name name of profile
 * @param profile the profile will be returned here
 * @return true if profile with @a name found.
 */
bool BatchImportConfig::getProfileByName(const QString& name,
                                         BatchImportProfile& profile) const
{
  for (auto namesIt = m_profileNames.constBegin(),
       sourcesIt = m_profileSources.constBegin();
       namesIt != m_profileNames.constEnd() &&
       sourcesIt != m_profileSources.constEnd();
       ++namesIt, ++sourcesIt) {
    if (name == *namesIt) {
      profile.setName(*namesIt);
      profile.setSourcesFromString(*sourcesIt);
      return true;
    }
  }
  return false;
}

void BatchImportConfig::setImportDest(Frame::TagVersion importDest)
{
  if (m_importDest != importDest) {
    m_importDest = importDest;
    emit importDestChanged(m_importDest);
  }
}

void BatchImportConfig::setWindowGeometry(const QByteArray& windowGeometry)
{
  if (m_windowGeometry != windowGeometry) {
    m_windowGeometry = windowGeometry;
    emit windowGeometryChanged(m_windowGeometry);
  }
}

void BatchImportConfig::setProfileIndex(int profileIdx)
{
  if (m_profileIdx != profileIdx) {
    m_profileIdx = profileIdx;
    emit profileIndexChanged(m_profileIdx);
  }
}

void BatchImportConfig::setProfileSources(const QStringList& profileSources)
{
  if (m_profileSources != profileSources) {
    m_profileSources = profileSources;
    emit profileSourcesChanged(m_profileSources);
  }
}

void BatchImportConfig::setProfileNames(const QStringList& profileNames)
{
  if (m_profileNames != profileNames) {
    m_profileNames = profileNames;
    emit profileNamesChanged(m_profileNames);
  }
}
