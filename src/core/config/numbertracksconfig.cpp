/**
 * \file numbertracksconfig.cpp
 * Configuration for track numbering.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 29 Jun 2013
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

#include "numbertracksconfig.h"

namespace {

/**
 * Convert tag version to number tracks destination value in configuration.
 * @param tagVersion tag version
 * @return value used in configuration, kept for backwards compatibility.
 */
inline int tagVersionToNumberTracksDestCfg(TrackData::TagVersion tagVersion) {
  return static_cast<int>(tagVersion) - 1;
}

/**
 * Convert number tracks destination value in configuration to tag version.
 * @param importDest value used in configuration, kept for backwards
 *                   compatibility.
 * @return tag version.
 */
inline TrackData::TagVersion numberTracksDestCfgToTagVersion(int importDest) {
  return TrackData::tagVersionCast(importDest + 1);
}

}

int NumberTracksConfig::s_index = -1;

/**
 * Constructor.
 */
NumberTracksConfig::NumberTracksConfig() :
  StoredConfig<NumberTracksConfig>(QLatin1String("NumberTracks")),
  m_numberTracksDst(TrackData::TagV1),
  m_numberTracksStart(1)
{
}

/**
 * Destructor.
 */
NumberTracksConfig::~NumberTracksConfig() {}

/**
 * Persist configuration.
 *
 * @param config configuration
 */
void NumberTracksConfig::writeToConfig(ISettings* config) const
{
  config->beginGroup(m_group);
  config->setValue(QLatin1String("NumberTracksDestination"),
                   QVariant(tagVersionToNumberTracksDestCfg(m_numberTracksDst)));
  config->setValue(QLatin1String("NumberTracksStartNumber"), QVariant(m_numberTracksStart));
  config->endGroup();
}

/**
 * Read persisted configuration.
 *
 * @param config configuration
 */
void NumberTracksConfig::readFromConfig(ISettings* config)
{
  config->beginGroup(m_group);
  m_numberTracksDst = numberTracksDestCfgToTagVersion(
        config->value(QLatin1String("NumberTracksDestination"), 0).toInt());
  m_numberTracksStart = config->value(QLatin1String("NumberTracksStartNumber"), 1).toInt();
  config->endGroup();
}
