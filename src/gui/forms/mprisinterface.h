/**
 * \file mprisinterface.h
 * MPRIS D-Bus interface for audio player.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 09-Dec-2016
 *
 * Copyright (C) 2016-2024  Urs Fleisch
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
#include "config.h"

#ifdef HAVE_QTDBUS

#include <QDBusAbstractAdaptor>
#include <QStringList>

class QDBusObjectPath;
class QTemporaryFile;
class AudioPlayer;

/**
 * MPRIS D-Bus Interface MediaPlayer2.
 * See https://specifications.freedesktop.org/mpris-spec/2.2/
 */
class MprisInterface : public QDBusAbstractAdaptor {
  Q_OBJECT
  Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2")
  /** false, Quit() is not supported. */
  Q_PROPERTY(bool CanQuit READ canQuit CONSTANT)
  /** false, not full screen. */
  Q_PROPERTY(bool Fullscreen READ fullscreen CONSTANT)
  /** false, Fullscreen is not supported. */
  Q_PROPERTY(bool CanSetFullscreen READ canSetFullscreen CONSTANT)
  /** false, Raise() is not supported. */
  Q_PROPERTY(bool CanRaise READ canRaise CONSTANT)
  /** false, org.mpris.MediaPlayer2.TrackList interface is not implemented. */
  Q_PROPERTY(bool HasTrackList READ hasTrackList CONSTANT)
  /** Media player identification "Kid3". */
  Q_PROPERTY(QString Identity READ identity CONSTANT)
  /** Base name of desktop file "kid3". */
  Q_PROPERTY(QString DesktopEntry READ desktopEntry CONSTANT)
  /** URI schemes supported by the media player, ["file"]. */
  Q_PROPERTY(QStringList SupportedUriSchemes READ supportedUriSchemes CONSTANT)
  /** MIME types supported by the media player. */
  Q_PROPERTY(QStringList SupportedMimeTypes READ supportedMimeTypes CONSTANT)

public:
  /**
   * Constructor.
   *
   * @param player parent audio player
   */
  explicit MprisInterface(AudioPlayer* player);

  /**
   * Destructor.
   */
  ~MprisInterface() override = default;

public slots:
  /**
   * Bring media player to front, not implemented.
   */
  void Raise() {}

  /**
   * Terminate media player, not implemented.
   */
  void Quit() {}

private:
  bool canQuit() const { return false; }
  bool fullscreen() const { return false; }
  bool canSetFullscreen() const { return false; }
  bool canRaise() const { return false; }
  bool hasTrackList() const { return false; }
  QString identity() const;
  QString desktopEntry() const;
  QStringList supportedUriSchemes() const;
  QStringList supportedMimeTypes() const;

  AudioPlayer* m_audioPlayer;
};


/**
 * MPRIS D-Bus Interface MediaPlayer2.Player.
 * See https://specifications.freedesktop.org/mpris-spec/2.2/
 */
class MprisPlayerInterface : public QDBusAbstractAdaptor {
  Q_OBJECT
  Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2.Player")
  /** Playback status, "Playing", "Paused" or "Stopped". */
  Q_PROPERTY(QString PlaybackStatus READ playbackStatus)
  /** Loop status, "None". */
  Q_PROPERTY(QString LoopStatus READ loopStatus CONSTANT)
  /** Playback rate 1.0. */
  Q_PROPERTY(double Rate READ rate CONSTANT)
  /** false, shuffle is not implemented. */
  Q_PROPERTY(bool Shuffle READ shuffle CONSTANT)
  /** Map with metadata. */
  Q_PROPERTY(QVariantMap Metadata READ metadata)
  /** Current volume, value between 0.0 and 1.0. */
  Q_PROPERTY(double Volume READ volume WRITE setVolume)
  /** Current track position in microseconds. */
  Q_PROPERTY(qlonglong Position READ position)
  /** Minimum playback rate 1.0. */
  Q_PROPERTY(double MinimumRate READ minimumRate CONSTANT)
  /** Maximum playback rate 1.0. */
  Q_PROPERTY(double MaximumRate READ maximumRate CONSTANT)
  /** true if there is a next track. */
  Q_PROPERTY(bool CanGoNext READ canGoNext)
  /** true if there is a previous track. */
  Q_PROPERTY(bool CanGoPrevious READ canGoPrevious)
  /** true if there is a current track. */
  Q_PROPERTY(bool CanPlay READ canPlay)
  /** true if there is a current track. */
  Q_PROPERTY(bool CanPause READ canPause)
  /** true, Seek() and SetPosition() are implemented. */
  Q_PROPERTY(bool CanSeek READ canSeek)
  /** true, media player can be controlled. */
  Q_PROPERTY(bool CanControl READ canControl CONSTANT)

public:
  /**
   * Constructor.
   *
   * @param player parent audio player
   */
  explicit MprisPlayerInterface(AudioPlayer* player);

