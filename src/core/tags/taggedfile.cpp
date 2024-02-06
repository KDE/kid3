/**
 * \file taggedfile.cpp
 * Handling of tagged files.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 25 Sep 2005
 *
 * Copyright (C) 2005-2024  Urs Fleisch
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

#include "taggedfile.h"
#include <QDir>
#include <QString>
#include <QRegularExpression>
#ifdef Q_OS_WIN32
#include <sys/types.h>
#include <sys/utime.h>
#else
#include <utime.h>
#endif
#include <sys/stat.h>
#include "tagconfig.h"
#include "formatconfig.h"
#include "genres.h"
#include "modeliterator.h"
#include "saferename.h"
#include "taggedfilesystemmodel.h"

/**
 * Constructor.
 *
 * @param idx index in tagged file system model
 */
TaggedFile::TaggedFile(const QPersistentModelIndex& idx)
  : m_index(idx), m_truncation(0), m_modified(false), m_marked(false)
{
  FOR_ALL_TAGS(tagNr) {
    m_changedFrames[tagNr] = 0;
    m_changed[tagNr] = false;
  }
  Q_ASSERT(m_index.model()->metaObject() == &TaggedFileSystemModel::staticMetaObject);
  if (const TaggedFileSystemModel* model = getTaggedFileSystemModel()) {
    m_newFilename = model->fileName(m_index);
    m_filename = m_newFilename;
  }
}

/**
 * Get tagged file model.
 * @return tagged file model.
 */
const TaggedFileSystemModel* TaggedFile::getTaggedFileSystemModel() const
{
  // The validity of this cast is checked in the constructor.
  return static_cast<const TaggedFileSystemModel*>(m_index.model());
}

/**
 * Get directory name.
 *
 * @return directory name
 */
QString TaggedFile::getDirname() const
{
  if (const TaggedFileSystemModel* model = getTaggedFileSystemModel()) {
    return model->filePath(m_index.parent());
  }
  return QString();
}

/**
 * Set file name.
 *
 * @param fn file name
 */
void TaggedFile::setFilename(const QString& fn)
{
  m_newFilename = fn;
  m_revertedFilename.clear();
  updateModifiedState();
}

/**
 * Set file name and format it if format while editing is switched on.
 *
 * @param fn file name
 */
void TaggedFile::setFilenameFormattedIfEnabled(QString fn)
{
  if (FilenameFormatConfig::instance().formatWhileEditing()) {
    FilenameFormatConfig::instance().formatString(fn);
  }
  setFilename(fn);
}

/**
 * Update the current filename after the file was renamed.
 */
void TaggedFile::updateCurrentFilename()
{
  if (const TaggedFileSystemModel* model = getTaggedFileSystemModel()) {
    if (const QString newName = model->fileName(m_index);
        !newName.isEmpty() && m_filename != newName) {
      if (m_newFilename == m_filename) {
        m_newFilename = newName;
      }
      m_filename = newName;
      updateModifiedState();
    }
  }
}

/**
 * Get current path to file.
 * @return absolute path.
 */
QString TaggedFile::currentFilePath() const
{
  if (const TaggedFileSystemModel* model = getTaggedFileSystemModel()) {
    return model->filePath(m_index);
  }
  return QString();
}

/**
 * Get features supported.
 * @return bit mask with Feature flags set.
 */
int TaggedFile::taggedFileFeatures() const
{
  return 0;
}

/**
 * Get currently active tagged file features.
 * @return active tagged file features.
 * @see setActiveTaggedFileFeatures()
 */
int TaggedFile::activeTaggedFileFeatures() const
{
  return 0;
}

/**
 * Activate some features provided by the tagged file.
 * For example, if the TF_ID3v24 feature is provided, it can be set, so that
 * writeTags() will write ID3v2.4.0 tags. If the feature is deactivated by
 * passing 0, tags in the default format will be written again.
 *
 * @param features bit mask with some of the Feature flags which are
 * provided by this file, as returned by taggedFileFeatures(), 0 to disable
 * special features.
 */
void TaggedFile::setActiveTaggedFileFeatures(int features)
{
  Q_UNUSED(features)
}

/**
 * Remove frames.
 *
 * @param tagNr tag number
 * @param flt filter specifying which frames to remove
 */
void TaggedFile::deleteFrames(Frame::TagNumber tagNr, const FrameFilter& flt)
{
  Frame frame;
  frame.setValue(QLatin1String(""));
  for (int i = Frame::FT_FirstFrame; i <= Frame::FT_LastV1Frame; ++i) {
    if (auto type = static_cast<Frame::Type>(i); flt.isEnabled(type)) {
      frame.setExtendedType(Frame::ExtendedType(type));
      setFrame(tagNr, frame);
    }
  }
}

/**
 * Check if file has an ID3v1 tag.
 *
 * @return true if a V1 tag is available.
 * @see isTagInformationRead()
 */
bool TaggedFile::hasTag(Frame::TagNumber) const
{
  return false;
}

