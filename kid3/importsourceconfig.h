/**
 * \file importsourceconfig.h
 * Configuration for import source.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 09 Oct 2006
 *
 * Copyright (C) 2006-2007  Urs Fleisch
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

#ifndef IMPORTSOURCECONFIG_H
#define IMPORTSOURCECONFIG_H

#include "generalconfig.h"
#include <qstring.h>

/**
 * Freedb configuration.
 */
class ImportSourceConfig : public GeneralConfig {
public:
	/**
	 * Constructor.
	 * Set default configuration.
	 *
	 * @param grp         configuration group
	 * @param cgiPathUsed true to use CgiPath configuration
	 * @param additionalTagsUsed true to use AdditionalTags configuration
	 */
	ImportSourceConfig(const QString& grp, bool cgiPathUsed = true,
										 bool additionalTagsUsed = false);

	/**
	 * Constructor.
	 * Used to create temporary configuration.
	 */
	ImportSourceConfig();

	/**
	 * Destructor.
	 */
	virtual ~ImportSourceConfig();

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

	/** server */
	QString m_server;

	/** CGI path used for access */
	QString m_cgiPath;

	/** window width */
	int m_windowWidth;

	/** window height */
	int m_windowHeight;

	/** true if CgiPath configuration is used */
	bool m_cgiPathUsed;

	/** true if additional tags configuration is used */
	bool m_additionalTagsUsed;

	/** additional tags imported */
	bool m_additionalTags;

	/** cover art imported */
	bool m_coverArt;
};

#endif
