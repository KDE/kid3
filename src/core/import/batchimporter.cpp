/**
 * \file batchimporter.cpp
 * Batch importer.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 3 Jan 2013
 *
 * Copyright (C) 2013  Urs Fleisch
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

#include "batchimporter.h"
#include "serverimporter.h"
#include "trackdatamodel.h"
#include "downloadclient.h"
#include "pictureframe.h"
#include "fileconfig.h"
#include "formatconfig.h"

/**
 * Flags to store types of data which have to be imported.
 */
enum DataFlags {
  StandardTags   = 1,
  AdditionalTags = 2,
  CoverArt       = 4
};

/**
 * Constructor.
 * @param netMgr network access manager
 */
BatchImporter::BatchImporter(QNetworkAccessManager* netMgr) : QObject(netMgr),
  m_downloadClient(new DownloadClient(netMgr)),
  m_currentImporter(0), m_trackDataModel(0), m_albumModel(0),
  m_albumListItem(0), m_tagVersion(TrackData::TagNone), m_state(Idle),
  m_trackListNr(-1), m_sourceNr(-1), m_albumNr(-1),
  m_requestedData(0), m_importedData(0)
{
  connect(m_downloadClient, SIGNAL(downloadFinished(QByteArray,QString,QString)),
          this, SLOT(onImageDownloaded(QByteArray,QString,QString)));
  m_frameFilter.enableAll();
}

/**
 * Destructor.
 */
BatchImporter::~BatchImporter()
{
}

/**
 * Set importers.
 * @param importers available importers
 * @param trackDataModel track data model used by importers
 */
void BatchImporter::setImporters(QList<ServerImporter*> importers,
                                 TrackDataModel* trackDataModel)
{
  m_importers = importers;
  m_trackDataModel = trackDataModel;
}

/**
 * Start batch import.
 * @param trackLists list of track data vectors with album tracks
 * @param profile batch import profile
 * @param tagVersion import destination tag version
 */
void BatchImporter::start(const QList<ImportTrackDataVector>& trackLists,
                          const BatchImportProfile& profile,
                          TrackData::TagVersion tagVersion)
{
  m_trackLists = trackLists;
  m_profile = profile;
  m_tagVersion = tagVersion;
  emit reportImportEvent(BatchImportProfile::Started, profile.getName());
  m_trackListNr = -1;
  m_state = CheckNextTrackList;
  stateTransition();
}

/**
 * Check if operation is aborted.
 *
 * @return true if aborted.
 */
bool BatchImporter::isAborted() const
{
  return m_state == Aborted;
}

/**
 * Clear state which is reported by isAborted().
 */
void BatchImporter::clearAborted()
{
  if (m_state == Aborted) {
    m_state = Idle;
    stateTransition();
  }
}

/**
 * Abort batch import.
 */
void BatchImporter::abort()
{
  State oldState = m_state;
  m_state = Aborted;
  if (oldState == Idle) {
    stateTransition();
  } else if (oldState == GettingCover) {
    m_downloadClient->cancelDownload();
    stateTransition();
  }
}