/**
 * Check if tags are supported by the format of this file.
 *
 * @param tagNr tag number
 * @return true if V1 tags are supported.
 */
bool TaggedFile::isTagSupported(Frame::TagNumber tagNr) const
{
  return tagNr == Frame::Tag_2;
}

/**
 * Get absolute filename.
 *
 * @return absolute file path.
 */
QString TaggedFile::getAbsFilename() const
{
  QDir dir(getDirname());
  return QDir::cleanPath(dir.absoluteFilePath(m_newFilename));
}

/**
 * Mark filename as unchanged.
 */
void TaggedFile::markFilenameUnchanged()
{
  m_filename = m_newFilename;
  m_revertedFilename.clear();
  updateModifiedState();
}

/**
 * Revert modification of filename.
 */
void TaggedFile::revertChangedFilename()
{
  m_revertedFilename = m_newFilename;
  m_newFilename = m_filename;
  updateModifiedState();
}

/**
 * Undo reverted modification of filename.
 * When writeTags() fails because the file is not writable, the filename is
 * reverted using revertChangedFilename() so that the file permissions can be
 * changed using the real filename. After changing the permissions, this
 * function can be used to change the filename back before saving the file.
 */
void TaggedFile::undoRevertChangedFilename()
{
  if (!m_revertedFilename.isEmpty()) {
    m_newFilename = m_revertedFilename;
    m_revertedFilename.clear();
    updateModifiedState();
  }
}

/**
 * Mark tag as changed.
 *
 * @param tagNr tag number
 * @param extendedType type of changed frame
 */
void TaggedFile::markTagChanged(Frame::TagNumber tagNr,
                                const Frame::ExtendedType& extendedType)
{
  Frame::Type type = extendedType.getType();
  m_changed[tagNr] = true;
  if (static_cast<unsigned>(type) < sizeof(m_changedFrames[tagNr]) * 8) {
    m_changedFrames[tagNr] |= 1ULL << type;
  }
  if (type == Frame::FT_Other) {
    if (const QString internalName = extendedType.getInternalName();
        !internalName.isEmpty()) {
      m_changedOtherFrameNames[tagNr].insert(internalName);
    }
  }
  updateModifiedState();
}

/**
 * Mark tag as unchanged.
 * @param tagNr tag number
 */
void TaggedFile::markTagUnchanged(Frame::TagNumber tagNr) {
  m_changed[tagNr] = false;
  m_changedFrames[tagNr] = 0;
  m_changedOtherFrameNames[tagNr].clear();
  clearTrunctionFlags(tagNr);
  updateModifiedState();
}

/**
 * Get the types of the changed frames in a tag.
 * @param tagNr tag number
 * @return types of changed frames.
 */
QList<Frame::ExtendedType> TaggedFile::getChangedFrames(
    Frame::TagNumber tagNr) const {
  QList<Frame::ExtendedType> types;
  if (tagNr < Frame::Tag_NumValues) {
    const QSet<QString> changedOtherFrameNames = m_changedOtherFrameNames[tagNr];
    const quint64 changedFrames = m_changedFrames[tagNr];
    quint64 mask;
    int i;
    for (i = Frame::FT_FirstFrame, mask = 1ULL;
         i <= Frame::FT_LastFrame;
         ++i, mask <<= 1) {
      if (changedFrames & mask) {
        types.append(Frame::ExtendedType(
                       static_cast<Frame::Type>(i), QString()));
      }
    }
    if (!changedOtherFrameNames.isEmpty()) {
      for (const QString& name : changedOtherFrameNames) {
        types.append(Frame::ExtendedType(Frame::FT_Other, name));
      }
    } else if (changedFrames & (1ULL << Frame::FT_Other)) {
      types.append(Frame::ExtendedType(Frame::FT_Other, QString()));
    }
    if (changedFrames & (1ULL << Frame::FT_UnknownFrame)) {
      types.append(Frame::ExtendedType());
    }
  }
  return types;
}

/**
 * Set the types of the changed frames in a tag.
 * @param tagNr tag number
 * @param types types of changed frames
 */
void TaggedFile::setChangedFrames(Frame::TagNumber tagNr,
                                  const QList<Frame::ExtendedType>& types) {
  quint64& mask = m_changedFrames[tagNr];
  QSet<QString>& changedOtherFrameNames = m_changedOtherFrameNames[tagNr];
  mask = 0;
  changedOtherFrameNames.clear();
  for (const auto& extendedType : types) {
    Frame::Type type = extendedType.getType();
    mask |= 1ULL << type;
    if (type == Frame::FT_Other) {
      if (const QString internalName = extendedType.getInternalName();
          !internalName.isEmpty()) {
        changedOtherFrameNames.insert(internalName);
      }
    }
  }
  m_changed[tagNr] = mask != 0;
  updateModifiedState();
}

