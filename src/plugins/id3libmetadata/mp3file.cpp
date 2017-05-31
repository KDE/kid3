/**
 * \file mp3file.cpp
 * Handling of tagged MP3 files.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
 *
 * Copyright (C) 2003-2013  Urs Fleisch
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

#include "mp3file.h"

#include <QDir>
#include <QString>
#include <QTextCodec>
#include <QByteArray>
#include <QtEndian>

#include <cstring>
#include <id3/tag.h>
#ifdef Q_OS_WIN32
#include <id3.h>
#endif

#include "id3libconfig.h"
#include "genres.h"
#include "attributedata.h"

#ifdef Q_OS_WIN32
/**
 * This will be set for id3lib versions with Unicode bugs.
 * ID3LIB_ symbols cannot be found on Windows ?!
 */
#define UNICODE_SUPPORT_BUGGY 1
#else
/** This will be set for id3lib versions with Unicode bugs. */
#define UNICODE_SUPPORT_BUGGY ((((ID3LIB_MAJOR_VERSION) << 16) + ((ID3LIB_MINOR_VERSION) << 8) + (ID3LIB_PATCH_VERSION)) <= 0x030803)
#endif

#if defined __GNUC__ && (__GNUC__ * 100 + __GNUC_MINOR__) >= 407
/** Defined if GCC is used and supports diagnostic pragmas */
#define GCC_HAS_DIAGNOSTIC_PRAGMA
#endif

namespace {

/** Text codec for ID3v1 tags, 0 to use default (ISO 8859-1) */
const QTextCodec* s_textCodecV1 = 0;

/** Default text encoding */
ID3_TextEnc s_defaultTextEncoding = ID3TE_ISO8859_1;

/**
 * Get the default text encoding.
 * @return default text encoding.
 */
ID3_TextEnc getDefaultTextEncoding() { return s_defaultTextEncoding; }

}

/**
 * Constructor.
 *
 * @param idx index in file proxy model
 */
Mp3File::Mp3File(const QPersistentModelIndex& idx) :
  TaggedFile(idx), m_tagV1(0), m_tagV2(0)
{
}

/**
 * Destructor.
 */
Mp3File::~Mp3File()
{
  if (m_tagV1) {
    delete m_tagV1;
  }
  if (m_tagV2) {
    delete m_tagV2;
  }
}

/**
 * Get key of tagged file format.
 * @return "Id3libMetadata".
 */
QString Mp3File::taggedFileKey() const
{
  return QLatin1String("Id3libMetadata");
}

/**
 * Get features supported.
 * @return bit mask with Feature flags set.
 */
int Mp3File::taggedFileFeatures() const
{
  return TF_ID3v11 | TF_ID3v23;
}

/**
 * Read tags from file.
 *
 * @param force true to force reading even if tags were already read.
 */
void Mp3File::readTags(bool force)
{
  bool priorIsTagInformationRead = isTagInformationRead();
  QByteArray fn = QFile::encodeName(currentFilePath());

  if (force && m_tagV1) {
    m_tagV1->Clear();
    m_tagV1->Link(fn, ID3TT_ID3V1);
    markTagUnchanged(Frame::Tag_1);
  }
  if (!m_tagV1) {
    m_tagV1 = new ID3_Tag;
    m_tagV1->Link(fn, ID3TT_ID3V1);
    markTagUnchanged(Frame::Tag_1);
  }

  if (force && m_tagV2) {
    m_tagV2->Clear();
    m_tagV2->Link(fn, ID3TT_ID3V2);
    markTagUnchanged(Frame::Tag_2);
  }
  if (!m_tagV2) {
    m_tagV2 = new ID3_Tag;
    m_tagV2->Link(fn, ID3TT_ID3V2);
    markTagUnchanged(Frame::Tag_2);
  }

  if (force) {
    setFilename(currentFilename());
  }

  notifyModelDataChanged(priorIsTagInformationRead);
}

/**
 * Write tags to file and rename it if necessary.
 *
 * @param force   true to force writing even if file was not changed.
 * @param renamed will be set to true if the file was renamed,
 *                i.e. the file name is no longer valid, else *renamed
 *                is left unchanged
 * @param preserve true to preserve file time stamps
 *
 * @return true if ok, false if the file could not be written or renamed.
 */
bool Mp3File::writeTags(bool force, bool* renamed, bool preserve)
{
  QString fnStr(currentFilePath());
  if (isChanged() && !QFileInfo(fnStr).isWritable()) {
    revertChangedFilename();
    return false;
  }

  // store time stamp if it has to be preserved
  quint64 actime = 0, modtime = 0;
  if (preserve) {
    getFileTimeStamps(fnStr, actime, modtime);
  }

  // There seems to be a bug in id3lib: The V1 genre is not
  // removed. So we check here and strip the whole header
  // if there are no frames.
  if (m_tagV1 && (force || isTagChanged(Frame::Tag_1)) && (m_tagV1->NumFrames() == 0)) {
    m_tagV1->Strip(ID3TT_ID3V1);
    markTagUnchanged(Frame::Tag_1);
  }
  // Even after removing all frames, HasV2Tag() still returns true,
  // so we strip the whole header.
  if (m_tagV2 && (force || isTagChanged(Frame::Tag_2)) && (m_tagV2->NumFrames() == 0)) {
    m_tagV2->Strip(ID3TT_ID3V2);
    markTagUnchanged(Frame::Tag_2);
  }
  // There seems to be a bug in id3lib: If I update an ID3v1 and then
  // strip the ID3v2 the ID3v1 is removed too and vice versa, so I
  // first make any stripping and then the updating.
  if (m_tagV1 && (force || isTagChanged(Frame::Tag_1)) && (m_tagV1->NumFrames() > 0)) {
    m_tagV1->Update(ID3TT_ID3V1);
    markTagUnchanged(Frame::Tag_1);
  }
  if (m_tagV2 && (force || isTagChanged(Frame::Tag_2)) && (m_tagV2->NumFrames() > 0)) {
    m_tagV2->Update(ID3TT_ID3V2);
    markTagUnchanged(Frame::Tag_2);
  }

  // restore time stamp
  if (actime || modtime) {
    setFileTimeStamps(fnStr, actime, modtime);
  }

  if (isFilenameChanged()) {
    if (!renameFile(currentFilename(), getFilename())) {
      return false;
    }
    markFilenameUnchanged();
    // link tags to new file name
    readTags(true);
    *renamed = true;
  }
  return true;
}

/**
 * Free resources allocated when calling readTags().
 *
 * @param force true to force clearing even if the tags are modified
 */
void Mp3File::clearTags(bool force)
{
  if (isChanged() && !force)
    return;

  bool priorIsTagInformationRead = isTagInformationRead();
  if (m_tagV1) {
    delete m_tagV1;
    m_tagV1 = 0;
    markTagUnchanged(Frame::Tag_1);
  }
  if (m_tagV2) {
    delete m_tagV2;
    m_tagV2 = 0;
    markTagUnchanged(Frame::Tag_2);
  }
  notifyModelDataChanged(priorIsTagInformationRead);
}

/**
 * Fix up a unicode string from id3lib.
 *
 * @param str      unicode string
 * @param numChars number of characters in str
 *
 * @return string as QString.
 */
static QString fixUpUnicode(const unicode_t* str, size_t numChars)
{
  QString text;
  if (numChars > 0 && str && *str) {
    QChar* qcarray = new QChar[numChars];
    // Unfortunately, Unicode support in id3lib is rather buggy
    // in the current version: The codes are mirrored.
    // In the hope that my patches will be included, I try here
    // to work around these bugs.
    size_t numZeroes = 0;
    for (size_t i = 0; i < numChars; i++) {
      qcarray[i] =
        UNICODE_SUPPORT_BUGGY ?
        (ushort)(((str[i] & 0x00ff) << 8) |
                 ((str[i] & 0xff00) >> 8)) :
        (ushort)str[i];
      if (qcarray[i].isNull()) { ++numZeroes; }
    }
    // remove a single trailing zero character
    if (numZeroes == 1 && qcarray[numChars - 1].isNull()) {
      --numChars;
    }
    text = QString(qcarray, numChars);
    delete [] qcarray;
  }
  return text;
}

/**
 * Get string from text field.
 *
 * @param field field
 * @param codec text codec to use, 0 for default
 *
 * @return string,
 *         "" if the field does not exist.
 */
static QString getString(ID3_Field* field, const QTextCodec* codec = 0)
{
  QString text(QLatin1String(""));
  if (field != NULL) {
    ID3_TextEnc enc = field->GetEncoding();
    if (enc == ID3TE_UTF16 || enc == ID3TE_UTF16BE) {
      size_t numItems = field->GetNumTextItems();
      if (numItems <= 1) {
        text = fixUpUnicode(field->GetRawUnicodeText(),
                            field->Size() / sizeof(unicode_t));
      } else {
        // if there are multiple items, put them into one string
        // separated by a special separator.
        // ID3_Field::GetRawUnicodeTextItem() returns a pointer to a temporary
        // object, so I do not use it.
        text = fixUpUnicode(field->GetRawUnicodeText(),
                            field->Size() / sizeof(unicode_t));
        text.replace(QLatin1Char('\0'), Frame::stringListSeparator());
      }
    } else {
      // (ID3TE_IS_SINGLE_BYTE_ENC(enc))
      // (enc == ID3TE_ISO8859_1 || enc == ID3TE_UTF8)
      size_t numItems = field->GetNumTextItems();
      if (numItems <= 1) {
        text = codec ?
          codec->toUnicode(field->GetRawText(), field->Size()) :
          QString::fromLatin1(field->GetRawText());
      } else {
        // if there are multiple items, put them into one string
        // separated by a special separator.
        for (size_t itemNr = 0; itemNr < numItems; ++itemNr) {
          if (itemNr == 0) {
            text = QString::fromLatin1(field->GetRawTextItem(0));
          } else {
            text += Frame::stringListSeparator();
            text += QString::fromLatin1(field->GetRawTextItem(itemNr));
          }
        }
      }
    }
  }
  return text;
}