void BatchImporter::stateTransition()
{
  switch (m_state) {
  case Idle:
    m_trackListNr = -1;
    break;
  case CheckNextTrackList:
    if (m_trackDataModel) {
      bool searchKeyFound = false;
      forever {
        ++m_trackListNr;
        if (m_trackListNr < 0 || m_trackListNr >= m_trackLists.size()) {
          break;
        }
        const ImportTrackDataVector& trackList = m_trackLists.at(m_trackListNr);
        if (!trackList.isEmpty()) {
          m_currentArtist = trackList.getArtist();
          m_currentAlbum = trackList.getAlbum();
          if (m_currentArtist.isEmpty() && m_currentAlbum.isEmpty()) {
            // No tags available, try to guess artist and album from file name
            if (TaggedFile* taggedFile = trackList.first().getTaggedFile()) {
              FrameCollection frames;
              taggedFile->getTagsFromFilename(frames,
                               FileConfig::instance().m_formatFromFilenameText);
              m_currentArtist = frames.getArtist();
              m_currentAlbum = frames.getAlbum();
            }
          }
          if (!m_currentArtist.isEmpty() || !m_currentAlbum.isEmpty()) {
            m_trackDataModel->setTrackData(trackList);
            searchKeyFound = true;
            break;
          }
        }
      }
      if (searchKeyFound) {
        m_sourceNr = -1;
        m_importedData = 0;
        m_state = CheckNextSource;
      } else {
        emit reportImportEvent(BatchImportProfile::Finished, QString());
        emit finished();
        m_state = Idle;
      }
      stateTransition();
    }
    break;
  case CheckNextSource:
    m_currentImporter = 0;
    forever {
      ++m_sourceNr;
      if (m_sourceNr < 0 || m_sourceNr >= m_profile.getSources().size()) {
        break;
      }
      const BatchImportProfile::Source& profileSource =
          m_profile.getSources().at(m_sourceNr);
      if ((m_currentImporter = getImporter(profileSource.getName())) != 0) {
        m_requestedData = 0;
        if (profileSource.standardTagsEnabled())
          m_requestedData |= StandardTags;
        if (m_currentImporter->additionalTags()) {
          if (profileSource.additionalTagsEnabled())
            m_requestedData |= AdditionalTags;
          if (profileSource.coverArtEnabled())
            m_requestedData |= CoverArt;
        }
        break;
      }
    }
    if (m_currentImporter) {
      emit reportImportEvent(BatchImportProfile::SourceSelected,
                             QString::fromLatin1(m_currentImporter->name()));
      m_state = GettingAlbumList;
    } else {
      m_state = CheckNextTrackList;
    }
    stateTransition();
    break;
  case GettingAlbumList:
    if (m_currentImporter) {
      emit reportImportEvent(BatchImportProfile::QueryingAlbumList,
                             m_currentArtist + QLatin1String(" - ") + m_currentAlbum);
      m_albumNr = -1;
      m_albumModel = 0;
      connect(m_currentImporter, SIGNAL(findFinished(QByteArray)),
              this, SLOT(onFindFinished(QByteArray)));
      connect(m_currentImporter, SIGNAL(progress(QString,int,int)),
              this, SLOT(onFindProgress(QString,int,int)));
      m_currentImporter->find(m_currentImporter->config(),
                              m_currentArtist, m_currentAlbum);
    }
    break;
  case CheckNextAlbum:
    m_albumListItem = 0;
    forever {
      ++m_albumNr;
      if (!m_albumModel ||
          m_albumNr < 0 || m_albumNr >= m_albumModel->rowCount()) {
        break;
      }
      if ((m_albumListItem =
           dynamic_cast<AlbumListItem*>(m_albumModel->item(m_albumNr, 0))) != 0) {
        break;
      }
    }
    if (m_albumListItem) {
      m_state = GettingTracks;
    } else {
      m_state = CheckNextSource;
    }
    stateTransition();
    break;
  case GettingTracks:
    if (m_albumListItem && m_currentImporter) {
      emit reportImportEvent(BatchImportProfile::FetchingTrackList,
                             m_albumListItem->text());
      int pendingData = m_requestedData & ~m_importedData;
      // Also fetch standard tags, so that accuracy can be measured
      m_currentImporter->setStandardTags(
            pendingData & (StandardTags | AdditionalTags | CoverArt));
      m_currentImporter->setAdditionalTags(pendingData & AdditionalTags);
      m_currentImporter->setCoverArt(pendingData & CoverArt);
      connect(m_currentImporter, SIGNAL(albumFinished(QByteArray)),
              this, SLOT(onAlbumFinished(QByteArray)));
      connect(m_currentImporter, SIGNAL(progress(QString,int,int)),
              this, SLOT(onAlbumProgress(QString,int,int)));
      m_currentImporter->getTrackList(m_currentImporter->config(),
                                      m_albumListItem->getCategory(),
                                      m_albumListItem->getId());
    }
    break;
  case GettingCover:
    if (m_trackDataModel) {
      QString imgUrl;
      if (m_tagVersion & TrackData::TagV2) {
        QString coverArtUrl = m_trackDataModel->getTrackData().getCoverArtUrl();
        if (!coverArtUrl.isEmpty()) {
          imgUrl = DownloadClient::getImageUrl(coverArtUrl);
          if (!imgUrl.isEmpty()) {
            emit reportImportEvent(BatchImportProfile::FetchingCoverArt,
                                   coverArtUrl);
            m_downloadClient->startDownload(imgUrl);
          }
        }
      }
      if (imgUrl.isEmpty()) {
        m_state = CheckIfDone;
        stateTransition();
      }
    }
    break;
  case CheckIfDone:
    if (m_requestedData & ~m_importedData) {
      m_state = CheckNextAlbum;
    } else {
      m_state = CheckNextTrackList;
    }
    stateTransition();
    break;
  case Aborted:
    emit reportImportEvent(BatchImportProfile::Aborted, QString());
    break;
  }
}

void BatchImporter::onFindFinished(const QByteArray& searchStr)
{
  disconnect(m_currentImporter, SIGNAL(findFinished(QByteArray)),
             this, SLOT(onFindFinished(QByteArray)));
  disconnect(m_currentImporter, SIGNAL(progress(QString,int,int)),
            this, SLOT(onFindProgress(QString,int,int)));
  if (m_state == Aborted) {
    stateTransition();
  } else if (m_currentImporter) {
    m_currentImporter->parseFindResults(searchStr);
    m_albumModel = m_currentImporter->getAlbumListModel();
    m_state = CheckNextAlbum;
    stateTransition();
  }
}

