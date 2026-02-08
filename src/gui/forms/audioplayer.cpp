/**
 * \file audioplayer.cpp
 * Audio player.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 03-Aug-2011
 *
 * Copyright (C) 2011-2024  Urs Fleisch
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

#include <QFile>
#include <QUrl>
#include <QMediaPlayer>
#if QT_VERSION >= 0x060200
#include <QAudioOutput>
#include <QAudioDevice>
#include <QMediaDevices>
#include "guiconfig.h"
#else
#include <QMediaPlaylist>
#endif
#include "kid3application.h"
#include "taggedfile.h"
#include "fileproxymodel.h"

#if QT_VERSION >= 0x060200

class MediaPlaylist : public QObject {
public:
  explicit MediaPlaylist(AudioPlayer* parent)
    : QObject(parent), m_audioPlayer(parent), m_currentPos(-1) {
  }

  ~MediaPlaylist() override;

  void clear() {
    m_playlist.clear();
  }

  void addMedia(const QUrl& content) {
    m_playlist.append(content);
  }

  int currentIndex() const {
    return m_currentPos;
  }

  int mediaCount() const {
    return m_playlist.size();
  }

  QUrl currentMedia() const {
    return m_currentPos >= 0 && m_currentPos < m_playlist.size()
        ? m_playlist.at(m_currentPos) : QUrl();
  }

  void setCurrentIndex(int playlistPosition) {
    if (playlistPosition >= 0 && playlistPosition < m_playlist.size() &&
        m_currentPos != playlistPosition) {
      m_currentPos = playlistPosition;
      m_audioPlayer->currentIndexChanged(m_currentPos);
    }
  }

  void previous() {
    setCurrentIndex(m_currentPos - 1);
  }

  void next() {
    setCurrentIndex(m_currentPos + 1);
  }

private:
  AudioPlayer* m_audioPlayer;
  QList<QUrl> m_playlist;
  int m_currentPos;
};

MediaPlaylist::~MediaPlaylist() = default;

#endif


/**
 * Constructor.
 *
 * @param app parent application
 */
AudioPlayer::AudioPlayer(Kid3Application* app) : QObject(app),
  m_app(app)
{
  setObjectName(QLatin1String("AudioPlayer"));

  m_mediaPlayer = new QMediaPlayer(this);
#if QT_VERSION >= 0x060200
  m_mediaPlaylist = new MediaPlaylist(this);
  m_mediaDevices = new QMediaDevices(this);
  m_audioOutput = new QAudioOutput(this);
  const GuiConfig& guiCfg = GuiConfig::instance();
  setPreferredAudioOutput();
  m_mediaPlayer->setAudioOutput(m_audioOutput);
  connect(m_mediaPlayer, &QMediaPlayer::positionChanged,
          this, &AudioPlayer::positionChanged);
  connect(m_mediaPlayer, &QMediaPlayer::playbackStateChanged,
          this, &AudioPlayer::onStateChanged);
  connect(m_mediaPlayer, &QMediaPlayer::mediaStatusChanged,
          this, &AudioPlayer::onMediaStatusChanged);
  connect(m_audioOutput, &QAudioOutput::volumeChanged,
          this, &AudioPlayer::volumeChanged);
  connect(m_mediaDevices, &QMediaDevices::audioOutputsChanged,
          this, &AudioPlayer::setPreferredAudioOutput);
  connect(&guiCfg, &GuiConfig::preferredAudioOutputChanged,
          this, &AudioPlayer::setPreferredAudioOutput);
#else
  m_mediaPlaylist = new QMediaPlaylist(m_mediaPlayer);
  m_mediaPlayer->setPlaylist(m_mediaPlaylist);
  connect(m_mediaPlaylist, &QMediaPlaylist::currentIndexChanged,
          this, &AudioPlayer::currentIndexChanged);
  connect(m_mediaPlayer, &QMediaPlayer::positionChanged,
          this, &AudioPlayer::positionChanged);
  connect(m_mediaPlayer, &QMediaPlayer::stateChanged,
          this, &AudioPlayer::onStateChanged);
  connect(m_mediaPlayer, &QMediaPlayer::volumeChanged,
          this, &AudioPlayer::volumeChanged);
#endif
}

