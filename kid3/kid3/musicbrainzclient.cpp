/**
 * \file musicbrainzclient.cpp
 * MusicBrainz client.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 15 Sep 2005
 */

#include "musicbrainzclient.h"
#ifdef HAVE_TUNEPIMP

#ifdef CONFIG_USE_KDE
#include <klocale.h>
#else
#define i18n(s) tr(s)
#define I18N_NOOP(s) QT_TR_NOOP(s)
#endif
#include <qfile.h>
#include "musicbrainzconfig.h"
#include "freedbclient.h"
#include "importtrackdata.h"

/**
 * Constructor.
 *
 * @param trackDataList track data to be filled with imported values,
 *                      is passed with filenames set
 */
MusicBrainzClient::MusicBrainzClient(ImportTrackDataVector& trackDataList) :
	m_trackDataVector(trackDataList), m_tp(0), m_ids(0), m_numFiles(0)
{
	m_tp = tp_New("kid3", VERSION);
#ifdef WIN32
	tp_WSAInit(m_tp);
#endif
#if HAVE_TUNEPIMP >= 4
	tp_SetID3Encoding(m_tp, eUTF8);
#else
	tp_SetUseUTF8(m_tp, 1);
#endif
	tp_SetAutoFileLookup(m_tp, 1);
	tp_SetRenameFiles(m_tp, 0);
	tp_SetMoveFiles(m_tp, 0);
	tp_SetWriteID3v1(m_tp, 0);
	tp_SetClearTags(m_tp, 0);
	tp_SetAutoSaveThreshold(m_tp, -1);
	tp_SetAutoRemovedSavedFiles(m_tp, 0);
}

/**
 * Destructor.
 */
MusicBrainzClient::~MusicBrainzClient()
{
	removeFiles();
	if (m_tp) {
#ifdef WIN32
		tp_WSAStop(m_tp);
#endif
		tp_Delete(m_tp);
	}
}

/**
 * Get i for m_id[i] == id.
 *
 * @return index, -1 if not found.
 */
int MusicBrainzClient::getIndexOfId(int id) const
{
	for (int i = 0; i < m_numFiles; ++i) {
		if (m_ids[i] == id) {
			return i;
		}
	}
	return -1;
}

/**
 * Get the file name for an ID.
 *
 * @param id ID of file
 *
 * @return absolute file name, QString::null if not found.
 */
QString MusicBrainzClient::getFilename(int id) const
{
	int idx = getIndexOfId(id);
	if (idx >= 0) {
		return m_trackDataVector[idx].getAbsFilename();
	}
	return QString::null;
}

/**
 * Get a text for a file status.
 *
 * @param statusCode file status code
 *
 * @return status text, 0 if not found.
 */
static const char* getFileStatusText(TPFileStatus statusCode)
{
	static const struct id_str_s { TPFileStatus id; const char *str; }
	id_str[] = {
#if HAVE_TUNEPIMP >= 4
    { eMetadataRead,  I18N_NOOP("Metadata Read") },
#endif
    { eUnrecognized,  I18N_NOOP("Unrecognized") },
    { eRecognized,    I18N_NOOP("Recognized") },
    { ePending,       I18N_NOOP("Pending") },
    { eTRMLookup,     I18N_NOOP("TRM Lookup") },
    { eTRMCollision,  I18N_NOOP("TRM Collision") },
    { eFileLookup,    I18N_NOOP("File Lookup") },
    { eUserSelection, I18N_NOOP("User Selection") },
    { eVerified,      I18N_NOOP("Verified") },
    { eSaved,         I18N_NOOP("Saved") },
    { eDeleted,       I18N_NOOP("Deleted") },
    { eError,         I18N_NOOP("Error") },
		{ eLastStatus,    0 }
	};

	const struct id_str_s *is = &id_str[0];
	while (is->str) {
		if (is->id == statusCode) {
			break;
		}
		++is;
	}
	return is->str;
}

/**
 * Poll the status of the MusicBrainz query.
 */
