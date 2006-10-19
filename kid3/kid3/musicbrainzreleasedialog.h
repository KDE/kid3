/**
 * \file musicbrainzreleasedialog.h
 * MusicBrainz release database import dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 13 Oct 2006
 */

#ifndef MUSICBRAINZRELEASEDIALOG_H
#define MUSICBRAINZRELEASEDIALOG_H

#include "importsourcedialog.h"

/**
 * MusicBrainzRelease import dialog.
 */
class MusicBrainzReleaseDialog : public ImportSourceDialog
{
public:
	/**
	 * Constructor.
	 *
	 * @param parent  parent widget
	 * @param trackDataVector track data to be filled with imported values
	 */
	MusicBrainzReleaseDialog(QWidget *parent,
													 ImportTrackDataVector& trackDataVector);

	/**
	 * Destructor.
	 */
	virtual ~MusicBrainzReleaseDialog();

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
