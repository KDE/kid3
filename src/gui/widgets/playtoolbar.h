/**
 * \file playtoolbar.h
 * Audio player toolbar.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 24-Aug-2010
 *
 * Copyright (C) 2010-2018  Urs Fleisch
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

#include <QToolBar>
#include <QIcon>
#include <QMediaPlayer>

class QAction;
class QLCDNumber;
class QLabel;
class AudioPlayer;
class QSlider;

/**
 * Audio player toolbar.
 */
class PlayToolBar : public QToolBar {
  Q_OBJECT
public:
  /** What time is displayed in m_timeLcd. */
  enum class TimeDisplayMode {
    Elapsed,  /**< Elapsed time */
    Remaining /**< Remaining time */
  };

  /**
   * Constructor.
   *
   * @param player audio player
   * @param parent parent widget
   */
  explicit PlayToolBar(AudioPlayer* player, QWidget* parent);

  /**
   * Destructor.
   */
  virtual ~PlayToolBar() override;

  /**
   * Toggle time display mode.
   */
  void toggleTimeDisplayMode();

  /**
   * Get media player actions.
   * @return list with named actions for "audio_play", "audio_stop",
   * "audio_previous", "audio_next".
   */
  QList<QAction*> mediaActions() const;

signals:
  /**
   * Emitted when an error occurs.
   * Parameter: description of error
   */
  void errorMessage(const QString& msg);

  /**
   * Emitted before a file starts playing.
   * @param filePath path to file
   */
  void aboutToPlay(const QString& filePath);

  /**
   * Emitted when the window is closed or destroyed.
   */
  void closed();

private slots:
  /**
   * Update displayed time.
   *
   * @param msec time in ms
   */
  void tick(qint64 msec);

  /**
   * Update button states when the media player state changed.
   *
   * @param newState new Phonon state
   */
  void stateChanged(int newState);

  /**
   * Update states when a media player error occurs.
   *
   * @param err error
   */
  void error(QMediaPlayer::Error err);

  /**
   * Called when the duration changes.
   * @param duration duration in milliseconds
   */
  void durationChanged(qint64 duration);

  /**
   * Set the tool tip for the volume slider.
   * @param volume current volume (0..100)
   */
  void setVolumeToolTip(int volume);

  /**
   * Set current position in track when slider position is changed.
   * @param action slider action
   */
  void seekAction(int action);

  /**
   * Set volume when slider position is changed.
   * @param action slider action
   */
  void volumeAction(int action);

  /**
   * Toggle muted state.
   */
  void toggleMute();

  /**
   * Update display and button state when the current track is changed.
   *
   * @param filePath path of currently played audio file
   * @param hasPrevious true if a previous track is available
   * @param hasNext true if a next track is available
   */
  void trackChanged(const QString& filePath, bool hasPrevious, bool hasNext);

protected:
  /**
   * Stop sound when window is closed.
   */
  virtual void closeEvent(QCloseEvent*) override;

private:
  QIcon m_playIcon;
  QIcon m_pauseIcon;

  QAction* m_playOrPauseAction;
  QAction* m_stopAction;
  QAction* m_previousAction;
  QAction* m_nextAction;

  QLCDNumber* m_timeLcd;
  QLabel* m_titleLabel;

  AudioPlayer* m_player;

  QAction* m_muteAction;
  QSlider* m_seekSlider;
  QSlider* m_volumeSlider;

  qint64 m_duration;
  TimeDisplayMode m_timeDisplayMode;
};
