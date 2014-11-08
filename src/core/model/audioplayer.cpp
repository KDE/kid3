/**
 * \file audioplayer.cpp
 * Audio player.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 03-Aug-2011
 *
 * Copyright (C) 2011-2013  Urs Fleisch
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

#if defined HAVE_PHONON || QT_VERSION >= 0x050000

#include <QFile>
#include <QUrl>
#ifdef HAVE_PHONON
#include <phonon/phononnamespace.h>
#include <phonon/audiooutput.h>
#include <phonon/mediaobject.h>
#else
#include <QMediaPlayer>
#include <QMediaPlaylist>
#endif

/**
 * Constructor.
 *
 * @param parent parent object
 */
AudioPlayer::AudioPlayer(QObject* parent) : QObject(parent)
#ifdef HAVE_PHONON
, m_fileNr(-1)
#endif
{
  setObjectName(QLatin1String("AudioPlayer"));

#ifdef HAVE_PHONON
  m_mediaObject = new Phonon::MediaObject(this);
  m_mediaObject->setTickInterval(1000);
  m_audioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);
  Phonon::createPath(m_mediaObject, m_audioOutput);

  connect(m_mediaObject, SIGNAL(aboutToFinish()),
          this, SLOT(aboutToFinish()));
  connect(m_mediaObject, SIGNAL(currentSourceChanged(Phonon::MediaSource)),
          this, SLOT(currentSourceChanged()));
  connect(m_mediaObject, SIGNAL(tick(qint64)),
          this, SIGNAL(positionChanged(qint64)));
#else
  m_mediaPlayer = new QMediaPlayer(this);
  m_mediaPlaylist = new QMediaPlaylist(m_mediaPlayer);
  m_mediaPlayer->setPlaylist(m_mediaPlaylist);
  connect(m_mediaPlaylist, SIGNAL(currentIndexChanged(int)),
          this, SLOT(currentIndexChanged(int)));
  connect(m_mediaPlayer, SIGNAL(positionChanged(qint64)),
          this, SIGNAL(positionChanged(qint64)));
#endif
}


/**
 * Destructor.
 */
AudioPlayer::~AudioPlayer()
{
}

/**
 * Set files to be played.
 *
 * @param files  paths to files
 * @param fileNr index of file to play (default 0), -1 to set without playing
 */
void AudioPlayer::setFiles(const QStringList& files, int fileNr)
{
#ifdef HAVE_PHONON
  m_files = files;
  if (fileNr != -1) {
    playTrack(fileNr);
  } else {
    selectTrack(0, false);
  }
#else
  m_mediaPlaylist->clear();
  foreach (const QString& file, files) {
    m_mediaPlaylist->addMedia(QUrl::fromLocalFile(file));
  }
  if (fileNr != -1) {
    m_mediaPlaylist->setCurrentIndex(fileNr);
    m_mediaPlayer->play();
  } else {
    m_mediaPlaylist->setCurrentIndex(0);
  }
#endif
}

/**
 * Get name of current file.
 * @return file name.
 */
QString AudioPlayer::getFileName() const
{
#ifdef HAVE_PHONON
  if (m_fileNr >= 0 && m_fileNr < m_files.size()) {
    return m_files.at(m_fileNr);
  }
  return QString();
#else
  return m_mediaPlaylist->currentMedia().canonicalUrl().toLocalFile();
#endif
}

/**
 * Get the current playback position in milliseconds.
 * @return time in milliseconds.
 */
quint64 AudioPlayer::getCurrentPosition() const
{
#ifdef HAVE_PHONON
  return m_mediaObject->currentTime();
#else
  return m_mediaPlayer->position();
#endif
}

/**
 * Set the current playback position.
 * @param position time in milliseconds
 */
void AudioPlayer::setCurrentPosition(quint64 position)
{
#ifdef HAVE_PHONON
  return m_mediaObject->seek(position);
#else
  return m_mediaPlayer->setPosition(position);
#endif
}

