/**
 * \file freedbconfig.h
 * Freedb configuration.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 18 Jan 2004
 *
 * Copyright (C) 2004-2007  Urs Fleisch
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

#ifndef FREEDBCONFIG_H
#define FREEDBCONFIG_H

#include "importsourceconfig.h"

/**
 * Freedb configuration.
 */
class FreedbConfig : public ImportSourceConfig {
public:
	/**
	 * Constructor.
	 * Set default configuration.
	 *
	 * @param grp configuration group
	 */
	FreedbConfig(const QString& grp);

	/**
	 * Destructor.
	 */
	virtual ~FreedbConfig();
};

#endif
