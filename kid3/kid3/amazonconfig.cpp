/**
 * \file amazonconfig.cpp
 * Amazon configuration.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 13 Dec 2009
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

#include "amazonconfig.h"
#include <qstring.h>

/**
 * Constructor.
 * Set default configuration.
 *
 * @param grp configuration group
 */
AmazonConfig::AmazonConfig(const QString& grp) :
	ImportSourceConfig(grp, false, true)
{
	m_server = "www.amazon.com:80";
}

/**
 * Destructor.
 */
AmazonConfig::~AmazonConfig() {}
