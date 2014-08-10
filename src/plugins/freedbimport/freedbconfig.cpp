/**
 * \file freedbconfig.cpp
 * freedb.org import configuration.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
 *
 * Copyright (C) 2003-2013  Urs Fleisch
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

#include "freedbconfig.h"

class QString;

int FreedbConfig::s_index = -1;

/**
 * Constructor.
 * Set default configuration.
 *
 * @param grp configuration group
 */
FreedbConfig::FreedbConfig(const QString& grp) :
  StoredConfig<FreedbConfig, ServerImporterConfig>(grp)
{
  setServer(QLatin1String("gnudb.gnudb.org:80"));
  setCgiPath(QLatin1String("/~cddb/cddb.cgi"));
}

/**
 * Destructor.
 */
FreedbConfig::~FreedbConfig() {}

/**
 * Read persisted configuration.
 *
 * @param config KDE configuration
 */
void FreedbConfig::readFromConfig(ISettings* config)
{
  ServerImporterConfig::readFromConfig(config);
  if (server() == QLatin1String("freedb2.org:80")) {
    setServer(QLatin1String("www.gnudb.org:80")); // replace old default
  }
}


int TrackTypeConfig::s_index = -1;

/**
 * Constructor.
 */
TrackTypeConfig::TrackTypeConfig() :
  StoredConfig<TrackTypeConfig, FreedbConfig>(QLatin1String("TrackType"))
{
}

/**
 * Destructor.
 */
TrackTypeConfig::~TrackTypeConfig()
{
}

/**
 * Read persisted configuration.
 *
 * @param config KDE configuration
 */
void TrackTypeConfig::readFromConfig(ISettings* config)
{
  ServerImporterConfig::readFromConfig(config);
  if (server() == QLatin1String("gnudb.gnudb.org:80")) {
    setServer(QLatin1String("tracktype.org:80")); // replace default
  }
}
