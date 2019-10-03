/**
 * \file trackdata.cpp
 * Track data, frames with association to tagged file.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 23 Feb 2007
 *
 * Copyright (C) 2007-2018  Urs Fleisch
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

#include "trackdata.h"
#include <QString>
#include <QUrl>
#include <QDir>
#include <QDateTime>
#include <QCoreApplication>
#include "fileproxymodel.h"

/**
 * Constructor.
 *
 * @param trackData track data
 * @param str       string with format codes
 */
TrackDataFormatReplacer::TrackDataFormatReplacer(
  const TrackData& trackData, const QString& str)
  : FrameFormatReplacer(trackData, str), m_trackData(trackData) {}

/**
 * Replace a format code (one character %c or multiple characters %{chars}).
 * Supported format fields:
 * Those supported by FrameFormatReplacer::getReplacement()
 * %f filename
 * %p path to file
 * %u URL of file
 * %d duration in minutes:seconds
 * %D duration in seconds
 * %n number of tracks
 *
 * @param code format code
 *
 * @return replacement string,
 *         QString::null if code not found.
 */
QString TrackDataFormatReplacer::getReplacement(const QString& code) const
{
  QString result = FrameFormatReplacer::getReplacement(code);
  if (result.isNull()) {
    QString name;

    if (code.length() == 1) {
      static const struct {
        const char* longCode;
        char shortCode;
      } shortToLong[] = {
        { "file", 'f' },
        { "filepath", 'p' },
        { "url", 'u' },
        { "duration", 'd' },
        { "seconds", 'D' },
        { "tracks", 'n' },
        { "extension", 'e' },
        { "tag1", 'O' },
        { "tag2", 'o' },
        { "bitrate", 'b' },
        { "vbr", 'v' },
        { "samplerate", 'r' },
        { "mode", 'm' },
        { "channels", 'C' },
        { "codec", 'k' },
        { "marked", 'w' }
      };
      const char c = code[0].toLatin1();
      for (const auto& s2l : shortToLong) {
        if (s2l.shortCode == c) {
          name = QString::fromLatin1(s2l.longCode);
          break;
        }
      }
    } else if (code.length() > 1) {
      name = code;
    }

    if (!name.isNull()) {
      TaggedFile::DetailInfo info;
      m_trackData.getDetailInfo(info);
      if (name == QLatin1String("file")) {
        QString filename(m_trackData.getAbsFilename());
        int sepPos = filename.lastIndexOf(QLatin1Char('/'));
        if (sepPos < 0) {
          sepPos = filename.lastIndexOf(QDir::separator());
        }
        if (sepPos >= 0) {
          filename.remove(0, sepPos + 1);
        }
        result = filename;
      } else if (name == QLatin1String("filepath")) {
        result = m_trackData.getAbsFilename();
      } else if (name == QLatin1String("modificationdate")) {
        return QFileInfo(m_trackData.getAbsFilename())
            .lastModified().toString(Qt::ISODate);
      } else if (name == QLatin1String("creationdate")) {
#if QT_VERSION >= 0x050a00
        return QFileInfo(m_trackData.getAbsFilename())
            .birthTime().toString(Qt::ISODate);
#else
        return QFileInfo(m_trackData.getAbsFilename())
            .created().toString(Qt::ISODate);
#endif
      } else if (name == QLatin1String("url")) {
        QUrl url;
        url.setPath(m_trackData.getAbsFilename());
        url.setScheme(QLatin1String("file"));
        result = url.toString();
      } else if (name == QLatin1String("dirname")) {
        const QString dirPath = m_trackData.getDirname();
        int sepPos = dirPath.lastIndexOf(QLatin1Char('/'));
        if (sepPos < 0) {
          sepPos = dirPath.lastIndexOf(QDir::separator());
        }
        result = sepPos >= 0 ? dirPath.mid(sepPos + 1) : dirPath;
      } else if (name == QLatin1String("duration")) {
        result = TaggedFile::formatTime(m_trackData.getFileDuration());
      } else if (name == QLatin1String("seconds")) {
        result = QString::number(m_trackData.getFileDuration());
      } else if (name == QLatin1String("tracks")) {
        result = QString::number(m_trackData.getTotalNumberOfTracksInDir());
      } else if (name == QLatin1String("extension")) {
        result = m_trackData.getFileExtension();
      } else if (name.startsWith(QLatin1String("tag")) && name.length() == 4) {
        Frame::TagNumber tagNr = Frame::tagNumberFromString(name.mid(3));
        if (tagNr < Frame::Tag_NumValues) {
          result = m_trackData.getTagFormat(tagNr);
        }
      } else if (name == QLatin1String("bitrate")) {
        result.setNum(info.bitrate);
      } else if (name == QLatin1String("vbr")) {
        result = info.vbr ? QLatin1String("VBR") : QLatin1String("");
      } else if (name == QLatin1String("samplerate")) {
        result.setNum(info.sampleRate);
      } else if (name == QLatin1String("mode")) {
        switch (info.channelMode) {
          case TaggedFile::DetailInfo::CM_Stereo:
            result = QLatin1String("Stereo");
            break;
          case TaggedFile::DetailInfo::CM_JointStereo:
            result = QLatin1String("Joint Stereo");
            break;
          case TaggedFile::DetailInfo::CM_None:
          default:
            result = QLatin1String("");
        }
      } else if (name == QLatin1String("channels")) {
        result.setNum(info.channels);
      } else if (name == QLatin1String("codec")) {
        result = info.format;
      } else if (name == QLatin1String("marked")) {
        TaggedFile* taggedFile = m_trackData.getTaggedFile();
        result = taggedFile && taggedFile->isMarked()
            ? QLatin1String("1") : QLatin1String("");
      }
    }
  }

  return result;
}