void TaggedFile::updateModifiedState()
{
  bool modified = false;
  FOR_ALL_TAGS(tagNr) {
    if (m_changed[tagNr]) {
      modified = true;
      break;
    }
  }
  modified = modified || m_newFilename != m_filename;
  if (m_modified != modified) {
    m_modified = modified;
    if (const TaggedFileSystemModel* model = getTaggedFileSystemModel()) {
      const_cast<TaggedFileSystemModel*>(model)->notifyModificationChanged(
            m_index, m_modified);
    }
  }
}

/**
 * Notify model about changes in extra model data, e.g. the information on
 * which the CoreTaggedFileIconProvider depends.
 *
 * This method shall be called when such data changes, e.g. at the end of
 * readTags() implementations.
 *
 * @param priorIsTagInformationRead prior value returned by
 * isTagInformationRead()
 */
void TaggedFile::notifyModelDataChanged(bool priorIsTagInformationRead) const
{
  if (isTagInformationRead() != priorIsTagInformationRead) {
    if (const TaggedFileSystemModel* model = getTaggedFileSystemModel()) {
      const_cast<TaggedFileSystemModel*>(model)->notifyModelDataChanged(m_index);
    }
  }
}

/**
 * Notify model about changes in the truncation state.
 *
 * This method shall be called when truncation is checked.
 *
 * @param priorTruncation prior value of m_truncation != 0
 */
void TaggedFile::notifyTruncationChanged(bool priorTruncation) const
{
  if (bool currentTruncation = m_truncation != 0;
      currentTruncation != priorTruncation) {
    if (const TaggedFileSystemModel* model = getTaggedFileSystemModel()) {
      const_cast<TaggedFileSystemModel*>(model)->notifyModelDataChanged(m_index);
    }
  }
}


namespace {

/**
 * Remove artist part from album string.
 * This is used when only the album is needed, but the regexp in
 * getTagsFromFilename() matched a "artist - album" string.
 *
 * @param album album string
 *
 * @return album with artist removed.
 */
QString removeArtist(const QString& album)
{
  QString str(album);
  if (int pos = str.indexOf(QLatin1String(" - ")); pos != -1) {
    str.remove(0, pos + 3);
  }
  return str;
}

}

/**
 * Get tags from filename.
 * Supported formats:
 * album/track - artist - song
 * artist - album/track song
 * /artist - album - track - song
 * album/artist - track - song
 * artist/album/track song
 * album/artist - song
 *
 * @param frames frames to put result
 * @param fmt format string containing the following codes:
 *            %s title (song)
 *            %l album
 *            %a artist
 *            %c comment
 *            %y year
 *            %t track
 */
