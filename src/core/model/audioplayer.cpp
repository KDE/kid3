/**
 * \file audioplayer.cpp
 * Audio player.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 03-Aug-2011
 *
 * Copyright (C) 2011  Urs Fleisch
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

#ifdef HAVE_PHONON

#include <QFile>
#include <phonon/phononnamespace.h>
#include <phonon/audiooutput.h>
#include <phonon/mediaobject.h>

/**
 * Constructor.
 *
 * @param parent parent object
 */
AudioPlayer::AudioPlayer(QObject* parent) : QObject(parent), m_fileNr(-1)
{
  setObjectName("AudioPlayer");

  m_mediaObject = new Phonon::MediaObject(this);
  m_mediaObject->setTickInterval(1000);
  m_audioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);
  Phonon::createPath(m_mediaObject, m_audioOutput);

  connect(m_mediaObject, SIGNAL(aboutToFinish()),
          this, SLOT(aboutToFinish()));
  connect(m_mediaObject, SIGNAL(currentSourceChanged(const Phonon::MediaSource&)),
          this, SLOT(currentSourceChanged()));
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
 * @param fileNr index of file to play (default 0)
 */
void AudioPlayer::setFiles(const QStringList& files, int fileNr)
{
  m_files = files;
  playTrack(fileNr);
}

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
      m_mediaObject->setCurrentSource(fileName);
      if (play) {
        m_mediaObject->play();
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

/**
 * Toggle between play and pause.
 */
void AudioPlayer::playOrPause()
{
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
}

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
 * Stop playback.
 */
void AudioPlayer::stop()
{
  m_mediaObject->stop();
  m_mediaObject->clearQueue();
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
      Phonon::MediaSource source(fileName);
      m_mediaObject->enqueue(source);
    }
  }
}

/**
 * Select previous track.
 */
void AudioPlayer::previous()
{
  if (m_fileNr > 0)
    selectTrack(m_fileNr - 1, m_mediaObject->state() == Phonon::PlayingState);
}

/**
 * Select next track.
 */
void AudioPlayer::next()
{
  if (m_fileNr + 1 < m_files.size())
    selectTrack(m_fileNr + 1, m_mediaObject->state() == Phonon::PlayingState);
}

#endif // HAVE_PHONON