void MusicBrainzClient::pollStatus()
{
	TPCallbackEnum type;
	int id;
#if HAVE_TUNEPIMP >= 4
	TPFileStatus statusCode;
	while (tp_GetNotification(m_tp, &type, &id, &statusCode))
#else
	while (tp_GetNotification(m_tp, &type, &id))
#endif
	{
		QString fn = getFilename(id);
		int index = getIndexOfId(id);
		switch (type) {
			case tpFileAdded:
				emit statusChanged(index, i18n("Pending"));
				break;
			case tpFileRemoved:
				emit statusChanged(index, i18n("Removed"));
				break;
			case tpFileChanged:
			{
#if HAVE_TUNEPIMP >= 4
				if (statusCode == eUnrecognized) {
					char trm[255];
					trm[0] = '\0';
					track_t track = tp_GetTrack(m_tp, id);
					if (track) {
						tr_Lock(track);
						tr_GetTRM(track, trm, sizeof(trm));
						if (trm[0] == '\0') {
							tr_SetStatus(track, ePending);
							tp_Wake(m_tp, track);
						}
						tr_Unlock(track);
						tp_ReleaseTrack(m_tp, track);
					}
				}
#else
				TPFileStatus statusCode = eLastStatus;
				track_t track = tp_GetTrack(m_tp, id);
				if (track) {
					tr_Lock(track);
					statusCode = tr_GetStatus(track);
					tr_Unlock(track);
					tp_ReleaseTrack(m_tp, track);
				}
#endif
				if (statusCode != eLastStatus) {
					const char* statusText = getFileStatusText(statusCode);
					emit statusChanged(index, i18n(statusText));
					if (statusCode == eRecognized) {
						ImportTrackData trackData;
						getMetaData(id, trackData);
						emit metaDataReceived(index, trackData);
					} else if (statusCode == eTRMCollision ||
										 statusCode == eUserSelection) {
						ImportTrackDataVector trackDataList;
						if (getResults(id, trackDataList)) {
							emit resultsReceived(index, trackDataList);
						}
					}
				}
				break;
			}
			case tpWriteTagsComplete:
				emit statusChanged(index, i18n("Written"));
				break;
			default:
				break;
		}
	}
}

/**
 * Set MusicBrainz configuration.
 *
 * @param cfg MusicBrainz configuration.
 */
void MusicBrainzClient::setMusicBrainzConfig(const MusicBrainzConfig* cfg)
{
	int port;
	QString ip;
	FreedbClient::splitNamePort(cfg->m_server, ip, port);
	tp_SetServer(m_tp, ip.latin1(), port);

	if (cfg->m_useProxy) {
		FreedbClient::splitNamePort(cfg->m_proxy, ip, port);
		tp_SetProxy(m_tp, ip.latin1(), port);
	}	else {
		tp_SetProxy(m_tp, "", 80);
	}
}

/**
 * Add the files in the file list.
 */
void MusicBrainzClient::addFiles()
{
	if (m_ids) {
		removeFiles();
	}
#if QT_VERSION >= 300
	m_numFiles = m_trackDataVector.size();
#else
	m_numFiles = m_trackDataVector.count();
#endif
	m_ids = new int[m_numFiles];
	int i = 0;
	for (
#if QT_VERSION >= 300
		ImportTrackDataVector::const_iterator
#else
		ImportTrackDataVector::ConstIterator
#endif
			 it = m_trackDataVector.begin();
			 it != m_trackDataVector.end();
			 ++it) {
#if HAVE_TUNEPIMP >= 4
		m_ids[i++] = tp_AddFile(m_tp, QFile::encodeName((*it).getAbsFilename()), 0);
#else
		m_ids[i++] = tp_AddFile(m_tp, QFile::encodeName((*it).getAbsFilename()));
#endif
	}
}

/**
 * Remove all files.
 */
void MusicBrainzClient::removeFiles()
{
	if (m_ids && m_numFiles > 0) {
		for (int i = 0; i < m_numFiles; ++i) {
			tp_Remove(m_tp, m_ids[i]);
		}
		delete [] m_ids;
		m_ids = 0;
		m_numFiles = 0;
	}
}