void TaggedFile::getTagsFromFilename(FrameCollection& frames, const QString& fmt) const
{
  QRegularExpression re;
  QRegularExpressionMatch match;
  QString fn(getAbsFilename());

  // construct regular expression from format string

  // if the format does not contain a '_', they are replaced by spaces
  // in the filename.
  QString fileName(fn);
  if (!fmt.contains(QLatin1Char('_'))) {
    fileName.replace(QLatin1Char('_'), QLatin1Char(' '));
  }

  QString pattern;
  QMap<QString, int> codePos;
  bool useCustomCaptures = fmt.contains(QLatin1String("}("));
  if (!useCustomCaptures) {
    // escape regexp characters
    const int fmtLen = fmt.length();
    static const QString escChars(QLatin1String("+?.*^$()[]{}|\\"));
    for (int i = 0; i < fmtLen; ++i) {
      const QChar ch = fmt.at(i);
      if (escChars.contains(ch)) {
        pattern += QLatin1Char('\\');
      }
      pattern += ch;
    }
  } else {
    pattern = fmt;
  }

  static const struct {
    const char* from;
    const char* to;
  } codeToName[] = {
    { "s", "title" },
    { "l", "album" },
    { "a", "artist" },
    { "c", "comment" },
    { "y", "date" },
    { "t", "track number" },
    { "g", "genre" },
    { "year", "date" },
    { "track", "track number" },
    { "tracknumber", "track number" },
    { "discnumber", "disc number" }
  };
  int percentIdx = 0, nr = 1;
  const QString prefix(QLatin1String(useCustomCaptures ? "%{" : "%\\{"));
  const QString suffix(QLatin1String(useCustomCaptures ? "}"  : "\\}"));
  const int prefixLen = prefix.length();
  for (const auto& c2n : codeToName) {
    QString from = QString::fromLatin1(c2n.from);
    QString to = QString::fromLatin1(c2n.to);
    from = from.size() == 1 ? QLatin1Char('%') + from : prefix + from + suffix;
    to = prefix + to + suffix;
    pattern.replace(from, to);
  }

  // remove %{} expressions and insert captures if without custom captures
  while ((percentIdx = pattern.indexOf(prefix, percentIdx)) >= 0 &&
         percentIdx < pattern.length() - 1) {
    if (int closingBracePos = pattern.indexOf(suffix, percentIdx + prefixLen);
        closingBracePos > percentIdx + prefixLen) {
      QString code = pattern.mid(percentIdx + prefixLen,
                                 closingBracePos - percentIdx - prefixLen);
      codePos[code] = nr++;
      const int braceExprLen = closingBracePos - percentIdx + prefixLen - 1;
      if (!useCustomCaptures) {
        QString capture(QLatin1String(
                          code == QLatin1String("track number")
                          ? "([A-Za-z]?\\d+[A-Za-z]?)"
                          : code == QLatin1String("date")
                            ? "(\\d{1,4}[\\dT :-]*)"
                            : code == QLatin1String("disc number") ||
                              code == QLatin1String("bpm")
                              ? "(\\d{1,4})"
                              : "([^-_\\./ ](?:[^/]*[^-_/ ])?)"));
        pattern.replace(percentIdx, braceExprLen, capture);
        percentIdx += capture.length();
      } else {
        pattern.remove(percentIdx, braceExprLen);
        percentIdx += 2;
      }
    } else {
      percentIdx += prefixLen;
    }
  }

  if (!useCustomCaptures) {
    // accept file names with spaces before the extension
    pattern += QLatin1String("\\s*");
  }

  // and finally a dot followed by 2 to 4 characters for the extension
  pattern += QLatin1String("\\..{2,4}$");

  re.setPattern(pattern);
  if ((match = re.match(fileName)).hasMatch()) {
    for (auto it = codePos.begin(); it != codePos.end(); ++it) {
      const QString& name = it.key();
      if (QString str = match.captured(*it); !str.isEmpty()) {
        if (!useCustomCaptures && name == QLatin1String("track number") &&
            str.length() == 2 && str[0] == QLatin1Char('0')) {
          // remove leading zero
          str = str.mid(1);
        }
        if (name != QLatin1String("ignore"))
          frames.setValue(Frame::ExtendedType(name), str);
      }
    }
    return;
  }

  // album/track - artist - song
  re.setPattern(QLatin1String(
    R"(([^/]+)/(\d{1,3})[-_\. ]+([^-_\./ ][^/]+)[_ ]-[_ ])"
    R"(([^-_\./ ][^/]+)\..{2,4}$)"));
  if ((match = re.match(fn)).hasMatch()) {
    frames.setAlbum(removeArtist(match.captured(1)));
    frames.setTrack(match.captured(2).toInt());
    frames.setArtist(match.captured(3));
    frames.setTitle(match.captured(4));
    return;
  }

  // artist - album (year)/track song
  re.setPattern(QLatin1String(
    R"(([^/]+)[_ ]-[_ ]([^/]+)[_ ]\((\d{4})\)/(\d{1,3})[-_\. ]+)"
    R"(([^-_\./ ][^/]+)\..{2,4}$)"));
  if ((match = re.match(fn)).hasMatch()) {
    frames.setArtist(match.captured(1));
    frames.setAlbum(match.captured(2));
    frames.setYear(match.captured(3).toInt());
    frames.setTrack(match.captured(4).toInt());
    frames.setTitle(match.captured(5));
    return;
  }

  // artist - album/track song
  re.setPattern(QLatin1String(
    R"(([^/]+)[_ ]-[_ ]([^/]+)/(\d{1,3})[-_\. ]+([^-_\./ ][^/]+)\..{2,4}$)"));
  if ((match = re.match(fn)).hasMatch()) {
    frames.setArtist(match.captured(1));
    frames.setAlbum(match.captured(2));
    frames.setTrack(match.captured(3).toInt());
    frames.setTitle(match.captured(4));
    return;
  }
  // /artist - album - track - song
  re.setPattern(QLatin1String(
    R"(/([^/]+[^-_/ ])[_ ]-[_ ]([^-_/ ][^/]+[^-_/ ])[-_\. ]+)"
    R"((\d{1,3})[-_\. ]+([^-_\./ ][^/]+)\..{2,4}$)"));
  if ((match = re.match(fn)).hasMatch()) {
    frames.setArtist(match.captured(1));
    frames.setAlbum(match.captured(2));
    frames.setTrack(match.captured(3).toInt());
    frames.setTitle(match.captured(4));
    return;
  }
  // album/artist - track - song
  re.setPattern(QLatin1String(
    R"(([^/]+)/([^/]+[^-_\./ ])[-_\. ]+(\d{1,3})[-_\. ]+)"
    R"(([^-_\./ ][^/]+)\..{2,4}$)"));
  if ((match = re.match(fn)).hasMatch()) {
    frames.setAlbum(removeArtist(match.captured(1)));
    frames.setArtist(match.captured(2));
    frames.setTrack(match.captured(3).toInt());
    frames.setTitle(match.captured(4));
    return;
  }
  // artist/album/track song
  re.setPattern(QLatin1String(
    R"(([^/]+)/([^/]+)/(\d{1,3})[-_\. ]+([^-_\./ ][^/]+)\..{2,4}$)"));
  if ((match = re.match(fn)).hasMatch()) {
    frames.setArtist(match.captured(1));
    frames.setAlbum(match.captured(2));
    frames.setTrack(match.captured(3).toInt());
    frames.setTitle(match.captured(4));
    return;
  }
  // album/artist - song
  re.setPattern(QLatin1String(
    "([^/]+)/([^/]+[^-_/ ])[_ ]-[_ ]([^-_/ ][^/]+)\\..{2,4}$"));
  if ((match = re.match(fn)).hasMatch()) {
    frames.setAlbum(removeArtist(match.captured(1)));
    frames.setArtist(match.captured(2));
    frames.setTitle(match.captured(3));
  }
}