/**
 * Set files to be played.
 *
 * @param files  paths to files
 * @param fileNr index of file to play (default 0), -1 to set without playing
 */
void AudioPlayer::setFiles(const QStringList& files, int fileNr)
{
  m_mediaPlaylist->clear();
  for (const QString& file : files) {
    m_mediaPlaylist->addMedia(QUrl::fromLocalFile(file));
  }
  if (fileNr != -1) {
    m_mediaPlaylist->setCurrentIndex(fileNr);
    m_mediaPlayer->play();
  } else {
    m_mediaPlaylist->setCurrentIndex(0);
  }
  emit fileCountChanged(getFileCount());
}

/**
 * Get number of files in play list.
 * @return number of files.
 */
int AudioPlayer::getFileCount() const
{
  return m_mediaPlaylist->mediaCount();
}

/**
 * Get path of current file.
 * @return file name.
 */
QString AudioPlayer::getFileName() const
{
#if QT_VERSION >= 0x060200
  return m_mediaPlaylist->currentMedia().toLocalFile();
#elif QT_VERSION >= 0x050e00
  return m_mediaPlaylist->currentMedia().request().url().toLocalFile();
#else
  return m_mediaPlaylist->currentMedia().canonicalUrl().toLocalFile();
#endif
}

/**
 * Get tagged file for current file.
 * @return tagged file, 0 if not available.
 */
TaggedFile* AudioPlayer::getTaggedFile() const
{
  FileProxyModel* model = m_app->getFileProxyModel();
  if (QModelIndex index = model->index(getFileName()); index.isValid()) {
    return FileProxyModel::getTaggedFileOfIndex(index);
  }
  return nullptr;
}

/**
 * Get index of current file in playlist.
 * @return index of current file.
 */
int AudioPlayer::getCurrentIndex() const
{
  return m_mediaPlaylist->currentIndex();
}

/**
 * Get the current playback position in milliseconds.
 * @return time in milliseconds.
 */
quint64 AudioPlayer::getCurrentPosition() const
{
  return m_mediaPlayer->position();
}

/**
 * Set the current playback position.
 * @param position time in milliseconds
 */
void AudioPlayer::setCurrentPosition(quint64 position)
{
  m_mediaPlayer->setPosition(position);
  emit currentPositionChanged(position);
}

/**
 * Get playing state.
 * @return state.
 */
AudioPlayer::State AudioPlayer::getState() const
{
#if QT_VERSION >= 0x060200
  switch (m_mediaPlayer->playbackState())
#else
  switch (m_mediaPlayer->state())
#endif
  {
  case QMediaPlayer::StoppedState:
    return StoppedState;
  case QMediaPlayer::PlayingState:
    return PlayingState;
  case QMediaPlayer::PausedState:
    return PausedState;
  }
  return StoppedState;
}

/**
 * Signal stateChanged() when the playing state is changed.
 */
void AudioPlayer::onStateChanged()
{
  emit stateChanged(getState());
}

#if QT_VERSION >= 0x060200
/**
 * Go to next track when end of media reached.
 * @param status media status (QMediaPlayer::MediaStatus)
 */
void AudioPlayer::onMediaStatusChanged(int status)
{
  if (status == QMediaPlayer::EndOfMedia &&
      m_mediaPlaylist->currentIndex() + 1 < m_mediaPlaylist->mediaCount()) {
    m_mediaPlaylist->next();
    m_mediaPlayer->play();
  }
}

/**
 * Set the output device to the preferred audio output.
 */