/**
 * Get text field.
 *
 * @param tag ID3 tag
 * @param id  frame ID
 * @param codec text codec to use, 0 for default
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */
static QString getTextField(const ID3_Tag* tag, ID3_FrameID id,
                            const QTextCodec* codec = 0)
{
  if (!tag) {
    return QString();
  }
  QString str(QLatin1String(""));
  ID3_Field* fld;
  ID3_Frame* frame = tag->Find(id);
  if (frame && ((fld = frame->GetField(ID3FN_TEXT)) != NULL)) {
    str = getString(fld, codec);
  }
  return str;
}

/**
 * Get year.
 *
 * @param tag ID3 tag
 * @return number,
 *         0 if the field does not exist,
 *         -1 if the tags do not exist.
 */
static int getYear(const ID3_Tag* tag)
{
  QString str = getTextField(tag, ID3FID_YEAR);
  if (str.isNull()) return -1;
  if (str.isEmpty()) return 0;
  return str.toInt();
}

/**
 * Get track.
 *
 * @param tag ID3 tag
 * @return number,
 *         0 if the field does not exist,
 *         -1 if the tags do not exist.
 */
static int getTrackNum(const ID3_Tag* tag)
{
  QString str = getTextField(tag, ID3FID_TRACKNUM);
  if (str.isNull()) return -1;
  if (str.isEmpty()) return 0;
  // handle "track/total number of tracks" format
  int slashPos = str.indexOf(QLatin1Char('/'));
  if (slashPos != -1) {
    str.truncate(slashPos);
  }
  return str.toInt();
}

/**
 * Get genre.
 *
 * @param tag ID3 tag
 * @return number,
 *         0xff if the field does not exist,
 *         -1 if the tags do not exist.
 */
static int getGenreNum(const ID3_Tag* tag)
{
  QString str = getTextField(tag, ID3FID_CONTENTTYPE);
  if (str.isNull()) return -1;
  if (str.isEmpty()) return 0xff;
  int cpPos = 0, n = 0xff;
  if ((str[0] == QLatin1Char('(')) && ((cpPos = str.indexOf(QLatin1Char(')'), 2)) > 1)) {
    bool ok;
    n = str.mid(1, cpPos - 1).toInt(&ok);
    if (!ok || n > 0xff) {
      n = 0xff;
    }
  } else {
    // ID3v2 genres can be stored as "(9)", "(9)Metal" or "Metal".
    // If the string does not start with '(', try to get the genre number
    // from a string containing a genre text.
    n = Genres::getNumber(str);
  }
  return n;
}

/**
 * Allocate a fixed up a unicode string for id3lib.
 *
 * @param text string
 *
 * @return new allocated unicode string, has to be freed with delete [].
 */
static unicode_t* newFixedUpUnicode(const QString& text)
{
  // Unfortunately, Unicode support in id3lib is rather buggy in the
  // current version: The codes are mirrored, a second different
  // BOM may be added, if the LSB >= 0x80, the MSB is set to 0xff.
  // If iconv is used (id3lib on Linux), the character do not come
  // back mirrored, but with a second (different)! BOM 0xfeff and
  // they are still written in the wrong order (big endian).
  // In the hope that my patches will be included, I try here to
  // work around these bugs, but there is no solution for the
  // LSB >= 0x80 bug.
  const QChar* qcarray = text.unicode();
  int unicode_size = text.length();
  unicode_t* unicode = new unicode_t[unicode_size + 1];
  for (int i = 0; i < unicode_size; ++i) {
    unicode[i] = (ushort)qcarray[i].unicode();
    if (UNICODE_SUPPORT_BUGGY) {
      unicode[i] = (ushort)(((unicode[i] & 0x00ff) << 8) |
                            ((unicode[i] & 0xff00) >> 8));
    }
  }
  unicode[unicode_size] = 0;
  return unicode;
}

/**
 * Set string list in text field.
 *
 * @param field field
 * @param lst   string list to set
 */
static void setStringList(ID3_Field* field, const QStringList& lst)
{
  ID3_TextEnc enc = field->GetEncoding();
  bool first = true;
  for (QStringList::const_iterator it = lst.begin(); it != lst.end(); ++it) {
    if (first) {
      if (enc == ID3TE_UTF16 || enc == ID3TE_UTF16BE) {
        unicode_t* unicode = newFixedUpUnicode(*it);
        if (unicode) {
          field->Set(unicode);
          delete [] unicode;
        }
      } else if (enc == ID3TE_UTF8) {
        field->Set((*it).toUtf8().data());
      } else {
        // enc == ID3TE_ISO8859_1
        field->Set((*it).toLatin1().data());
      }
      first = false;
    } else {
      // This will not work with buggy id3lib. A BOM 0xfffe is written before
      // the first string, but not before the subsequent strings. Prepending a
      // BOM or changing the byte order does not help when id3lib rewrites
      // this field when another frame is changed. So you cannot use
      // string lists with Unicode encoding.
      if (enc == ID3TE_UTF16 || enc == ID3TE_UTF16BE) {
        unicode_t* unicode = newFixedUpUnicode(*it);
        if (unicode) {
          field->Add(unicode);
          delete [] unicode;
        }
      } else if (enc == ID3TE_UTF8) {
        field->Add((*it).toUtf8().data());
      } else {
        // enc == ID3TE_ISO8859_1
        field->Add((*it).toLatin1().data());
      }
    }
  }
}

/**
 * Set string in text field.
 *
 * @param field        field
 * @param text         text to set
 * @param codec        text codec to use, 0 for default
 */
static void setString(ID3_Field* field, const QString& text,
                      const QTextCodec* codec = 0)
{
  if (text.indexOf(Frame::stringListSeparator()) == -1) {
    ID3_TextEnc enc = field->GetEncoding();
    // (ID3TE_IS_DOUBLE_BYTE_ENC(enc))
    if (enc == ID3TE_UTF16 || enc == ID3TE_UTF16BE) {
      unicode_t* unicode = newFixedUpUnicode(text);
      if (unicode) {
        field->Set(unicode);
        delete [] unicode;
      }
    } else if (enc == ID3TE_UTF8) {
      field->Set(text.toUtf8().data());
    } else {
      // enc == ID3TE_ISO8859_1
      field->Set(codec ? codec->fromUnicode(text).data() : text.toLatin1().data());
    }
  } else {
    setStringList(field, text.split(Frame::stringListSeparator()));
  }
}

/**
 * Set text field.
 *
 * @param tag          ID3 tag
 * @param id           frame ID
 * @param text         text to set
 * @param allowUnicode true to allow setting of Unicode encoding if necessary
 * @param replace      true to replace an existing field
 * @param removeEmpty  true to remove a field if text is empty
 * @param codec        text codec to use, 0 for default
 *
 * @return true if the field was changed.
 */
static bool setTextField(ID3_Tag* tag, ID3_FrameID id, const QString& text,
                         bool allowUnicode = false, bool replace = true,
                         bool removeEmpty = true, const QTextCodec* codec = 0)
{
  bool changed = false;
  if (tag && !text.isNull()) {
    ID3_Frame* frame = NULL;
    bool removeOnly = removeEmpty && text.isEmpty();
    if (replace || removeOnly) {
      if (id == ID3FID_COMMENT && tag->HasV2Tag()) {
        frame = tag->Find(ID3FID_COMMENT, ID3FN_DESCRIPTION, "");
      } else {
        frame = tag->Find(id);
      }
      if (frame) {
        frame = tag->RemoveFrame(frame);
        delete frame;
        changed = true;
      }
    }
    if (!removeOnly && (replace || tag->Find(id) == NULL)) {
      frame = new ID3_Frame(id);
      if (frame) {
        ID3_Field* fld = frame->GetField(ID3FN_TEXT);
        if (fld) {
          ID3_TextEnc enc = tag->HasV2Tag() ?
            getDefaultTextEncoding() : ID3TE_ISO8859_1;
          if (allowUnicode && enc == ID3TE_ISO8859_1) {
            // check if information is lost if the string is not unicode
            int i, unicode_size = text.length();
            const QChar* qcarray = text.unicode();
            for (i = 0; i < unicode_size; ++i) {
              char ch = qcarray[i].toLatin1();
              if (ch == 0 || (ch & 0x80) != 0) {
                enc = ID3TE_UTF16;
                break;
              }
            }
          }
          ID3_Field* encfld = frame->GetField(ID3FN_TEXTENC);
          if (encfld) {
            encfld->Set(enc);
          }
          fld->SetEncoding(enc);
          setString(fld, text, codec);
          tag->AttachFrame(frame);
        }
      }
      changed = true;
    }
  }
  return changed;
}

/**
 * Set year.
 *
 * @param tag ID3 tag
 * @param num number to set, 0 to remove field.
 *
 * @return true if the field was changed.
 */
static bool setYear(ID3_Tag* tag, int num)
{
  bool changed = false;
  if (num >= 0) {
    QString str;
    if (num != 0) {
      str.setNum(num);
    } else {
      str.clear();
    }
    changed = getTextField(tag, ID3FID_YEAR) != str &&
              setTextField(tag, ID3FID_YEAR, str);
  }
  return changed;
}

/**
 * Set track.
 *
 * @param tag ID3 tag
 * @param num number to set, 0 to remove field.
 * @param numTracks total number of tracks, <=0 to ignore
 *
 * @return true if the field was changed.
 */
bool Mp3File::setTrackNum(ID3_Tag* tag, int num, int numTracks) const
{
  bool changed = false;
  if (num >= 0 && getTrackNum(tag) != num) {
    QString str = trackNumberString(num, numTracks);
    changed = getTextField(tag, ID3FID_TRACKNUM) != str &&
              setTextField(tag, ID3FID_TRACKNUM, str);
  }
  return changed;
}

