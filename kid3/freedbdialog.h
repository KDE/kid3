/**
 * \file freedbdialog.h
 * freedb.org import dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 3 Jan 2004
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

#ifndef FREEDBDIALOG_H
#define FREEDBDIALOG_H

#include "importsourcedialog.h"

/**
 * freedb.org import dialog.
 */
class FreedbDialog : public ImportSourceDialog
{
public:
	/**
	 * Constructor.
	 *
	 * @param parent  parent widget
	 * @param trackDataVector track data to be filled with imported values
	 */
	FreedbDialog(QWidget* parent,
							 ImportTrackDataVector& trackDataVector);

	/**
	 * Destructor.
	 */
	virtual ~FreedbDialog();

	/**
	 * Process finished findCddbAlbum request.
	 *
	 * @param searchStr search data received
	 */
	virtual void parseFindResults(const QByteArray& searchStr);

	/**
	 * Parse result of album request and populate m_trackDataVector with results.
	 *
	 * @param albumStr album data received
	 */
	virtual void parseAlbumResults(const QByteArray& albumStr);

protected:
	/**
	 * Constructor.
	 *
	 * @param parent  parent widget
	 * @param caption dialog title
	 * @param trackDataVector track data to be filled with imported values
	 * @param client  client to use, this object takes ownership of it
	 * @param props   constant dialog properties, must exist while dialog exists
	 */
	FreedbDialog(QWidget* parent, QString caption,
							 ImportTrackDataVector& trackDataVector,
							 ImportSourceClient* client,
							 const Properties& props);
};

#endif
