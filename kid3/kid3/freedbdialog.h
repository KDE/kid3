/**
 * \file freedbdialog.h
 * freedb.org import dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 3 Jan 2004
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