/**
 * Set genre.
 *
 * @param tag ID3 tag
 * @param num number to set, 0xff to remove field.
 *
 * @return true if the field was changed.
 */
static bool setGenreNum(ID3_Tag* tag, int num)
{
  bool changed = false;
  if (num >= 0) {
    QString str;
    if (num != 0xff) {
      str = QString(QLatin1String("(%1)")).arg(num);
    } else {
      str = QLatin1String("");
    }
    changed = getTextField(tag, ID3FID_CONTENTTYPE) != str &&
              setTextField(tag, ID3FID_CONTENTTYPE, str);
  }
  return changed;
}

/**
 * Check if tag information has already been read.
 *
 * @return true if information is available,
 *         false if the tags have not been read yet, in which case
 *         hasTag() does not return meaningful information.
 */
bool Mp3File::isTagInformationRead() const
{
  return m_tagV1 || m_tagV2;
}

/**
 * Check if tags are supported by the format of this file.
 *
 * @param tagNr tag number
 * @return true.
 */
bool Mp3File::isTagSupported(Frame::TagNumber tagNr) const
{
  return tagNr == Frame::Tag_1 || tagNr == Frame::Tag_2;
}

/**
 * Check if file has a tag.
 *
 * @param tagNr tag number
 * @return true if a tag is available.
 * @see isTagInformationRead()
 */
bool Mp3File::hasTag(Frame::TagNumber tagNr) const
{
  return (tagNr == Frame::Tag_1 && m_tagV1 && m_tagV1->HasV1Tag()) ||
         (tagNr == Frame::Tag_2 && m_tagV2 && m_tagV2->HasV2Tag());
}

/**
 * Get technical detail information.
 *
 * @param info the detail information is returned here
 */
void Mp3File::getDetailInfo(DetailInfo& info) const
{
  if (getFilename().right(4).toLower() == QLatin1String(".aac")) {
    info.valid = true;
    info.format = QLatin1String("AAC");
    return;
  }

  const Mp3_Headerinfo* headerInfo = 0;
  if (m_tagV2) {
    headerInfo = m_tagV2->GetMp3HeaderInfo();
  }
  if (!headerInfo && m_tagV1) {
    headerInfo = m_tagV1->GetMp3HeaderInfo();
  }
  if (headerInfo) {
    info.valid = true;
    switch (headerInfo->version) {
      case MPEGVERSION_1:
        info.format = QLatin1String("MPEG 1 ");
        break;
      case MPEGVERSION_2:
        info.format = QLatin1String("MPEG 2 ");
        break;
      case MPEGVERSION_2_5:
        info.format = QLatin1String("MPEG 2.5 ");
        break;
      default:
        ; // nothing
    }
    switch (headerInfo->layer) {
      case MPEGLAYER_I:
        info.format += QLatin1String("Layer 1");
        break;
      case MPEGLAYER_II:
        info.format += QLatin1String("Layer 2");
        break;
      case MPEGLAYER_III:
        info.format += QLatin1String("Layer 3");
        break;
      default:
        ; // nothing
    }
    info.bitrate = headerInfo->bitrate / 1000;
#ifndef HAVE_NO_ID3LIB_VBR
    if (headerInfo->vbr_bitrate > 1000) {
      info.vbr = true;
      info.bitrate = headerInfo->vbr_bitrate / 1000;
    }
#endif
    info.sampleRate = headerInfo->frequency;
    switch (headerInfo->channelmode) {
      case MP3CHANNELMODE_STEREO:
        info.channelMode = DetailInfo::CM_Stereo;
        info.channels = 2;
        break;
      case MP3CHANNELMODE_JOINT_STEREO:
        info.channelMode = DetailInfo::CM_JointStereo;
        info.channels = 2;
        break;
      case MP3CHANNELMODE_DUAL_CHANNEL:
        info.channels = 2;
        break;
      case MP3CHANNELMODE_SINGLE_CHANNEL:
        info.channels = 1;
        break;
      default:
        ; // nothing
    }
    info.duration = headerInfo->time;
  } else {
    info.valid = false;
  }
}

/**
 * Get duration of file.
 *
 * @return duration in seconds,
 *         0 if unknown.
 */
unsigned Mp3File::getDuration() const
{
  unsigned duration = 0;
  const Mp3_Headerinfo* info = NULL;
  if (m_tagV2) {
    info = m_tagV2->GetMp3HeaderInfo();
  }
  if (!info && m_tagV1) {
    info = m_tagV1->GetMp3HeaderInfo();
  }
  if (info && info->time > 0) {
    duration = info->time;
  }
  return duration;
}

/**
 * Get file extension including the dot.
 *
 * @return file extension ".mp3".
 */
QString Mp3File::getFileExtension() const
{
  QString ext(getFilename().right(4).toLower());
  if (ext == QLatin1String(".aac") || ext == QLatin1String(".mp2"))
    return ext;
  else
    return QLatin1String(".mp3");
}

/**
 * Get the format of a tag.
 *
 * @param tagNr tag number
 * @return string describing format of tag,
 *         e.g. "ID3v1.1".
 */
QString Mp3File::getTagFormat(Frame::TagNumber tagNr) const
{
  if (tagNr == Frame::Tag_1 && m_tagV1 && m_tagV1->HasV1Tag()) {
    return QLatin1String("ID3v1.1");
  } else if (tagNr == Frame::Tag_2 && m_tagV2 && m_tagV2->HasV2Tag()) {
    switch (m_tagV2->GetSpec()) {
      case ID3V2_3_0:
        return QLatin1String("ID3v2.3.0");
      case ID3V2_4_0:
        return QLatin1String("ID3v2.4.0");
      case ID3V2_2_0:
        return QLatin1String("ID3v2.2.0");
      case ID3V2_2_1:
        return QLatin1String("ID3v2.2.1");
      default:
        break;
    }
  }
  return QString();
}

