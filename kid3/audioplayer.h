/**
 * \file audioplayer.h
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

#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <QObject>
#include "config.h"

#ifdef HAVE_PHONON

#include <QStringList>

namespace Phonon {
  class AudioOutput;
  class MediaObject;
}

/**
 * Audio player toolbar.
 */
class AudioPlayer : public QObject {
  Q_OBJECT

public:
  /**
   * Constructor.
   *
   * @param parent parent object
   */
  explicit AudioPlayer(QObject* parent);

  /**
   * Destructor.
   */
  virtual ~AudioPlayer();

  /**
   * Set files to be played.
   *
   * @param files  paths to files
   * @param fileNr index of file to play (default 0)
   */
  void setFiles(const QStringList& files, int fileNr = 0);

  /**
   * Play a track from the files.
   *
   * @param fileNr index in list of files set with setFiles()
   */
  void playTrack(int fileNr);

  /**
   * Access to media object.
   * @return media object.
   */
  Phonon::MediaObject* mediaObject() { return m_mediaObject; }

  /**
   * Access to audio output.
   * @return audio output.
   */
  Phonon::AudioOutput* audioOutput() { return m_audioOutput; }

signals:
  /**
   * Emitted when the current track is changed.
   * @param filePath path of currently played audio file
   * @param hasPrevious true if a previous track is available
   * @param hasNext true if a next track is available
   */
  void trackChanged(const QString& filePath, bool hasPrevious, bool hasNext);

public slots:
  /**
   * Toggle between play and pause.
   */
  void playOrPause();

  /**
   * Stop playback.
   */
  void stop();

  /**
   * Select previous track.
   */
  void previous();

  /**
   * Select next track.
   */
  void next();

private slots:
  /**
   * Update display and button state when the current source is changed.
   */
  void currentSourceChanged();

  /**
   * Queue next track when the current track is about to finish.
   */
  void aboutToFinish();

private:
  /**
   * Select a track from the files and optionally start playing it.
   *
   * @param fileNr index in list of files set with setFiles()
   * @param play   true to play track
   */
  void selectTrack(int fileNr, bool play);

  Phonon::MediaObject* m_mediaObject;
  Phonon::AudioOutput* m_audioOutput;

  QStringList m_files;
  int m_fileNr;
};
#else // HAVE_PHONON

// Just to suppress moc "No relevant classes found" warning.
class AudioPlayer : public QObject {
Q_OBJECT
};

#endif // HAVE_PHONON

#endif // AUDIOPLAYER_H