/**
 * Get help text for supported format codes.
 *
 * @param onlyRows if true only the tr elements are returned,
 *                 not the surrounding table
 *
 * @return help text.
 */
QString TrackDataFormatReplacer::getToolTip(bool onlyRows)
{
  QString str;
  if (!onlyRows) str += QLatin1String("<table>\n");
  str += FrameFormatReplacer::getToolTip(true);

  str += QLatin1String("<tr><td>%f</td><td>%{file}</td><td>");
  str += QCoreApplication::translate("@default", "Filename");
  str += QLatin1String("</td></tr>\n");

  str += QLatin1String("<tr><td>%p</td><td>%{filepath}</td><td>");
  const char* const absolutePathToFileStr =
      QT_TRANSLATE_NOOP("@default", "Absolute path to file");
  str += QCoreApplication::translate("@default", absolutePathToFileStr);
  str += QLatin1String("</td></tr>\n");

  str += QLatin1String("<tr><td></td><td>%{modificationdate}</td><td>");
  const char* const modificationDateStr =
      QT_TRANSLATE_NOOP("@default", "Modification date");
  str += QCoreApplication::translate("@default", modificationDateStr);
  str += QLatin1String("</td></tr>\n");

  str += QLatin1String("<tr><td></td><td>%{creationdate}</td><td>");
  const char* const creationDateStr =
      QT_TRANSLATE_NOOP("@default", "Creation date");
  str += QCoreApplication::translate("@default", creationDateStr);
  str += QLatin1String("</td></tr>\n");

  str += QLatin1String("<tr><td>%u</td><td>%{url}</td><td>");
  str += QCoreApplication::translate("@default", "URL");
  str += QLatin1String("</td></tr>\n");

  str += QLatin1String("<tr><td></td><td>%{dirname}</td><td>");
  const char* const directoryNameStr =
      QT_TRANSLATE_NOOP("@default", "Directory name");
  str += QCoreApplication::translate("@default", directoryNameStr);
  str += QLatin1String("</td></tr>\n");

  str += QLatin1String("<tr><td>%d</td><td>%{duration}</td><td>");
  const char* const lengthStr = QT_TRANSLATE_NOOP("@default", "Length");
  str += QCoreApplication::translate("@default", lengthStr);
  str += QLatin1String(" &quot;M:S&quot;</td></tr>\n");

  str += QLatin1String("<tr><td>%D</td><td>%{seconds}</td><td>");
  str += QCoreApplication::translate("@default", lengthStr);
  str += QLatin1String(" &quot;S&quot;</td></tr>\n");

  str += QLatin1String("<tr><td>%n</td><td>%{tracks}</td><td>");
  const char* const numberOfTracksStr =
      QT_TRANSLATE_NOOP("@default", "Number of tracks");
  str += QCoreApplication::translate("@default", numberOfTracksStr);
  str += QLatin1String("</td></tr>\n");

  str += QLatin1String("<tr><td>%e</td><td>%{extension}</td><td>");
  const char* const extensionStr = QT_TRANSLATE_NOOP("@default", "Extension");
  str += QCoreApplication::translate("@default", extensionStr);
  str += QLatin1String("</td></tr>\n");

  str += QLatin1String("<tr><td>%O</td><td>%{tag1}</td><td>");
  str += QCoreApplication::translate("@default", "Tag 1");
  str += QLatin1String("</td></tr>\n");

  str += QLatin1String("<tr><td>%o</td><td>%{tag2}</td><td>");
  str += QCoreApplication::translate("@default", "Tag 2");
  str += QLatin1String("</td></tr>\n");

  str += QLatin1String("<tr><td>%b</td><td>%{bitrate}</td><td>");
  const char* const bitrateStr = QT_TRANSLATE_NOOP("@default", "Bitrate");
  str += QCoreApplication::translate("@default", bitrateStr);
  str += QLatin1String("</td></tr>\n");

  str += QLatin1String("<tr><td>%v</td><td>%{vbr}</td><td>");
  const char* const vbrStr = QT_TRANSLATE_NOOP("@default", "VBR");
  str += QCoreApplication::translate("@default", vbrStr);
  str += QLatin1String("</td></tr>\n");

  str += QLatin1String("<tr><td>%r</td><td>%{samplerate}</td><td>");
  const char* const samplerateStr = QT_TRANSLATE_NOOP("@default", "Samplerate");
  str += QCoreApplication::translate("@default", samplerateStr);
  str += QLatin1String("</td></tr>\n");

  str += QLatin1String("<tr><td>%m</td><td>%{mode}</td><td>Stereo, Joint Stereo</td></tr>\n");

  str += QLatin1String("<tr><td>%C</td><td>%{channels}</td><td>");
  const char* const channelsStr = QT_TRANSLATE_NOOP("@default", "Channels");
  str += QCoreApplication::translate("@default", channelsStr);
  str += QLatin1String("</td></tr>\n");

  str += QLatin1String("<tr><td>%k</td><td>%{codec}</td><td>");
  const char* const codecStr = QT_TRANSLATE_NOOP("@default", "Codec");
  str += QCoreApplication::translate("@default", codecStr);
  str += QLatin1String("</td></tr>\n");

  str += QLatin1String("<tr><td>%w</td><td>%{marked}</td><td>");
  const char* const markedStr = QT_TRANSLATE_NOOP("@default", "Marked");
  str += QCoreApplication::translate("@default", markedStr);
  str += QLatin1String("</td></tr>\n");

  str += QLatin1String("<tr><td>%ha...</td><td>%h{artist}...</td><td>");
  const char* const escapeForHtmlStr =
      QT_TRANSLATE_NOOP("@default", "Escape for HTML");
  str += QCoreApplication::translate("@default", escapeForHtmlStr);
  str += QLatin1String("</td></tr>\n");

  if (!onlyRows) str += QLatin1String("</table>\n");
  return str;
}