#ifdef HAVE_PHONON
/**
 * Select a track from the files and optionally start playing it.
 *
 * @param fileNr index in list of files set with setFiles()
 * @param play   true to play track
 */
void AudioPlayer::selectTrack(int fileNr, bool play)
{
  if (fileNr >= 0 && fileNr < m_files.size()) {
    m_fileNr = fileNr;
    const QString& fileName = m_files[m_fileNr];
    if (QFile::exists(fileName)) {
      m_mediaObject->clearQueue();
      emit aboutToPlay(fileName);
      m_mediaObject->setCurrentSource(QUrl::fromLocalFile(fileName));
      if (play) {
        m_mediaObject->play();
      } else {
        emit trackChanged(fileName,
                          m_fileNr > 0, m_fileNr + 1 < m_files.size());
      }
    }
  } else {
    m_fileNr = -1;
  }
}

/**
 * Play a track from the files.
 *
 * @param fileNr index in list of files set with setFiles()
 */
void AudioPlayer::playTrack(int fileNr)
{
  selectTrack(fileNr, true);
}
#endif // HAVE_PHONON

/**
 * Toggle between play and pause.
 */
void AudioPlayer::playOrPause()
{
#ifdef HAVE_PHONON
  switch (m_mediaObject->state()) {
    case Phonon::PlayingState:
      m_mediaObject->pause();
      break;
    case Phonon::PausedState:
      m_mediaObject->play();
      break;
    default:
      playTrack(m_fileNr);
      break;
  }
#else
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
#endif
}

/**
 * Stop playback.
 */
void AudioPlayer::stop()
{
#ifdef HAVE_PHONON
  m_mediaObject->stop();
  m_mediaObject->clearQueue();
  m_mediaObject->setCurrentSource(QUrl());
#else
  m_mediaPlayer->stop();
#endif
}

#ifdef HAVE_PHONON
/**
 * Update display and button state when the current source is changed.
 */
void AudioPlayer::currentSourceChanged()
{
  if (m_fileNr >= 0 && m_fileNr < m_files.size()) {
    emit trackChanged(m_files[m_fileNr],
                      m_fileNr > 0, m_fileNr + 1 < m_files.size());
  }
}

/**
 * Queue next track when the current track is about to finish.
 */
void AudioPlayer::aboutToFinish()
{
  int nextFileNr = m_fileNr + 1;
  if (nextFileNr >= 0 && nextFileNr < m_files.size()) {
    m_fileNr = nextFileNr;
    const QString& fileName = m_files[m_fileNr];
    if (QFile::exists(fileName)) {
      emit aboutToPlay(fileName);
      Phonon::MediaSource source(QUrl::fromLocalFile(fileName));
      m_mediaObject->enqueue(source);
    }
  }
}
#else
/**
 * Update display and button state when the current source is changed.
 * @param position number of song in play list
 */
void AudioPlayer::currentIndexChanged(int position)
{
  if (position >= 0 && position < m_mediaPlaylist->mediaCount()) {
    emit trackChanged(
          m_mediaPlaylist->currentMedia().canonicalUrl().toLocalFile(),
          position > 0, position + 1 < m_mediaPlaylist->mediaCount());
  }
}
#endif

/**
 * Select previous track.
 */
void AudioPlayer::previous()
{
#ifdef HAVE_PHONON
  if (m_fileNr > 0)
    selectTrack(m_fileNr - 1, m_mediaObject->state() == Phonon::PlayingState);
#else
  m_mediaPlaylist->previous();
#endif
}

/**
 * Select next track.
 */
void AudioPlayer::next()
{
#ifdef HAVE_PHONON
  if (m_fileNr + 1 < m_files.size())
    selectTrack(m_fileNr + 1, m_mediaObject->state() == Phonon::PlayingState);
#else
  m_mediaPlaylist->next();
#endif
}

#endif // HAVE_PHONON