void AudioPlayer::setPreferredAudioOutput()
{
  const GuiConfig& guiCfg = GuiConfig::instance();
  QString description = guiCfg.preferredAudioOutput();
  QByteArray preferredId;
  if (description.endsWith(QLatin1Char(']'))) {
    int idPos = description.lastIndexOf(QLatin1Char('['));
    if (idPos != -1) {
      preferredId = description.mid(idPos + 1,
                           description.length() - idPos - 2).toLatin1();
    }
  }

  QAudioDevice defaultAudioOutput = QMediaDevices::defaultAudioOutput();
  QByteArray defaultId = defaultAudioOutput.id();
  QList<QAudioDevice> audioOutputs = QMediaDevices::audioOutputs();
  int preferredIndex = -1;
  int defaultIndex = -1;
  for (int i = 0; i < audioOutputs.size(); ++i) {
    QByteArray id = audioOutputs.at(i).id();
    if (preferredId == id) {
      preferredIndex = i;
    }
    if (defaultId == id) {
      defaultIndex = i;
    }
  }

  QAudioDevice currentAudioOutput = m_audioOutput->device();
  if (preferredIndex >= 0 && preferredIndex < audioOutputs.size()) {
    if (currentAudioOutput.id() != preferredId) {
      qDebug("Changing audio output to %s",
             qPrintable(QString::fromUtf8(preferredId)));
      m_audioOutput->setDevice(audioOutputs.at(preferredIndex));
    }
  } else if (defaultIndex >= 0 && defaultIndex < audioOutputs.size()) {
    if (currentAudioOutput.id() != defaultId) {
      qDebug("Changing audio output to default %s",
             qPrintable(QString::fromUtf8(defaultId)));
      m_audioOutput->setDevice(audioOutputs.at(defaultIndex));
    }
  } else if (!audioOutputs.isEmpty()) {
    qDebug("Falling back to first audio output %s",
           qPrintable(QString::fromUtf8(audioOutputs.first().id())));
    m_audioOutput->setDevice(audioOutputs.first());
  }
}

#endif

/**
 * Get duration of current track in milliseconds.
 * @return duration.
 */
qint64 AudioPlayer::getDuration() const
{
  return m_mediaPlayer->duration();
}

/**
 * Get volume.
 * @return volume level between 0 and 100.
 */
int AudioPlayer::getVolume() const
{
#if QT_VERSION >= 0x060200
  return m_audioOutput->volume() * 100.0f;
#else
  return m_mediaPlayer->volume();
#endif
}

/**
 * Set volume.
 * @param volume level between 0 and 100
 */
void AudioPlayer::setVolume(int volume)
{
#if QT_VERSION >= 0x060200
  m_audioOutput->setVolume(static_cast<float>(volume) / 100.0f);
#else
  m_mediaPlayer->setVolume(volume);
#endif
}

/**
 * Toggle between play and pause.
 */
void AudioPlayer::playOrPause()
{
  if (getFileCount() == 0) {
    // The play tool bar was restored without a play list.
    m_app->playAudio();
    return;
  }

#if QT_VERSION >= 0x060200
  switch (m_mediaPlayer->playbackState())
#else
  switch (m_mediaPlayer->state())
#endif
  {
  case QMediaPlayer::PlayingState:
    m_mediaPlayer->pause();
    break;
  case QMediaPlayer::PausedState:
  case QMediaPlayer::StoppedState:
  default:
    m_mediaPlayer->play();
    break;
  }
}

/**
 * Resume playback.
 */
void AudioPlayer::play()
{
  m_mediaPlayer->play();
}

/**
 * Pause playback.
 */
void AudioPlayer::pause()
{
  m_mediaPlayer->pause();
}

/**
 * Stop playback.
 */
void AudioPlayer::stop()
{
  m_mediaPlayer->stop();
}

/**
 * Update display and button state when the current source is changed.
 * @param position number of song in play list
 */
void AudioPlayer::currentIndexChanged(int position)
{
  if (position >= 0 && position < m_mediaPlaylist->mediaCount()) {
#if QT_VERSION >= 0x060200
    auto state = m_mediaPlayer->playbackState();
    QString filePath = m_mediaPlaylist->currentMedia().toLocalFile();
    m_mediaPlayer->setSource(m_mediaPlaylist->currentMedia());
    if (state == QMediaPlayer::PlayingState) {
      m_mediaPlayer->play();
    }
#elif QT_VERSION >= 0x050e00
    QString filePath =
        m_mediaPlaylist->currentMedia().request().url().toLocalFile();
#else
    QString filePath =
        m_mediaPlaylist->currentMedia().canonicalUrl().toLocalFile();
#endif
    emit aboutToPlay(filePath);
    emit trackChanged(
          filePath,
          position > 0, position + 1 < m_mediaPlaylist->mediaCount());
  }
}

/**
 * Select previous track.
 */
void AudioPlayer::previous()
{
  m_mediaPlaylist->previous();
}

/**
 * Select next track.
 */
void AudioPlayer::next()
{
  m_mediaPlaylist->next();
}
