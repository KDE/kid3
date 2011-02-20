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
#include "qtcompatmac.h"

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
void PlaylistConfig::writeToConfig(
#ifdef CONFIG_USE_KDE
	KConfig* config
#else
	Kid3Settings* config
#endif
	) const
{
#ifdef CONFIG_USE_KDE
	KCM_KConfigGroup(cfg, config, m_group);
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
	config->QCM_writeEntry("/UseFileNameFormat", m_useFileNameFormat);
	config->QCM_writeEntry("/OnlySelectedFiles", m_onlySelectedFiles);
	config->QCM_writeEntry("/UseSortTagField", m_useSortTagField);
	config->QCM_writeEntry("/UseFullPath", m_useFullPath);
	config->QCM_writeEntry("/WriteInfo", m_writeInfo);
	config->QCM_writeEntry("/Location", static_cast<int>(m_location));
	config->QCM_writeEntry("/Format", static_cast<int>(m_format));
	config->QCM_writeEntry("/FileNameFormat", m_fileNameFormat);
	config->QCM_writeEntry("/SortTagField", m_sortTagField);
	config->QCM_writeEntry("/InfoFormat", m_infoFormat);
	config->endGroup();
#endif
}

/**
 * Read persisted configuration.
 *
 * @param config KDE configuration
 */
void PlaylistConfig::readFromConfig(
#ifdef CONFIG_USE_KDE
	KConfig* config
#else
	Kid3Settings* config
#endif
	)
{
#ifdef CONFIG_USE_KDE
	KCM_KConfigGroup(cfg, config, m_group);
	m_useFileNameFormat = cfg.KCM_readBoolEntry("UseFileNameFormat",
	  m_useFileNameFormat);
	m_onlySelectedFiles = cfg.KCM_readBoolEntry("OnlySelectedFiles",
	  m_onlySelectedFiles);
	m_useSortTagField = cfg.KCM_readBoolEntry("UseSortTagField",
	  m_useSortTagField);
	m_useFullPath = cfg.KCM_readBoolEntry("UseFullPath", m_useFullPath);
	m_writeInfo = cfg.KCM_readBoolEntry("WriteInfo", m_writeInfo);
	m_location = static_cast<PlaylistLocation>(cfg.KCM_readNumEntry("Location",
	  static_cast<int>(m_location)));
	m_format = static_cast<PlaylistFormat>(cfg.KCM_readNumEntry("Format",
	  static_cast<int>(m_format)));
	m_fileNameFormat = cfg.readEntry("FileNameFormat", m_fileNameFormat);
	m_sortTagField = cfg.readEntry("SortTagField", m_sortTagField);
	m_infoFormat = cfg.readEntry("InfoFormat", m_infoFormat);
#else
	config->beginGroup("/" + m_group);
	m_useFileNameFormat = config->QCM_readBoolEntry("/UseFileNameFormat",
	  m_useFileNameFormat);
	m_onlySelectedFiles = config->QCM_readBoolEntry("/OnlySelectedFiles",
	  m_onlySelectedFiles);
	m_useSortTagField = config->QCM_readBoolEntry("/UseSortTagField",
	  m_useSortTagField);
	m_useFullPath = config->QCM_readBoolEntry("/UseFullPath", m_useFullPath);
	m_writeInfo = config->QCM_readBoolEntry("/WriteInfo", m_writeInfo);
	m_location = static_cast<PlaylistLocation>(config->QCM_readNumEntry("Location",
	  static_cast<int>(m_location)));
	m_format = static_cast<PlaylistFormat>(config->QCM_readNumEntry("Format",
	  static_cast<int>(m_format)));
	m_fileNameFormat = config->QCM_readEntry("/FileNameFormat", m_fileNameFormat);
	m_sortTagField = config->QCM_readEntry("/SortTagField", m_sortTagField);
	m_infoFormat = config->QCM_readEntry("/InfoFormat", m_infoFormat);
	config->endGroup();
#endif
}
