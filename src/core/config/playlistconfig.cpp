/**
 * \file playlistconfig.cpp
 * Configuration for playlist dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 16 Sep 2009
 *
 * Copyright (C) 2009-2013  Urs Fleisch
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

#include "playlistconfig.h"
#include "config.h"

/**
 * Constructor.
 * Set default configuration.
 *
 * @param grp configuration group
 */
PlaylistConfig::PlaylistConfig(const QString& grp) :
  StoredConfig<PlaylistConfig>(grp),
  m_useFileNameFormat(false),
  m_onlySelectedFiles(false),
  m_useSortTagField(false), m_useFullPath(false), m_writeInfo(false),
  m_location(PL_CurrentDirectory), m_format(PF_M3U),
  m_fileNameFormat(QLatin1String("%{artist} - %{album}")), m_sortTagField(QLatin1String("%{track.3}")),
  m_infoFormat(QLatin1String("%{artist} - %{title}"))
{
}

/**
 * Destructor.
 */
PlaylistConfig::~PlaylistConfig() {}

/**
 * Persist configuration.
 *
 * @param config configuration
 */
void PlaylistConfig::writeToConfig(ISettings* config) const
{
  config->beginGroup(m_group);
  config->setValue(QLatin1String("UseFileNameFormat"), QVariant(m_useFileNameFormat));
  config->setValue(QLatin1String("OnlySelectedFiles"), QVariant(m_onlySelectedFiles));
  config->setValue(QLatin1String("UseSortTagField"), QVariant(m_useSortTagField));
  config->setValue(QLatin1String("UseFullPath"), QVariant(m_useFullPath));
  config->setValue(QLatin1String("WriteInfo"), QVariant(m_writeInfo));
  config->setValue(QLatin1String("Location"), QVariant(static_cast<int>(m_location)));
  config->setValue(QLatin1String("Format"), QVariant(static_cast<int>(m_format)));
  config->setValue(QLatin1String("FileNameFormat"), QVariant(m_fileNameFormat));
  config->setValue(QLatin1String("SortTagField"), QVariant(m_sortTagField));
  config->setValue(QLatin1String("InfoFormat"), QVariant(m_infoFormat));
  config->endGroup();
}

/**
 * Read persisted configuration.
 *
 * @param config configuration
 */
void PlaylistConfig::readFromConfig(ISettings* config)
{
  config->beginGroup(m_group);
  m_useFileNameFormat = config->value(QLatin1String("UseFileNameFormat"),
                                      m_useFileNameFormat).toBool();
  m_onlySelectedFiles = config->value(QLatin1String("OnlySelectedFiles"),
                                      m_onlySelectedFiles).toBool();
  m_useSortTagField = config->value(QLatin1String("UseSortTagField"),
                                    m_useSortTagField).toBool();
  m_useFullPath = config->value(QLatin1String("UseFullPath"), m_useFullPath).toBool();
  m_writeInfo = config->value(QLatin1String("WriteInfo"), m_writeInfo).toBool();
  m_location = static_cast<PlaylistLocation>(config->value(QLatin1String("Location"),
    static_cast<int>(m_location)).toInt());
  m_format = static_cast<PlaylistFormat>(config->value(QLatin1String("Format"),
    static_cast<int>(m_format)).toInt());
  m_fileNameFormat = config->value(QLatin1String("FileNameFormat"), m_fileNameFormat).toString();
  m_sortTagField = config->value(QLatin1String("SortTagField"), m_sortTagField).toString();
  m_infoFormat = config->value(QLatin1String("InfoFormat"), m_infoFormat).toString();
  config->endGroup();
}
