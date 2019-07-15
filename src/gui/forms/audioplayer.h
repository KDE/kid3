/**
 * \file audioplayer.h
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

#pragma once

#include <QObject>
#include "kid3api.h"

class QMediaPlayer;
class QMediaPlaylist;
class QStringList;
class Kid3Application;
class TaggedFile;

/**
 * Audio player toolbar.
 */
class KID3_GUI_EXPORT AudioPlayer : public QObject {
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
  virtual ~AudioPlayer() override = default;

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

  /**
   * Access to media player.
   * @return media player.
   */
  QMediaPlayer* mediaPlayer() { return m_mediaPlayer; }

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
   * Set files to be played.
   *
   * @param files  paths to files
   * @param fileNr index of file to play (default 0), -1 to set without playing
   */
  void setFiles(const QStringList& files, int fileNr = 0);

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
  /**
   * Update display and button state when the current source is changed.
   * @param position number of song in play list
   */
  void currentIndexChanged(int position);

  /**
   * Signal stateChanged() when the playing state is changed.
   */
  void onStateChanged();

private:
  Kid3Application* m_app;
  QMediaPlayer* m_mediaPlayer;
  QMediaPlaylist* m_mediaPlaylist;
};
