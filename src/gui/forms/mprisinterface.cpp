/**
 * \file mprisinterface.cpp
 * MPRIS D-Bus interface for audio player.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 09-Dec-2016
 *
 * Copyright (C) 2016-2018  Urs Fleisch
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

#include "mprisinterface.h"

#ifdef HAVE_QTDBUS

#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusObjectPath>
#include <QUrl>
#include <QDir>
#include <QTemporaryFile>
#include <QCoreApplication>
#include "audioplayer.h"
#include "taggedfile.h"
#include "trackdata.h"
#include "pictureframe.h"

MprisInterface::MprisInterface(AudioPlayer* player)
  : QDBusAbstractAdaptor(player), m_audioPlayer(player)
{
}

QString MprisInterface::identity() const
{
  return QLatin1String("Kid3");
}

QString MprisInterface::desktopEntry() const
{
  // Organization domain is only set in the KDE application.
  return QCoreApplication::organizationDomain().isEmpty()
      ? QLatin1String("kid3-qt") : QLatin1String("kid3");
}

QStringList MprisInterface::supportedUriSchemes() const
{
  return {QLatin1String("file")};
}

QStringList MprisInterface::supportedMimeTypes() const
{
  return {
   QLatin1String("audio/mpeg"),
   QLatin1String("audio/ogg"),
   QLatin1String("application/ogg"),
   QLatin1String("audio/x-flac"),
   QLatin1String("audio/x-flac+ogg"),
   QLatin1String("audio/x-vorbis+ogg"),
   QLatin1String("audio/x-speex+ogg"),
   QLatin1String("audio/x-oggflac"),
   QLatin1String("audio/x-musepack"),
   QLatin1String("audio/aac"),
   QLatin1String("audio/mp4"),
   QLatin1String("audio/x-speex"),
   QLatin1String("audio/x-tta"),
   QLatin1String("audio/x-wavpack"),
   QLatin1String("audio/x-aiff"),
   QLatin1String("audio/x-it"),
   QLatin1String("audio/x-mod"),
   QLatin1String("audio/x-s3m"),
   QLatin1String("audio/x-ms-wma"),
   QLatin1String("audio/x-wav"),
   QLatin1String("audio/x-xm"),
   QLatin1String("audio/opus"),
   QLatin1String("audio/x-opus+ogg"),
   QLatin1String("audio/x-dsf")
  };
}


MprisPlayerInterface::MprisPlayerInterface(AudioPlayer* player)
  : QDBusAbstractAdaptor(player), m_audioPlayer(player),
    m_hasPrevious(false), m_hasNext(false),
    m_hasFiles(m_audioPlayer->getFileCount() > 0),
    m_tempCoverArtFile(nullptr)
{
  connect(m_audioPlayer, &AudioPlayer::stateChanged,
          this, &MprisPlayerInterface::onStateChanged);
  connect(m_audioPlayer, &AudioPlayer::trackChanged,
          this, &MprisPlayerInterface::onTrackChanged);
  connect(m_audioPlayer, &AudioPlayer::volumeChanged,
          this, &MprisPlayerInterface::onVolumeChanged);
  connect(m_audioPlayer, &AudioPlayer::fileCountChanged,
          this, &MprisPlayerInterface::onFileCountChanged);
  connect(m_audioPlayer, &AudioPlayer::currentPositionChanged,
          this, &MprisPlayerInterface::onCurrentPositionChanged);
}

MprisPlayerInterface::~MprisPlayerInterface()
{
  if (m_tempCoverArtFile) {
    m_tempCoverArtFile->deleteLater();
  }
}

void MprisPlayerInterface::Next()
{
  m_audioPlayer->next();
}

void MprisPlayerInterface::Previous()
{
  m_audioPlayer->previous();
}

void MprisPlayerInterface::Pause()
{
  m_audioPlayer->pause();
}

void MprisPlayerInterface::PlayPause()
{
  m_audioPlayer->playOrPause();
}

void MprisPlayerInterface::Stop()
{
  m_audioPlayer->stop();
}

void MprisPlayerInterface::Play()
{
  m_audioPlayer->play();
}

void MprisPlayerInterface::Seek(qlonglong offsetUs)
{
  qlonglong posMs = m_audioPlayer->getCurrentPosition() + offsetUs / 1000;
  if (posMs < 0) {
    posMs = 0;
  }

  qint64 duration = m_audioPlayer->getDuration();
  if (duration < 0 || posMs <= duration) {
    m_audioPlayer->setCurrentPosition(posMs);
  } else {
    m_audioPlayer->next();
  }
}

void MprisPlayerInterface::SetPosition(const QDBusObjectPath& trackId,
                                       qlonglong positionUs)
{
  if (trackId == getCurrentTrackId() && positionUs >= 0) {
    qlonglong posMs = positionUs / 1000;
    qlonglong duration = m_audioPlayer->getDuration();
    if (duration < 0 || posMs <= duration) {
      m_audioPlayer->setCurrentPosition(posMs);
    }
  }
}

void MprisPlayerInterface::OpenUri(const QString& uri)
{
  m_audioPlayer->setFiles({QUrl(uri).toLocalFile()});
}


QString MprisPlayerInterface::playbackStatus() const
{
  QString status;
  switch (m_audioPlayer->getState()) {
  case AudioPlayer::PlayingState:
    status = QLatin1String("Playing");
    break;
  case AudioPlayer::PausedState:
    status = QLatin1String("Paused");
    break;
  case AudioPlayer::StoppedState:
  default:
    status = QLatin1String("Stopped");
    break;
  }
  return status;
}

QVariantMap MprisPlayerInterface::metadata() const
{
  QVariantMap map;
  QString filePath = m_audioPlayer->getFileName();
  if (!filePath.isEmpty()) {
    map.insert(QLatin1String("mpris:trackid"),
               QVariant::fromValue<QDBusObjectPath>(getCurrentTrackId()));
    qint64 duration = m_audioPlayer->getDuration();
    map.insert(QLatin1String("xesam:url"), QUrl::fromLocalFile(filePath).toString());
    if (TaggedFile* taggedFile = m_audioPlayer->getTaggedFile()) {
      // https://www.freedesktop.org/wiki/Specifications/mpris-spec/metadata/
      taggedFile->readTags(false);
      const TrackData trackData(*taggedFile, Frame::TagVAll);

      // Phonon often returns a duration of -1 or from the last track.
      // In such cases, get the duration from the tagged file and convert it to
      // milliseconds.
      qint64 seconds = taggedFile->getDuration();
      if ((duration < 0 || duration / 1000 != seconds) && seconds > 0) {
        duration = seconds * 1000;
      }

      QString artPath;
      QStringList albumArtists, artists, comments, composers, genres, lyricists;
      for (auto it = trackData.cbegin(); it != trackData.cend(); ++it) {
        const Frame& frame = *it;
        switch (frame.getType()) {
        case Frame::FT_Album:
          map.insert(QLatin1String("xesam:album"), frame.getValue());
          break;
        case Frame::FT_AlbumArtist:
          albumArtists.append(frame.getValue());
          break;
        case Frame::FT_Artist:
          artists.append(frame.getValue());
          break;
        case Frame::FT_Lyrics:
          map.insert(QLatin1String("xesam:asText"), frame.getValue());
          break;
        case Frame::FT_Bpm:
          if (int bpm = frame.getValue().toInt()) {
            map.insert(QLatin1String("xesam:audioBPM"), bpm);
          }
          break;
        case Frame::FT_Comment:
          comments.append(frame.getValue());
          break;
        case Frame::FT_Composer:
          composers.append(frame.getValue());
          break;
        case Frame::FT_Date:
          map.insert(QLatin1String("xesam:contentCreated"), frame.getValue());
          break;
        case Frame::FT_Disc:
          if (int disc = frame.getValue().toInt()) {
            map.insert(QLatin1String("xesam:discNumber"), disc);
          }
          break;
        case Frame::FT_Genre:
          genres.append(frame.getValue());
          break;
        case Frame::FT_Lyricist:
          lyricists.append(frame.getValue());
          break;
        case Frame::FT_Title:
          map.insert(QLatin1String("xesam:title"), frame.getValue());
          break;
        case Frame::FT_Track:
          if (int track = frame.getValue().toInt()) {
            map.insert(QLatin1String("xesam:tracknumber"), track);
          }
          break;
        case Frame::FT_Picture:
          if (artPath.isEmpty()) {
            QByteArray data;
            if (PictureFrame::getData(frame, data)) {
              if (m_tempCoverArtFile)
                m_tempCoverArtFile->deleteLater();
              m_tempCoverArtFile = new QTemporaryFile;
              m_tempCoverArtFile->open();
              m_tempCoverArtFile->write(data);
              artPath = m_tempCoverArtFile->fileName();
              m_tempCoverArtFile->close();
            }
          }
          break;
        default:
          break;
        }
      }
      if (artPath.isEmpty()) {
        artPath = findCoverArtInDirectory(taggedFile->getDirname());
      }
      if (!albumArtists.isEmpty())
        map.insert(QLatin1String("xesam:albumArtist"), albumArtists);
      if (!artists.isEmpty())
        map.insert(QLatin1String("xesam:artist"), artists);
      if (!comments.isEmpty())
        map.insert(QLatin1String("xesam:comment"), comments);
      if (!composers.isEmpty())
        map.insert(QLatin1String("xesam:composer"), composers);
      if (!genres.isEmpty())
        map.insert(QLatin1String("xesam:genre"), genres);
      if (!lyricists.isEmpty())
        map.insert(QLatin1String("xesam:lyricist"), lyricists);
      if (!artPath.isEmpty())
        map.insert(QLatin1String("mpris:artUrl"),
                   QUrl::fromLocalFile(artPath).toString());
    }
    if (duration >= 0)
      map.insert(QLatin1String("mpris:length"), duration * 1000);
  }
  return map;
}

double MprisPlayerInterface::volume() const
{
  return m_audioPlayer->getVolume() / 100.0;
}

void MprisPlayerInterface::setVolume(double volume)
{
  if (volume < 0.0) {
    volume = 0.0;
  }
  m_audioPlayer->setVolume(static_cast<int>(volume * 100.0));
}

qlonglong MprisPlayerInterface::position() const
{
  return m_audioPlayer->getCurrentPosition() * 1000;
}

bool MprisPlayerInterface::canGoNext() const
{
  return m_hasNext;
}

bool MprisPlayerInterface::canGoPrevious() const
{
  return m_hasPrevious;
}

bool MprisPlayerInterface::canPlay() const
{
  return m_audioPlayer->getFileCount() > 0;
}

bool MprisPlayerInterface::canPause() const
{
  return m_audioPlayer->getFileCount() > 0;
}

void MprisPlayerInterface::onStateChanged()
{
  QString status = playbackStatus();
  if (m_status != status) {
    m_status = status;
    sendPropertiesChangedSignal(QLatin1String("PlaybackStatus"), status);
  }
}

void MprisPlayerInterface::onTrackChanged(
    const QString&, bool hasPrevious, bool hasNext)
{
  if (m_hasPrevious != hasPrevious) {
    m_hasPrevious = hasPrevious;
    sendPropertiesChangedSignal(QLatin1String("CanGoPrevious"), m_hasPrevious);
  }
  if (m_hasNext != hasNext) {
    m_hasNext = hasNext;
    sendPropertiesChangedSignal(QLatin1String("CanGoNext"), m_hasNext);
  }
  sendPropertiesChangedSignal(QLatin1String("Metadata"), metadata());
}

void MprisPlayerInterface::onVolumeChanged()
{
  sendPropertiesChangedSignal(QLatin1String("Volume"), volume());
}

void MprisPlayerInterface::onFileCountChanged(int count)
{
  bool hasFiles = count > 0;
  if (m_hasFiles != hasFiles) {
    m_hasFiles = hasFiles;
    sendPropertiesChangedSignal(QLatin1String("CanPlay"), canPlay());
    sendPropertiesChangedSignal(QLatin1String("CanPause"), canPause());
  }
}

void MprisPlayerInterface::onCurrentPositionChanged(qint64 position)
{
  emit Seeked(position * 1000);
}

void MprisPlayerInterface::sendPropertiesChangedSignal(
    const QString& name, const QVariant& value)
{
  QVariantMap changedProps;
  changedProps.insert(name, value);
  QDBusConnection::sessionBus().send(
        QDBusMessage::createSignal(
                QLatin1String("/org/mpris/MediaPlayer2"),
                QLatin1String("org.freedesktop.DBus.Properties"),
                QLatin1String("PropertiesChanged"))
        << QLatin1String("org.mpris.MediaPlayer2.Player")
        << changedProps
        << QStringList());
}

QDBusObjectPath MprisPlayerInterface::getCurrentTrackId() const {
  int index = m_audioPlayer->getCurrentIndex();
  if (index < 0) {
    return QDBusObjectPath();
  }
  return QDBusObjectPath(QLatin1String("/org/kde/kid3/playlist/")
                         + QString::number(index));
}

QString MprisPlayerInterface::findCoverArtInDirectory(const QString& dirPath)
const
{
  if (m_coverArtDirName != dirPath) {
    m_coverArtDirName = dirPath;
    QStringList files = QDir(dirPath).entryList(
        {QLatin1String("*.jpg"), QLatin1String("*.jpeg"),
         QLatin1String("*.png"), QLatin1String("*.webp")},
        QDir::Files);
    m_coverArtFileName = !files.isEmpty() ? files.first() : QString();
  }
  return !m_coverArtFileName.isEmpty()
      ? m_coverArtDirName + QLatin1Char('/') + m_coverArtFileName : QString();
}
#endif
