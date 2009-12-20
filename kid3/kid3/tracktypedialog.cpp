/**
 * \file tracktypedialog.cpp
 * TrackType.org import dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 25 Apr 2007
 *
 * Copyright (C) 2007  Urs Fleisch
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

#include <qregexp.h>
#include "kid3.h"
#include "tracktypeclient.h"
#include "tracktypedialog.h"

static const char* serverList[] = {
	"tracktype.org:80",
	0                  // end of StrList
};

static const ImportSourceDialog::Properties props = {
	serverList,
	"tracktype.org:80",
	"/~cddb/cddb.cgi",
	"import-freedb",
	&Kid3App::s_trackTypeCfg,
	false
};


/**
 * Constructor.
 *
 * @param parent  parent widget
 * @param trackDataVector track data to be filled with imported values
 */
TrackTypeDialog::TrackTypeDialog(QWidget* parent,
													 ImportTrackDataVector& trackDataVector)
	: FreedbDialog(parent, "TrackType.org", trackDataVector,
											 new TrackTypeClient, props)
{
}

/**
 * Destructor.
 */
TrackTypeDialog::~TrackTypeDialog()
{
}

/**
 * Process finished findCddbAlbum request.
 *
 * @param searchStr search data received
 */
void TrackTypeDialog::parseFindResults(const QByteArray& searchStr)
{
/*
210 exact matches found
categ discid dtitle
(more matches...)
.
or
211 close matches found
rock 920b810c Catharsis / Imago
.
theoretically, but never seen
200	categ discid dtitle
*/
	QString str = QString::fromUtf8(searchStr);
	QRegExp catIdTitleRe("([a-z]+)\\s+([0-9a-f]+)\\s+([^/]+ / .+)");
	QStringList lines = QCM_split(QRegExp("[\\r\\n]+"), str);
	bool inEntries = false;
	m_albumListBox->clear();
	for (QStringList::const_iterator it = lines.begin(); it != lines.end(); ++it) {
		if (*it == ".") {
			break;
		}
		if (inEntries) {
			if (catIdTitleRe.exactMatch(*it)) {
				new AlbumListItem(
					m_albumListBox,
					catIdTitleRe.cap(3),
					catIdTitleRe.cap(1),
					catIdTitleRe.cap(2));
			}
		} else {
			if ((*it).startsWith("21") && (*it).QCM_indexOf(" match") != -1) {
				inEntries = true;
			} else if ((*it).startsWith("200 ")) {
				if (catIdTitleRe.exactMatch((*it).mid(4))) {
					new AlbumListItem(
						m_albumListBox,
						catIdTitleRe.cap(3),
						catIdTitleRe.cap(1),
						catIdTitleRe.cap(2));
				}
			}
		}
	}
	m_albumListBox->setFocus();
}
