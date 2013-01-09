/**
 * \file batchimportconfig.cpp
 * Configuration for batch import.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 3 Jan 2013
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

#include "batchimportconfig.h"
#include <QString>
#include "config.h"

#ifdef CONFIG_USE_KDE
#include <kconfiggroup.h>
#endif

/**
 * Constructor.
 * Set default configuration.
 *
 * @param grp configuration group
 */
BatchImportConfig::BatchImportConfig(const QString& grp) :
  GeneralConfig(grp), m_importDest(TrackData::TagV2), m_profileIdx(0)
{
  /**
   * Preset profile expressions.
   */
  m_profileNames <<
    "All" <<
    "MusicBrainz" <<
    "Discogs" <<
    "Cover Art" <<
    "Custom Profile";
  m_profileSources <<
    "MusicBrainz Release:75:SAC;Discogs:75:SAC;Amazon:75:SAC;gnudb.org:75:S;TrackType.org:75:S" <<
    "MusicBrainz Release:75:SAC" <<
    "Discogs:75:SAC" <<
    "Amazon:75:C;Discogs:75:C;MusicBrainz Release:75:C" <<
    "";
}

/**
 * Destructor.
 */
BatchImportConfig::~BatchImportConfig()
{
}

/**
 * Persist configuration.
 *
 * @param config KDE configuration
 */
void BatchImportConfig::writeToConfig(Kid3Settings* config) const
{
#ifdef CONFIG_USE_KDE
  KConfigGroup cfg = config->group(m_group);
  cfg.writeEntry("ImportDestination", static_cast<int>(m_importDest));
  cfg.writeEntry("ProfileNames", m_profileNames);
  cfg.writeEntry("ProfileSources", m_profileSources);
  cfg.writeEntry("ProfileIdx", m_profileIdx);
  cfg.writeEntry("WindowGeometry", m_windowGeometry);
#else
  config->beginGroup("/" + m_group);
  config->setValue("/ImportDestination", QVariant(m_importDest));
  config->setValue("/ProfileNames", QVariant(m_profileNames));
  config->setValue("/ProfileSources", QVariant(m_profileSources));
  config->setValue("/ProfileIdx", QVariant(m_profileIdx));
  config->setValue("/WindowGeometry", QVariant(m_windowGeometry));

  config->endGroup();
#endif
}

/**
 * Read persisted configuration.
 *
 * @param config KDE configuration
 */
void BatchImportConfig::readFromConfig(Kid3Settings* config)
{
  QStringList names, sources;
#ifdef CONFIG_USE_KDE
  KConfigGroup cfg = config->group(m_group);
  m_importDest = TrackData::tagVersionCast(
        cfg.readEntry("ImportDestination", static_cast<int>(m_importDest)));
  names = cfg.readEntry("ProfileNames", QStringList());
  sources = cfg.readEntry("ProfileSources", QStringList());
  m_profileIdx = cfg.readEntry("ProfileIdx", m_profileIdx);
  m_windowGeometry = cfg.readEntry("WindowGeometry", QByteArray());

  // KConfig seems to strip empty entries from the end of the string lists,
  // so we have to append them again.
  unsigned numNames = names.size();
  while (static_cast<unsigned>(sources.size()) < numNames)
    sources.append("");
#else
  config->beginGroup("/" + m_group);
  m_importDest = TrackData::tagVersionCast(
        config->value("/ImportDestination", m_importDest).toInt());
  names = config->value("/ProfileNames").toStringList();
  sources = config->value("/ProfileSources").toStringList();
  m_profileIdx = config->value("/ProfileIdx", m_profileIdx).toInt();
  m_windowGeometry = config->value("/WindowGeometry").toByteArray();
  config->endGroup();
#endif
  /* Use defaults if no configuration found */
  QStringList::const_iterator namesIt, sourcesIt;
  for (namesIt = names.begin(), sourcesIt = sources.begin();
       namesIt != names.end() && sourcesIt != sources.end();
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