/** Types and descriptions for id3lib frame IDs */
static const struct TypeStrOfId {
  Frame::Type type;
  const char* str;
} typeStrOfId[] = {
  { Frame::FT_UnknownFrame,   0 },                                                                                       /* ???? */
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "AENC - Audio encryption") },                                /* AENC */
  { Frame::FT_Picture,        QT_TRANSLATE_NOOP("@default", "APIC - Attached picture") },                                /* APIC */
  { Frame::FT_Other,          0 },                                                                                       /* ASPI */
  { Frame::FT_Comment,        QT_TRANSLATE_NOOP("@default", "COMM - Comments") },                                        /* COMM */
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "COMR - Commercial") },                                      /* COMR */
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "ENCR - Encryption method registration") },                  /* ENCR */
  { Frame::FT_Other,          0 },                                                                                       /* EQU2 */
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "EQUA - Equalization") },                                    /* EQUA */
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "ETCO - Event timing codes") },                              /* ETCO */
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "GEOB - General encapsulated object") },                     /* GEOB */
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "GRID - Group identification registration") },               /* GRID */
  { Frame::FT_Arranger,       QT_TRANSLATE_NOOP("@default", "IPLS - Involved people list") },                            /* IPLS */
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "LINK - Linked information") },                              /* LINK */
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "MCDI - Music CD identifier") },                             /* MCDI */
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "MLLT - MPEG location lookup table") },                      /* MLLT */
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "OWNE - Ownership frame") },                                 /* OWNE */
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "PRIV - Private frame") },                                   /* PRIV */
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "PCNT - Play counter") },                                    /* PCNT */
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "POPM - Popularimeter") },                                   /* POPM */
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "POSS - Position synchronisation frame") },                  /* POSS */
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "RBUF - Recommended buffer size") },                         /* RBUF */
  { Frame::FT_Other,          0 },                                                                                       /* RVA2 */
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "RVAD - Relative volume adjustment") },                      /* RVAD */
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "RVRB - Reverb") },                                          /* RVRB */
  { Frame::FT_Other,          0 },                                                                                       /* SEEK */
  { Frame::FT_Other,          0 },                                                                                       /* SIGN */
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "SYLT - Synchronized lyric/text") },                         /* SYLT */
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "SYTC - Synchronized tempo codes") },                        /* SYTC */
  { Frame::FT_Album,          QT_TRANSLATE_NOOP("@default", "TALB - Album/Movie/Show title") },                          /* TALB */
  { Frame::FT_Bpm,            QT_TRANSLATE_NOOP("@default", "TBPM - BPM (beats per minute)") },                          /* TBPM */
  { Frame::FT_Composer,       QT_TRANSLATE_NOOP("@default", "TCOM - Composer") },                                        /* TCOM */
  { Frame::FT_Genre,          QT_TRANSLATE_NOOP("@default", "TCON - Content type") },                                    /* TCON */
  { Frame::FT_Copyright,      QT_TRANSLATE_NOOP("@default", "TCOP - Copyright message") },                               /* TCOP */
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "TDAT - Date") },                                            /* TDAT */
  { Frame::FT_Other,          0 },                                                                                       /* TDEN */
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "TDLY - Playlist delay") },                                  /* TDLY */
  { Frame::FT_Other,          0 },                                                                                       /* TDOR */
  { Frame::FT_Other,          0 },                                                                                       /* TDRC */
  { Frame::FT_Other,          0 },                                                                                       /* TDRL */
  { Frame::FT_Other,          0 },                                                                                       /* TDTG */
  { Frame::FT_Other,          0 },                                                                                       /* TIPL */
  { Frame::FT_EncodedBy,      QT_TRANSLATE_NOOP("@default", "TENC - Encoded by") },                                      /* TENC */
  { Frame::FT_Lyricist,       QT_TRANSLATE_NOOP("@default", "TEXT - Lyricist/Text writer") },                            /* TEXT */
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "TFLT - File type") },                                       /* TFLT */
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "TIME - Time") },                                            /* TIME */
  { Frame::FT_Grouping,       QT_TRANSLATE_NOOP("@default", "TIT1 - Content group description") },                       /* TIT1 */
  { Frame::FT_Title,          QT_TRANSLATE_NOOP("@default", "TIT2 - Title/songname/content description") },              /* TIT2 */
  { Frame::FT_Subtitle,       QT_TRANSLATE_NOOP("@default", "TIT3 - Subtitle/Description refinement") },                 /* TIT3 */
  { Frame::FT_InitialKey,     QT_TRANSLATE_NOOP("@default", "TKEY - Initial key") },                                     /* TKEY */
  { Frame::FT_Language,       QT_TRANSLATE_NOOP("@default", "TLAN - Language(s)") },                                     /* TLAN */
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "TLEN - Length") },                                          /* TLEN */
  { Frame::FT_Other,          0 },                                                                                       /* TMCL */
  { Frame::FT_Media,          QT_TRANSLATE_NOOP("@default", "TMED - Media type") },                                      /* TMED */
  { Frame::FT_Other,          0 },                                                                                       /* TMOO */
  { Frame::FT_OriginalAlbum,  QT_TRANSLATE_NOOP("@default", "TOAL - Original album/movie/show title") },                 /* TOAL */
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "TOFN - Original filename") },                               /* TOFN */
  { Frame::FT_Author,         QT_TRANSLATE_NOOP("@default", "TOLY - Original lyricist(s)/text writer(s)") },             /* TOLY */
  { Frame::FT_OriginalArtist, QT_TRANSLATE_NOOP("@default", "TOPE - Original artist(s)/performer(s)") },                 /* TOPE */
  { Frame::FT_OriginalDate,   QT_TRANSLATE_NOOP("@default", "TORY - Original release year") },                           /* TORY */
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "TOWN - File owner/licensee") },                             /* TOWN */
  { Frame::FT_Artist,         QT_TRANSLATE_NOOP("@default", "TPE1 - Lead performer(s)/Soloist(s)") },                    /* TPE1 */
  { Frame::FT_AlbumArtist,    QT_TRANSLATE_NOOP("@default", "TPE2 - Band/orchestra/accompaniment") },                    /* TPE2 */
  { Frame::FT_Conductor,      QT_TRANSLATE_NOOP("@default", "TPE3 - Conductor/performer refinement") },                  /* TPE3 */
  { Frame::FT_Remixer,        QT_TRANSLATE_NOOP("@default", "TPE4 - Interpreted, remixed, or otherwise modified by") },  /* TPE4 */
  { Frame::FT_Disc,           QT_TRANSLATE_NOOP("@default", "TPOS - Part of a set") },                                   /* TPOS */
  { Frame::FT_Other,          0 },                                                                                       /* TPRO */
  { Frame::FT_Publisher,      QT_TRANSLATE_NOOP("@default", "TPUB - Publisher") },                                       /* TPUB */
  { Frame::FT_Track,          QT_TRANSLATE_NOOP("@default", "TRCK - Track number/Position in set") },                    /* TRCK */
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "TRDA - Recording dates") },                                 /* TRDA */
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "TRSN - Internet radio station name") },                     /* TRSN */
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "TRSO - Internet radio station owner") },                    /* TRSO */
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "TSIZ - Size") },                                            /* TSIZ */
  { Frame::FT_Other,          0 },                                                                                       /* TSOA */
  { Frame::FT_Other,          0 },                                                                                       /* TSOP */
  { Frame::FT_Other,          0 },                                                                                       /* TSOT */
  { Frame::FT_Isrc,           QT_TRANSLATE_NOOP("@default", "TSRC - ISRC (international standard recording code)") },    /* TSRC */
  { Frame::FT_EncoderSettings,QT_TRANSLATE_NOOP("@default", "TSSE - Software/Hardware and settings used for encoding") },/* TSSE */
  { Frame::FT_Part,           0 },                                                                                       /* TSST */
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "TXXX - User defined text information") },                   /* TXXX */
  { Frame::FT_Date,           QT_TRANSLATE_NOOP("@default", "TYER - Year") },                                            /* TYER */
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "UFID - Unique file identifier") },                          /* UFID */
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "USER - Terms of use") },                                    /* USER */
  { Frame::FT_Lyrics,         QT_TRANSLATE_NOOP("@default", "USLT - Unsynchronized lyric/text transcription") },         /* USLT */
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "WCOM - Commercial information") },                          /* WCOM */
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "WCOP - Copyright/Legal information") },                     /* WCOP */
  { Frame::FT_WWWAudioFile,   QT_TRANSLATE_NOOP("@default", "WOAF - Official audio file webpage") },                     /* WOAF */
  { Frame::FT_Website,        QT_TRANSLATE_NOOP("@default", "WOAR - Official artist/performer webpage") },               /* WOAR */
  { Frame::FT_WWWAudioSource, QT_TRANSLATE_NOOP("@default", "WOAS - Official audio source webpage") },                   /* WOAS */
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "WORS - Official internet radio station homepage") },        /* WORS */
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "WPAY - Payment") },                                         /* WPAY */
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "WPUB - Official publisher webpage") },                      /* WPUB */
  { Frame::FT_Other,          QT_TRANSLATE_NOOP("@default", "WXXX - User defined URL link") }                            /* WXXX */
};

/** Not instantiated struct to check array size at compilation time. */
struct not_used { int array_size_check[
    sizeof(typeStrOfId) / sizeof(typeStrOfId[0]) == ID3FID_WWWUSER + 1
    ? 1 : -1 ]; /**< not used */ };

/**
 * Get type and description of frame.
 *
 * @param id ID of frame
 * @param type the type is returned here
 * @param str  the description is returned here, 0 if not available
 */
static void getTypeStringForId3libFrameId(ID3_FrameID id, Frame::Type& type,
                                          const char*& str)
{
  const TypeStrOfId& ts = typeStrOfId[id <= ID3FID_WWWUSER ? id : 0];
  type = ts.type;
  str = ts.str;
}

/**
 * Get id3lib frame ID for frame type.
 *
 * @param type frame type
 *
 * @return id3lib frame ID or ID3FID_NOFRAME if type not found.
 */
static ID3_FrameID getId3libFrameIdForType(Frame::Type type)
{
  // IPLS is mapped to FT_Arranger and FT_Performer
  if (type == Frame::FT_Performer) {
    return ID3FID_INVOLVEDPEOPLE;
  } else if (type == Frame::FT_CatalogNumber ||
             type == Frame::FT_ReleaseCountry) {
    return ID3FID_USERTEXT;
  }

  static int typeIdMap[Frame::FT_LastFrame + 1] = { -1, };
  if (typeIdMap[0] == -1) {
    // first time initialization
    for (int i = 0; i <= ID3FID_WWWUSER; ++i) {
      int t = typeStrOfId[i].type;
      if (t <= Frame::FT_LastFrame) {
        typeIdMap[t] = i;
      }
    }
  }
  return type <= Frame::FT_LastFrame ?
    static_cast<ID3_FrameID>(typeIdMap[type]) : ID3FID_NOFRAME;
}

/**
 * Get id3lib frame ID for frame name.
 *
 * @param name frame name
 *
 * @return id3lib frame ID or ID3FID_NOFRAME if name not found.
 */
static ID3_FrameID getId3libFrameIdForName(const QString& name)
{
  if (name.length() >= 4) {
    QByteArray nameBytes = name.toLatin1();
    const char* nameStr = nameBytes.constData();
    for (int i = 0; i <= ID3FID_WWWUSER; ++i) {
      const char* s = typeStrOfId[i].str;
      if (s && ::strncmp(s, nameStr, 4) == 0) {
        return static_cast<ID3_FrameID>(i);
      }
    }
  }
  return ID3FID_NOFRAME;
}

/**
 * Convert the binary field of a SYLT frame to a list of alternating
 * time stamps and strings.
 * @param bytes binary field
 * @param enc encoding
 * @return list containing time, text, time, text, ...
 */
static QVariantList syltBytesToList(const QByteArray& bytes, ID3_TextEnc enc)
{
  QVariantList timeEvents;
  const int numBytes = bytes.size();
  int textBegin = 0, textEnd;
  while (textBegin < numBytes) {
    if (enc == ID3TE_ISO8859_1 || enc == ID3TE_UTF8) {
      textEnd = bytes.indexOf('\0', textBegin);
      if (textEnd != -1) {
        ++textEnd;
      }
    } else {
      const ushort* unicode =
          reinterpret_cast<const ushort*>(bytes.constData() + textBegin);
      textEnd = textBegin;
      while (textEnd < numBytes) {
        textEnd += 2;
        if (*unicode++ == 0) {
          break;
        }
      }
    }
    if (textEnd < 0 || textEnd >= numBytes)
      break;

    QString str;
    QByteArray text = bytes.mid(textBegin, textEnd - textBegin);
    switch (enc) {
    case ID3TE_UTF16BE:
      text.prepend(0xff);
      text.prepend(0xfe);
      // starting with FEFF BOM
      // fallthrough
    case ID3TE_UTF16:
      str = QString::fromUtf16(reinterpret_cast<const ushort*>(text.constData()));
      break;
    case ID3TE_UTF8:
      str = QString::fromUtf8(text.constData());
      break;
    case ID3TE_ISO8859_1:
    default:
      str = QString::fromLatin1(text.constData());
    }
    textBegin = textEnd + 4;
    if (textBegin > numBytes)
      break;

    quint32 time = qFromBigEndian<quint32>(
          reinterpret_cast<const uchar*>(bytes.constData()) + textEnd);
    timeEvents.append(time);
    timeEvents.append(str);
  }
  return timeEvents;
}

