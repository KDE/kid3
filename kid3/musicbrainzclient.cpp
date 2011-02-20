/**
 * \file musicbrainzclient.cpp
 * MusicBrainz client.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 15 Sep 2005
 *
 * Copyright (C) 2005-2007  Urs Fleisch
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

#include "musicbrainzclient.h"
#ifdef HAVE_TUNEPIMP

#include <qfile.h>
#if QT_VERSION >= 0x040000
#include <QByteArray>
#endif
#if HAVE_TUNEPIMP >= 5
#include <qdom.h>
#endif
#include "freedbclient.h"
#include "importtrackdata.h"

#if HAVE_TUNEPIMP >= 5
/**
 * Constructor.
 *
 * @param numFiles   number of files to be queried
 * @param serverName server name
 * @param serverPort server port
 * @param proxyName  proxy name, empty if no proxy
 * @param proxyPort  proxy port
 */
LookupQuery::LookupQuery(int numFiles,
												 const QString& serverName, unsigned short serverPort,
												 const QString& proxyName, unsigned short proxyPort) :
	m_numFiles(numFiles), m_serverName(serverName), m_serverPort(serverPort),
	m_proxyName(proxyName), m_proxyPort(proxyPort),
	m_currentFile(-1), m_fileQueries(new FileQuery[numFiles]),
#if QT_VERSION >= 0x040000
	m_sock(new QTcpSocket)
#else
	m_sock(new QSocket)
#endif
{
	for (int i = 0; i < m_numFiles; ++i) {
		m_fileQueries[i].requested = false;
		m_fileQueries[i].puid = "";
	}
	connect(m_sock, SIGNAL(connected()),
			this, SLOT(socketConnected()));
#if QT_VERSION >= 0x040000
	connect(m_sock, SIGNAL(error(QAbstractSocket::SocketError)),
			this, SLOT(socketError(QAbstractSocket::SocketError)));
	connect(m_sock, SIGNAL(disconnected()),
			this, SLOT(socketConnectionClosed()));
#else
	connect(m_sock, SIGNAL(error(int)),
			this, SLOT(socketError(int)));
	connect(m_sock, SIGNAL(connectionClosed()),
			this, SLOT(socketConnectionClosed()));
#endif
}

/**
 * Destructor.
 */
LookupQuery::~LookupQuery()
{
	m_sock->close();
	m_sock->disconnect();
	delete m_sock;
	delete [] m_fileQueries;
}

/**
 * Connect to server to query information about the current file.
 */
void LookupQuery::socketQuery()
{
	if (m_currentFile >= 0 && m_currentFile < m_numFiles) {
		QString  destName = m_proxyName.isEmpty() ? m_serverName : m_proxyName;
		unsigned short destPort = m_proxyName.isEmpty() ? m_serverPort : m_proxyPort;
		m_request = "GET http://";
		m_request += m_serverName;
		if (m_serverPort != 80) {
			m_request += ':';
			m_request += QString::number(m_serverPort);
		}
		m_request += "/ws/1/track/?type=xml&puid=";
		m_request += m_fileQueries[m_currentFile].puid;
		m_request += " HTTP/1.0\r\nHost: ";
		m_request += m_serverName;
		m_request += "\r\nUser-agent: Kid3/" VERSION "\r\n\r\n";
		m_sock->connectToHost(destName, destPort);
		m_fileQueries[m_currentFile].requested = true;
	}
}

/**
 * Query the next file.
 */
void LookupQuery::queryNext()
{
	// handle the first pending query
	for (int i = 0; i < m_numFiles; ++i) {
		if (!m_fileQueries[i].requested &&
				!m_fileQueries[i].puid.isEmpty()) {
			m_currentFile = i;
			socketQuery();
			return;
		}
	}
	// no pending query => socketQuery() will be done in next query()
	m_currentFile = -1;
}

/**
 * Query a PUID from the server.
 *
 * @param puid  PUID
 * @param index index of file
 */
void LookupQuery::query(const char* puid, int index)
{
	m_fileQueries[index].puid = QString(puid);
	// if no request is being executed, start the current request
	if (m_currentFile < 0 || m_currentFile >= m_numFiles ||
			!m_fileQueries[m_currentFile].requested) {
		m_currentFile = index;
		socketQuery();
	}
}

/**
 * Send query when the socket is connected.
 */
void LookupQuery::socketConnected()
{
	m_sock->QCM_writeBlock(m_request.QCM_latin1(), m_request.length());
}

/**
 * Error on socket connection.
 */
#if QT_VERSION >= 0x040000
void LookupQuery::socketError(QAbstractSocket::SocketError err)
{
	if (err != QAbstractSocket::RemoteHostClosedError) {
		qDebug("Socket Error: %s", m_sock->errorString().QCM_latin1());
		queryNext();
	}
}
#else
void LookupQuery::socketError(int)
{
	queryNext();
}
#endif

/**
 * Read received data when the server has closed the connection.
 */
