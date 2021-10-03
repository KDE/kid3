/**
 * \file playlistconfig.cpp
 * Configuration for playlist dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 16 Sep 2009
 *
 * Copyright (C) 2009-2018  Urs Fleisch
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

#include "playlistconfig.h"
#include "isettings.h"
#include "config.h"

int PlaylistConfig::s_index = -1;

/**
 * Constructor.
 */
PlaylistConfig::PlaylistConfig()
  : StoredConfig<PlaylistConfig>(QLatin1String("Playlist")),
    m_location(PL_CurrentDirectory), m_format(PF_M3U),
    m_fileNameFormat(QLatin1String("%{artist} - %{album}")),
    m_sortTagField(QLatin1String("%{track.3}")),
    m_infoFormat(QLatin1String("%{artist} - %{title}")),
    m_useFileNameFormat(false),
    m_onlySelectedFiles(false),
    m_useSortTagField(false), m_useFullPath(false), m_writeInfo(false)
{
}

/**
 * Copy constructor.
 * @param other instance to be copied
 */
PlaylistConfig::PlaylistConfig(const PlaylistConfig& other)
  : StoredConfig<PlaylistConfig>(QLatin1String("Playlist")),
    m_location(other.m_location),
    m_format(other.m_format),
    m_fileNameFormat(other.m_fileNameFormat),
    m_sortTagField(other.m_sortTagField),
    m_infoFormat(other.m_infoFormat),
    m_useFileNameFormat(other.m_useFileNameFormat),
    m_onlySelectedFiles(other.m_onlySelectedFiles),
    m_useSortTagField(other.m_useSortTagField),
    m_useFullPath(other.m_useFullPath),
    m_writeInfo(other.m_writeInfo)
{
}

/**
 * Assignment operator.
 * @param other instance to be copied
 * @return reference to this instance.
 */
PlaylistConfig& PlaylistConfig::operator=(const PlaylistConfig& other)
{
  if (&other != this) {
    m_location = other.m_location;
    m_format = other.m_format;
    m_fileNameFormat = other.m_fileNameFormat;
    m_sortTagField = other.m_sortTagField;
    m_infoFormat = other.m_infoFormat;
    m_useFileNameFormat = other.m_useFileNameFormat;
    m_onlySelectedFiles = other.m_onlySelectedFiles;
    m_useSortTagField = other.m_useSortTagField;
    m_useFullPath = other.m_useFullPath;
    m_writeInfo = other.m_writeInfo;
  }
  return *this;
}

/**
 * Persist configuration.
 *
 * @param config configuration
 */
void PlaylistConfig::writeToConfig(ISettings* config) const
{
  config->beginGroup(m_group);
  config->setValue(QLatin1String("UseFileNameFormat"),
                   QVariant(m_useFileNameFormat));
  config->setValue(QLatin1String("OnlySelectedFiles"),
                   QVariant(m_onlySelectedFiles));
  config->setValue(QLatin1String("UseSortTagField"),
                   QVariant(m_useSortTagField));
  config->setValue(QLatin1String("UseFullPath"), QVariant(m_useFullPath));
  config->setValue(QLatin1String("WriteInfo"), QVariant(m_writeInfo));
  config->setValue(QLatin1String("Location"),
                   QVariant(static_cast<int>(m_location)));
  config->setValue(QLatin1String("Format"),
                   QVariant(static_cast<int>(m_format)));
  config->setValue(QLatin1String("FileNameFormat"), QVariant(m_fileNameFormat));
  config->setValue(QLatin1String("SortTagField"), QVariant(m_sortTagField));
  config->setValue(QLatin1String("InfoFormat"), QVariant(m_infoFormat));
  config->endGroup();
  config->beginGroup(m_group, true);
  config->setValue(QLatin1String("WindowGeometry"), QVariant(m_windowGeometry));
  config->endGroup();
}

/**
 * Read persisted configuration.
 *
 * @param config configuration
 */
