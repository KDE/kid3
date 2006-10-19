/**
 * \file discogsdialog.h
 * Discogs import dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 13 Oct 2006
 */

#ifndef DISCOGSDIALOG_H
#define DISCOGSDIALOG_H

#include "importsourcedialog.h"

/**
 * Discogs import dialog.
 */
class DiscogsDialog : public ImportSourceDialog
{
public:
	/**
	 * Constructor.
	 *
	 * @param parent  parent widget
	 * @param trackDataVector track data to be filled with imported values
	 */
	DiscogsDialog(QWidget *parent,
								ImportTrackDataVector& trackDataVector);

	/**
	 * Destructor.
	 */
	virtual ~DiscogsDialog();

	/**
	 * Process finished findCddbAlbum request.
	 *
	 * @param searchStr search data received
	 */
	virtual void parseFindResults(const QCString& searchStr);

	/**
	 * Parse result of album request and populate m_trackDataVector with results.
	 *
	 * @param albumStr album data received
	 */
	virtual void parseAlbumResults(const QCString& albumStr);
};

#endif