/**
 * Convert a list of alternating time stamps and strings to the binary field of
 * a SYLT frame.
 * @param synchedData list containing time, text, time, text, ...
 * @return binary field bytes.
 */
static QByteArray syltListToBytes(const QVariantList& synchedData,
                                  ID3_TextEnc enc)
{
  QByteArray bytes;
  QListIterator<QVariant> it(synchedData);
  while (it.hasNext()) {
    quint32 milliseconds = it.next().toUInt();
    if (!it.hasNext())
      break;

    QString str = it.next().toString();
    switch (enc) {
    case ID3TE_UTF16:
      bytes.append(0xff);
      bytes.append(0xfe);
      // starting with FFFE BOM
      // fallthrough
    case ID3TE_UTF16BE:
    {
      const ushort* unicode = str.utf16();
      do {
        uchar lsb = *unicode & 0xff;
        uchar msb = *unicode >> 8;
        if (enc == ID3TE_UTF16) {
          bytes.append(static_cast<char>(lsb));
          bytes.append(static_cast<char>(msb));
        } else {
          bytes.append(static_cast<char>(msb));
          bytes.append(static_cast<char>(lsb));
        }
      } while (*unicode++);
      break;
    }
    case ID3TE_UTF8:
      bytes.append(str.toUtf8());
      bytes.append('\0');
      break;
    case ID3TE_ISO8859_1:
    default:
      bytes.append(str.toLatin1());
      bytes.append('\0');
    }
    uchar timeStamp[4];
    qToBigEndian(milliseconds, timeStamp);
    bytes.append(reinterpret_cast<const char*>(timeStamp), sizeof(timeStamp));
  }
  if (bytes.isEmpty()) {
    // id3lib bug: Empty binary fields are not written, so add a minimal field
    bytes = QByteArray(4 + (enc == ID3TE_UTF16 ||
                            enc == ID3TE_UTF16BE ? 2 : 1),
                       '\0');
  }
  return bytes;
}

/**
 * Convert the binary field of an ETCO frame to a list of alternating
 * time stamps and codes.
 * @param bytes binary field
 * @return list containing time, code, time, code, ...
 */
static QVariantList etcoBytesToList(const QByteArray& bytes)
{
  QVariantList timeEvents;
  const int numBytes = bytes.size();
  // id3lib bug: There is only a single data field for ETCO frames,
  // but it should be preceded by an ID_TimestampFormat field.
  // Start with the second byte.
  int pos = 1;
  while (pos < numBytes) {
    int code = static_cast<uchar>(bytes.at(pos));
    ++pos;
    if (pos + 4 > numBytes)
      break;

    quint32 time = qFromBigEndian<quint32>(
          reinterpret_cast<const uchar*>(bytes.constData()) + pos);
    pos += 4;
    timeEvents.append(time);
    timeEvents.append(code);
  }
  return timeEvents;
}

/**
 * Convert a list of alternating time stamps and codes to the binary field of
 * an ETCO frame.
 * @param synchedData list containing time, code, time, code, ...
 * @return binary field bytes.
 */
static QByteArray etcoListToBytes(const QVariantList& synchedData)
{
  QByteArray bytes;
  QListIterator<QVariant> it(synchedData);
  while (it.hasNext()) {
    quint32 milliseconds = it.next().toUInt();
    if (!it.hasNext())
      break;

    int code = it.next().toInt();
    bytes.append(static_cast<char>(code));
    uchar timeStamp[4];
    qToBigEndian(milliseconds, timeStamp);
    bytes.append(reinterpret_cast<const char*>(timeStamp), sizeof(timeStamp));
  }
  return bytes;
}

/**
 * Get the fields from an ID3v2 tag.
 *
 * @param id3Frame frame
 * @param fields   the fields are appended to this list
 *
 * @return text representation of fields (Text or URL).
 */
static QString getFieldsFromId3Frame(ID3_Frame* id3Frame,
                                     Frame::FieldList& fields)
{
  QString text;
  ID3_Frame::Iterator* iter = id3Frame->CreateIterator();
  ID3_FrameID id3Id = id3Frame->GetID();
  ID3_TextEnc enc = ID3TE_NONE;
  ID3_Field* id3Field;
  Frame::Field field;
  while ((id3Field = iter->GetNext()) != 0) {
    ID3_FieldID id = id3Field->GetID();
    ID3_FieldType type = id3Field->GetType();
    field.m_id = id;
    if (type == ID3FTY_INTEGER) {
      uint32 intVal = id3Field->Get();
      field.m_value = intVal;
      if (id == ID3FN_TEXTENC) {
        enc = static_cast<ID3_TextEnc>(intVal);
      }
    }
    else if (type == ID3FTY_BINARY) {
      QByteArray ba(reinterpret_cast<const char*>(id3Field->GetRawBinary()),
                    static_cast<int>(id3Field->Size()));
      if (id3Id == ID3FID_SYNCEDLYRICS) {
        field.m_value = syltBytesToList(ba, enc);
      } else if (id3Id == ID3FID_EVENTTIMING) {
        field.m_value = etcoBytesToList(ba);
      } else {
        field.m_value = ba;
      }
    }
    else if (type == ID3FTY_TEXTSTRING) {
      if (id == ID3FN_TEXT || id == ID3FN_DESCRIPTION || id == ID3FN_URL) {
        text = getString(id3Field);
        if (id3Id == ID3FID_CONTENTTYPE) {
          text = Genres::getNameString(text);
        }
        field.m_value = text;
      } else {
        field.m_value = getString(id3Field);
      }
    } else {
      field.m_value.clear();
    }
    fields.push_back(field);
  }
#ifdef Q_OS_WIN32
  /* allocated in Windows DLL => must be freed in the same DLL */
  ID3TagIterator_Delete(reinterpret_cast<ID3TagIterator*>(iter));
#else
#ifdef GCC_HAS_DIAGNOSTIC_PRAGMA
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdelete-non-virtual-dtor"
/* Is safe because iterator implementation has default destructor */
#endif
  delete iter;
#ifdef GCC_HAS_DIAGNOSTIC_PRAGMA
#pragma GCC diagnostic pop
#endif
#endif
  return text;
}

/**
 * Get ID3v2 frame by index.
 *
 * @param tag   ID3v2 tag
 * @param index index
 *
 * @return frame, 0 if index invalid.
 */
static ID3_Frame* getId3v2Frame(ID3_Tag* tag, int index)
{
  ID3_Frame* result = 0;
  if (tag) {
    ID3_Frame* frame;
    ID3_Tag::Iterator* iter = tag->CreateIterator();
    int i = 0;
    while ((frame = iter->GetNext()) != 0) {
      if (i == index) {
        result = frame;
        break;
      }
      ++i;
    }
#ifdef Q_OS_WIN32
    /* allocated in Windows DLL => must be freed in the same DLL */
    ID3TagIterator_Delete(reinterpret_cast<ID3TagIterator*>(iter));
#else
#ifdef GCC_HAS_DIAGNOSTIC_PRAGMA
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdelete-non-virtual-dtor"
/* Is safe because iterator implementation has default destructor */
#endif
    delete iter;
#ifdef GCC_HAS_DIAGNOSTIC_PRAGMA
#pragma GCC diagnostic pop
#endif
#endif
  }
  return result;
}

/**
 * Set the fields in an id3lib frame from the field in the frame.
 *
 * @param id3Frame id3lib frame
 * @param frame    frame with fields
 */
void Mp3File::setId3v2Frame(ID3_Frame* id3Frame, const Frame& frame) const
{
  ID3_Frame::Iterator* iter = id3Frame->CreateIterator();
  ID3_FrameID id3Id = id3Frame->GetID();
  ID3_TextEnc enc = ID3TE_NONE;
  for (Frame::FieldList::const_iterator fldIt = frame.getFieldList().begin();
       fldIt != frame.getFieldList().end();
       ++fldIt) {
    ID3_Field* id3Field = iter->GetNext();
    if (!id3Field) {
      qDebug("early end of ID3 fields");
      break;
    }
    const Frame::Field& fld = *fldIt;
    switch (fld.m_value.type()) {
      case QVariant::Int:
      case QVariant::UInt:
      {
        int intVal = fld.m_value.toInt();
        if (fld.m_id == ID3FN_TEXTENC) {
          if (intVal == ID3TE_UTF8) intVal = ID3TE_UTF16;
          enc = static_cast<ID3_TextEnc>(intVal);
        }
        id3Field->Set(intVal);
        break;
      }

      case QVariant::String:
      {
        if (enc != ID3TE_NONE) {
          id3Field->SetEncoding(enc);
        }
        QString value(fld.m_value.toString());
        if (id3Id == ID3FID_CONTENTTYPE) {
          if (!TagConfig::instance().genreNotNumeric()) {
            value = Genres::getNumberString(value, true);
          }
        } else if (id3Id == ID3FID_TRACKNUM) {
          formatTrackNumberIfEnabled(value, true);
        }
        setString(id3Field, value);
        break;
      }

      case QVariant::ByteArray:
      {
        const QByteArray& ba = fld.m_value.toByteArray();
        id3Field->Set(reinterpret_cast<const unsigned char*>(ba.data()),
                      static_cast<size_t>(ba.size()));
        break;
      }

      case QVariant::List:
      {
        if (id3Id == ID3FID_SYNCEDLYRICS) {
          QByteArray ba = syltListToBytes(fld.m_value.toList(), enc);
          id3Field->Set(reinterpret_cast<const unsigned char*>(ba.data()),
                        static_cast<size_t>(ba.size()));
        } else if (id3Id == ID3FID_EVENTTIMING) {
          QByteArray ba = etcoListToBytes(fld.m_value.toList());
          // id3lib bug: There is only a single data field for ETCO frames,
          // but it should be preceded by an ID_TimestampFormat field.
          ba.prepend(2);
          id3Field->Set(reinterpret_cast<const unsigned char*>(ba.data()),
                        static_cast<size_t>(ba.size()));
        } else {
          qDebug("Unexpected QVariantList in field %d", fld.m_id);
        }
        break;
      }

      default:
        qDebug("Unknown type %d in field %d", fld.m_value.type(), fld.m_id);
    }
  }
#ifdef Q_OS_WIN32
  /* allocated in Windows DLL => must be freed in the same DLL */
  ID3TagIterator_Delete(reinterpret_cast<ID3TagIterator*>(iter));
#else
#ifdef GCC_HAS_DIAGNOSTIC_PRAGMA
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdelete-non-virtual-dtor"
/* Is safe because iterator implementation has default destructor */
#endif
  delete iter;
#ifdef GCC_HAS_DIAGNOSTIC_PRAGMA
#pragma GCC diagnostic pop
#endif
#endif
}

