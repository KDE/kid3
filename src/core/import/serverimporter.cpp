/**
 * \file serverimporter.cpp
 * Generic baseclass to import from a server.
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

#include "serverimporter.h"
#include <QStandardItemModel>
#include "serverimporterconfig.h"
#include "importclient.h"
#include "trackdata.h"
#include "qtcompatmac.h"

/**
 * Constructor.
 *
 * @param netMgr network access manager
 * @param trackDataModel track data to be filled with imported values
 */
ServerImporter::ServerImporter(QNetworkAccessManager* netMgr,
                               TrackDataModel* trackDataModel)
  : ImportClient(netMgr),
    m_albumListModel(new QStandardItemModel(this)),
    m_trackDataModel(trackDataModel), m_standardTagsEnabled(true),
    m_additionalTagsEnabled(false), m_coverArtEnabled(false)
{
  setObjectName("ServerImporter");
}

/**
 * Destructor.
 */
ServerImporter::~ServerImporter()
{
}

/** NULL-terminated array of server strings, 0 if not used */
const char** ServerImporter::serverList() const { return 0; }

/** default server, 0 to disable */
const char* ServerImporter::defaultServer() const { return 0; }

/** default CGI path, 0 to disable */
const char* ServerImporter::defaultCgiPath() const { return 0; }

/** anchor to online help, 0 to disable */
const char* ServerImporter::helpAnchor() const { return 0; }

/** configuration, 0 if not used */
ServerImporterConfig* ServerImporter::config() const { return 0; }

/** additional tags option, false if not used */
bool ServerImporter::additionalTags() const { return false; }

/**
 * Clear model data.
 */
void ServerImporter::clear()
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
QString ServerImporter::replaceHtmlEntities(QString str)
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
QString ServerImporter::removeHtml(QString str)
{
  QRegExp htmlTagRe("<[^>]+>");
  return replaceHtmlEntities(str.remove(htmlTagRe)).trimmed();
}
