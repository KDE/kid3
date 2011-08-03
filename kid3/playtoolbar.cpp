/**
 * \file playtoolbar.cpp
 * Audio player toolbar.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 24-Aug-2010
 *
 * Copyright (C) 2010  Urs Fleisch
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

#ifdef HAVE_PHONON

#include <QAction>
#include <QLCDNumber>
#include <QHBoxLayout>
#include <QFileInfo>
#include <QApplication>
#include <QStyle>
#include <QLabel>
#include <QSplitter>
#include <phonon/mediaobject.h>
#include <phonon/seekslider.h>
#include <phonon/volumeslider.h>
#include "audioplayer.h"
#include "qtcompatmac.h"

static const QString zeroTime(" 0:00");

/**
 * Constructor.
 *
 * @param player audio player
 * @param parent parent widget
 */
PlayToolBar::PlayToolBar(AudioPlayer* player, QWidget* parent) :
  QToolBar(parent), m_player(player)
{
  setObjectName("Kid3Player");
  setWindowTitle(i18n("Play"));

  m_playIcon = style()->standardIcon(QStyle::SP_MediaPlay);
  m_pauseIcon = style()->standardIcon(QStyle::SP_MediaPause);

  m_playOrPauseAction = new QAction(m_playIcon, i18n("Play/Pause"), this);
  m_stopAction = new QAction(
    style()->standardIcon(QStyle::SP_MediaStop), i18n("Stop playback"), this);
  m_previousAction = new QAction(
    style()->standardIcon(QStyle::SP_MediaSkipBackward), i18n("Previous Track"), this);
  m_nextAction = new QAction(
    style()->standardIcon(QStyle::SP_MediaSkipForward), i18n("Next Track"), this);
  QAction* closeAction = new QAction(
    style()->standardIcon(QStyle::SP_TitleBarCloseButton), i18n("Close"), this);

  QSplitter* splitter = new QSplitter(this);
  m_titleLabel = new QLabel(splitter);

  Phonon::MediaObject* mediaObject = m_player->mediaObject();
  Phonon::SeekSlider* seekSlider = new Phonon::SeekSlider(splitter);
  seekSlider->setMediaObject(mediaObject);
  seekSlider->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  seekSlider->setIconVisible(false);
  Phonon::VolumeSlider* volumeSlider = new Phonon::VolumeSlider(this);
  volumeSlider->setAudioOutput(m_player->audioOutput());
  volumeSlider->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  m_timeLcd = new QLCDNumber(this);
  m_timeLcd->setSegmentStyle(QLCDNumber::Flat);
  m_timeLcd->setFrameStyle(QFrame::NoFrame);
  m_timeLcd->display(zeroTime);

  addAction(m_playOrPauseAction);
  addAction(m_stopAction);
  addAction(m_previousAction);
  addAction(m_nextAction);
  addWidget(splitter);
  addWidget(volumeSlider);
  addWidget(m_timeLcd);
  addAction(closeAction);

  connect(mediaObject, SIGNAL(tick(qint64)), this, SLOT(tick(qint64)));
  connect(mediaObject, SIGNAL(stateChanged(Phonon::State, Phonon::State)),
          this, SLOT(stateChanged(Phonon::State)));
  connect(m_player, SIGNAL(trackChanged(QString,bool,bool)),
          this, SLOT(trackChanged(QString,bool,bool)));
  connect(m_playOrPauseAction, SIGNAL(triggered()),
          m_player, SLOT(playOrPause()));
  connect(m_stopAction, SIGNAL(triggered()), m_player, SLOT(stop()));
  connect(m_previousAction, SIGNAL(triggered()), m_player, SLOT(previous()));
  connect(m_nextAction, SIGNAL(triggered()), m_player, SLOT(next()));
  connect(closeAction, SIGNAL(triggered()), this, SLOT(close()));
}

/**
 * Destructor.
  */
PlayToolBar::~PlayToolBar()
{
}

/**
 * Stop sound when window is closed.
 */
void PlayToolBar::closeEvent(QCloseEvent*)
{
  m_player->stop();;
}

/**
 * Update displayed time.
 *
 * @param msec time in ms
 */
void PlayToolBar::tick(qint64 msec)
{
  int minutes = (msec / (60 * 1000)) % 60;
  int seconds = (msec / 1000) % 60;
  m_timeLcd->display(QString("%1:%2").arg(minutes, 2, 10, QChar(' '))
                     .arg(seconds, 2, 10, QChar('0')));
}

/**
 * Update button states when the Phonon state changed.
 *
 * @param newState new Phonon state
 */
void PlayToolBar::stateChanged(Phonon::State newState)
{
  switch (newState) {
    case Phonon::ErrorState:
      m_playOrPauseAction->setEnabled(false);
      m_stopAction->setEnabled(false);
      emit errorMessage(m_player->mediaObject()->errorString());
      break;
    case Phonon::PlayingState:
      m_playOrPauseAction->setEnabled(true);
      m_playOrPauseAction->setIcon(m_pauseIcon);
      m_stopAction->setEnabled(true);
      break;
    case Phonon::PausedState:
      m_playOrPauseAction->setEnabled(true);
      m_playOrPauseAction->setIcon(m_playIcon);
      m_stopAction->setEnabled(true);
      break;
    case Phonon::StoppedState:
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
 * Update display and button state when the current track is changed.
 *
 * @param filePath path of currently played audio file
 * @param hasPrevious true if a previous track is available
 * @param hasNext true if a next track is available
 */
void  PlayToolBar::trackChanged(const QString& filePath,
                                bool hasPrevious, bool hasNext)
{
  m_playOrPauseAction->setIcon(m_pauseIcon);
  m_timeLcd->display(zeroTime);
  QFileInfo fi(filePath);
  m_titleLabel->setText(fi.fileName());

  m_previousAction->setEnabled(hasPrevious);
  m_nextAction->setEnabled(hasNext);
}

#endif // HAVE_PHONON