/**
 * Get a specific frame from the tags.
 *
 * @param tagNr tag number
 * @param type  frame type
 * @param frame the frame is returned here
 *
 * @return true if ok.
 */
bool Mp3File::getFrame(Frame::TagNumber tagNr, Frame::Type type, Frame& frame) const
{
  if (type < Frame::FT_FirstFrame || type > Frame::FT_LastV1Frame)
    return false;

  ID3_FrameID frameId = getId3libFrameIdForType(type);
  if (frameId == ID3FID_NOFRAME)
    return false;

  const ID3_Tag* tag;
  const QTextCodec* codec;
  if (tagNr == Frame::Tag_1) {
    tag = m_tagV1;
    codec = s_textCodecV1;
  } else if (tagNr == Frame::Tag_2) {
    tag = m_tagV2;
    codec = 0;
  } else {
    return false;
  }

  switch (type) {
  case Frame::FT_Album:
  case Frame::FT_Artist:
  case Frame::FT_Comment:
  case Frame::FT_Title:
    frame.setValue(getTextField(tag, frameId, codec));
    break;
  case Frame::FT_Track:
    if (tagNr == Frame::Tag_1) {
      frame.setValueAsNumber(getTrackNum(tag));
    } else {
      frame.setValue(getTextField(tag, frameId));
    }
    break;
  case Frame::FT_Date:
    frame.setValueAsNumber(getYear(tag));
    break;
  case Frame::FT_Genre:
  {
    int num = getGenreNum(tag);
    if (tagNr == Frame::Tag_1) {
      frame.setValue(num == -1
                     ? QString()
                     : num == 0xff
                       ? QLatin1String("")
                       : QString::fromLatin1(Genres::getName(num)));
    } else {
      frame.setValue(num != 0xff && num != -1
          ? QString::fromLatin1(Genres::getName(num))
          : getTextField(tag, frameId));
    }
    break;
  }
  default:
    return false;
  }
  frame.setType(type);
  return true;
}

/**
 * Set a frame in the tags.
 *
 * @param tagNr tag number
 * @param frame frame to set
 *
 * @return true if ok.
 */
bool Mp3File::setFrame(Frame::TagNumber tagNr, const Frame& frame)
{
  if (tagNr == Frame::Tag_2) {
    // If the frame has an index, change that specific frame
    int index = frame.getIndex();
    if (index != -1 && m_tagV2) {
      ID3_Frame* id3Frame = getId3v2Frame(m_tagV2, index);
      if (id3Frame) {
        // If value is changed or field list is empty,
        // set from value, else from FieldList.
        if (frame.isValueChanged() || frame.getFieldList().empty()) {
          QString value(frame.getValue());
          ID3_Field* fld;
          if ((fld = id3Frame->GetField(ID3FN_URL)) != 0) {
            if (getString(fld) != value) {
              fld->Set(value.toLatin1().data());
              markTagChanged(Frame::Tag_2, frame.getType());
            }
            return true;
          } else if ((fld = id3Frame->GetField(ID3FN_TEXT)) != 0 ||
              (fld = id3Frame->GetField(ID3FN_DESCRIPTION)) != 0) {
            ID3_FrameID id = id3Frame->GetID();
            if (id == ID3FID_CONTENTTYPE) {
              if (!TagConfig::instance().genreNotNumeric()) {
                value = Genres::getNumberString(value, true);
              }
            } else if (id == ID3FID_TRACKNUM) {
              formatTrackNumberIfEnabled(value, true);
            }
            bool hasEnc;
            const ID3_TextEnc enc = fld->GetEncoding();
            ID3_TextEnc newEnc = static_cast<ID3_TextEnc>(
                  frame.getFieldValue(Frame::ID_TextEnc).toInt(&hasEnc));
            if (!hasEnc) {
              newEnc = enc;
            }
            if (newEnc != ID3TE_ISO8859_1 && newEnc != ID3TE_UTF16) {
              // only ISO-8859-1 and UTF16 are allowed for ID3v2.3.0
              newEnc = ID3TE_UTF16;
            }
            if (newEnc == ID3TE_ISO8859_1) {
              // check if information is lost if the string is not unicode
              int i, unicode_size = value.length();
              const QChar* qcarray = value.unicode();
              for (i = 0; i < unicode_size; i++) {
                char ch = qcarray[i].toLatin1();
                if (ch == 0 || (ch & 0x80) != 0) {
                  newEnc = ID3TE_UTF16;
                  break;
                }
              }
            }
            if (enc != newEnc && id != ID3FID_SYNCEDLYRICS) {
              ID3_Field* encfld = id3Frame->GetField(ID3FN_TEXTENC);
              if (encfld) {
                encfld->Set(newEnc);
              }
              fld->SetEncoding(newEnc);
              markTagChanged(Frame::Tag_2, frame.getType());
            }
            if (getString(fld) != value) {
              setString(fld, value);
              markTagChanged(Frame::Tag_2, frame.getType());
            }
            return true;
          } else if (id3Frame->GetID() == ID3FID_PRIVATE &&
                     (fld = id3Frame->GetField(ID3FN_DATA)) != 0) {
            ID3_Field* ownerFld = id3Frame->GetField(ID3FN_OWNER);
            QString owner;
            QByteArray newData, oldData;
            if (ownerFld && !(owner = getString(ownerFld)).isEmpty() &&
                AttributeData(owner).toByteArray(value, newData)) {
              oldData = QByteArray(reinterpret_cast<const char*>(fld->GetRawBinary()),
                                   static_cast<int>(fld->Size()));
              if (newData != oldData) {
                fld->Set(reinterpret_cast<const unsigned char*>(newData.data()),
                         newData.size());
                markTagChanged(Frame::Tag_2, frame.getType());
              }
              return true;
            }
          } else if (id3Frame->GetID() == ID3FID_CDID &&
                     (fld = id3Frame->GetField(ID3FN_DATA)) != 0) {
            QByteArray newData, oldData;
            if (AttributeData::isHexString(value, 'F', QLatin1String("+")) &&
                AttributeData(AttributeData::Utf16).toByteArray(value, newData)) {
              oldData = QByteArray(reinterpret_cast<const char*>(fld->GetRawBinary()),
                                   static_cast<int>(fld->Size()));
              if (newData != oldData) {
                fld->Set(reinterpret_cast<const unsigned char*>(newData.data()),
                         static_cast<size_t>(newData.size()));
                markTagChanged(Frame::Tag_2, frame.getType());
              }
              return true;
            }
          } else if (id3Frame->GetID() == ID3FID_UNIQUEFILEID &&
                     (fld = id3Frame->GetField(ID3FN_DATA)) != 0) {
            QByteArray newData, oldData;
            if (AttributeData::isHexString(value, 'Z')) {
              newData = (value + QLatin1Char('\0')).toLatin1();
              oldData = QByteArray(reinterpret_cast<const char*>(fld->GetRawBinary()),
                                   static_cast<int>(fld->Size()));
              if (newData != oldData) {
                fld->Set(reinterpret_cast<const unsigned char*>(newData.data()),
                         static_cast<size_t>(newData.size()));
                markTagChanged(Frame::Tag_2, frame.getType());
              }
              return true;
            }
          } else if (id3Frame->GetID() == ID3FID_POPULARIMETER &&
                     (fld = id3Frame->GetField(ID3FN_RATING)) != 0) {
            if (getString(fld) != value) {
              fld->Set(value.toInt());
              markTagChanged(Frame::Tag_2, frame.getType());
            }
            return true;
          }
        } else {
          setId3v2Frame(id3Frame, frame);
          markTagChanged(Frame::Tag_2, frame.getType());
          return true;
        }
      }
    }
  }

  // Try the basic method
  Frame::Type type = frame.getType();
  if (type < Frame::FT_FirstFrame || type > Frame::FT_LastV1Frame)
    return false;

  ID3_FrameID frameId = getId3libFrameIdForType(type);
  if (frameId == ID3FID_NOFRAME)
    return false;

  ID3_Tag* tag;
  const QTextCodec* codec;
  bool allowUnicode;
  if (tagNr == Frame::Tag_1) {
    tag = m_tagV1;
    codec = s_textCodecV1;
    allowUnicode = false;
  } else if (tagNr == Frame::Tag_2) {
    tag = m_tagV2;
    codec = 0;
    allowUnicode = true;
  } else {
    return false;
  }

  switch (type) {
  case Frame::FT_Album:
  case Frame::FT_Artist:
  case Frame::FT_Comment:
  case Frame::FT_Title:
  {
    QString str = frame.getValue();
    if (getTextField(tag, frameId, codec) != str &&
        setTextField(tag, frameId, str, allowUnicode, true, true, codec)) {
      markTagChanged(tagNr, type);
      QString s = checkTruncation(tagNr, str, 1ULL << type, type == Frame::FT_Comment ? 28 : 30);
      if (!s.isNull()) setTextField(tag, frameId, s, allowUnicode, true, true, codec);
    }
    break;
  }
  case Frame::FT_Date:
  {
    int num = frame.getValueAsNumber();
    if (setYear(tag, num)) {
      markTagChanged(tagNr, type);
    }
    break;
  }
  case Frame::FT_Genre:
  {
    QString str = frame.getValue();
    if (tagNr == Frame::Tag_1) {
      if (!str.isNull()) {
        int num = Genres::getNumber(str);
        if (setGenreNum(tag, num)) {
          markTagChanged(tagNr, type);
        }
        // if the string cannot be converted to a number, set the truncation flag
        checkTruncation(tagNr, num == 0xff && !str.isEmpty() ? 1 : 0,
                        1ULL << type, 0);
      }
    } else {
      if (!str.isNull()) {
        int num = 0xff;
        if (!TagConfig::instance().genreNotNumeric()) {
          num = Genres::getNumber(str);
        }
        if (num >= 0 && num != 0xff) {
          if (getGenreNum(tag) != num &&
              setGenreNum(tag, num)) {
            markTagChanged(tagNr, type);
          }
        } else {
          if (getTextField(tag, frameId, codec) != str &&
              setTextField(tag, frameId, str, allowUnicode, true, true, codec)) {
            markTagChanged(tagNr, type);
          }
        }
      }
    }
    break;
  }
  case Frame::FT_Track:
  {
    if (tagNr == Frame::Tag_1) {
      int num = frame.getValueAsNumber();
      if (setTrackNum(tag, num)) {
        markTagChanged(tagNr, type);
        int n = checkTruncation(tagNr, num, 1ULL << type);
        if (n != -1) setTrackNum(tag, n);
      }
    } else {
      QString str = frame.getValue();
      int numTracks;
      int num = splitNumberAndTotal(str, &numTracks);
      if (setTrackNum(tag, num, numTracks)) {
        markTagChanged(tagNr, type);
      }
    }
    break;
  }
  default:
    return false;
  }
  return true;
}

