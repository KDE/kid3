/**
 * \file playtoolbar.h
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

#ifndef PLAYTOOLBAR_H
#define PLAYTOOLBAR_H

#include "config.h"
#include <qtoolbar.h>

#ifdef HAVE_PHONON

#include <QStringList>
#include <QIcon>
#include <phonon/phononnamespace.h>

class QAction;
class QLCDNumber;
class QLabel;

namespace Phonon
{
    class AudioOutput;
    class MediaObject;
    class SeekSlider;
    class VolumeSlider;
}

#else // HAVE_PHONON

#if QT_VERSION < 0x040000
/** 64-bit signed integer */
typedef Q_INT64 qint64;
#endif
namespace Phonon {
	/** empty enum to make generated moc files compile without Phonon */
	enum State {};
}

#endif // HAVE_PHONON

/**
 * Audio player toolbar.
 */
class PlayToolBar : public QToolBar {
Q_OBJECT

#ifdef HAVE_PHONON
public:
	/**
	 * Constructor.
	 *
	 * @param parent parent widget
	 */
	PlayToolBar(QWidget* parent);

	/**
	 * Destructor.
	 */
	virtual ~PlayToolBar();

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

#endif // HAVE_PHONON
signals:
	/**
	 * Emitted when an error occurs.
	 * Parameter: description of error
	 */
	void errorMessage(const QString& msg);

private slots:
	/**
	 * Toggle between play and pause.
	 */
	void playOrPause();

	/**
	 * Update display and button state when the current source is changed.
	 */
	void currentSourceChanged();

	/**
	 * Stop playback.
	 */
	void stop();

	/**
	 * Update displayed time.
	 *
	 * @param msec time in ms
	 */
	void tick(qint64 msec);

	/**
	 * Update button states when the Phonon state changed.
	 *
	 * @param newState new Phonon state
	 */
	void stateChanged(Phonon::State newState);

	/**
	 * Queue next track when the current track is about to finish.
	 */
	void aboutToFinish();

	/**
	 * Select previous track.
	 */
	void previous();

	/**
	 * Select next track.
	 */
	void next();

#ifdef HAVE_PHONON
protected:
	/**
	 * Stop sound when window is closed.
	 */
	virtual void closeEvent(QCloseEvent*);

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

	QIcon m_playIcon;
	QIcon m_pauseIcon;

	QAction* m_playOrPauseAction;
	QAction* m_stopAction;
	QAction* m_previousAction;
	QAction* m_nextAction;

	QLCDNumber* m_timeLcd;
	QLabel* m_titleLabel;

	QStringList m_files;
	int m_fileNr;
#endif // HAVE_PHONON
};

#endif