/**
 * Constructor.
 */
TrackData::TrackData() = default;

/**
 * Constructor.
 * All fields except the import duration are set from the tagged file,
 * which should be read using readTags() before.
 *
 * @param taggedFile tagged file providing track data
 * @param tagVersion source of frames
 */
TrackData::TrackData(TaggedFile& taggedFile, Frame::TagVersion tagVersion)
  : m_taggedFileIndex(taggedFile.getIndex())
{
  for (Frame::TagNumber tagNr : Frame::tagNumbersFromMask(tagVersion)) {
    if (empty()) {
      taggedFile.getAllFrames(tagNr, *this);
    } else {
      FrameCollection frames;
      taggedFile.getAllFrames(tagNr, frames);
      merge(frames);
    }
  }
}

/**
 * Get tagged file associated with this track data.
 * @return tagged file, 0 if none assigned.
 */
TaggedFile* TrackData::getTaggedFile() const {
  return FileProxyModel::getTaggedFileOfIndex(m_taggedFileIndex);
}

/**
 * Get duration of file.
 * @return duration of file.
 */
int TrackData::getFileDuration() const
{
  TaggedFile* taggedFile = getTaggedFile();
  return taggedFile ? taggedFile->getDuration() : 0;
}

/**
 * Get absolute filename.
 *
 * @return absolute file path.
 */
QString TrackData::getAbsFilename() const
{
  TaggedFile* taggedFile = getTaggedFile();
  return taggedFile ? taggedFile->getAbsFilename() : QString();
}

/**
 * Get filename.
 *
 * @return filename.
 */
QString TrackData::getFilename() const
{
  TaggedFile* taggedFile = getTaggedFile();
  return taggedFile ? taggedFile->getFilename() : QString();
}

