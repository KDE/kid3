/**
 * \file playlistcreator.cpp
 * Playlist creator.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 21 Sep 2009
 *
 * Copyright (C) 2009-2024  Urs Fleisch
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
#if QT_VERSION >= 0x060000
#include <QStringConverter>
#endif
#include "fileconfig.h"
#include "formatconfig.h"
#include "taggedfile.h"
#include "trackdata.h"
#include "fileproxymodel.h"
#include "saferename.h"
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
  if (m_cfg.location() == PlaylistConfig::PL_TopLevelDirectory) {
    m_playlistDirName = topLevelDir;
    if (!m_playlistDirName.endsWith(QLatin1Char('/'))) {
      m_playlistDirName += QLatin1Char('/');
    }
  }
}

/**
 * Write a playlist from a list of model indexes.
 * @param playlistPath file path to be used for playlist
 * @param indexes indexes in FileProxyModel
 * @return true if ok.
 */
bool PlaylistCreator::write(const QString& playlistPath,
                            const QList<QPersistentModelIndex>& indexes)
{
  QFileInfo fileInfo(playlistPath);
  QDir playlistDir = fileInfo.absoluteDir();
  m_playlistDirName = fileInfo.absolutePath();
  if (!m_playlistDirName.endsWith(QLatin1Char('/'))) {
    m_playlistDirName += QLatin1Char('/');
  }
  m_playlistFileName = fileInfo.fileName();

  QList<Entry> entries;
  for (const QPersistentModelIndex& index : indexes) {
    if (const auto model =
        qobject_cast<const FileProxyModel*>(index.model())) {
      QString filePath = model->filePath(index);
      PlaylistCreator::Entry entry;
      entry.filePath = m_cfg.useFullPath()
          ? filePath
          : playlistDir.relativeFilePath(filePath);
      if (m_cfg.writeInfo()) {
        Item(index, *this).getInfo(entry.info, entry.duration);
      }
      entries.append(entry);
    }
  }
  return write(entries);
}

/**
 * Write playlist containing added Entry elements.
 *
 * @return true if ok.
 */
bool PlaylistCreator::write()
{
  if (m_playlistFileName.isEmpty()) {
    return true;
  }
  if (write(m_entries.values())) {
    m_entries.clear();
    m_playlistFileName = QLatin1String("");
    return true;
  }
  return false;
}

/**
 * Write a playlist from a list entries.
 * @param entries playlist entries
 * @return true if ok.
 */
bool PlaylistCreator::write(const QList<Entry>& entries)
{
  QFile file(m_playlistDirName + m_playlistFileName);
  bool ok = file.open(QIODevice::WriteOnly);
  if (ok) {
    QTextStream stream(&file);
    if (QString codecName = FileConfig::instance().textEncoding();
        codecName != QLatin1String("System")) {
#if QT_VERSION >= 0x060000
      if (auto encoding = QStringConverter::encodingForName(codecName.toLatin1())) {
        stream.setEncoding(*encoding);
      }
#else
      stream.setCodec(codecName.toLatin1());
#endif
    }

    switch (m_cfg.format()) {
      case PlaylistConfig::PF_M3U:
        if (m_cfg.writeInfo()) {
          stream << "#EXTM3U\n";
        }
        if (entries.isEmpty() && m_cfg.useFullPath()) {
          stream << "# Kid3: useFullPath\n";
        }
        for (auto it = entries.constBegin(); it != entries.constEnd(); ++it) {
          if (m_cfg.writeInfo()) {
            stream << QString(QLatin1String("#EXTINF:%1,%2\n"))
              .arg(it->duration).arg(it->info);
          }
          stream << it->filePath << "\n";
        }
        break;
      case PlaylistConfig::PF_PLS:
      {
        unsigned nr = 1;
        stream << "[playlist]\n";
        stream << QString(QLatin1String("NumberOfEntries=%1\n")).arg(entries.size());
        for (auto it = entries.constBegin(); it != entries.constEnd(); ++it) {
          stream << QString(QLatin1String("File%1=%2\n")).arg(nr).arg(it->filePath);
          if (m_cfg.writeInfo()) {
            stream << QString(QLatin1String("Title%1=%2\n")).arg(nr).arg(it->info);
            stream << QString(QLatin1String("Length%1=%2\n")).arg(nr).arg(it->duration);
          }
          ++nr;
        }
        stream << "Version=2\n";
        if (entries.isEmpty() && (m_cfg.useFullPath() || m_cfg.writeInfo())) {
          stream << "; Kid3:";
          if (m_cfg.useFullPath()) {
            stream << " useFullPath";
          }
          if (m_cfg.writeInfo()) {
            stream << " writeInfo";
          }
          stream << "\n";
        }
      }
      break;
      case PlaylistConfig::PF_XSPF:
      {
        stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        // Using a raw string literal here causes clang to issue
        // "Unbalanced opening parenthesis in C++ code".
        QString line = QLatin1String("<playlist version=\"1\" xmlns=\"http://xspf.org/ns/0/\"");
        if (!m_cfg.useFullPath()) {
          QUrl url(m_playlistDirName);
          url.setScheme(QLatin1String("file"));
          line += QString(QLatin1String(" xml:base=\"%1\""))
              .arg(QString::fromLatin1(url.toEncoded().constData()));
        }
        line += QLatin1String(">\n");
        stream << line;
        stream << "  <trackList>\n";

        for (auto it = entries.constBegin(); it != entries.constEnd(); ++it) {
          stream << "    <track>\n";
          QUrl url(it->filePath);
          if (m_cfg.useFullPath()) {
            url.setScheme(QLatin1String("file"));
          }
          stream << QString(QLatin1String("      <location>%1</location>\n"))
                    .arg(QString::fromLatin1(url.toEncoded().constData()));
          if (m_cfg.writeInfo()) {
            // the info is already formatted in the case of XSPF
            stream << it->info;
          }
          stream << "    </track>\n";
        }

        stream << "  </trackList>\n";
        if (entries.isEmpty() && m_cfg.writeInfo()) {
          stream << "  <!-- Kid3: writeInfo -->\n";
        }
        stream << "</playlist>\n";
      }
      break;
    }

    file.close();
  }
  return ok;
}

