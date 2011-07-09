/**
 * \file generalconfig.h
 * General configuration.
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

#ifndef GENERALCONFIG_H
#define GENERALCONFIG_H

#include "config.h"
#include <QString>

#ifdef CONFIG_USE_KDE
#include <kconfig.h>
typedef KConfig Kid3Settings;
#else
#include <QSettings>
typedef QSettings Kid3Settings;
#endif

/**
 * Abstract base class for configurations.
 */
class GeneralConfig {
public:
	/**
	 * Constructor.
	 * Set default configuration.
	 *
	 * @param grp configuration group
	 */
	GeneralConfig(const QString& grp);

	/**
	 * Destructor.
	 */
	virtual ~GeneralConfig();

	/**
	 * Persist configuration.
	 *
	 * @param config KDE configuration
	 */
	virtual void writeToConfig(Kid3Settings* config) const = 0;

	/**
	 * Read persisted configuration.
	 *
	 * @param config KDE configuration
	 */
	virtual void readFromConfig(Kid3Settings* config) = 0;

protected:
	/** Configuration group. */
	QString m_group;
};

#endif
