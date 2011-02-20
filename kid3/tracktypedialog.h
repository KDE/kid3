/**
 * \file tracktypedialog.h
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

#ifndef TRACKTYPEDIALOG_H
#define TRACKTYPEDIALOG_H

#include "freedbdialog.h"

/**
 * TrackType.org import dialog.
 */
class TrackTypeDialog : public FreedbDialog
{
public:
	/**
	 * Constructor.
	 *
	 * @param parent  parent widget
	 * @param trackDataVector track data to be filled with imported values
	 */
	TrackTypeDialog(QWidget* parent,
							 ImportTrackDataVector& trackDataVector);

	/**
	 * Destructor.
	 */
	virtual ~TrackTypeDialog();

	/**
	 * Process finished findCddbAlbum request.
	 *
	 * @param searchStr search data received
	 */
	virtual void parseFindResults(const QByteArray& searchStr);
};

#endif
