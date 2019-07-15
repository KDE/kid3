/**
 * \file audioplayer.cpp
 * Audio player.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 03-Aug-2011
 *
 * Copyright (C) 2011-2018  Urs Fleisch
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

#include "audioplayer.h"

#include <QFile>
#include <QUrl>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include "kid3application.h"
#include "taggedfile.h"
#include "fileproxymodel.h"

/**
 * Constructor.
 *
 * @param app parent application
 */
AudioPlayer::AudioPlayer(Kid3Application* app) : QObject(app),
  m_app(app)
{
  setObjectName(QLatin1String("AudioPlayer"));

  m_mediaPlayer = new QMediaPlayer(this);
  m_mediaPlaylist = new QMediaPlaylist(m_mediaPlayer);
  m_mediaPlayer->setPlaylist(m_mediaPlaylist);
  connect(m_mediaPlaylist, &QMediaPlaylist::currentIndexChanged,
          this, &AudioPlayer::currentIndexChanged);
  connect(m_mediaPlayer, &QMediaPlayer::positionChanged,
          this, &AudioPlayer::positionChanged);
  connect(m_mediaPlayer, &QMediaPlayer::stateChanged,
          this, &AudioPlayer::onStateChanged);
  connect(m_mediaPlayer, &QMediaPlayer::volumeChanged,
          this, &AudioPlayer::volumeChanged);
}

/**
 * Set files to be played.
 *
 * @param files  paths to files
 * @param fileNr index of file to play (default 0), -1 to set without playing
 */
void AudioPlayer::setFiles(const QStringList& files, int fileNr)
{
  m_mediaPlaylist->clear();
  for (const QString& file : files) {
    m_mediaPlaylist->addMedia(QUrl::fromLocalFile(file));
  }
  if (fileNr != -1) {
    m_mediaPlaylist->setCurrentIndex(fileNr);
    m_mediaPlayer->play();
  } else {
    m_mediaPlaylist->setCurrentIndex(0);
  }
  emit fileCountChanged(getFileCount());
}

/**
 * Get number of files in play list.
 * @return number of files.
 */
int AudioPlayer::getFileCount() const
{
  return m_mediaPlaylist->mediaCount();
}

/**
 * Get path of current file.
 * @return file name.
 */
QString AudioPlayer::getFileName() const
{
  return m_mediaPlaylist->currentMedia().canonicalUrl().toLocalFile();
}

/**
 * Get tagged file for current file.
 * @return tagged file, 0 if not available.
 */
TaggedFile* AudioPlayer::getTaggedFile() const
{
  FileProxyModel* model = m_app->getFileProxyModel();
  QModelIndex index = model->index(getFileName());
  if (index.isValid()) {
    return FileProxyModel::getTaggedFileOfIndex(index);
  }
  return nullptr;
}

/**
 * Get index of current file in playlist.
 * @return index of current file.
 */
int AudioPlayer::getCurrentIndex() const
{
  return m_mediaPlaylist->currentIndex();
}

/**
 * Get the current playback position in milliseconds.
 * @return time in milliseconds.
 */
quint64 AudioPlayer::getCurrentPosition() const
{
  return m_mediaPlayer->position();
}

/**
 * Set the current playback position.
 * @param position time in milliseconds
 */
void AudioPlayer::setCurrentPosition(quint64 position)
{
  m_mediaPlayer->setPosition(position);
  emit currentPositionChanged(position);
}

/**
 * Get playing state.
 * @return state.
 */
AudioPlayer::State AudioPlayer::getState() const
{
  switch (m_mediaPlayer->state()) {
  case QMediaPlayer::StoppedState:
    return StoppedState;
  case QMediaPlayer::PlayingState:
    return PlayingState;
  case QMediaPlayer::PausedState:
    return PausedState;
  }
  return StoppedState;
}

/**
 * Signal stateChanged() when the playing state is changed.
 */
void AudioPlayer::onStateChanged()
{
  emit stateChanged(getState());
}

/**
 * Get duration of current track in milliseconds.
 * @return duration.
 */
qint64 AudioPlayer::getDuration() const
{
  return m_mediaPlayer->duration();
}

/**
 * Get volume.
 * @return volume level between 0 and 100.
 */
int AudioPlayer::getVolume() const
{
  return m_mediaPlayer->volume();
}

/**
 * Set volume.
 * @param volume level between 0 and 100
 */
void AudioPlayer::setVolume(int volume)
{
  m_mediaPlayer->setVolume(volume);
}

/**
 * Toggle between play and pause.
 */
void AudioPlayer::playOrPause()
{
  switch (m_mediaPlayer->state()) {
  case QMediaPlayer::PlayingState:
    m_mediaPlayer->pause();
    break;
  case QMediaPlayer::PausedState:
  case QMediaPlayer::StoppedState:
  default:
    m_mediaPlayer->play();
    break;
  }
}

/**
 * Resume playback.
 */
void AudioPlayer::play()
{
  m_mediaPlayer->play();
}

/**
 * Pause playback.
 */
void AudioPlayer::pause()
{
  m_mediaPlayer->pause();
}

/**
 * Stop playback.
 */
void AudioPlayer::stop()
{
  m_mediaPlayer->stop();
}

/**
 * Update display and button state when the current source is changed.
 * @param position number of song in play list
 */
void AudioPlayer::currentIndexChanged(int position)
{
  if (position >= 0 && position < m_mediaPlaylist->mediaCount()) {
    QString filePath =
        m_mediaPlaylist->currentMedia().canonicalUrl().toLocalFile();
    emit aboutToPlay(filePath);
    emit trackChanged(
          filePath,
          position > 0, position + 1 < m_mediaPlaylist->mediaCount());
  }
}

/**
 * Select previous track.
 */
void AudioPlayer::previous()
{
  m_mediaPlaylist->previous();
}

/**
 * Select next track.
 */
void AudioPlayer::next()
{
  m_mediaPlaylist->next();
}
