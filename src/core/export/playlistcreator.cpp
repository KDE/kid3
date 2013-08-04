/**
 * \file playlistcreator.cpp
 * Playlist creator.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 21 Sep 2009
 *
 * Copyright (C) 2009-2013  Urs Fleisch
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

#include "playlistcreator.h"
#include <QDir>
#include <QUrl>
#include <QFile>
#include <QTextStream>
#include "playlistconfig.h"
#include "taggedfile.h"
#include "trackdata.h"
#include "fileproxymodel.h"
#include "config.h"

/**
 * Constructor.
 *
 * @param topLevelDir top-level directory of playlist
 * @param cfg         playlist configuration
 */
PlaylistCreator::PlaylistCreator(const QString& topLevelDir,
                                 const PlaylistConfig& cfg) :
  m_cfg(cfg)
{
  if (m_cfg.m_location == PlaylistConfig::PL_TopLevelDirectory) {
    m_playlistDirName = topLevelDir;
    if (!m_playlistDirName.endsWith(QChar(QDir::separator()))) {
      m_playlistDirName += QDir::separator();
    }
  }
}

/**
 * Write playlist containing added Entry elements.
 *
 * @return true if ok.
 */
bool PlaylistCreator::write()
{
  bool ok = true;
  if (!m_playlistFileName.isEmpty()) {
    QFile file(m_playlistDirName + m_playlistFileName);
    ok = file.open(QIODevice::WriteOnly);
    if (ok) {
      QTextStream stream(&file);

      switch (m_cfg.m_format) {
        case PlaylistConfig::PF_M3U:
          if (m_cfg.m_writeInfo) {
            stream << "#EXTM3U\n";
          }
          for (QMap<QString, Entry>::const_iterator it = m_entries.begin();
               it != m_entries.end();
               ++it) {
            if (m_cfg.m_writeInfo) {
              stream << QString(QLatin1String("#EXTINF:%1,%2\n")).
                arg((*it).duration).arg((*it).info);
            }
            stream << (*it).filePath << "\n";
          }
          break;
        case PlaylistConfig::PF_PLS:
        {
          unsigned nr = 1;
          stream << "[playlist]\n";
          stream << QString(QLatin1String("NumberOfEntries=%1\n")).arg(m_entries.size());
          for (QMap<QString, Entry>::const_iterator it = m_entries.begin();
               it != m_entries.end();
               ++it) {
            stream << QString(QLatin1String("File%1=%2\n")).arg(nr).arg((*it).filePath);
            if (m_cfg.m_writeInfo) {
              stream << QString(QLatin1String("Title%1=%2\n")).arg(nr).arg((*it).info);
              stream << QString(QLatin1String("Length%1=%2\n")).arg(nr).arg((*it).duration);
            }
            ++nr;
          }
          stream << "Version=2\n";
        }
        break;
        case PlaylistConfig::PF_XSPF:
        {
          stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
          QString line = QLatin1String("<playlist version=\"1\" xmlns=\"http://xspf.org/ns/0/\"");
          if (!m_cfg.m_useFullPath) {
            QUrl url(m_playlistDirName);
            url.setScheme(QLatin1String("file"));
            line += QString(QLatin1String(" xml:base=\"%1\"")).arg(QString::fromLatin1(url.toEncoded().data()));
          }
          line += QLatin1String(">\n");
          stream << line;
          stream << "  <trackList>\n";

          for (QMap<QString, Entry>::const_iterator it = m_entries.begin();
               it != m_entries.end();
               ++it) {
            stream << "    <track>\n";
            QUrl url((*it).filePath);
            if (m_cfg.m_useFullPath) {
              url.setScheme(QLatin1String("file"));
            }
            stream << QString(QLatin1String("      <location>%1</location>\n")).arg(
              QString::fromLatin1(url.toEncoded().data()));
            if (m_cfg.m_writeInfo) {
              // the info is already formatted in the case of XSPF
              stream << (*it).info;
            }
            stream << "    </track>\n";
          }

          stream << "  </trackList>\n";
          stream << "</playlist>\n";
        }
        break;
      }

      m_entries.clear();
      m_playlistFileName = QLatin1String("");
      file.close();
    }
  }
  return ok;
}