void BatchImporter::onFindProgress(const QString& text, int step, int total)
{
  if (step == -1 && total == -1) {
    disconnect(m_currentImporter, SIGNAL(findFinished(QByteArray)),
               this, SLOT(onFindFinished(QByteArray)));
    disconnect(m_currentImporter, SIGNAL(progress(QString,int,int)),
              this, SLOT(onFindProgress(QString,int,int)));
    emit reportImportEvent(BatchImportProfile::Error, text);
    m_state = CheckNextAlbum;
    stateTransition();
  }
}

void BatchImporter::onAlbumFinished(const QByteArray& albumStr)
{
  disconnect(m_currentImporter, SIGNAL(albumFinished(QByteArray)),
             this, SLOT(onAlbumFinished(QByteArray)));
  disconnect(m_currentImporter, SIGNAL(progress(QString,int,int)),
             this, SLOT(onAlbumProgress(QString,int,int)));
  if (m_state == Aborted) {
    stateTransition();
  } else if (m_trackDataModel && m_currentImporter) {
    m_currentImporter->parseAlbumResults(albumStr);

    int accuracy = m_trackDataModel->calculateAccuracy();
    emit reportImportEvent(BatchImportProfile::TrackListReceived,
                           tr("Accuracy") + QLatin1Char(' ') +
                           (accuracy >= 0
                            ? QString::number(accuracy) + QLatin1Char('%')
                            : tr("Unknown")));
    const BatchImportProfile::Source& profileSource =
        m_profile.getSources().at(m_sourceNr);
    if (accuracy >= profileSource.getRequiredAccuracy()) {
      if (m_requestedData & (StandardTags | AdditionalTags)) {
        // Set imported data in tags of files.
        ImportTrackDataVector trackDataVector(m_trackDataModel->getTrackData());
        for (ImportTrackDataVector::iterator it = trackDataVector.begin();
             it != trackDataVector.end();
             ++it) {
          if (TaggedFile* taggedFile = it->getTaggedFile()) {
            taggedFile->readTags(false);
            it->removeDisabledFrames(m_frameFilter);
            TagFormatConfig::instance().formatFramesIfEnabled(*it);
            if (m_tagVersion & TrackData::TagV1) {
              taggedFile->setFramesV1(*it, false);
            }
            if (m_tagVersion & TrackData::TagV2) {
              taggedFile->setFramesV2(*it, false);
            }
          }
        }
        trackDataVector.setCoverArtUrl(QString());
        m_trackLists[m_trackListNr] = trackDataVector;
      } else {
        // Revert imported data.
        ImportTrackDataVector trackDataVector(m_trackLists.at(m_trackListNr));
        trackDataVector.setCoverArtUrl(
              m_trackDataModel->getTrackData().getCoverArtUrl());
        m_trackDataModel->setTrackData(trackDataVector);
      }

      if (m_requestedData & StandardTags)
        m_importedData |= StandardTags;
      if (m_requestedData & AdditionalTags)
        m_importedData |= AdditionalTags;
    } else {
      // Accuracy not sufficient => Revert imported data, check next album.
      m_trackDataModel->setTrackData(m_trackLists.at(m_trackListNr));
    }
    m_state = GettingCover;
    stateTransition();
  }
}

void BatchImporter::onAlbumProgress(const QString& text, int step, int total)
{
  if (step == -1 && total == -1) {
    disconnect(m_currentImporter, SIGNAL(albumFinished(QByteArray)),
               this, SLOT(onAlbumFinished(QByteArray)));
    disconnect(m_currentImporter, SIGNAL(progress(QString,int,int)),
               this, SLOT(onAlbumProgress(QString,int,int)));
    emit reportImportEvent(BatchImportProfile::Error, text);
    m_state = GettingCover;
    stateTransition();
  }
}

void BatchImporter::onImageDownloaded(const QByteArray& data,
                                    const QString& mimeType, const QString& url)
{
  if (m_state == Aborted) {
    stateTransition();
  } else {
    if (data.size() >= 1024) {
      if (mimeType.startsWith(QLatin1String("image")) && m_trackDataModel) {
        emit reportImportEvent(BatchImportProfile::CoverArtReceived, url);
        PictureFrame frame(data, url, PictureFrame::PT_CoverFront, mimeType);
        ImportTrackDataVector trackDataVector(m_trackDataModel->getTrackData());
        for (ImportTrackDataVector::iterator it = trackDataVector.begin();
             it != trackDataVector.end();
             ++it) {
          if (TaggedFile* taggedFile = it->getTaggedFile()) {
            taggedFile->readTags(false);
            taggedFile->addFrameV2(frame);
          }
        }
        m_importedData |= CoverArt;
      }
    } else {
      // Probably an invalid 1x1 picture from Amazon
      emit reportImportEvent(BatchImportProfile::CoverArtReceived,
                             tr("Invalid File"));
    }
    m_state = CheckIfDone;
    stateTransition();
  }
}

ServerImporter* BatchImporter::getImporter(const QString& name)
{
  foreach (ServerImporter* importer, m_importers) {
    if (QString::fromLatin1(importer->name()) == name) {
      return importer;
    }
  }
  return 0;
}