/**
 * Get directory name.
 *
 * @return directory name.
 */
QString TrackData::getDirname() const
{
  TaggedFile* taggedFile = getTaggedFile();
  return taggedFile ? taggedFile->getDirname() : QString();
}

/**
 * Get the format of tag.
 *
 * @param tagNr tag number
 * @return string describing format of tag 1,
 *         e.g. "ID3v1.1", "ID3v2.3", "Vorbis", "APE",
 *         QString::null if unknown.
 */
QString TrackData::getTagFormat(Frame::TagNumber tagNr) const
{
  TaggedFile* taggedFile = getTaggedFile();
  return taggedFile ? taggedFile->getTagFormat(tagNr) : QString();
}

/**
 * Get detail info.
 * @param info the detail information is returned here
 */
void TrackData::getDetailInfo(TaggedFile::DetailInfo& info) const
{
  if (TaggedFile* taggedFile = getTaggedFile()) {
    taggedFile->getDetailInfo(info);
  }
}

/**
 * Format a string from track data.
 * Supported format fields:
 * Those supported by TrackDataFormatReplacer::getReplacement()
 *
 * @param format    format specification
 *
 * @return formatted string.
 */
QString TrackData::formatString(const QString& format) const
{
  TrackDataFormatReplacer fmt(*this, format);
  fmt.replaceEscapedChars();
  fmt.replacePercentCodes(FormatReplacer::FSF_SupportHtmlEscape);
  return fmt.getString();
}

/**
 * Create filename from tags according to format string.
 *
 * @param str       format string containing codes supported by
 *                  TrackDataFormatReplacer::getReplacement()
 * @param isDirname true to generate a directory name
 *
 * @return format string with format codes replaced by tags.
 */
QString TrackData::formatFilenameFromTags(QString str, bool isDirname) const
{
  if (!isDirname) {
    // first remove directory part from str
    const int sepPos = str.lastIndexOf(QLatin1Char('/'));
    if (sepPos >= 0) {
      str.remove(0, sepPos + 1);
    }
    // add extension to str
    str += getFileExtension(true);
  }

  TrackDataFormatReplacer fmt(*this, str);
  fmt.replacePercentCodes(isDirname ?
                          FormatReplacer::FSF_ReplaceSeparators : 0);
  return fmt.getString();
}

/**
 * Get help text for format codes supported by formatString().
 *
 * @param onlyRows if true only the tr elements are returned,
 *                 not the surrounding table
 *
 * @return help text.
 */
QString TrackData::getFormatToolTip(bool onlyRows)
{
  return TrackDataFormatReplacer::getToolTip(onlyRows);
}

/**
 * Get file extension including the dot.
 *
 * @param preferFromFilename true to prefer extension from current filename
 *                           over default extension for file type
 *
 * @return file extension, e.g. ".mp3".
 */
QString TrackData::getFileExtension(bool preferFromFilename) const
{
  QString fileExtension;
  QString absFilename;
  if (TaggedFile* taggedFile = getTaggedFile()) {
    fileExtension = taggedFile->getFileExtension();
    absFilename = taggedFile->getAbsFilename();
  }
  if (preferFromFilename || fileExtension.isEmpty()) {
    int dotPos = absFilename.lastIndexOf(QLatin1Char('.'));
    if (dotPos != -1) {
      return absFilename.mid(dotPos);
    }
  }
  return fileExtension;
}

/**
 * Get the total number of tracks in the directory.
 *
 * @return total number of tracks, -1 if unavailable.
 */
int TrackData::getTotalNumberOfTracksInDir() const
{
  TaggedFile* taggedFile = getTaggedFile();
  return taggedFile ? taggedFile->getTotalNumberOfTracksInDir() : -1;
}


/**
 * Get the difference between the imported duration and the track's duration.
 * @return absolute value of time difference in seconds, -1 if not available.
 */
int ImportTrackData::getTimeDifference() const
{
  int fileDuration = getFileDuration();
  int importDuration = getImportDuration();
  return fileDuration != 0 && importDuration != 0
      ? fileDuration > importDuration
        ? fileDuration - importDuration
        : importDuration - fileDuration
      : -1;
}