void PlaylistConfig::readFromConfig(ISettings* config)
{
  config->beginGroup(m_group);
  m_useFileNameFormat = config->value(QLatin1String("UseFileNameFormat"),
                                      m_useFileNameFormat).toBool();
  m_onlySelectedFiles = config->value(QLatin1String("OnlySelectedFiles"),
                                      m_onlySelectedFiles).toBool();
  m_useSortTagField = config->value(QLatin1String("UseSortTagField"),
                                    m_useSortTagField).toBool();
  m_useFullPath = config->value(QLatin1String("UseFullPath"),
                                m_useFullPath).toBool();
  m_writeInfo = config->value(QLatin1String("WriteInfo"), m_writeInfo).toBool();
  m_location = static_cast<PlaylistLocation>(
        config->value(QLatin1String("Location"),
                      static_cast<int>(m_location)).toInt());
  m_format = static_cast<PlaylistFormat>(config->value(QLatin1String("Format"),
    static_cast<int>(m_format)).toInt());
  m_fileNameFormat = config->value(QLatin1String("FileNameFormat"),
                                   m_fileNameFormat).toString();
  m_sortTagField = config->value(QLatin1String("SortTagField"),
                                 m_sortTagField).toString();
  m_infoFormat = config->value(QLatin1String("InfoFormat"),
                               m_infoFormat).toString();
  config->endGroup();
  config->beginGroup(m_group, true);
  m_windowGeometry = config->value(QLatin1String("WindowGeometry"),
                                   m_windowGeometry).toByteArray();
  config->endGroup();
}

void PlaylistConfig::setLocation(PlaylistLocation location)
{
  if (m_location != location) {
    m_location = location;
    emit locationChanged(m_location);
  }
}

void PlaylistConfig::setFormat(PlaylistFormat format)
{
  if (m_format != format) {
    m_format = format;
    emit formatChanged(m_format);
  }
}

void PlaylistConfig::setFileNameFormat(const QString& fileNameFormat)
{
  if (m_fileNameFormat != fileNameFormat) {
    m_fileNameFormat = fileNameFormat;
    emit fileNameFormatChanged(m_fileNameFormat);
  }
}

void PlaylistConfig::setSortTagField(const QString& sortTagField)
{
  if (m_sortTagField != sortTagField) {
    m_sortTagField = sortTagField;
    emit sortTagFieldChanged(m_sortTagField);
  }
}

void PlaylistConfig::setInfoFormat(const QString& infoFormat)
{
  if (m_infoFormat != infoFormat) {
    m_infoFormat = infoFormat;
    emit infoFormatChanged(m_infoFormat);
  }
}

void PlaylistConfig::setUseFileNameFormat(bool useFileNameFormat)
{
  if (m_useFileNameFormat != useFileNameFormat) {
    m_useFileNameFormat = useFileNameFormat;
    emit useFileNameFormatChanged(m_useFileNameFormat);
  }
}

void PlaylistConfig::setOnlySelectedFiles(bool onlySelectedFiles)
{
  if (m_onlySelectedFiles != onlySelectedFiles) {
    m_onlySelectedFiles = onlySelectedFiles;
    emit onlySelectedFilesChanged(m_onlySelectedFiles);
  }
}

void PlaylistConfig::setUseSortTagField(bool useSortTagField)
{
  if (m_useSortTagField != useSortTagField) {
    m_useSortTagField = useSortTagField;
    emit useSortTagFieldChanged(m_useSortTagField);
  }
}

void PlaylistConfig::setUseFullPath(bool useFullPath)
{
  if (m_useFullPath != useFullPath) {
    m_useFullPath = useFullPath;
    emit useFullPathChanged(m_useFullPath);
  }
}

void PlaylistConfig::setWriteInfo(bool writeInfo)
{
  if (m_writeInfo != writeInfo) {
    m_writeInfo = writeInfo;
    emit writeInfoChanged(m_writeInfo);
  }
}

void PlaylistConfig::setWindowGeometry(const QByteArray& windowGeometry)
{
  if (m_windowGeometry != windowGeometry) {
    m_windowGeometry = windowGeometry;
    emit windowGeometryChanged(m_windowGeometry);
  }
}

QString PlaylistConfig::fileExtensionForFormat() const
{
  switch (m_format) {
  case PlaylistConfig::PF_M3U:
    return QLatin1String(".m3u");
  case PlaylistConfig::PF_PLS:
    return QLatin1String(".pls");
  case PlaylistConfig::PF_XSPF:
    return QLatin1String(".xspf");
  }
  return QString();
}

PlaylistConfig::PlaylistFormat PlaylistConfig::formatFromFileExtension(
    const QString& path, bool* ok)
{
  PlaylistFormat format = PlaylistConfig::PF_M3U;
  bool unknown = false;
  if (path.endsWith(QLatin1String(".m3u"), Qt::CaseInsensitive)) {
    format = PlaylistConfig::PF_M3U;
  } else if (path.endsWith(QLatin1String(".pls"), Qt::CaseInsensitive)) {
    format = PlaylistConfig::PF_PLS;
  } else if (path.endsWith(QLatin1String(".xspf"), Qt::CaseInsensitive)) {
    format = PlaylistConfig::PF_XSPF;
  } else {
    unknown = true;
  }
  if (ok) {
    *ok = !unknown;
  }
  return format;
}