/**
 * Create an id3lib frame from a frame.
 * @param frame frame
 * @return id3lib frame, 0 if invalid.
 */
ID3_Frame* Mp3File::createId3FrameFromFrame(Frame& frame) const
{
  ID3_Frame* id3Frame = 0;
  ID3_FrameID id;
  if (frame.getType() != Frame::FT_Other) {
    id = getId3libFrameIdForType(frame.getType());
  } else {
    id = getId3libFrameIdForName(frame.getName());
    if (id == ID3FID_NOFRAME) {
      if (frame.getName() == QLatin1String("AverageLevel") ||
          frame.getName() == QLatin1String("PeakValue") ||
          frame.getName().startsWith(QLatin1String("WM/"))) {
        id = ID3FID_PRIVATE;
      } else if (frame.getName().startsWith(QLatin1String("iTun"))) {
        id = ID3FID_COMMENT;
      } else {
        id = ID3FID_USERTEXT;
      }
    }
  }
  if (id != ID3FID_NOFRAME && id != ID3FID_SETSUBTITLE) {
    id3Frame = new ID3_Frame(id);
    ID3_Field* fld = id3Frame->GetField(ID3FN_TEXT);
    if (fld) {
      ID3_TextEnc enc = getDefaultTextEncoding();
      ID3_Field* encfld = id3Frame->GetField(ID3FN_TEXTENC);
      if (encfld) {
        encfld->Set(enc);
      }
      fld->SetEncoding(enc);
    }
    if (id == ID3FID_USERTEXT && !frame.getName().startsWith(QLatin1String("TXXX"))) {
      fld = id3Frame->GetField(ID3FN_DESCRIPTION);
      if (fld) {
        QString description;
        if (frame.getType() == Frame::FT_CatalogNumber) {
          description = QLatin1String("CATALOGNUMBER");
        } else if (frame.getType() == Frame::FT_ReleaseCountry) {
          description = QLatin1String("RELEASECOUNTRY");
        } else {
          description = frame.getName();
        }
        setString(fld, description);
      }
    } else if (id == ID3FID_COMMENT) {
      fld = id3Frame->GetField(ID3FN_LANGUAGE);
      if (fld) {
        setString(fld, QLatin1String("eng"));
      }
      if (frame.getType() == Frame::FT_Other) {
        fld = id3Frame->GetField(ID3FN_DESCRIPTION);
        if (fld) {
          setString(fld, frame.getName());
        }
      }
    } else if (id == ID3FID_PRIVATE && !frame.getName().startsWith(QLatin1String("PRIV"))) {
      fld = id3Frame->GetField(ID3FN_OWNER);
      if (fld) {
        setString(fld, frame.getName());
        QByteArray data;
        if (AttributeData(frame.getName()).toByteArray(frame.getValue(), data)) {
          fld = id3Frame->GetField(ID3FN_DATA);
          if (fld) {
            fld->Set(reinterpret_cast<const unsigned char*>(data.data()),
                     static_cast<size_t>(data.size()));
          }
        }
      }
    } else if (id == ID3FID_UNIQUEFILEID) {
      fld = id3Frame->GetField(ID3FN_OWNER);
      if (fld) {
        setString(fld, QLatin1String("http://www.id3.org/dummy/ufid.html"));
      }
      QByteArray data;
      if (AttributeData::isHexString(frame.getValue(), 'Z')) {
        data = (frame.getValue() + QLatin1Char('\0')).toLatin1();
        fld = id3Frame->GetField(ID3FN_DATA);
        if (fld) {
          fld->Set(reinterpret_cast<const unsigned char*>(data.data()),
                   static_cast<size_t>(data.size()));
        }
      }
    } else if (id == ID3FID_PICTURE) {
      fld = id3Frame->GetField(ID3FN_MIMETYPE);
      if (fld) {
        setString(fld, QLatin1String("image/jpeg"));
      }
      fld = id3Frame->GetField(ID3FN_PICTURETYPE);
      if (fld) {
        fld->Set(ID3PT_COVERFRONT);
      }
    } else if (id == ID3FID_SYNCEDLYRICS) {
      fld = id3Frame->GetField(ID3FN_LANGUAGE);
      if (fld) {
        setString(fld, QLatin1String("eng"));
      }
      fld = id3Frame->GetField(ID3FN_TIMESTAMPFORMAT);
      if (fld) {
        fld->Set(ID3TSF_MS);
      }
      fld = id3Frame->GetField(ID3FN_CONTENTTYPE);
      if (fld) {
        fld->Set(ID3CT_LYRICS);
      }
    } else if (id == ID3FID_UNSYNCEDLYRICS || id == ID3FID_TERMSOFUSE) {
      fld = id3Frame->GetField(ID3FN_LANGUAGE);
      if (fld) {
        setString(fld, QLatin1String("eng"));
      }
    }
    if (!frame.fieldList().empty()) {
      setId3v2Frame(id3Frame, frame);
    }
    Frame::Type type;
    const char* name;
    getTypeStringForId3libFrameId(id, type, name);
    frame.setExtendedType(Frame::ExtendedType(type, QString::fromLatin1(name)));
  }
  return id3Frame;
}

/**
 * Add a frame in the tags.
 *
 * @param tagNr tag number
 * @param frame frame to add, a field list may be added by this method
 *
 * @return true if ok.
 */
bool Mp3File::addFrame(Frame::TagNumber tagNr, Frame& frame)
{
  if (tagNr == Frame::Tag_2) {
    // Add a new frame.
    ID3_Frame* id3Frame;
    if (m_tagV2 && (id3Frame = createId3FrameFromFrame(frame)) != 0) {
      m_tagV2->AttachFrame(id3Frame);
      frame.setIndex(m_tagV2->NumFrames() - 1);
      if (frame.fieldList().empty()) {
        // add field list to frame
        getFieldsFromId3Frame(id3Frame, frame.fieldList());
        frame.setFieldListFromValue();
      }
      markTagChanged(Frame::Tag_2, frame.getType());
      return true;
    }
  }

  // Try the superclass method
  return TaggedFile::addFrame(tagNr, frame);
}

/**
 * Delete a frame from the tags.
 *
 * @param tagNr tag number
 * @param frame frame to delete.
 *
 * @return true if ok.
 */
bool Mp3File::deleteFrame(Frame::TagNumber tagNr, const Frame& frame)
{
  if (tagNr == Frame::Tag_2) {
    // If the frame has an index, delete that specific frame
    int index = frame.getIndex();
    if (index != -1 && m_tagV2) {
      ID3_Frame* id3Frame = getId3v2Frame(m_tagV2, index);
      if (id3Frame) {
        m_tagV2->RemoveFrame(id3Frame);
        markTagChanged(Frame::Tag_2, frame.getType());
        return true;
      }
    }
  }

  // Try the superclass method
  return TaggedFile::deleteFrame(tagNr, frame);
}

/**
 * Create a frame from an id3lib frame.
 * @param id3Frame id3lib frame
 * @param index, -1 if not used
 * @return frame.
 */