/**
 * Format a time string "h:mm:ss".
 * If the time is less than an hour, the hour is not put into the
 * string and the minute is not padded with zeroes.
 *
 * @param seconds time in seconds
 *
 * @return string with the time in hours, minutes and seconds.
 */
QString TaggedFile::formatTime(unsigned seconds)
{
  unsigned hours = seconds / 3600;
  seconds %= 3600;
  unsigned minutes = seconds / 60;
  seconds %= 60;
  QString timeStr;
  if (hours > 0) {
    timeStr = QString(QLatin1String("%1:%2:%3"))
        .arg(hours)
        .arg(minutes, 2, 10, QLatin1Char('0'))
        .arg(seconds, 2, 10, QLatin1Char('0'));
  } else {
    timeStr = QString(QLatin1String("%1:%2"))
        .arg(minutes).arg(seconds, 2, 10, QLatin1Char('0'));
  }
  return timeStr;
}

/**
 * Rename a file.
 * This methods takes care of case insensitive filesystems.
 * @return true if ok.
 */
bool TaggedFile::renameFile() const
{
  const QString dirname = getDirname();
  const QString fnOld = currentFilename();
  const QString fnNew = getFilename();
  auto model = const_cast<TaggedFileSystemModel*>(getTaggedFileSystemModel());

  if (fnNew.toLower() == fnOld.toLower()) {
    // If the filenames only differ in case, the new file is reported to
    // already exist on case insensitive filesystems (e.g. Windows),
    // so it is checked if the new file is really the old file by
    // comparing inodes and devices. If the files are not the same,
    // another file would be overwritten and an error is reported.
    if (QFile::exists(dirname + QDir::separator() + fnNew)) {
      struct stat statOld, statNew;
      if (::stat((dirname + QDir::separator() + fnOld).toLatin1().data(),
                 &statOld) == 0 &&
          ::stat((dirname + QDir::separator() + fnNew).toLatin1().data(),
                 &statNew) == 0 &&
          !(statOld.st_ino == statNew.st_ino &&
            statOld.st_dev == statNew.st_dev)) {
        qDebug("rename(%s, %s): %s already exists", fnOld.toLatin1().data(),
               fnNew.toLatin1().data(), fnNew.toLatin1().data());
        return false;
      }
    }

    // if the filenames only differ in case, first rename to a
    // temporary filename, so that it works also with case
    // insensitive filesystems (e.g. Windows).
    QString temp_filename(fnNew);
    temp_filename.append(QLatin1String("_CASE"));
    if (!((model && model->rename(m_index, temp_filename)) ||
          Utils::safeRename(dirname, fnOld, temp_filename))) {
      qDebug("rename(%s, %s) failed", fnOld.toLatin1().data(),
             temp_filename.toLatin1().data());
      return false;
    }
    if (!((model && model->rename(m_index, fnNew)) ||
          Utils::safeRename(dirname, temp_filename, fnNew))) {
      qDebug("rename(%s, %s) failed", temp_filename.toLatin1().data(),
             fnNew.toLatin1().data());
      return false;
    }
  } else if (QFile::exists(dirname + QDir::separator() + fnNew)) {
    qDebug("rename(%s, %s): %s already exists", fnOld.toLatin1().data(),
           fnNew.toLatin1().data(), fnNew.toLatin1().data());
    return false;
  } else if (!((model && model->rename(m_index, fnNew)) ||
               Utils::safeRename(dirname, fnOld, fnNew))) {
    qDebug("rename(%s, %s) failed", fnOld.toLatin1().data(),
           fnNew.toLatin1().data());
    return false;
  }
  return true;
}

/**
 * Get field name for comment from configuration.
 *
 * @return field name.
 */
QString TaggedFile::getCommentFieldName() const
{
  return TagConfig::instance().commentName();
}

/**
 * Split a track string into number and total.
 *
 * @param str track
 * @param total the total is returned here if found, else 0
 *
 * @return number, 0 if parsing failed, -1 if str is null
 */
int TaggedFile::splitNumberAndTotal(const QString& str, int* total)
{
  if (total)
    *total = 0;
  if (str.isNull())
    return -1;

  int slashPos = str.indexOf(QLatin1Char('/'));
  if (slashPos == -1)
    return str.toInt();

#if QT_VERSION >= 0x060000
  if (total)
    *total = str.mid(slashPos + 1).toInt();
  return str.left(slashPos).toInt();
#else
  if (total)
    *total = str.midRef(slashPos + 1).toInt();
  return str.leftRef(slashPos).toInt();
#endif
}

