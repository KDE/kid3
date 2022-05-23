/**
 * \file playtoolbar.cpp
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

#include "playtoolbar.h"

#include <QAction>
#include <QLCDNumber>
#include <QHBoxLayout>
#include <QFileInfo>
#include <QApplication>
#include <QStyle>
#include <QLabel>
#include <QSplitter>
#if QT_VERSION >= 0x060200
#include <QAudioOutput>
#else
#include <QMediaPlaylist>
#endif
#include <QSlider>
#include "audioplayer.h"

namespace {

const QString zeroTime(QLatin1String(" 0:00"));

}

/**
 * Constructor.
 *
 * @param player audio player
 * @param parent parent widget
 */
PlayToolBar::PlayToolBar(AudioPlayer* player, QWidget* parent)
  : QToolBar(parent), m_player(player)
{
  setObjectName(QLatin1String("Kid3Player"));
  setWindowTitle(tr("Play"));

  m_playIcon = style()->standardIcon(QStyle::SP_MediaPlay);
  m_pauseIcon = style()->standardIcon(QStyle::SP_MediaPause);

  m_playOrPauseAction = new QAction(m_playIcon, tr("Play/Pause"), this);
  m_stopAction = new QAction(
    style()->standardIcon(QStyle::SP_MediaStop), tr("Stop playback"), this);
  m_previousAction = new QAction(
    style()->standardIcon(QStyle::SP_MediaSkipBackward), tr("Previous Track"), this);
  m_nextAction = new QAction(
    style()->standardIcon(QStyle::SP_MediaSkipForward), tr("Next Track"), this);
  QAction* closeAction = new QAction(
    style()->standardIcon(QStyle::SP_TitleBarCloseButton), tr("Close"), this);

  auto splitter = new QSplitter(this);
  m_titleLabel = new QLabel(splitter);

  QMediaPlayer* mediaPlayer = m_player->mediaPlayer();
  m_seekSlider = new QSlider(Qt::Horizontal, splitter);
  m_seekSlider->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  m_seekSlider->setMinimum(0);
  // Setting a maximum of 0 crashes with Qt 5.4.0 on Mac OS X.
  int maximum = mediaPlayer->duration() / 1000;
  if (maximum > 0) {
    m_seekSlider->setMaximum(maximum);
  }
  connect(m_seekSlider, &QAbstractSlider::actionTriggered,
          this, &PlayToolBar::seekAction);
  m_muteAction = new QAction(
        style()->standardIcon(QStyle::SP_MediaVolume), tr("Mute"), this);
  m_volumeSlider = new QSlider(Qt::Horizontal, this);
  m_volumeSlider->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  m_volumeSlider->setRange(0, 100);
#if QT_VERSION >= 0x060200
  int volume = mediaPlayer->audioOutput()->volume() * 100.0f;
#else
  int volume = mediaPlayer->volume();
#endif
  m_volumeSlider->setValue(volume);
  setVolumeToolTip(volume);
  connect(m_volumeSlider, &QAbstractSlider::actionTriggered,
          this, &PlayToolBar::volumeAction);

  m_timeLcd = new QLCDNumber(this);
  m_timeLcd->setSegmentStyle(QLCDNumber::Flat);
  m_timeLcd->setFrameStyle(QFrame::NoFrame);
  m_timeLcd->display(zeroTime);
  m_timeLcd->setDigitCount(7);

  addAction(m_playOrPauseAction);
  addAction(m_stopAction);
  addAction(m_previousAction);
  addAction(m_nextAction);
  addWidget(splitter);
  addAction(m_muteAction);
  addWidget(m_volumeSlider);
  addWidget(m_timeLcd);
  addAction(closeAction);

#if QT_VERSION >= 0x060200
  connect(mediaPlayer, &QMediaPlayer::playbackStateChanged,
          this, &PlayToolBar::stateChanged);
  connect(mediaPlayer, &QMediaPlayer::errorOccurred,
          this, &PlayToolBar::error);
  connect(mediaPlayer->audioOutput(), &QAudioOutput::volumeChanged,
          this, [this](float volume) { setVolumeToolTip(volume * 100.0f); });
#else
  connect(mediaPlayer, &QMediaPlayer::stateChanged,
          this, &PlayToolBar::stateChanged);
  connect(mediaPlayer, static_cast<void (QMediaPlayer::*)(QMediaPlayer::Error)>(
            &QMediaPlayer::error),
          this, &PlayToolBar::error);
  connect(mediaPlayer, &QMediaPlayer::volumeChanged,
          this, &PlayToolBar::setVolumeToolTip);
#endif
  connect(mediaPlayer, &QMediaPlayer::durationChanged,
          this, &PlayToolBar::durationChanged);
  connect(m_muteAction, &QAction::triggered, this, &PlayToolBar::toggleMute);
  connect(m_player, &AudioPlayer::positionChanged, this, &PlayToolBar::tick);
  connect(m_player, &AudioPlayer::trackChanged,
          this, &PlayToolBar::trackChanged);
  connect(m_player, &AudioPlayer::aboutToPlay,
          this, &PlayToolBar::aboutToPlay);
  connect(m_playOrPauseAction, &QAction::triggered,
          m_player, &AudioPlayer::playOrPause);
  connect(m_stopAction, &QAction::triggered, m_player, &AudioPlayer::stop);
  connect(m_previousAction, &QAction::triggered, m_player, &AudioPlayer::previous);
  connect(m_nextAction, &QAction::triggered, m_player, &AudioPlayer::next);
  connect(closeAction, &QAction::triggered, this, &QWidget::close);

#ifdef Q_OS_MAC
  setStyleSheet(QLatin1String("QToolButton { border: 0; }"));
#endif
}

