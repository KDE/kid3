/**
 * \file batchimporter.h
 * Batch importer.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 3 Jan 2013
 *
 * Copyright (C) 2013-2024  Urs Fleisch
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

#pragma once

#include <QObject>
#include "trackdata.h"
#include "batchimportprofile.h"
#include "iabortable.h"

class QNetworkAccessManager;
class DownloadClient;
class ServerImporter;
class TrackDataModel;
class AlbumListModel;

/**
 * Batch importer.
 */
class KID3_CORE_EXPORT BatchImporter : public QObject, public IAbortable {
  Q_OBJECT
  Q_ENUMS(ImportEventType)
public:
  /** Events occurring during batch import. */
  enum ImportEventType {
    ReadingDirectory,
    Started,
    SourceSelected,
    QueryingAlbumList,
    FetchingTrackList,
    TrackListReceived,
    FetchingCoverArt,
    CoverArtReceived,
    Finished,
    Aborted,
    Error
  };

  /**
   * Constructor.
   * @param netMgr network access manager
   */
  explicit BatchImporter(QNetworkAccessManager* netMgr);

  /**
   * Destructor.
   */
  ~BatchImporter() override = default;

  /**
   * Check if operation is aborted.
   *
   * @return true if aborted.
   */
  bool isAborted() const override;

  /**
   * Clear state which is reported by isAborted().
   */
  void clearAborted() override;

  /**
   * Set importers.
   * @param importers available importers
   * @param trackDataModel track data model used by importers
   */
  void setImporters(const QList<ServerImporter*>& importers,
                    TrackDataModel* trackDataModel);

  /**
   * Start batch import.
   * @param trackLists list of track data vectors with album tracks
   * @param profile batch import profile
   * @param tagVersion import destination tag version
   */
  void start(const QList<ImportTrackDataVector>& trackLists,
             const BatchImportProfile& profile,
             Frame::TagVersion tagVersion);

  /**
   * Set frame filter to be used when importing.
   * @param flt frame filter
   */
  void setFrameFilter(const FrameFilter& flt) { m_frameFilter = flt; }

  /**
   * Emit a report event.
   * @param type type of event
   * @param text additional message
   */
  void emitReportImportEvent(ImportEventType type,
                             const QString& text) {
    emit reportImportEvent(type, text);
  }

signals:
  /**
   * Report event.
   * @param type type of event, enum BatchImporter::ImportEventType
   * @param text additional message
   */
  void reportImportEvent(int type, const QString& text);

  /**
   * Emitted when the batch import is finished.
   */
  void finished();

public slots:
  /**
   * Abort batch import.
   */
  void abort() override;

private slots:
  void onFindFinished(const QByteArray& searchStr);
  void onFindProgress(const QString& text, int step, int total);
  void onAlbumFinished(const QByteArray& albumStr);
  void onAlbumProgress(const QString& text, int step, int total);
  void onImageDownloaded(const QByteArray& data, const QString& mimeType,
                         const QString& url);

private:
  enum State {
    Idle,
    CheckNextTrackList,
    CheckNextSource,
    GettingAlbumList,
    CheckNextAlbum,
    GettingTracks,
    GettingCover,
    CheckIfDone,
    ImportAborted
  };

  void stateTransition();
  ServerImporter* getImporter(const QString& name);

  DownloadClient* m_downloadClient;
  QList<ServerImporter*> m_importers;
  ServerImporter* m_currentImporter;
  TrackDataModel* m_trackDataModel;
  AlbumListModel* m_albumModel;
  QString m_albumListItemText;
  QString m_albumListItemCategory;
  QString m_albumListItemId;
  QList<ImportTrackDataVector> m_trackLists;
  BatchImportProfile m_profile;
  Frame::TagVersion m_tagVersion;
  State m_state;
  int m_trackListNr;
  int m_sourceNr;
  int m_albumNr;
  int m_requestedData;
  int m_importedData;
  QString m_currentArtist;
  QString m_currentAlbum;
  FrameFilter m_frameFilter;
};
