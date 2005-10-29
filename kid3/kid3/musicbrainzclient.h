/**
 * \file musicbrainzclient.h
 * MusicBrainz client.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 15 Sep 2005
 */

#ifndef MUSICBRAINZCLIENT_H
#define MUSICBRAINZCLIENT_H

#include "config.h"

#include "importtrackdata.h"
#include <qobject.h>

#ifdef HAVE_TUNEPIMP
#include <tunepimp/tp_c.h>
#endif // HAVE_TUNEPIMP

class MusicBrainzConfig;

/**
 * MusicBrainz client.
 */
class MusicBrainzClient : public QObject
{
Q_OBJECT

public:
	/**
	 * Constructor.
	 *
	 * @param trackDataList track data to be filled with imported values,
	 *                      is passed with filenames set
	 */
	explicit MusicBrainzClient(ImportTrackDataVector& trackDataList);
	/**
	 * Destructor.
	 */
	virtual ~MusicBrainzClient();

#ifdef HAVE_TUNEPIMP
	/**
	 * Poll the status of the MusicBrainz query.
	 */
	void pollStatus();

	/**
	 * Set MusicBrainz configuration.
	 *
	 * @param cfg MusicBrainz configuration.
	 */
	void setMusicBrainzConfig(const MusicBrainzConfig* cfg);

	/**
	 * Add the files in the file list.
	 */
	void addFiles();

	/**
	 * Remove all files.
	 */
	void removeFiles();
#endif // HAVE_TUNEPIMP

signals:
	/**
	 * Emitted when status of a file changed.
	 * Parameter: index of file, status text
	 */
	void statusChanged(int, QString);

	/**
	 * Emitted when meta data for a recognized file are received.
	 * Parameter index of file, track data
	 */
	void metaDataReceived(int, ImportTrackData&);

	/**
	 * Emitted when results for an ambiguous file are received.
	 * Parameter index of file, track data list
	 */
	void resultsReceived(int, ImportTrackDataVector&);

#ifdef HAVE_TUNEPIMP
private:
	/**
	 * Get i for m_id[i] == id.
	 *
	 * @return index, -1 if not found.
	 */
	int getIndexOfId(int id) const;

	/**
	 * Get the file name for an ID.
	 *
	 * @param id ID of file
	 *
	 * @return absolute file name, QString::null if not found.
	 */
	QString getFilename(int id) const;

	/**
	 * Get meta data for recognized file.
	 *
	 * @param id        ID of file
	 * @param trackData the meta data is returned here
	 */
	void getMetaData(int id, ImportTrackData& trackData);

	/**
	 * Get results for an ambiguous file.
	 *
	 * @param id            ID of file
	 * @param trackDataList the results are returned here
	 *
	 * @return true if some results were received,
	 *         false if no results available.
	 */
	bool getResults(int id, ImportTrackDataVector& trackDataList);

	ImportTrackDataVector& m_trackDataVector;
	tunepimp_t m_tp;
	int* m_ids;
	int m_numFiles;
#endif // HAVE_TUNEPIMP
};

#endif // MUSICBRAINZCLIENT_H