/**
 * Fix up a key to be valid.
 * If the key contains new line characters because it is coming from an ID3
 * frame (e.g. "COMM - COMMENTS\nDescription"), the description part is taken.
 * Illegal characters depending on @a tagType are removed.
 *
 * @param key key which might have invalid characters.
 * @param tagType tag type
 * @return key which can be used for tag type.
 */
QString TaggedFile::fixUpTagKey(const QString& key, TagType tagType)
{
  int len = key.length();
  int i = key.indexOf(QLatin1Char('\n'));
  if (i < 0) {
    // key does not contain '\n' => 0..len
    i = 0;
  } else if (i >= len - 1) {
    // '\n' at end of key => 0..len-1
    i = 0;
    --len;
  } else {
    // key contains '\n' at i => i+1..len
    ++i;
  }

  // Allowed characters depending on tag type:
  // TT_Vorbis: != '=' && >= 0x20 && <= 0x7D
  // TT_Ape: >= 0x20 && <= 0x7E
  QChar forbidden;
  QChar firstAllowed;
  QChar lastAllowed;
  if (tagType == TT_Vorbis) {
    forbidden = QLatin1Char('=');
    firstAllowed = QLatin1Char('\x20');
    lastAllowed = QLatin1Char('\x7d');
  } else if (tagType == TT_Ape) {
    firstAllowed = QLatin1Char('\x20');
    lastAllowed = QLatin1Char('\x7e');
  }

  QString result;
  result.reserve(len - i);
  if (forbidden.isNull() && firstAllowed.isNull() && lastAllowed.isNull()) {
    result = key.mid(i, len - i);
  } else {
    while (i < len) {
      if (QChar ch = key.at(i);
          ch != forbidden &&
          ch >= firstAllowed && ch <= lastAllowed) {
        result.append(ch);
      }
      ++i;
    }
  }
  return result;
}

/**
 * Get the total number of tracks in the directory.
 *
 * @return total number of tracks, -1 if unavailable.
 */
int TaggedFile::getTotalNumberOfTracksInDir() const {
  int numTracks = -1;
  if (QModelIndex parentIdx = m_index.parent(); parentIdx.isValid()) {
    numTracks = 0;
    TaggedFileOfDirectoryIterator it(parentIdx);
    while (it.hasNext()) {
      it.next();
      ++numTracks;
    }
  }
  return numTracks;
}

/**
 * Get the total number of tracks if it is enabled.
 *
 * @return total number of tracks,
 *         -1 if disabled or unavailable.
 */
int TaggedFile::getTotalNumberOfTracksIfEnabled() const
{
  return TagConfig::instance().enableTotalNumberOfTracks()
      ? getTotalNumberOfTracksInDir() : -1;
}

/**
 * Format track number/total number of tracks with configured digits.
 *
 * @param num track number, <= 0 if empty
 * @param numTracks total number of tracks, <= 0 to disable
 *
 * @return formatted "track/total" string.
 */
QString TaggedFile::trackNumberString(int num, int numTracks) const
{
  int numDigits = getTrackNumberDigits();
  QString str;
  if (num != 0) {
    if (numDigits > 0) {
      str = QString(QLatin1String("%1"))
          .arg(num, numDigits, 10, QLatin1Char('0'));
    } else {
      str.setNum(num);
    }
    if (numTracks > 0) {
      str += QLatin1Char('/');
      if (numDigits > 0) {
        str += QString(QLatin1String("%1"))
            .arg(numTracks, numDigits, 10, QLatin1Char('0'));
      } else {
        str += QString::number(numTracks);
      }
    }
  } else {
    str = QLatin1String("");
  }
  return str;
}

/**
 * Format the track number (digits, total number of tracks) if enabled.
 *
 * @param value    string containing track number, will be modified
 * @param addTotal true to add total number of tracks if enabled
 *                 "/t" with t = total number of tracks will be appended
 *                 if enabled and value contains a number
 */
void TaggedFile::formatTrackNumberIfEnabled(QString& value, bool addTotal) const
{
  int numDigits = getTrackNumberDigits();
  if (int numTracks = addTotal ? getTotalNumberOfTracksIfEnabled() : -1;
      numTracks > 0 || numDigits > 1) {
    bool ok;
    if (int trackNr = value.toInt(&ok); ok && trackNr > 0) {
      if (numTracks > 0) {
        value = QString(QLatin1String("%1/%2"))
            .arg(trackNr, numDigits, 10, QLatin1Char('0'))
            .arg(numTracks, numDigits, 10, QLatin1Char('0'));
      } else {
        value = QString(QLatin1String("%1"))
            .arg(trackNr, numDigits, 10, QLatin1Char('0'));
      }
    }
  }
}

/**
 * Get the number of track number digits configured.
 *
 * @return track number digits,
 *         1 if invalid or unavailable.
 */
int TaggedFile::getTrackNumberDigits() const
{
  int numDigits = TagConfig::instance().trackNumberDigits();
  if (numDigits < 1 || numDigits > 5)
    numDigits = 1;
  return numDigits;
}

/**
 * Get the format of tag 1.
 *
 * @return string describing format of tag 1,
 *         e.g. "ID3v1.1", "ID3v2.3", "Vorbis", "APE",
 *         QString::null if unknown.
 */