void LookupQuery::socketConnectionClosed()
{
	unsigned long len = m_sock->bytesAvailable();
	QCM_QCString buf;
	buf.resize(len + 1 );
	m_sock->QCM_readBlock(buf.data(), len);
	m_sock->close();

	int xmlStart = buf.QCM_indexOf("<?xml");
	if (xmlStart >= 0 &&
			m_currentFile >= 0 && m_currentFile < m_numFiles &&
			m_fileQueries[m_currentFile].requested) {
		emit queryResponseReceived(m_currentFile, buf.mid(xmlStart, len - xmlStart));
	}
	queryNext();
}

#endif

/**
 * Constructor.
 *
 * @param trackDataList track data to be filled with imported values,
 *                      is passed with filenames set
 */
MusicBrainzClient::MusicBrainzClient(ImportTrackDataVector& trackDataList) :
	m_trackDataVector(trackDataList), m_tp(0), m_ids(0), m_numFiles(0)
#if HAVE_TUNEPIMP >= 5
	, m_lookupQuery(0)
#endif
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
#if HAVE_TUNEPIMP >= 5
	tp_SetMusicDNSClientId(m_tp, "a95f5c7cd37fd4bce12dc86d196fb4fe");
#else
	tp_SetAutoFileLookup(m_tp, 1);
#endif
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
	static const struct id_str_s { TPFileStatus id; const char* str; }
	id_str[] = {
#if HAVE_TUNEPIMP >= 4
		{ eMetadataRead,  I18N_NOOP("Metadata Read") },
#endif
		{ eUnrecognized,  I18N_NOOP("Unrecognized") },
		{ eRecognized,    I18N_NOOP("Recognized") },
		{ ePending,       I18N_NOOP("Pending") },
#if HAVE_TUNEPIMP >= 5
		{ ePUIDLookup,     I18N_NOOP("PUID Lookup") },
		{ ePUIDCollision,  I18N_NOOP("PUID Collision") },
#else
		{ eTRMLookup,     I18N_NOOP("TRM Lookup") },
		{ eTRMCollision,  I18N_NOOP("TRM Collision") },
#endif
		{ eFileLookup,    I18N_NOOP("File Lookup") },
		{ eUserSelection, I18N_NOOP("User Selection") },
		{ eVerified,      I18N_NOOP("Verified") },
		{ eSaved,         I18N_NOOP("Saved") },
		{ eDeleted,       I18N_NOOP("Deleted") },
		{ eError,         I18N_NOOP("Error") },
		{ eLastStatus,    0 }
	};

	const struct id_str_s* is = &id_str[0];
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
#if HAVE_TUNEPIMP >= 5
						tr_GetPUID(track, trm, sizeof(trm));
#else
						tr_GetTRM(track, trm, sizeof(trm));
#endif
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
					emit statusChanged(index, QCM_translate(statusText));
					if (statusCode == eRecognized) {
						ImportTrackData trackData;
						getMetaData(id, trackData);
						emit metaDataReceived(index, trackData);
					}
#if HAVE_TUNEPIMP >= 5
					else if (statusCode == ePUIDLookup ||
									 statusCode == ePUIDCollision ||
									 statusCode == eFileLookup) {
						char puid[255];
						puid[0] = '\0';
						track_t track = tp_GetTrack(m_tp, id);
						if (track) {
							tr_Lock(track);
							tr_GetPUID(track, puid, sizeof(puid));
							tr_Unlock(track);
							tp_ReleaseTrack(m_tp, track);
						}
						if (m_lookupQuery) {
							m_lookupQuery->query(puid, index);
						}
					}
#else
					else if (statusCode == eTRMCollision ||
									 statusCode == eUserSelection) {
						ImportTrackDataVector trackDataList;
						if (getResults(id, trackDataList)) {
							emit resultsReceived(index, trackDataList);
						}
					}
#endif
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
 * Set configuration.
 *
 * @param server   server
 * @param proxy    proxy
 * @param useProxy true if proxy has to be used
 */
void MusicBrainzClient::setConfig(
	const QString& server, const QString& proxy, bool useProxy)
{
	int port;
	QString ip;
	FreedbClient::splitNamePort(server, ip, port);
	tp_SetServer(m_tp, ip.QCM_latin1(), port);

	if (useProxy) {
		FreedbClient::splitNamePort(proxy, ip, port);
		tp_SetProxy(m_tp, ip.QCM_latin1(), port);
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
	m_numFiles = m_trackDataVector.size();
	m_ids = new int[m_numFiles];
#if HAVE_TUNEPIMP >= 5
	char serverName[80], proxyName[80];
	short serverPort, proxyPort;
	tp_GetServer(m_tp, serverName, sizeof(serverName) - 1, &serverPort);
	tp_GetProxy(m_tp, proxyName, sizeof(proxyName) - 1, &proxyPort);
	m_lookupQuery = new LookupQuery(m_numFiles, serverName, serverPort,
																	proxyName, proxyPort);
	connect(m_lookupQuery, SIGNAL(queryResponseReceived(int, const QByteArray&)),
					this, SLOT(parseLookupResponse(int, const QByteArray&)));
#endif
	int i = 0;
	for (ImportTrackDataVector::const_iterator it = m_trackDataVector.begin();
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
#if HAVE_TUNEPIMP >= 5
		delete m_lookupQuery;
		m_lookupQuery = 0;
#endif
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
			trackData.setTitle(QString::fromUtf8(data->track));
			trackData.setArtist(QString::fromUtf8(data->artist));
			trackData.setAlbum(QString::fromUtf8(data->album));
			trackData.setTrack(data->trackNum);
			trackData.setYear(data->releaseYear);
			// year does not seem to work, so at least we should not
			// overwrite it with 0
			if (trackData.getYear() == 0) {
				trackData.setYear(-1);
			}
			trackData.setImportDuration(data->duration / 1000);
			tr_Unlock(track);
			tp_ReleaseTrack(m_tp, track);
		}
		md_Delete(data);
	}
}

