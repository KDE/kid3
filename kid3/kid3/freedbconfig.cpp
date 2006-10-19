/**
 * \file freedbconfig.cpp
 * freedb.org import configuration.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
 */

#include "freedbconfig.h"
#include <qstring.h>

/**
 * Constructor.
 * Set default configuration.
 *
 * @param grp configuration group
 */
FreedbConfig::FreedbConfig(const QString &grp) : ImportSourceConfig(grp)
{
	m_server = "freedb.freedb.org:80";
	m_cgiPath = "/~cddb/cddb.cgi";
}

/**
 * Destructor.
 */
FreedbConfig::~FreedbConfig() {}