static Frame createFrameFromId3libFrame(ID3_Frame* id3Frame, int index)
{
  Frame::Type type;
  const char* name;
  getTypeStringForId3libFrameId(id3Frame->GetID(), type, name);
  Frame frame(type, QLatin1String(""), QString::fromLatin1(name), index);
  frame.setValue(getFieldsFromId3Frame(id3Frame, frame.fieldList()));
  if (id3Frame->GetID() == ID3FID_USERTEXT ||
      id3Frame->GetID() == ID3FID_WWWUSER ||
      id3Frame->GetID() == ID3FID_COMMENT) {
    QVariant fieldValue = frame.getFieldValue(Frame::ID_Description);
    if (fieldValue.isValid()) {
      QString description = fieldValue.toString();
      if (!description.isEmpty()) {
        if (description == QLatin1String("CATALOGNUMBER")) {
          frame.setType(Frame::FT_CatalogNumber);
        } else if (description == QLatin1String("RELEASECOUNTRY")) {
          frame.setType(Frame::FT_ReleaseCountry);
        } else {
          frame.setExtendedType(Frame::ExtendedType(Frame::FT_Other,
              frame.getInternalName() + QLatin1Char('\n') + description));
        }
      }
    }
  } else if (id3Frame->GetID() == ID3FID_PRIVATE) {
    const Frame::FieldList& fields = frame.getFieldList();
    QString owner;
    QByteArray data;
    for (Frame::FieldList::const_iterator it = fields.begin();
         it != fields.end();
         ++it) {
      if ((*it).m_id == Frame::ID_Owner) {
        owner = (*it).m_value.toString();
        if (!owner.isEmpty()) {
          frame.setExtendedType(Frame::ExtendedType(Frame::FT_Other,
                    frame.getInternalName() + QLatin1Char('\n') + owner));
        }
      } else if ((*it).m_id == Frame::ID_Data) {
        data = (*it).m_value.toByteArray();
      }
    }
    if (!owner.isEmpty() && !data.isEmpty()) {
      QString str;
      if (AttributeData(owner).toString(data, str)) {
        frame.setValue(str);
      }
    }
  } else if (id3Frame->GetID() == ID3FID_CDID) {
    QVariant fieldValue = frame.getFieldValue(Frame::ID_Data);
    if (fieldValue.isValid()) {
      QString str;
      if (AttributeData(AttributeData::Utf16).toString(fieldValue.toByteArray(), str) &&
          AttributeData::isHexString(str, 'F', QLatin1String("+"))) {
        frame.setValue(str);
      }
    }
  } else if (id3Frame->GetID() == ID3FID_UNIQUEFILEID) {
    QVariant fieldValue = frame.getFieldValue(Frame::ID_Data);
    if (fieldValue.isValid()) {
      QByteArray ba(fieldValue.toByteArray());
      QString str(QString::fromLatin1(ba));
      if (ba.size() - str.length() <= 1 &&
          AttributeData::isHexString(str, 'Z')) {
        frame.setValue(str);
      }
    }
  } else if (id3Frame->GetID() == ID3FID_POPULARIMETER) {
    QVariant fieldValue = frame.getFieldValue(Frame::ID_Rating);
    if (fieldValue.isValid()) {
      QString str(fieldValue.toString());
      if (!str.isEmpty()) {
        frame.setValue(str);
      }
    }
  }
  return frame;
}

/**
 * Remove frames.
 *
 * @param tagNr tag number
 * @param flt filter specifying which frames to remove
 */
void Mp3File::deleteFrames(Frame::TagNumber tagNr, const FrameFilter& flt)
{
  if (tagNr == Frame::Tag_1) {
    if (m_tagV1) {
      if (flt.areAllEnabled()) {
        ID3_Tag::Iterator* iter = m_tagV1->CreateIterator();
        ID3_Frame* frame;
        while ((frame = iter->GetNext()) != NULL) {
          m_tagV1->RemoveFrame(frame);
        }
#ifdef Q_OS_WIN32
        /* allocated in Windows DLL => must be freed in the same DLL */
        ID3TagIterator_Delete(reinterpret_cast<ID3TagIterator*>(iter));
#else
#ifdef GCC_HAS_DIAGNOSTIC_PRAGMA
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdelete-non-virtual-dtor"
/* Is safe because iterator implementation has default destructor */
#endif
        delete iter;
#ifdef GCC_HAS_DIAGNOSTIC_PRAGMA
#pragma GCC diagnostic pop
#endif
#endif
        markTagChanged(Frame::Tag_1, Frame::FT_UnknownFrame);
        clearTrunctionFlags(Frame::Tag_1);
      } else {
        TaggedFile::deleteFrames(Frame::Tag_1, flt);
      }
    }
  } else if (tagNr == Frame::Tag_2) {
    if (m_tagV2) {
      if (flt.areAllEnabled()) {
        ID3_Tag::Iterator* iter = m_tagV2->CreateIterator();
        ID3_Frame* frame;
        while ((frame = iter->GetNext()) != NULL) {
          m_tagV2->RemoveFrame(frame);
        }
#ifdef Q_OS_WIN32
        /* allocated in Windows DLL => must be freed in the same DLL */
        ID3TagIterator_Delete(reinterpret_cast<ID3TagIterator*>(iter));
#else
#ifdef GCC_HAS_DIAGNOSTIC_PRAGMA
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdelete-non-virtual-dtor"
/* Is safe because iterator implementation has default destructor */
#endif
        delete iter;
#ifdef GCC_HAS_DIAGNOSTIC_PRAGMA
#pragma GCC diagnostic pop
#endif
#endif
        markTagChanged(Frame::Tag_2, Frame::FT_UnknownFrame);
      } else {
        ID3_Tag::Iterator* iter = m_tagV2->CreateIterator();
        ID3_Frame* id3Frame;
        while ((id3Frame = iter->GetNext()) != NULL) {
          Frame frame(createFrameFromId3libFrame(id3Frame, -1));
          if (flt.isEnabled(frame.getType(), frame.getName())) {
            m_tagV2->RemoveFrame(id3Frame);
          }
        }
  #ifdef Q_OS_WIN32
        /* allocated in Windows DLL => must be freed in the same DLL */
        ID3TagIterator_Delete(reinterpret_cast<ID3TagIterator*>(iter));
  #else
  #ifdef GCC_HAS_DIAGNOSTIC_PRAGMA
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wdelete-non-virtual-dtor"
  /* Is safe because iterator implementation has default destructor */
  #endif
        delete iter;
  #ifdef GCC_HAS_DIAGNOSTIC_PRAGMA
  #pragma GCC diagnostic pop
  #endif
  #endif
        markTagChanged(Frame::Tag_2, Frame::FT_UnknownFrame);
      }
    }
  }
}

/**
 * Get all frames in tag.
 *
 * @param tagNr tag number
 * @param frames frame collection to set.
 */
void Mp3File::getAllFrames(Frame::TagNumber tagNr, FrameCollection& frames)
{
  if (tagNr == Frame::Tag_2) {
    frames.clear();
    if (m_tagV2) {
      ID3_Tag::Iterator* iter = m_tagV2->CreateIterator();
      ID3_Frame* id3Frame;
      int i = 0;
      while ((id3Frame = iter->GetNext()) != 0) {
        Frame frame(createFrameFromId3libFrame(id3Frame, i++));
        frames.insert(frame);
      }
#ifdef Q_OS_WIN32
      /* allocated in Windows DLL => must be freed in the same DLL */
      ID3TagIterator_Delete(reinterpret_cast<ID3TagIterator*>(iter));
#else
#ifdef GCC_HAS_DIAGNOSTIC_PRAGMA
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdelete-non-virtual-dtor"
/* Is safe because iterator implementation has default destructor */
#endif
      delete iter;
#ifdef GCC_HAS_DIAGNOSTIC_PRAGMA
#pragma GCC diagnostic pop
#endif
#endif
    }
    updateMarkedState(tagNr, frames);
    frames.addMissingStandardFrames();
    return;
  }

  TaggedFile::getAllFrames(tagNr, frames);
}

/**
 * Add a suitable field list for the frame if missing.
 * If a frame is created, its field list is empty. This method will create
 * a field list appropriate for the frame type and tagged file type if no
 * field list exists.
 * @param tagNr tag number
 * @param frame frame where field list is added
 */
void Mp3File::addFieldList(Frame::TagNumber tagNr, Frame& frame) const
{
  if (tagNr == Frame::Tag_2 && frame.fieldList().isEmpty()) {
    ID3_Frame* id3Frame = createId3FrameFromFrame(frame);
    getFieldsFromId3Frame(id3Frame, frame.fieldList());
    frame.setFieldListFromValue();
    delete id3Frame;
  }
}

/**
 * Get a list of frame IDs which can be added.
 * @param tagNr tag number
 * @return list with frame IDs.
 */
QStringList Mp3File::getFrameIds(Frame::TagNumber tagNr) const
{
  if (tagNr != Frame::Tag_2)
    return QStringList();

  QStringList lst;
  for (int type = Frame::FT_FirstFrame; type <= Frame::FT_LastFrame; ++type) {
    if (type != Frame::FT_Part) {
      lst.append(Frame::ExtendedType(static_cast<Frame::Type>(type), QLatin1String("")).
                 getName());
    }
  }
  for (int i = 0; i <= ID3FID_WWWUSER; ++i) {
    if (typeStrOfId[i].type == Frame::FT_Other) {
      const char* s = typeStrOfId[i].str;
      if (s) {
        lst.append(QString::fromLatin1(s));
      }
    }
  }
  return lst;
}

/**
 * Set the text codec to be used for tag 1.
 *
 * @param codec text codec, 0 to use default (ISO 8859-1)
 */
void Mp3File::setTextCodecV1(const QTextCodec* codec)
{
  s_textCodecV1 = codec;
}

/**
 * Set the default text encoding.
 *
 * @param textEnc default text encoding
 */
void Mp3File::setDefaultTextEncoding(TagConfig::TextEncoding textEnc)
{
  // UTF8 encoding is buggy, so UTF16 is used when UTF8 is configured
  s_defaultTextEncoding = textEnc == TagConfig::TE_ISO8859_1 ?
    ID3TE_ISO8859_1 : ID3TE_UTF16;
}

/**
 * Notify about configuration change.
 * This method shall be called when the configuration changes.
 */
void Mp3File::notifyConfigurationChange()
{
  const QTextCodec* id3v1TextCodec =
    TagConfig::instance().textEncodingV1() != QLatin1String("ISO-8859-1") ?
    QTextCodec::codecForName(TagConfig::instance().textEncodingV1().toLatin1().data()) : 0;
  setDefaultTextEncoding(
    static_cast<TagConfig::TextEncoding>(TagConfig::instance().textEncoding()));
  setTextCodecV1(id3v1TextCodec);
}
