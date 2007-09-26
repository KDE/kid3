/**
 * \file importconfig.h
 * Configuration for import dialog.
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

#ifndef IMPORTCONFIG_H
#define IMPORTCONFIG_H

#include <qstringlist.h>
#include "config.h"
#include "generalconfig.h"

/**
 * Import configuration.
 */
class ImportConfig : public GeneralConfig {
public:
	/** Import destinations */
	enum ImportDestination { DestV1, DestV2, DestV1V2 };

	/** Import servers */
	enum ImportServer {
		ServerFreedb, ServerTrackType, ServerDiscogs,
		ServerMusicBrainzRelease, ServerMusicBrainzFingerprint
	};

	/**
	 * Constructor.
	 * Set default configuration.
	 *
	 * @param grp configuration group
	 */
	ImportConfig(const QString& grp);

	/**
	 * Destructor.
	 */
	virtual ~ImportConfig();

	/**
	 * Persist configuration.
	 *
	 * @param config KDE configuration
	 */
	virtual void writeToConfig(
#ifdef CONFIG_USE_KDE
		KConfig* config
#else
		Kid3Settings* config
#endif
		) const;

	/**
	 * Read persisted configuration.
	 *
	 * @param config KDE configuration
	 */
	virtual void readFromConfig(
#ifdef CONFIG_USE_KDE
		KConfig* config
#else
		Kid3Settings* config
#endif
		);

	/** import server */
	ImportServer m_importServer;
	/** true to import into ID3v1 tags, else into ID3v2 tags */
	ImportDestination m_importDest;
	/** Names of import formats */
	QStringList m_importFormatNames;
	/** regexp describing header import format */
	QStringList m_importFormatHeaders;
	/** regexp describing track import format */
	QStringList m_importFormatTracks;
	/** selected import format */
	int m_importFormatIdx;
	/** check maximum allowable time difference */
	bool m_enableTimeDifferenceCheck;
	/** maximum allowable time difference */
	int m_maxTimeDifference;

	/** true to export ID3v1 tags, else ID3v2 tags */
	bool m_exportSrcV1;
	/** Names of export formats */
	QStringList m_exportFormatNames;
	/** regexp describing header export format */
	QStringList m_exportFormatHeaders;
	/** regexp describing track export format */
	QStringList m_exportFormatTracks;
	/** regexp describing trailer export format */
	QStringList m_exportFormatTrailers;
	/** selected export format */
	int m_exportFormatIdx;
	/** export window width */
	int m_exportWindowWidth;
	/** export window height */
	int m_exportWindowHeight;
};

#endif