  /**
   * Destructor.
   */
  ~MprisPlayerInterface() override;

public slots:
  /**
   * Skip to next track in tracklist.
   */
  void Next();

  /**
   * Skip to previous track in tracklist.
   */
  void Previous();

  /**
   * Pause playback.
   */
  void Pause();

  /**
   * If playback is paused, resume playback, if playback is stopped,
   * start playback.
   */
  void PlayPause();

  /**
   * Stop playback.
   */
  void Stop();

  /**
   * Start or resume playback.
   */
  void Play();

  /**
   * Seek forward in the current track by the specified number of microseconds.
   *
   * A negative value seeks back. If this would mean seeking back further than
   * the start of the track, the position is set to 0. If the value passed in
   * would mean seeking beyond the end of the track, acts like a call to Next().
   *
   * @param offsetUs microseconds to seek forward
   */
  void Seek(qlonglong offsetUs);

  /**
   * Set the current track position in microseconds.
   *
   * If @a positionUs is less than 0, do nothing. If @a positionUs is greater
   * than the track length, do nothing.
   *
   * @param trackId the currently playing track's identifier
   * @param positionUs track position in microseconds
   */
  void SetPosition(const QDBusObjectPath& trackId, qlonglong positionUs);

  /**
   * Open file.
   * @param uri URL of track to load
   */
  void OpenUri(const QString& uri);

signals:
  /**
   * Indicates that the track position has changed in a way that is
   * inconsistent with the current playing state.
   * @param positionUs new position in microseconds
   */
  void Seeked(qlonglong positionUs);

private slots:
  void onStateChanged();
  void onTrackChanged(const QString& filePath, bool hasPrevious, bool hasNext);
  void onVolumeChanged();
  void onFileCountChanged(int count);
  void onCurrentPositionChanged(qint64 position);

private:
  QString playbackStatus() const;
  QString loopStatus() const { return QLatin1String("None"); }
  double rate() const { return 1.0; }
  bool shuffle() const { return false; }
  QVariantMap metadata() const;
  double volume() const;
  void setVolume(double volume);
  qlonglong position() const;
  double minimumRate() const { return 1.0; }
  double maximumRate() const { return 1.0; }
  bool canGoNext() const;
  bool canGoPrevious() const;
  bool canPlay() const;
  bool canPause() const;
  bool canSeek() const { return true; }
  bool canControl() const { return true; }

  void sendPropertiesChangedSignal(const QString& name, const QVariant& value);
  QDBusObjectPath getCurrentTrackId() const;
  QString findCoverArtInDirectory(const QString& dirPath) const;

  AudioPlayer* m_audioPlayer;
  QString m_status;
  bool m_hasPrevious;
  bool m_hasNext;
  bool m_hasFiles;
  mutable QTemporaryFile* m_tempCoverArtFile;
  mutable QString m_coverArtDirName;
  mutable QString m_coverArtFileName;
};

#else

// Just to suppress moc "No relevant classes found" warning.
class MprisInterface : public QObject {
  Q_OBJECT
};

#endif