/**
 * Read playlist from file
 * @param playlistPath path to playlist file
 * @param filePaths absolute paths to the playlist files are returned here
 * @param format the playlist format is returned here
 * @param hasFullPath true is returned here if the files use absolute paths
 * @param hasInfo true is returned here if the playlist contains additional
 *                information
 * @return true if ok.
 */
bool PlaylistCreator::read(
    const QString& playlistPath, QStringList& filePaths,
    PlaylistConfig::PlaylistFormat& format,
    bool& hasFullPath, bool& hasInfo) const
{
  QFile file(playlistPath);
  if (file.open(QIODevice::ReadOnly)) {
    QFileInfo fileInfo(playlistPath);
    QDir playlistDir = fileInfo.absoluteDir();
    QString playlistFileName = fileInfo.fileName();

    hasFullPath = false;
    hasInfo = false;
    format = PlaylistConfig::formatFromFileExtension(playlistFileName);

    QTextStream stream(&file);
    if (QString codecName = FileConfig::instance().textEncoding();
        codecName != QLatin1String("System")) {
#if QT_VERSION >= 0x060000
      if (auto encoding = QStringConverter::encodingForName(codecName.toLatin1())) {
        stream.setEncoding(*encoding);
      }
#else
      stream.setCodec(codecName.toLatin1());
#endif
    }

    filePaths.clear();

    QString line;
    while (!(line = stream.readLine()).isNull()) {
      QString path;
      switch (format) {
      case PlaylistConfig::PF_M3U:
        if (line.startsWith(QLatin1Char('#'))) {
          if (line.startsWith(QLatin1String("#EXT"))) {
            hasInfo = true;
          } else if (line.startsWith(QLatin1String("# Kid3:")) &&
                     line.contains(QLatin1String("useFullPath"))) {
            hasFullPath = true;
          }
        } else {
          path = line.trimmed();
        }
        break;
      case PlaylistConfig::PF_PLS:
        if (line.startsWith(QLatin1String("File"))) {
          if (int colonPos = line.indexOf(QLatin1Char('='), 4);
              colonPos != -1) {
            path = line.mid(colonPos + 1).trimmed();
          }
        } else if (line.startsWith(QLatin1String("Title")) ||
                   line.startsWith(QLatin1String("Length"))) {
          hasInfo = true;
        } else if (line.startsWith(QLatin1String("; Kid3:"))) {
          if (line.contains(QLatin1String("useFullPath"))) {
            hasFullPath = true;
          }
          if (line.contains(QLatin1String("writeInfo"))) {
            hasInfo = true;
          }
        }
        break;
      case PlaylistConfig::PF_XSPF:
        if (line.contains(QLatin1String("<location>"))) {
          if (int startPos = line.indexOf(QLatin1String("<location>"));
              startPos != -1) {
            startPos += 10;
            if (int endPos = line.indexOf(QLatin1String("</location>"), startPos);
                endPos != -1) {
              QUrl url = QUrl::fromEncoded(
                    line.mid(startPos, endPos - startPos).toLatin1());
              path = url.toLocalFile();
              if (path.isEmpty()) {
                // For relative paths, QUrl::toLocalFile() returns "".
                path = url.toString();
              }
            }
          }
        } else if (line.contains(QLatin1String("<title>")) ||
                   line.contains(QLatin1String("<creator>")) ||
                   line.contains(QLatin1String("<album>")) ||
                   line.contains(QLatin1String("<trackNum>")) ||
                   line.contains(QLatin1String("<duration>")) ||
                   line.contains(QLatin1String("<!-- Kid3: writeInfo -->"))) {
          hasInfo = true;
        } else if (line.startsWith(QLatin1String("<playlist")) &&
                   !line.contains(QLatin1String("xml:base="))) {
          hasFullPath = true;
        }
        break;
      }
      if (!path.isEmpty()) {
        if (QDir::isAbsolutePath(path)) {
          hasFullPath = true;
        } else {
          path = playlistDir.absoluteFilePath(path);
        }
        filePaths.append(path);
      }
    }

    file.close();
    return true;
  }
  return false;
}


