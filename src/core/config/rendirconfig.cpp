/**
 * \file rendirconfig.cpp
 * Configuration for directory renaming.
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

#include "rendirconfig.h"

/** Default directory format list */
static const char* dirFmt[] = {
  "%{artist} - %{album}",
  "%{artist} - [%{year}] %{album}",
  "%{artist} - %{album} (%{year})",
  "%{artist}/%{album}",
  "%{artist}/[%{year}] %{album}",
  "%{album}",
  "[%{year}] %{album}",
  0                  // end of StrList
};

/** Default directory format list */
const char** RenDirConfig::s_defaultDirFmtList = &dirFmt[0];

namespace {

/**
 * Convert tag version to rename directory value in configuration.
 * @param tagVersion tag version
 * @return value used in configuration, kept for backwards compatibility.
 */
inline int tagVersionToRenDirCfg(TrackData::TagVersion tagVersion) {
  int renDirSrc = static_cast<int>(tagVersion);
  if (renDirSrc == 3)
    renDirSrc = 0;
  return renDirSrc;
}

/**
 * Convert rename directory value in configuration to tag version.
 * @param renDirSrc value used in configuration, kept for backwards
 *                  compatibility.
 * @return tag version.
 */
inline TrackData::TagVersion renDirCfgToTagVersion(int renDirSrc) {
  if (renDirSrc == 0)
    renDirSrc = 3;
  return TrackData::tagVersionCast(renDirSrc);
}

}

int RenDirConfig::s_index = -1;

/**
 * Constructor.
 */
RenDirConfig::RenDirConfig() :
  StoredConfig<RenDirConfig>(QLatin1String("RenameDirectory")),
  m_dirFormatText(QString::fromLatin1(s_defaultDirFmtList[0])),
  m_dirFormatItem(0),
  m_renDirSrc(TrackData::TagV2V1)
{
}

/**
 * Destructor.
 */
RenDirConfig::~RenDirConfig() {}

/**
 * Persist configuration.
 *
 * @param config configuration
 */
void RenDirConfig::writeToConfig(ISettings* config) const
{
  config->beginGroup(m_group);
  config->setValue(QLatin1String("DirFormatItem"), QVariant(m_dirFormatItem));
  config->setValue(QLatin1String("DirFormatText"), QVariant(m_dirFormatText));
  config->setValue(QLatin1String("RenameDirectorySource"), QVariant(tagVersionToRenDirCfg(m_renDirSrc)));
  config->endGroup();
}

/**
 * Read persisted configuration.
 *
 * @param config configuration
 */
void RenDirConfig::readFromConfig(ISettings* config)
{
  config->beginGroup(m_group);
  m_dirFormatItem =
      config->value(QLatin1String("DirFormatItem"), 0).toInt();
  m_renDirSrc = renDirCfgToTagVersion(config->value(QLatin1String("RenameDirectorySource"), 0).toInt());
  m_dirFormatText =
      config->value(QLatin1String("DirFormatText"), QString::fromLatin1(s_defaultDirFmtList[0])).toString();
  config->endGroup();
}
