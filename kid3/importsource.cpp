/**
 * \file importsource.cpp
 * Generic baseclass to import from an external source.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 09 Oct 2006
 *
 * Copyright (C) 2006-2011  Urs Fleisch
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

#include "importsource.h"
#include <QStandardItemModel>
#include "importsourceconfig.h"
#include "importsourceclient.h"
#include "kid3.h"
#include "trackdata.h"
#include "qtcompatmac.h"

/**
 * Constructor.
 *
 * @param parent  parent object
 * @param trackDataVector track data to be filled with imported values
 */
ImportSource::ImportSource(QObject* parent,
													 ImportTrackDataVector& trackDataVector)
	: ImportSourceClient(parent),
		m_albumListModel(new QStandardItemModel(this)),
		m_trackDataVector(trackDataVector),
		m_additionalTagsEnabled(false), m_coverArtEnabled(false)
{
	setObjectName("ImportSource");
}

/**
 * Destructor.
 */
ImportSource::~ImportSource()
{
}

/** NULL-terminated array of server strings, 0 if not used */
const char** ImportSource::serverList() const { return 0; }

/** default server, 0 to disable */
const char* ImportSource::defaultServer() const { return 0; }

/** default CGI path, 0 to disable */
const char* ImportSource::defaultCgiPath() const { return 0; }

/** anchor to online help, 0 to disable */
const char* ImportSource::helpAnchor() const { return 0; }

/** configuration, 0 if not used */
ImportSourceConfig* ImportSource::cfg() const { return 0; }

/** additional tags option, false if not used */
bool ImportSource::additionalTags() const { return false; }

/**
 * Clear model data.
 */
void ImportSource::clear()
{
	m_albumListModel->clear();
}

/**
 * Replace HTML entities in a string.
 *
 * @param str string with HTML entities (e.g. &quot;)
 *
 * @return string with replaced HTML entities.
 */
QString ImportSource::replaceHtmlEntities(QString str)
{
	str.replace("&quot;", "\"");
	str.replace("&nbsp;", " ");
	str.replace("&lt;", "<");
	str.replace("&gt;", ">");
	str.replace("&amp;", "&");
	str.replace("&times;", QString(QChar(0xd7)));
	str.replace("&ndash;", "-");

	QRegExp numEntityRe("&#(\\d+);");
	int pos = 0;
	while ((pos = numEntityRe.indexIn(str, pos)) != -1) {
		str.replace(pos, numEntityRe.matchedLength(),
								QChar(numEntityRe.cap(1).toInt()));
		pos += numEntityRe.matchedLength();
	}
	return str;
}

/**
 * Replace HTML entities and remove HTML tags.
 *
 * @param str string containing HTML
 *
 * @return clean up string
 */
QString ImportSource::removeHtml(QString str)
{
	QRegExp htmlTagRe("<[^>]+>");
	return replaceHtmlEntities(str.remove(htmlTagRe)).trimmed();
}