/**
 * Constructor.
 *
 * @param index model index
 * @param ctr  associated playlist creator
 */
PlaylistCreator::Item::Item(const QModelIndex& index, PlaylistCreator& ctr)
  : m_ctr(ctr), m_taggedFile(FileProxyModel::getTaggedFileOfIndex(index)),
    m_isDir(false)
{
  if (m_taggedFile) {
    m_dirName = m_taggedFile->getDirname();
  } else {
    m_dirName = FileProxyModel::getPathIfIndexOfDir(index);
    m_isDir = !m_dirName.isNull();
  }
  if (!m_dirName.endsWith(QLatin1Char('/'))) {
    m_dirName += QLatin1Char('/');
  }
  // fix double separators
  m_dirName.replace(QLatin1String("//"), QLatin1String("/"));
}

/**
 * Get additional information for item.
 * @param info additional information is returned here
 * @param duration the duration of the track is returned here
 */
void PlaylistCreator::Item::getInfo(QString& info, unsigned long& duration)
{
  if (m_ctr.m_cfg.format() != PlaylistConfig::PF_XSPF) {
    info = formatString(m_ctr.m_cfg.infoFormat());
  } else {
    info = formatString(QLatin1String(
      "      <title>%{title}</title>\n"
      "      <creator>%{artist}</creator>\n"
      "      <album>%{album}</album>\n"
      "      <trackNum>%{track.1}</trackNum>\n"
      "      <duration>%{seconds}000</duration>\n"));
  }
  TaggedFile::DetailInfo detailInfo;
  m_taggedFile->getDetailInfo(detailInfo);
  duration = detailInfo.duration;
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
    m_taggedFile = FileProxyModel::readTagsFromTaggedFile(m_taggedFile);
    m_trackData.reset(new ImportTrackData(*m_taggedFile, Frame::TagVAll));
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
  if (m_ctr.m_cfg.location() != PlaylistConfig::PL_TopLevelDirectory) {
    if (m_ctr.m_playlistDirName != m_dirName) {
      ok = m_ctr.write();
      m_ctr.m_playlistDirName = m_dirName;
    }
  }
  if (m_ctr.m_playlistFileName.isEmpty()) {
    if (!m_ctr.m_cfg.useFileNameFormat()) {
      m_ctr.m_playlistFileName = QDir(m_ctr.m_playlistDirName).dirName();
    } else {
      m_ctr.m_playlistFileName = formatString(m_ctr.m_cfg.fileNameFormat());
      Utils::replaceIllegalFileNameCharacters(m_ctr.m_playlistFileName);
    }
    FormatConfig& fnCfg = FilenameFormatConfig::instance();
    if (fnCfg.useForOtherFileNames()) {
      bool isFilenameFormatter = fnCfg.switchFilenameFormatter(false);
      fnCfg.formatString(m_ctr.m_playlistFileName);
      fnCfg.switchFilenameFormatter(isFilenameFormatter);
    }
    m_ctr.m_playlistFileName = fnCfg.joinFileName(
          m_ctr.m_playlistFileName, m_ctr.m_cfg.fileExtensionForFormat());
  }
  QString filePath = m_dirName + m_taggedFile->getFilename();
  if (!m_ctr.m_cfg.useFullPath() &&
      filePath.startsWith(m_ctr.m_playlistDirName)) {
    filePath = filePath.mid(m_ctr.m_playlistDirName.length());
  }
  QString sortKey;
  if (m_ctr.m_cfg.useSortTagField()) {
    sortKey = formatString(m_ctr.m_cfg.sortTagField());
  }
  sortKey += filePath;
  PlaylistCreator::Entry entry;
  entry.filePath = filePath;
  if (m_ctr.m_cfg.writeInfo()) {
    getInfo(entry.info, entry.duration);
  }
  m_ctr.m_entries.insert(sortKey, entry);
  return ok;
}