/**
 * Destructor.
  */
PlayToolBar::~PlayToolBar()
{
  m_player->stop();
  emit closed();
}

/**
 * Stop sound when window is closed.
 */
void PlayToolBar::closeEvent(QCloseEvent*)
{
  m_player->stop();
  emit closed();
}

/**
 * Update displayed time.
 *
 * @param msec time in ms
 */
void PlayToolBar::tick(qint64 msec)
{
  int hours = msec / (60 * 60 * 1000);
  int minutes = (msec / (60 * 1000)) % 60;
  int seconds = (msec / 1000) % 60;
  if (msec % 1000 >= 500) {
    ++seconds;
  }
  if (hours == 0) {
    m_timeLcd->display(QString(QLatin1String("%1:%2"))
                       .arg(minutes, 2, 10, QLatin1Char(' '))
                       .arg(seconds, 2, 10, QLatin1Char('0')));
  } else {
    m_timeLcd->display(QString(QLatin1String("%1:%2:%3"))
                       .arg(hours, 2, 10, QLatin1Char(' '))
                       .arg(minutes, 2, 10, QLatin1Char('0'))
                       .arg(seconds, 2, 10, QLatin1Char('0')));
  }
  if (!m_seekSlider->isSliderDown()) {
    m_seekSlider->setValue(msec / 1000);
  }
}

/**
 * Update button states when the Phonon state changed.
 *
 * @param newState new Phonon state
 */
void PlayToolBar::stateChanged(int newState)
{
  switch (newState) {
    case QMediaPlayer::PlayingState:
      m_playOrPauseAction->setEnabled(true);
      m_playOrPauseAction->setIcon(m_pauseIcon);
      m_stopAction->setEnabled(true);
      break;
    case QMediaPlayer::PausedState:
      m_playOrPauseAction->setEnabled(true);
      m_playOrPauseAction->setIcon(m_playIcon);
      m_stopAction->setEnabled(true);
      break;
    case QMediaPlayer::StoppedState:
      m_playOrPauseAction->setEnabled(true);
      m_playOrPauseAction->setIcon(m_playIcon);
      m_stopAction->setEnabled(false);
      m_timeLcd->display(zeroTime);
      break;
    default:
      m_playOrPauseAction->setEnabled(false);
      break;
  }
}

/**
 * Update states when a media player error occurs.
 *
 * @param err error
 */
void PlayToolBar::error(QMediaPlayer::Error err)
{
  Q_UNUSED(err)
  m_playOrPauseAction->setEnabled(false);
  m_stopAction->setEnabled(false);
  emit errorMessage(m_player->mediaPlayer()->errorString());
}

/**
 * Called when the duration changes.
 * @param duration duration in milliseconds
 */
void PlayToolBar::durationChanged(qint64 duration)
{
  int maximum = duration / 1000;
  // Setting a maximum of 0 crashes with Qt 5.4.0 on Mac OS X.
  if (maximum > 0) {
    m_seekSlider->setMaximum(maximum);
  }
}

/**
 * Called when the volume changes.
 * @param volume current volume (0..100)
 */
void PlayToolBar::setVolumeToolTip(int volume)
{
  m_volumeSlider->setToolTip(tr("Volume: %1%").arg(volume));
}

/**
 * Set current position in track when slider position is changed.
 * @param action slider action
 */
void PlayToolBar::seekAction(int action)
{
  Q_UNUSED(action);
  m_player->setCurrentPosition(m_seekSlider->sliderPosition() * 1000);
}

/**
 * Set volume when slider position is changed.
 * @param action slider action
 */
void PlayToolBar::volumeAction(int action)
{
  Q_UNUSED(action);
#if QT_VERSION >= 0x060200
  m_player->mediaPlayer()->audioOutput()->setVolume(
        static_cast<float>(m_volumeSlider->sliderPosition()) / 100.0f);
#else
  m_player->mediaPlayer()->setVolume(m_volumeSlider->sliderPosition());
#endif
}

/**
 * Toggle muted state.
 */
void PlayToolBar::toggleMute()
{
#if QT_VERSION >= 0x060200
  bool muted = !m_player->mediaPlayer()->audioOutput()->isMuted();
  m_player->mediaPlayer()->audioOutput()->setMuted(muted);
#else
  bool muted = !m_player->mediaPlayer()->isMuted();
  m_player->mediaPlayer()->setMuted(muted);
#endif
  m_muteAction->setIcon(style()->standardIcon(muted
      ? QStyle::SP_MediaVolumeMuted : QStyle::SP_MediaVolume));
}

/**
 * Update display and button state when the current track is changed.
 *
 * @param filePath path of currently played audio file
 * @param hasPrevious true if a previous track is available
 * @param hasNext true if a next track is available
 */
void  PlayToolBar::trackChanged(const QString& filePath,
                                bool hasPrevious, bool hasNext)
{
  QFileInfo fi(filePath);
  m_titleLabel->setText(fi.fileName());

  m_previousAction->setEnabled(hasPrevious);
  m_nextAction->setEnabled(hasNext);

  int maximum = m_player->mediaPlayer()->duration() / 1000;
  // Setting a maximum of 0 crashes with Qt 5.4.0 on Mac OS X.
  if (maximum > 0) {
    m_seekSlider->setMaximum(maximum);
  }
}
