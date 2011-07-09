/**
 * \file playlistconfig.cpp
 * Configuration for playlist dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 16 Sep 2009
 *
 * Copyright (C) 2009  Urs Fleisch
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

#ifdef CONFIG_USE_KDE
#include <kconfig.h>
#include <kconfigskeleton.h>
#endif

/**
 * Constructor.
 * Set default configuration.
 *
 * @param grp configuration group
 */
PlaylistConfig::PlaylistConfig(const QString& grp) :
	GeneralConfig(grp),
	m_useFileNameFormat(false),
	m_onlySelectedFiles(false),
	m_useSortTagField(false), m_useFullPath(false), m_writeInfo(false),
	m_location(PL_CurrentDirectory), m_format(PF_M3U),
	m_fileNameFormat("%{artist} - %{album}"), m_sortTagField("%{track.3}"),
	m_infoFormat("%{artist} - %{title}")
{
}

/**
 * Destructor.
 */
PlaylistConfig::~PlaylistConfig() {}

/**
 * Persist configuration.
 *
 * @param config KDE configuration
 */
void PlaylistConfig::writeToConfig(Kid3Settings* config) const
{
#ifdef CONFIG_USE_KDE
	KConfigGroup cfg = config->group(m_group);
	cfg.writeEntry("UseFileNameFormat", m_useFileNameFormat);
	cfg.writeEntry("OnlySelectedFiles", m_onlySelectedFiles);
	cfg.writeEntry("UseSortTagField", m_useSortTagField);
	cfg.writeEntry("UseFullPath", m_useFullPath);
	cfg.writeEntry("WriteInfo", m_writeInfo);
	cfg.writeEntry("Location", static_cast<int>(m_location));
	cfg.writeEntry("Format", static_cast<int>(m_format));
	cfg.writeEntry("FileNameFormat", m_fileNameFormat);
	cfg.writeEntry("SortTagField", m_sortTagField);
	cfg.writeEntry("InfoFormat", m_infoFormat);
#else
	config->beginGroup("/" + m_group);
	config->setValue("/UseFileNameFormat", QVariant(m_useFileNameFormat));
	config->setValue("/OnlySelectedFiles", QVariant(m_onlySelectedFiles));
	config->setValue("/UseSortTagField", QVariant(m_useSortTagField));
	config->setValue("/UseFullPath", QVariant(m_useFullPath));
	config->setValue("/WriteInfo", QVariant(m_writeInfo));
	config->setValue("/Location", QVariant(static_cast<int>(m_location)));
	config->setValue("/Format", QVariant(static_cast<int>(m_format)));
	config->setValue("/FileNameFormat", QVariant(m_fileNameFormat));
	config->setValue("/SortTagField", QVariant(m_sortTagField));
	config->setValue("/InfoFormat", QVariant(m_infoFormat));
	config->endGroup();
#endif
}

/**
 * Read persisted configuration.
 *
 * @param config KDE configuration
 */
void PlaylistConfig::readFromConfig(Kid3Settings* config)
{
#ifdef CONFIG_USE_KDE
	KConfigGroup cfg = config->group(m_group);
	m_useFileNameFormat = cfg.readEntry("UseFileNameFormat",
	  m_useFileNameFormat);
	m_onlySelectedFiles = cfg.readEntry("OnlySelectedFiles",
	  m_onlySelectedFiles);
	m_useSortTagField = cfg.readEntry("UseSortTagField",
	  m_useSortTagField);
	m_useFullPath = cfg.readEntry("UseFullPath", m_useFullPath);
	m_writeInfo = cfg.readEntry("WriteInfo", m_writeInfo);
	m_location = static_cast<PlaylistLocation>(cfg.readEntry("Location",
	  static_cast<int>(m_location)));
	m_format = static_cast<PlaylistFormat>(cfg.readEntry("Format",
	  static_cast<int>(m_format)));
	m_fileNameFormat = cfg.readEntry("FileNameFormat", m_fileNameFormat);
	m_sortTagField = cfg.readEntry("SortTagField", m_sortTagField);
	m_infoFormat = cfg.readEntry("InfoFormat", m_infoFormat);
#else
	config->beginGroup("/" + m_group);
	m_useFileNameFormat = config->value("/UseFileNameFormat",
																			m_useFileNameFormat).toBool();
	m_onlySelectedFiles = config->value("/OnlySelectedFiles",
																			m_onlySelectedFiles).toBool();
	m_useSortTagField = config->value("/UseSortTagField",
																		m_useSortTagField).toBool();
	m_useFullPath = config->value("/UseFullPath", m_useFullPath).toBool();
	m_writeInfo = config->value("/WriteInfo", m_writeInfo).toBool();
	m_location = static_cast<PlaylistLocation>(config->value("Location",
		static_cast<int>(m_location)).toInt());
	m_format = static_cast<PlaylistFormat>(config->value("Format",
		static_cast<int>(m_format)).toInt());
	m_fileNameFormat = config->value("/FileNameFormat", m_fileNameFormat).toString();
	m_sortTagField = config->value("/SortTagField", m_sortTagField).toString();
	m_infoFormat = config->value("/InfoFormat", m_infoFormat).toString();
	config->endGroup();
#endif
}