QString TaggedFile::getTagFormat(Frame::TagNumber) const
{
  return QString();
}

/**
 * Check if a string has to be truncated.
 *
 * @param tagNr tag number
 * @param str  string to be checked
 * @param flag flag to be set if string has to be truncated
 * @param len  maximum length of string
 *
 * @return str truncated to len characters if necessary, else QString::null.
 */
QString TaggedFile::checkTruncation(
  Frame::TagNumber tagNr, const QString& str, quint64 flag, int len)
{
  if (tagNr != Frame::Tag_Id3v1)
    return QString();

  bool priorTruncation = m_truncation != 0;
  QString result;
  if (str.length() > len) {
    result = str;
    result.truncate(len);
    m_truncation |= flag;
  } else {
    m_truncation &= ~flag;
  }
  notifyTruncationChanged(priorTruncation);
  return result;
}

/**
 * Check if a number has to be truncated.
 *
 * @param tagNr tag number
 * @param val  value to be checked
 * @param flag flag to be set if number has to be truncated
 * @param max  maximum value
 *
 * @return val truncated to max if necessary, else -1.
 */
int TaggedFile::checkTruncation(Frame::TagNumber tagNr, int val, quint64 flag,
                                int max)
{
  if (tagNr != Frame::Tag_Id3v1)
    return -1;

  bool priorTruncation = m_truncation != 0;
  int result;
  if (val > max) {
    m_truncation |= flag;
    result = max;
  } else {
    m_truncation &= ~flag;
    result = -1;
  }
  notifyTruncationChanged(priorTruncation);
  return result;
}

/**
 * Add a frame in the tags.
 *
 * @param tagNr tag number
 * @param frame frame to add, a field list may be added by this method
 *
 * @return true if ok.
 */
bool TaggedFile::addFrame(Frame::TagNumber tagNr, Frame& frame)
{
  if (tagNr == Frame::Tag_Id3v1)
    return false;

  return setFrame(tagNr, frame);
}

/**
 * Delete a frame from the tags.
 *
 * @param tagNr tag number
 * @param frame frame to delete
 *
 * @return true if ok.
 */
bool TaggedFile::deleteFrame(Frame::TagNumber tagNr, const Frame& frame)
{
  if (tagNr == Frame::Tag_Id3v1)
    return false;

  Frame emptyFrame(frame);
  emptyFrame.setValue(QLatin1String(""));
  return setFrame(tagNr, emptyFrame);
}

/**
 * Get all frames in tag.
 *
 * @param tagNr tag number
 * @param frames frame collection to set.
 */
void TaggedFile::getAllFrames(Frame::TagNumber tagNr, FrameCollection& frames)
{
  frames.clear();
  Frame frame;
  for (int i = Frame::FT_FirstFrame; i <= Frame::FT_LastV1Frame; ++i) {
    if (getFrame(tagNr, static_cast<Frame::Type>(i), frame)) {
      frames.insert(frame);
    }
  }
}

/**
 * Update marked property of frames.
 * Mark frames which violate configured rules. This method should be called
 * in reimplementations of getAllFrames().
 *
 * @param tagNr tag number
 * @param frames frames to check
 */
void TaggedFile::updateMarkedState(Frame::TagNumber tagNr,
                                   FrameCollection& frames)
{
  // As long as there is only a single m_marked flag, only support tag 2.
  if (tagNr != Frame::Tag_2)
    return;

  m_marked = false;
  const TagConfig& tagCfg = TagConfig::instance();

  if (tagCfg.markStandardViolations() &&
      getTagFormat(tagNr).startsWith(QLatin1String("ID3v2")) &&
      FrameNotice::addId3StandardViolationNotice(frames)) {
    m_marked = true;
  }

  if (tagCfg.markOversizedPictures()) {
    auto it =
        frames.findByExtendedType(Frame::ExtendedType(Frame::FT_Picture));
    while (it != frames.cend() && it->getType() == Frame::FT_Picture) {
      if (auto& frame = const_cast<Frame&>(*it);
          FrameNotice::addPictureTooLargeNotice(
            frame, tagCfg.maximumPictureSize())) {
        m_marked = true;
      }
      ++it;
    }
  }
}

/**
 * Close any file handles which are held open by the tagged file object.
 * The default implementation does nothing. If a concrete subclass holds
 * any file handles open, it has to close them in this method. This method
 * can be used before operations which require that a file is not open,
 * e.g. file renaming on Windows.
 */
void TaggedFile::closeFileHandle()
{
}

/**
 * Add a suitable field list for the frame if missing.
 * If a frame is created, its field list is empty. This method will create
 * a field list appropriate for the frame type and tagged file type if no
 * field list exists. The default implementation does nothing.
 */
void TaggedFile::addFieldList(Frame::TagNumber, Frame&) const
{
}

/**
 * Set frames in tag.
 *
 * @param tagNr tag number
 * @param frames      frame collection
 * @param onlyChanged only frames with value marked as changed are set
 */