/**
 * Constructor.
 *
 * @param index model index
 * @param ctr  associated playlist creator
 */
PlaylistCreator::Item::Item(const QModelIndex& index, PlaylistCreator& ctr) :
    m_ctr(ctr), m_isDir(false),
    m_taggedFile(FileProxyModel::getTaggedFileOfIndex(index)), m_trackData(0)
{
  if (m_taggedFile) {
    m_dirName = m_taggedFile->getDirname();
  } else {
    m_dirName = FileProxyModel::getPathIfIndexOfDir(index);
    m_isDir = !m_dirName.isNull();
  }
  QChar separator = QDir::separator();
  if (!m_dirName.endsWith(separator)) {
    m_dirName += separator;
  }
  // fix double separators
  m_dirName.replace(QString(separator) + separator, separator);
}

/**
 * Destructor.
 */
PlaylistCreator::Item::~Item()
{
  delete m_trackData;
}

/**
 * Format string using tags and properties of item.
 *
 * @param format format string
 *
 * @return string with percent codes replaced.
 */
QString PlaylistCreator::Item::formatString(const QString& format)
{
  if (!m_trackData) {
    m_taggedFile->readTags(false);
    m_taggedFile = FileProxyModel::readWithId3V24IfId3V24(m_taggedFile);
    m_trackData = new ImportTrackData(*m_taggedFile, ImportTrackData::TagV2V1);
  }
  return m_trackData->formatString(format);
}

/**
 * Add item to playlist.
 * This operation will write a playlist if the configuration is set to write
 * a playlist in every directory and a new directory is entered.
 *
 * @return true if ok.
 */
bool PlaylistCreator::Item::add()
{
  bool ok = true;
  if (m_ctr.m_cfg.m_location != PlaylistConfig::PL_TopLevelDirectory) {
    if (m_ctr.m_playlistDirName != m_dirName) {
      ok = m_ctr.write();
      m_ctr.m_playlistDirName = m_dirName;
    }
  }
  if (m_ctr.m_playlistFileName.isEmpty()) {
    if (!m_ctr.m_cfg.m_useFileNameFormat) {
      m_ctr.m_playlistFileName = QDir(m_ctr.m_playlistDirName).dirName();
    } else {
      m_ctr.m_playlistFileName = formatString(m_ctr.m_cfg.m_fileNameFormat);
    }
    switch (m_ctr.m_cfg.m_format) {
      case PlaylistConfig::PF_M3U:
        m_ctr.m_playlistFileName += QLatin1String(".m3u");
        break;
      case PlaylistConfig::PF_PLS:
        m_ctr.m_playlistFileName += QLatin1String(".pls");
        break;
      case PlaylistConfig::PF_XSPF:
        m_ctr.m_playlistFileName += QLatin1String(".xspf");
        break;
    }
  }
  QString filePath = m_dirName + m_taggedFile->getFilename();
  if (!m_ctr.m_cfg.m_useFullPath &&
      filePath.startsWith(m_ctr.m_playlistDirName)) {
    filePath = filePath.mid(m_ctr.m_playlistDirName.length());
  }
  QString sortKey;
  if (m_ctr.m_cfg.m_useSortTagField) {
    sortKey = formatString(m_ctr.m_cfg.m_sortTagField);
  }
  sortKey += filePath;
  PlaylistCreator::Entry entry;
  entry.filePath = filePath;
  if (m_ctr.m_cfg.m_writeInfo) {
    if (m_ctr.m_cfg.m_format != PlaylistConfig::PF_XSPF) {
      entry.info = formatString(m_ctr.m_cfg.m_infoFormat);
    } else {
      entry.info = formatString(QLatin1String(
        "      <title>%{title}</title>\n"
        "      <creator>%{artist}</creator>\n"
        "      <album>%{album}</album>\n"
        "      <trackNum>%{track.1}</trackNum>\n"
        "      <duration>%{seconds}000</duration>\n"));
    }
    TaggedFile::DetailInfo detailInfo;
    m_taggedFile->getDetailInfo(detailInfo);
    entry.duration = detailInfo.duration;
  } else {
    entry.info = QString();
    entry.duration = 0;
  }
  m_ctr.m_entries.insert(sortKey, entry);
  return ok;
}
