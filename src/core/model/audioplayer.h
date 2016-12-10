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

#if defined HAVE_PHONON || QT_VERSION >= 0x050000

#include <QStringList>
#include "kid3api.h"

#ifdef HAVE_PHONON
namespace Phonon {
  class AudioOutput;
  class MediaObject;
}
#else
class QMediaPlayer;
class QMediaPlaylist;
#endif

class Kid3Application;
class TaggedFile;

/**
 * Audio player toolbar.
 */
class KID3_CORE_EXPORT AudioPlayer : public QObject {
  Q_OBJECT

public:
  /** Playing state. */
  enum State {
    StoppedState, /**< Stopped */
    PlayingState, /**< Playing */
    PausedState   /**< Paused */
  };

  /**
   * Constructor.
   *
   * @param app parent application
   */
  explicit AudioPlayer(Kid3Application* app);

  /**
   * Destructor.
   */
  virtual ~AudioPlayer();

  /**
   * Set files to be played.
   *
   * @param files  paths to files
   * @param fileNr index of file to play (default 0), -1 to set without playing
   */
  void setFiles(const QStringList& files, int fileNr = 0);

  /**
   * Get number of files in play list.
   * @return number of files.
   */
  int getFileCount() const;

  /**
   * Get path of current file.
   * @return file name.
   */
  QString getFileName() const;

  /**
   * Get tagged file for current file.
   * @return tagged file, 0 if not available.
   */
  TaggedFile* getTaggedFile() const;

  /**
   * Get index of current file in playlist.
   * @return index of current file.
   */
  int getCurrentIndex() const;

  /**
   * Get the current playback position in milliseconds.
   * @return time in milliseconds.
   */
  quint64 getCurrentPosition() const;

  /**
   * Set the current playback position.
   * @param position time in milliseconds
   */
  void setCurrentPosition(quint64 position);

  /**
   * Get playing state.
   * @return state.
   */
  State getState() const;

  /**
   * Get duration of current track in milliseconds.
   * @return duration.
   */
  qint64 getDuration() const;

  /**
   * Get volume.
   * @return volume level between 0 and 100.
   */
  int getVolume() const;

  /**
   * Set volume.
   * @param volume level between 0 and 100
   */
  void setVolume(int volume);

#ifdef HAVE_PHONON
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
#else
  /**
   * Access to media player.
   * @return media player.
   */
  QMediaPlayer* mediaPlayer() { return m_mediaPlayer; }
#endif

signals:
  /**
   * Emitted before a file starts playing.
   * @param filePath path to file
   */
  void aboutToPlay(const QString& filePath);

  /**
   * Emitted when the current track is changed.
   * @param filePath path of currently played audio file
   * @param hasPrevious true if a previous track is available
   * @param hasNext true if a next track is available
   */
  void trackChanged(const QString& filePath, bool hasPrevious, bool hasNext);

  /**
   * Emitted when the current track position changed.
   * @param position time in milliseconds
   */
  void positionChanged(qint64 position);

  /**
   * Emitted when the current position is changed using setCurrentPosition().
   * @param position time in milliseconds
   */
  void currentPositionChanged(qint64 position);

  /**
   * Emitted when the playing state is changed.
   * @param state playing state
   */
  void stateChanged(AudioPlayer::State state);

  /**
   * Emitted when the volume is changed.
   * @param volume level between 0 and 100
   */
  void volumeChanged(int volume);

  /**
   * Emitted when the file count changed.
   * @param count number of files in play list
   */
  void fileCountChanged(int count);

public slots:
  /**
   * Toggle between play and pause.
   */
  void playOrPause();

  /**
   * Resume playback.
   */
  void play();

  /**
   * Pause playback.
   */
  void pause();

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
#ifdef HAVE_PHONON
  /**
   * Update display and button state when the current source is changed.
   */
  void currentSourceChanged();

  /**
   * Queue next track when the current track is about to finish.
   */
  void aboutToFinish();

  /**
   * Signal volumeChanged() when the volume is changed.
   * @param volume volume
   */
  void onVolumeChanged(qreal volume);
#else
  /**
   * Update display and button state when the current source is changed.
   * @param position number of song in play list
   */
  void currentIndexChanged(int position);
#endif
  /**
   * Signal stateChanged() when the playing state is changed.
   */
  void onStateChanged();

private:
  Kid3Application* m_app;
#ifdef HAVE_PHONON
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
#else
  QMediaPlayer* m_mediaPlayer;
  QMediaPlaylist* m_mediaPlaylist;
#endif
};

#else // HAVE_PHONON

// Just to suppress moc "No relevant classes found" warning.
class AudioPlayer : public QObject {
Q_OBJECT
};

#endif // HAVE_PHONON

#endif // AUDIOPLAYER_H