namespace {

/**
 * Get lower case words found in string.
 * @return lower case words.
 */
QSet<QString> getLowerCaseWords(const QString& str)
{
  if (!str.isEmpty()) {
    QString normalized = str.normalized(QString::NormalizationForm_D).toLower();
    QString simplified;
    for (auto it = normalized.constBegin(); it != normalized.constEnd(); ++it) {
      if (it->isLetter()) {
        simplified += *it;
      } else if (it->isPunct() || it->isSpace() || it->isSymbol()) {
        simplified += QLatin1Char(' ');
      }
    }
    return simplified.split(QLatin1Char(' '), QString::SkipEmptyParts).toSet();
  }
  return QSet<QString>();
}

}

/**
 * Get words of file name.
 * @return lower case words found in file name.
 */
QSet<QString> ImportTrackData::getFilenameWords() const
{
  QString fileName = getFilename();
  int endIndex = fileName.lastIndexOf(QLatin1Char('.'));
  if (endIndex > 0) {
    fileName.truncate(endIndex);
  }
  return getLowerCaseWords(fileName);
}

/**
 * Get words of title.
 * @return lower case words found in title.
 */
QSet<QString> ImportTrackData::getTitleWords() const
{
  return getLowerCaseWords(getTitle());
}


/**
 * Clear vector and associated data.
 */
void ImportTrackDataVector::clearData()
{
  clear();
  m_coverArtUrl.clear();
}

/**
 * Get album artist.
 * @return album artist.
 */
QString ImportTrackDataVector::getArtist() const
{
  return getFrame(Frame::FT_Artist);
}

/**
 * Get album title.
 * @return album title.
 */
QString ImportTrackDataVector::getAlbum() const
{
  return getFrame(Frame::FT_Album);
}

/**
 * Check if tag is supported in the first track.
 * @param tagNr tag number
 * @return true if tag is supported.
 */
bool ImportTrackDataVector::isTagSupported(Frame::TagNumber tagNr) const
{
  if (!isEmpty()) {
    TaggedFile* taggedFile = at(0).getTaggedFile();
    if (taggedFile) {
      return taggedFile->isTagSupported(tagNr);
    }
  }
  return true;
}

/**
 * Get frame from first track.
 * @param type frame type
 * @return value of frame.
 */
QString ImportTrackDataVector::getFrame(Frame::Type type) const
{
  QString result;
  if (!isEmpty()) {
    const ImportTrackData& trackData = at(0);
    result = trackData.getValue(type);
    if (!result.isEmpty())
      return result;
    TaggedFile* taggedFile = trackData.getTaggedFile();
    FrameCollection frames;
    for (Frame::TagNumber tagNr : Frame::allTagNumbers()) {
      taggedFile->getAllFrames(tagNr, frames);
      result = frames.getValue(type);
      if (!result.isEmpty())
        return result;
    }
  }
  return result;
}

/**
 * Read the tags from the files.
 * This can be used to fill the track data with another tag version.
 *
 * @param tagVersion tag version to read
 */
void ImportTrackDataVector::readTags(Frame::TagVersion tagVersion)
{
  for (iterator it = begin(); it != end(); ++it) {
    if (TaggedFile* taggedFile = it->getTaggedFile()) {
      it->clear();
      for (Frame::TagNumber tagNr : Frame::tagNumbersFromMask(tagVersion)) {
        if (it->empty()) {
          taggedFile->getAllFrames(tagNr, *it);
        } else {
          FrameCollection frames;
          taggedFile->getAllFrames(tagNr, frames);
          it->merge(frames);
        }
      }
    }
    it->setImportDuration(0);
    it->setEnabled(true);
  }
  setCoverArtUrl(QUrl());
}

#ifndef QT_NO_DEBUG
/**
 * Dump contents of tracks to debug console.
 */
void ImportTrackDataVector::dump() const
{
  qDebug("ImportTrackDataVector (%s - %s, %s):",
         qPrintable(getArtist()), qPrintable(getAlbum()),
         qPrintable(getCoverArtUrl().toString()));
  for (const_iterator it = constBegin();
       it != constEnd();
       ++it) {
    const ImportTrackData& trackData = *it;
    int fileDuration = trackData.getFileDuration();
    int importDuration = trackData.getImportDuration();
    qDebug("%d:%02d, %d:%02d, %s, %d, %s, %s, %s, %d, %s",
           fileDuration / 60, fileDuration % 60,
           importDuration / 60, importDuration % 60,
           qPrintable(trackData.getFilename()),
           trackData.getTrack(),
           qPrintable(trackData.getTitle()),
           qPrintable(trackData.getArtist()),
           qPrintable(trackData.getAlbum()),
           trackData.getYear(),
           qPrintable(trackData.getGenre()));
  }
}
#endif