void TaggedFile::setFrames(Frame::TagNumber tagNr,
                           const FrameCollection& frames, bool onlyChanged)
{
  if (tagNr == Frame::Tag_Id3v1) {
    for (auto it = frames.cbegin(); it != frames.cend(); ++it) {
      if (!onlyChanged || it->isValueChanged()) {
        setFrame(tagNr, *it);
      }
    }
  } else {
    bool myFramesValid = false;
    FrameCollection myFrames;
    QSet<int> replacedIndexes;

    for (auto it = frames.cbegin(); it != frames.cend(); ++it) {
      if (!onlyChanged || it->isValueChanged()) {
        if (it->getIndex() != -1) {
          // The frame has an index, so the original tag can be modified
          setFrame(tagNr, *it);
        } else {
          // The frame does not have an index
          // The frame has to be looked up and modified
          if (!myFramesValid) {
            getAllFrames(tagNr, myFrames);
            myFramesValid = true;
          }
          auto myIt = myFrames.find(*it);
          int myIndex = -1;
          while (myIt != myFrames.end() && !(*it < *myIt) &&
                 (myIndex = myIt->getIndex()) != -1) {
            if (!replacedIndexes.contains(myIndex)) {
              break;
            }
            myIndex = -1;
            ++myIt;
          }
          if (myIndex != -1) {
            replacedIndexes.insert(myIndex);
            if (!myIt->isFuzzyEqual(*it)) {
              Frame myFrame(*it);
              myFrame.setIndex(myIndex);
              setFrame(tagNr, myFrame);
            }
          } else {
            // Such a frame does not exist, add a new one.
            if (!it->getValue().isEmpty() || !it->getFieldList().isEmpty()) {
              Frame addedFrame(*it);
              addFrame(tagNr, addedFrame);
              Frame myFrame(*it);
              myFrame.setIndex(addedFrame.getIndex());
              setFrame(tagNr, myFrame);
            }
          }
        }
      }
    }
  }
}

/**
 * Get access and modification time of file.
 * @param path file path
 * @param actime the last access time is returned here
 * @param modtime the last modification time is returned here
 * @return true if ok.
 */
bool TaggedFile::getFileTimeStamps(const QString& path,
                                   quint64& actime, quint64& modtime)
{
#ifdef Q_OS_WIN32
  int len = path.length();
  QVarLengthArray<wchar_t> a(len + 1);
  wchar_t* ws = a.data();
  len = path.toWCharArray(ws);
  ws[len] = 0;
  struct _stat fileStat;
  if (::_wstat(ws, &fileStat) == 0) {
    actime  = fileStat.st_atime;
    modtime = fileStat.st_mtime;
    return true;
  }
#else
  struct stat fileStat;
  if (::stat(QFile::encodeName(path), &fileStat) == 0) {
    actime  = fileStat.st_atime;
    modtime = fileStat.st_mtime;
    return true;
  }
#endif
  return false;
}

/**
 * Set access and modification time of file.
 * @param path file path
 * @param actime last access time
 * @param modtime last modification time
 * @return true if ok.
 */
bool TaggedFile::setFileTimeStamps(const QString& path,
                                   quint64 actime, quint64 modtime)
{
#ifdef Q_OS_WIN32
  int len = path.length();
  QVarLengthArray<wchar_t> a(len + 1);
  wchar_t* ws = a.data();
  len = path.toWCharArray(ws);
  ws[len] = 0;
  struct _utimbuf times;
  times.actime = actime;
  times.modtime = modtime;
  return ::_wutime(ws, &times) == 0;
#else
  struct utimbuf times;
  times.actime = actime;
  times.modtime = modtime;
  return ::utime(QFile::encodeName(path), &times) == 0;
#endif
}


/**
 * Constructor.
 */
TaggedFile::DetailInfo::DetailInfo()
  : channelMode(CM_None), channels(0), sampleRate(0), bitrate(0), duration(0),
    valid(false), vbr(false)
{
}

/**
 * Get string representation of detail information.
 * @return information summary as string.
 */
QString TaggedFile::DetailInfo::toString() const
{
  QString str;
  if (valid) {
    str = format;
    str += QLatin1Char(' ');
    if (bitrate > 0 && bitrate < 16384) {
      if (vbr) str += QLatin1String("VBR ");
      str += QString::number(bitrate);
      str += QLatin1String(" kbps ");
    }
    if (sampleRate > 0) {
      str += QString::number(sampleRate);
      str += QLatin1String(" Hz ");
    }
    switch (channelMode) {
      case TaggedFile::DetailInfo::CM_Stereo:
        str += QLatin1String("Stereo ");
        break;
      case TaggedFile::DetailInfo::CM_JointStereo:
        str += QLatin1String("Joint Stereo ");
        break;
      default:
        if (channels > 0) {
          str += QString::number(channels);
          str += QLatin1String(" Channels ");
        }
    }
    if (duration > 0) {
      str += TaggedFile::formatTime(duration);
    }
  }
  return str;
}