/**
 * Get meta data for recognized file.
 *
 * @param id        ID of file
 * @param trackData the meta data is returned here
 */
void MusicBrainzClient::getMetaData(int id, ImportTrackData& trackData)
{
	metadata_t* data = md_New();
	if (data) {
		track_t track = tp_GetTrack(m_tp, id);
		if (track) {
			tr_Lock(track);
			md_Clear(data);
			tr_GetServerMetadata(track, data);
			trackData.title = QString::fromUtf8(data->track);
			trackData.artist = QString::fromUtf8(data->artist);
			trackData.album = QString::fromUtf8(data->album);
			trackData.track = data->trackNum;
			trackData.year = data->releaseYear;
			// year does not seem to work, so at least we should not
			// overwrite it with 0
			if (trackData.year == 0) {
				trackData.year = -1;
			}
			trackData.setImportDuration(data->duration / 1000);
			tr_Unlock(track);
			tp_ReleaseTrack(m_tp, track);
		}
		md_Delete(data);
	}
}

/**
 * Get results for an ambiguous file.
 *
 * @param id            ID of file
 * @param trackDataList the results are returned here
 *
 * @return true if some results were received,
 *         false if no results available.
 */
bool MusicBrainzClient::getResults(int id, ImportTrackDataVector& trackDataList)
{
	bool resultsAvailable = false;
	track_t track = tp_GetTrack(m_tp, id);
	if (track) {
		tr_Lock(track);
		int num = tr_GetNumResults(track);
		result_t* results;
		if (num > 0 && (results = new result_t[num]) != 0) {
			TPResultType type;
			tr_GetResults(track, &type, results, &num);
			if (type == eTrackList) {
				albumtrackresult_t** albumTrackResults =
					reinterpret_cast<albumtrackresult_t**>(results);
				for (int i = 0; i < num; ++i) {
					albumtrackresult_t* res = *albumTrackResults++;
					ImportTrackData trackData;
					trackData.title = QString::fromUtf8(res->name);
#if HAVE_TUNEPIMP >= 4
					trackData.artist = QString::fromUtf8(res->artist.name);
					trackData.album = QString::fromUtf8(res->album.name);
					trackData.year = res->album.releaseYear;
#else
					trackData.artist = QString::fromUtf8(res->artist->name);
					trackData.album = QString::fromUtf8(res->album->name);
					trackData.year = res->album->releaseYear;
#endif
					trackData.track = res->trackNum;
					// year does not seem to work, so at least we should not
					// overwrite it with 0
					if (trackData.year == 0) {
						trackData.year = -1;
					}
					trackData.setImportDuration(res->duration / 1000);
#if QT_VERSION >= 300
					trackDataList.push_back(trackData);
#else
					trackDataList.append(trackData);
#endif
					resultsAvailable = true;
				}
			}
// Handling eArtistList and eAlbumList results does not help much,
// so it is not done.
//			else if (type == eArtistList) {
//				artistresult_t** artistResults =
//					reinterpret_cast<artistresult_t**>(results);
//				qDebug("Artist List for %d:", id);
//				for (int i = 0; i < num; ++i) {
//					artistresult_t* res = *artistResults++;
//					qDebug("%2d. %d%% %s", i, res->relevance, res->name);
//				}
//			}	else if (type == eAlbumList) {
//				albumresult_t** albumResults =
//					reinterpret_cast<albumresult_t**>(results);
//				qDebug("Album List for %d:", id);
//				for (int i = 0; i < num; ++i) {
//					albumresult_t* res = *albumResults++;
//					qDebug("%2d. %d%% %s - %s", i, res->relevance, res->artist->name, res->name);
//				}
//			}

			rs_Delete(type, results, num);
			delete [] results;
		}
		tr_Unlock(track);
		tp_ReleaseTrack(m_tp, track);
	}
	return resultsAvailable;
}

#else // HAVE_TUNEPIMP

MusicBrainzClient::MusicBrainzClient(ImportTrackDataVector&) {}
MusicBrainzClient::~MusicBrainzClient() {}

#endif // HAVE_TUNEPIMP
