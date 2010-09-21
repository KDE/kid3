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

#include "qtcompatmac.h"
#include <QAction>
#include <QLCDNumber>
#include <QHBoxLayout>
#include <QFileInfo>
#include <QApplication>
#include <QStyle>
#include <QLabel>
#include <phonon/audiooutput.h>
#include <phonon/mediaobject.h>
#include <phonon/seekslider.h>
#include <phonon/volumeslider.h>

static const QString zeroTime(" 0:00");

/**
 * Constructor.
 *
 * @param parent parent widget
 */
PlayToolBar::PlayToolBar(QWidget* parent) : QToolBar(parent), m_fileNr(-1)
{
	setObjectName("Kid3Player");

	m_playIcon = style()->standardIcon(QStyle::SP_MediaPlay);
	m_pauseIcon = style()->standardIcon(QStyle::SP_MediaPause);

	m_mediaObject = new Phonon::MediaObject(this);
	m_mediaObject->setTickInterval(1000);
	m_audioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);
	Phonon::createPath(m_mediaObject, m_audioOutput);

	m_playOrPauseAction = new QAction(m_playIcon, i18n("Play/Pause"), this);
	m_stopAction = new QAction(
		style()->standardIcon(QStyle::SP_MediaStop), i18n("Stop playback"), this);
	m_previousAction = new QAction(
		style()->standardIcon(QStyle::SP_MediaSkipBackward), i18n("Previous Track"), this);
	m_nextAction = new QAction(
		style()->standardIcon(QStyle::SP_MediaSkipForward), i18n("Next Track"), this);
	QAction* closeAction = new QAction(
		style()->standardIcon(QStyle::SP_TitleBarCloseButton), i18n("Close"), this);

	m_titleLabel = new QLabel(this);
	m_titleLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

	Phonon::SeekSlider* seekSlider = new Phonon::SeekSlider(this);
	seekSlider->setMediaObject(m_mediaObject);
	seekSlider->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	Phonon::VolumeSlider* volumeSlider = new Phonon::VolumeSlider(this);
	volumeSlider->setAudioOutput(m_audioOutput);
	volumeSlider->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	m_timeLcd = new QLCDNumber(this);
	m_timeLcd->setSegmentStyle(QLCDNumber::Flat);
	m_timeLcd->setFrameStyle(QFrame::NoFrame);
	m_timeLcd->display(zeroTime);

	addWidget(m_titleLabel);
	addAction(m_playOrPauseAction);
	addAction(m_stopAction);
	addAction(m_previousAction);
	addAction(m_nextAction);
	addWidget(seekSlider);
	addWidget(volumeSlider);
	addWidget(m_timeLcd);
	addAction(closeAction);

	connect(m_mediaObject, SIGNAL(tick(qint64)), this, SLOT(tick(qint64)));
	connect(m_mediaObject, SIGNAL(stateChanged(Phonon::State, Phonon::State)),
					this, SLOT(stateChanged(Phonon::State)));
	connect(m_mediaObject, SIGNAL(aboutToFinish()),
					this, SLOT(aboutToFinish()));
	connect(m_mediaObject, SIGNAL(currentSourceChanged(const Phonon::MediaSource&)),
					this, SLOT(currentSourceChanged()));
	connect(m_playOrPauseAction, SIGNAL(triggered()),
					this, SLOT(playOrPause()));
	connect(m_stopAction, SIGNAL(triggered()), this, SLOT(stop()));
	connect(m_previousAction, SIGNAL(triggered()), this, SLOT(previous()));
	connect(m_nextAction, SIGNAL(triggered()), this, SLOT(next()));
	connect(closeAction, SIGNAL(triggered()), this, SLOT(close()));
}

/**
 * Destructor.
  */
PlayToolBar::~PlayToolBar()
{
}

/**
 * Set files to be played.
 *
 * @param files  paths to files
 * @param fileNr index of file to play (default 0)
 */
void PlayToolBar::setFiles(const QStringList& files, int fileNr)
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
void PlayToolBar::selectTrack(int fileNr, bool play)
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
			if (!m_playOrPauseAction->isEnabled()) {
				m_playOrPauseAction->setEnabled(true);
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
void PlayToolBar::playTrack(int fileNr)
{
	selectTrack(fileNr, true);
}

/**
 * Stop sound when window is closed.
 */
void PlayToolBar::closeEvent(QCloseEvent*)
{
	stop();
}

/**
 * Toggle between play and pause.
 */
void PlayToolBar::playOrPause()
{
	switch (m_mediaObject->state()) {
		case Phonon::PlayingState:
			m_mediaObject->pause();
			m_playOrPauseAction->setIcon(m_playIcon);
			break;
		case Phonon::PausedState:
			m_mediaObject->play();
			m_playOrPauseAction->setIcon(m_pauseIcon);
			break;
		default:
			playTrack(m_fileNr);
			break;
	}
}

/**
 * Update display and button state when the current source is changed.
 */
void PlayToolBar::currentSourceChanged()
{
	if (m_fileNr >= 0 && m_fileNr < m_files.size()) {
		m_playOrPauseAction->setIcon(m_pauseIcon);
		m_timeLcd->display(zeroTime);
		QFileInfo fi(m_files[m_fileNr]);
		m_titleLabel->setText(fi.fileName());

		m_previousAction->setEnabled(m_fileNr > 0);
		m_nextAction->setEnabled(m_fileNr + 1 < m_files.size());
	}
}

/**
 * Stop playback.
 */
void PlayToolBar::stop()
{
	m_mediaObject->stop();
	m_mediaObject->clearQueue();
	m_playOrPauseAction->setIcon(m_playIcon);
	m_timeLcd->display(zeroTime);
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
			emit errorMessage(m_mediaObject->errorString());
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
 * Queue next track when the current track is about to finish.
 */
void PlayToolBar::aboutToFinish()
{
	int nextFileNr = m_fileNr + 1;
	if (nextFileNr >= 0 && nextFileNr < m_files.size()) {
		m_fileNr = nextFileNr;
		Phonon::MediaSource source(m_files[m_fileNr]);
		m_mediaObject->enqueue(source);
	}
}

/**
 * Select previous track.
 */
void PlayToolBar::previous()
{
	if (m_fileNr > 0)
		selectTrack(m_fileNr - 1, m_mediaObject->state() == Phonon::PlayingState);
}

/**
 * Select next track.
 */
void PlayToolBar::next()
{
	if (m_fileNr + 1 < m_files.size())
		selectTrack(m_fileNr + 1, m_mediaObject->state() == Phonon::PlayingState);
}

#else // HAVE_PHONON

void PlayToolBar::playOrPause() {}
void PlayToolBar::currentSourceChanged() {}
void PlayToolBar::stop() {}
void PlayToolBar::tick(qint64) {}
void PlayToolBar::stateChanged(Phonon::State) {}
void PlayToolBar::aboutToFinish() {}
void PlayToolBar::previous() {}
void PlayToolBar::next() {}

#endif // HAVE_PHONON