#if HAVE_TUNEPIMP >= 5

bool MusicBrainzClient::getResults(int, ImportTrackDataVector&) {
	return false;
}

/**
 * Process server response with lookup data.
 *
 * @param index    index of file
 * @param response response from server
 */
void MusicBrainzClient::parseLookupResponse(int index, const QByteArray& response)
{
	ImportTrackDataVector trackDataList;
	QDomDocument doc;
#if QT_VERSION >= 0x040000
	QByteArray xmlStr = response;
	int end = xmlStr.indexOf("</metadata>");
#else
	QCString xmlStr(response.data(), response.size());
	int end = xmlStr.find("</metadata>");
#endif
	if (end >= 0 && end + 12 < static_cast<int>(xmlStr.size())) {
		xmlStr.resize(end + 12);
	}
	if (doc.setContent(xmlStr, false)) {
		QDomElement trackList =
			doc.namedItem("metadata").toElement().namedItem("track-list").toElement();

		for (QDomNode trackNode = trackList.namedItem("track");
				 !trackNode.isNull();
				 trackNode = trackNode.nextSibling()) {
			QDomElement track = trackNode.toElement();

			ImportTrackData trackData;
			trackData.setArtist(
				track.namedItem("artist").toElement().namedItem("name").toElement().text());
			trackData.setTitle(track.namedItem("title").toElement().text());

			for (QDomNode releaseNode =
						 track.namedItem("release-list").toElement().namedItem("release");
					 !releaseNode.isNull();
					 releaseNode = releaseNode.nextSibling() ) {
				QDomElement release = releaseNode.toElement();

				trackData.setAlbum(release.namedItem("title").toElement().text());
				trackData.setTrack(-1);
				QDomNode releaseTrackNode = release.namedItem("track-list");
				if (!releaseTrackNode.isNull()) {
					QDomElement releaseTrack = releaseTrackNode.toElement();
					if (!releaseTrack.attribute("offset").isEmpty())
						trackData.setTrack(releaseTrack.attribute("offset").toInt() + 1);
				}
			}
			trackDataList.append(trackData);
		}
	}

	if (trackDataList.size() > 1) {
		emit resultsReceived(index, trackDataList);
		emit statusChanged(index, i18n("User Selection"));
	} else if (trackDataList.size() == 1) {
		emit metaDataReceived(index, *trackDataList.begin());
		emit statusChanged(index, i18n("Recognized"));
	} else {
		emit statusChanged(index, i18n("Unrecognized"));
	}
}

#else

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
					trackData.setTitle(QString::fromUtf8(res->name));
#if HAVE_TUNEPIMP >= 4
					trackData.setArtist(QString::fromUtf8(res->artist.name));
					trackData.setAlbum(QString::fromUtf8(res->album.name));
					trackData.setYear(res->album.releaseYear);
#else
					trackData.setArtist(QString::fromUtf8(res->artist->name));
					trackData.setAlbum(QString::fromUtf8(res->album->name));
					trackData.setYear(res->album->releaseYear);
#endif
					trackData.setTrack(res->trackNum);
					// year does not seem to work, so at least we should not
					// overwrite it with 0
					if (trackData.getYear() == 0) {
						trackData.setYear(-1);
					}
					trackData.setImportDuration(res->duration / 1000);
					trackDataList.push_back(trackData);
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

#endif

#else // HAVE_TUNEPIMP

MusicBrainzClient::MusicBrainzClient(ImportTrackDataVector&) {}
MusicBrainzClient::~MusicBrainzClient() {}

#endif // HAVE_TUNEPIMP

#if !(defined HAVE_TUNEPIMP && HAVE_TUNEPIMP >= 5)

LookupQuery::LookupQuery(int, const QString&, unsigned short, const QString&, unsigned short) {}
LookupQuery::~LookupQuery() {}
void LookupQuery::socketConnected() {}
#if QT_VERSION >= 0x040000
void LookupQuery::socketError(QAbstractSocket::SocketError) {}
#else
void LookupQuery::socketError(int) {}
#endif
void LookupQuery::socketConnectionClosed() {}
void MusicBrainzClient::parseLookupResponse(int, const QByteArray&) {}

#endif
