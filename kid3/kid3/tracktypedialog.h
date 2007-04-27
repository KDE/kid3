/**
 * \file tracktypedialog.h
 * TrackType.org import dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 25 Apr 2007
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
