/**
 * \file m4afile.cpp
 * Handling of MPEG-4 audio files.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 25 Oct 2007
 *
 * Copyright (C) 2007-2023  Urs Fleisch
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

#include "m4afile.h"
#include "mp4v2config.h"

#include <QFile>
#include <QDir>
#include <QByteArray>
#include <stdio.h>
#ifdef HAVE_MP4V2_MP4V2_H
#include <mp4v2/mp4v2.h>
#else
#include <mp4.h>
#endif
#include <cstdlib>
#include <cstring>
#include "genres.h"
#include "pictureframe.h"

/** MPEG4IP version as 16-bit hex number with major and minor version. */
#if defined MP4V2_PROJECT_version_major && defined MP4V2_PROJECT_version_minor
#define MPEG4IP_MAJOR_MINOR_VERSION ((MP4V2_PROJECT_version_major << 8) | \
  MP4V2_PROJECT_version_minor)
#elif defined MPEG4IP_MAJOR_VERSION && defined MPEG4IP_MINOR_VERSION
#define MPEG4IP_MAJOR_MINOR_VERSION ((MPEG4IP_MAJOR_VERSION << 8) | \
  MPEG4IP_MINOR_VERSION)
#else
#define MPEG4IP_MAJOR_MINOR_VERSION 0x0009
#endif

#if MPEG4IP_MAJOR_MINOR_VERSION < 0x0200
/** Set content ID. */
#define MP4TagsSetContentID MP4TagsSetCNID
/** Set artist ID. */
#define MP4TagsSetArtistID MP4TagsSetATID
/** Set playlist ID. */
#define MP4TagsSetPlaylistID MP4TagsSetPLID
/** Set genre ID. */
#define MP4TagsSetGenreID MP4TagsSetGEID
#endif

namespace {

/** Mapping between frame types and field names. */
const struct {
  const char* name;
  Frame::Type type;
} nameTypes[] = {
  { "\251nam", Frame::FT_Title },
  { "\251ART", Frame::FT_Artist },
  { "\251wrt", Frame::FT_Composer },
  { "\251alb", Frame::FT_Album },
  { "\251day", Frame::FT_Date },
  { "\251enc", Frame::FT_EncodedBy },
  { "\251cmt", Frame::FT_Comment },
  { "\251gen", Frame::FT_Genre },
  { "trkn", Frame::FT_Track },
  { "disk", Frame::FT_Disc },
  { "gnre", Frame::FT_Genre },
  { "cpil", Frame::FT_Compilation },
  { "tmpo", Frame::FT_Bpm },
#if MPEG4IP_MAJOR_MINOR_VERSION >= 0x0105
  { "\251grp", Frame::FT_Grouping },
#endif
#if MPEG4IP_MAJOR_MINOR_VERSION >= 0x0106
  { "aART", Frame::FT_AlbumArtist },
  { "pgap", Frame::FT_Other },
#endif
#if MPEG4IP_MAJOR_MINOR_VERSION >= 0x0109
  { "cprt", Frame::FT_Copyright },
  { "\251lyr", Frame::FT_Lyrics },
  { "tvsh", Frame::FT_Other },
  { "tvnn", Frame::FT_Other },
  { "tven", Frame::FT_Other },
  { "tvsn", Frame::FT_Other },
  { "tves", Frame::FT_Other },
  { "desc", Frame::FT_Description },
  { "ldes", Frame::FT_Other },
  { "sonm", Frame::FT_SortName },
  { "soar", Frame::FT_SortArtist },
  { "soaa", Frame::FT_SortAlbumArtist },
  { "soal", Frame::FT_SortAlbum },
  { "soco", Frame::FT_SortComposer },
  { "sosn", Frame::FT_Other },
  { "\251too", Frame::FT_EncoderSettings },
  { "\251wrk", Frame::FT_Work },
  { "purd", Frame::FT_Other },
  { "pcst", Frame::FT_Other },
  { "keyw", Frame::FT_Other },
  { "catg", Frame::FT_Other },
  { "hdvd", Frame::FT_Other },
  { "stik", Frame::FT_Other },
  { "rtng", Frame::FT_Other },
  { "apID", Frame::FT_Other },
  { "akID", Frame::FT_Other },
  { "sfID", Frame::FT_Other },
  { "cnID", Frame::FT_Other },
  { "atID", Frame::FT_Other },
  { "plID", Frame::FT_Other },
  { "geID", Frame::FT_Other },
  { "purl", Frame::FT_Other },
  { "egid", Frame::FT_Other },
  { "cmID", Frame::FT_Other },
  { "xid ", Frame::FT_Other },
#endif
  { "covr", Frame::FT_Picture }
},
freeFormNameTypes[] = {
#if !(MPEG4IP_MAJOR_MINOR_VERSION >= 0x0105)
  { "GROUPING", Frame::FT_Grouping },
#endif
#if !(MPEG4IP_MAJOR_MINOR_VERSION >= 0x0106)
  { "ALBUMARTIST", Frame::FT_AlbumArtist },
#endif
  { "ARRANGER", Frame::FT_Arranger },
  { "AUTHOR", Frame::FT_Author },
  { "CATALOGNUMBER", Frame::FT_CatalogNumber },
  { "CONDUCTOR", Frame::FT_Conductor },
  { "ENCODINGTIME", Frame::FT_EncodingTime },
  { "INITIALKEY", Frame::FT_InitialKey },
#if !(MPEG4IP_MAJOR_MINOR_VERSION >= 0x0109)
  { "COPYRIGHT", Frame::FT_Copyright },
#endif
  { "ISRC", Frame::FT_Isrc },
  { "LANGUAGE", Frame::FT_Language },
  { "LYRICIST", Frame::FT_Lyricist },
#if !(MPEG4IP_MAJOR_MINOR_VERSION >= 0x0109)
  { "LYRICS", Frame::FT_Lyrics },
#endif
  { "MOOD", Frame::FT_Mood },
  { "SOURCEMEDIA", Frame::FT_Media },
  { "ORIGINALALBUM", Frame::FT_OriginalAlbum },
  { "ORIGINALARTIST", Frame::FT_OriginalArtist },
  { "ORIGINALDATE", Frame::FT_OriginalDate },
  { "PERFORMER", Frame::FT_Performer },
  { "PUBLISHER", Frame::FT_Publisher },
  { "RELEASECOUNTRY", Frame::FT_ReleaseCountry },
  { "REMIXER", Frame::FT_Remixer },
  { "SUBTITLE", Frame::FT_Subtitle },
  { "WEBSITE", Frame::FT_Website },
  { "WWWAUDIOFILE", Frame::FT_WWWAudioFile },
  { "WWWAUDIOSOURCE", Frame::FT_WWWAudioSource },
  { "RELEASEDATE", Frame::FT_ReleaseDate },
  { "rate", Frame::FT_Rating }
};

/**
 * Get the predefined field name for a type.
 *
 * @param type frame type
 *
 * @return field name, QString::null if not defined.
 */
QString getNameForType(Frame::Type type)
{
  static QMap<Frame::Type, QString> typeNameMap;
  if (typeNameMap.empty()) {
    // first time initialization
    for (const auto& nameType : nameTypes) {
      if (nameType.type != Frame::FT_Other) {
        typeNameMap.insert(nameType.type, QString::fromLatin1(nameType.name));
      }
    }
    for (const auto& freeFormNameType : freeFormNameTypes) {
      typeNameMap.insert(freeFormNameType.type,
                         QString::fromLatin1(freeFormNameType.name));
    }
  }
  if (type != Frame::FT_Other) {
    auto it = typeNameMap.constFind(type);
    if (it != typeNameMap.constEnd()) {
      return *it;
    } else {
      auto customFrameName = Frame::getNameForCustomFrame(type);
      if (!customFrameName.isEmpty()) {
        return QString::fromLatin1(customFrameName);
      }
    }
  }
  return QString();
}

/**
 * Get the type for a predefined field name.
 *
 * @param name           field name
 * @param onlyPredefined if true, FT_Unknown is returned for fields which
 *                       are not predefined, else FT_Other
 *
 * @return type, FT_Unknown or FT_Other if not predefined field.
 */
Frame::Type getTypeForName(const QString& name, bool onlyPredefined = false)
{
  if (name.length() == 4) {
    static QMap<QString, Frame::Type> nameTypeMap;
    if (nameTypeMap.empty()) {
      // first time initialization
      for (const auto& nameType : nameTypes) {
        nameTypeMap.insert(QString::fromLatin1(nameType.name), nameType.type);
      }
    }
    auto it = nameTypeMap.constFind(name);
    if (it != nameTypeMap.constEnd()) {
      Frame::Type type = *it;
      if (type == Frame::FT_Other) {
        type = Frame::getTypeFromCustomFrameName(name.toLatin1());
      }
      return type;
    }
  }
  if (!onlyPredefined) {
    static QMap<QString, Frame::Type> freeFormNameTypeMap;
    if (freeFormNameTypeMap.empty()) {
      // first time initialization
      for (const auto& freeFormNameType : freeFormNameTypes) {
        freeFormNameTypeMap.insert(QString::fromLatin1(freeFormNameType.name),
                                   freeFormNameType.type);
      }
    }
    auto it = freeFormNameTypeMap.constFind(name);
    if (it != freeFormNameTypeMap.constEnd()) {
      return *it;
    }
    return Frame::getTypeFromCustomFrameName(name.toLatin1());
  }
  return Frame::FT_UnknownFrame;
}

#if MPEG4IP_MAJOR_MINOR_VERSION >= 0x0109
#elif defined HAVE_MP4V2_MP4GETMETADATABYINDEX_CHARPP_ARG
#else
/**
 * Check if a name is a free form field.
 *
 * @param hFile handle
 * @param name  field name
 *
 * @return true if a free form field.
 */
bool isFreeFormMetadata(MP4FileHandle hFile, const char* name)
{
  bool result = false;
  if (getTypeForName(name, true) == Frame::FT_UnknownFrame) {
    uint8_t* pValue = 0;
    uint32_t valueSize = 0;
    result = MP4GetMetadataFreeForm(hFile, const_cast<char*>(name),
                                    &pValue, &valueSize);
    if (pValue && valueSize > 0) {
      free(pValue);
    }
  }
  return result;
}
#endif

/**
 * Get a byte array for a value.
 *
 * @param name  field name
 * @param value field value
 * @param size  size of value in bytes
 *
 * @return byte array with string representation.
 */
QByteArray getValueByteArray(const char* name,
                             const uint8_t* value, uint32_t size)
{
  QByteArray str;
  if (name[0] == '\251') {
    str = QByteArray(reinterpret_cast<const char*>(value), size);
  } else if (std::strcmp(name, "trkn") == 0) {
    if (size >= 6) {
      unsigned track = value[3] + (value[2] << 8);
      unsigned totalTracks = value[5] + (value[4] << 8);
      str.setNum(track);
      if (totalTracks > 0) {
        str += '/';
        str += QByteArray().setNum(totalTracks);
      }
    }
  } else if (std::strcmp(name, "disk") == 0) {
    if (size >= 6) {
      unsigned disk = value[3] + (value[2] << 8);
      unsigned totalDisks = value[5] + (value[4] << 8);
      str.setNum(disk);
      if (totalDisks > 0) {
        str += '/';
        str += QByteArray().setNum(totalDisks);
      }
    }
  } else if (std::strcmp(name, "gnre") == 0) {
    if (size >= 2) {
      unsigned genreNum = value[1] + (value[0] << 8);
      if (genreNum > 0) {
        str = Genres::getName(genreNum - 1);
      }
    }
  } else if (std::strcmp(name, "cpil") == 0) {
    if (size >= 1) {
      str.setNum(value[0]);
    }
  } else if (std::strcmp(name, "tmpo") == 0) {
    if (size >= 2) {
      unsigned bpm = value[1] + (value[0] << 8);
      if (bpm > 0) {
        str.setNum(bpm);
      }
    }
  } else if (std::strcmp(name, "covr") == 0) {
    QByteArray ba;
    ba = QByteArray(reinterpret_cast<const char*>(value), size);
    return ba;
#if MPEG4IP_MAJOR_MINOR_VERSION >= 0x0106
  } else if (std::strcmp(name, "pgap") == 0) {
    if (size >= 1) {
      str.setNum(value[0]);
    }
#endif
#if MPEG4IP_MAJOR_MINOR_VERSION >= 0x0109
  } else if (std::strcmp(name, "tvsn") == 0 || std::strcmp(name, "tves") == 0 ||
             std::strcmp(name, "sfID") == 0 || std::strcmp(name, "cnID") == 0 ||
             std::strcmp(name, "atID") == 0 || std::strcmp(name, "geID") == 0 ||
             std::strcmp(name, "cmID") == 0) {
    if (size >= 4) {
      uint val = value[3] + (value[2] << 8) +
        (value[1] << 16) + (value[0] << 24);
      if (val > 0) {
        str.setNum(val);
      }
    }
  } else if (std::strcmp(name, "pcst") == 0 || std::strcmp(name, "hdvd") == 0 ||
             std::strcmp(name, "stik") == 0 || std::strcmp(name, "rtng") == 0 ||
             std::strcmp(name, "akID") == 0) {
    if (size >= 1) {
      str.setNum(value[0]);
    }
  } else if (std::strcmp(name, "plID") == 0) {
    if (size >= 8) {
      qulonglong val =
          static_cast<qulonglong>(value[7]) +
          (static_cast<qulonglong>(value[6]) << 8) +
          (static_cast<qulonglong>(value[5]) << 16) +
          (static_cast<qulonglong>(value[4]) << 24) +
          (static_cast<qulonglong>(value[3]) << 32) +
          (static_cast<qulonglong>(value[2]) << 40) +
          (static_cast<qulonglong>(value[1]) << 48) +
          (static_cast<qulonglong>(value[0]) << 56);
      if (val > 0) {
        str.setNum(val);
      }
    }
#endif
  } else {
    str = QByteArray(reinterpret_cast<const char*>(value), size);
  }
  return str;
}

/**
 * Set a SYLT frame with data from MP4 chapters.
 * @param frame frame to set
 * @param data list with time stamps and chapter titles
 */
void setMp4ChaptersFields(Frame& frame,
                          const QVariantList& data = QVariantList())
{
  frame.setExtendedType(Frame::ExtendedType(Frame::FT_Other,
                                            QLatin1String("Chapters")));
  frame.setValue(QString());

  Frame::Field field;
  Frame::FieldList& fields = frame.fieldList();
  fields.clear();

  field.m_id = Frame::ID_TimestampFormat;
  field.m_value = 2; // milliseconds
  fields.append(field);

  field.m_id = Frame::ID_ContentType;
  field.m_value = 0; // other
  fields.append(field);

  field.m_id = Frame::ID_Description;
  field.m_value = QString();
  fields.append(field);

  field.m_id = Frame::ID_Data;
  field.m_value = data;
  fields.append(field);
}

/**
 * Set a SYLT frame from MP4 chapters.
 * @param chapterList MP4 chapters
 * @param chapterCount number of elements in chapterList
 * @param frame the SYLT frame is returned here
 */
void mp4ChaptersToFrame(const MP4Chapter_t* chapterList, uint32_t chapterCount,
                        Frame& frame)
{
  QVariantList data;
  quint32 time = 0;
  for (uint32_t i = 0; i < chapterCount; ++i) {
    MP4Chapter_t chapter = chapterList[i];
    data.append(time);
    data.append(QString::fromUtf8(chapter.title));
    time += chapter.duration;
  }
  data.append(time);
  data.append(QString());
  setMp4ChaptersFields(frame, data);
}

/**
 * Set MP4 chapters from a SYLT frame.
 * @param frame SYLT frame
 * @param chapterList the chapters are returned here and must be freed using
 * delete[] afterwards
 * @param chapterCount the number of elements in @a chapterList is returned here
 */
void frameToMp4Chapters(const Frame& frame,
                        MP4Chapter_t*& chapterList, uint32_t& chapterCount)
{
  QVariantList data = Frame::getField(frame, Frame::ID_Data).toList();
  int dataLen = data.size();
  if (dataLen >= 2) {
    quint32 lastTime = data.at(dataLen - 2).toUInt();
    QString lastTitle = data.at(dataLen - 1).toString();
    if (!lastTitle.trimmed().isEmpty()) {
      data.append(lastTime);
      data.append(QString());
      dataLen += 2;
    }
  }
  if (dataLen > 2 && (dataLen & 1) == 0) {
    chapterCount = (dataLen - 2) / 2;
    chapterList = new MP4Chapter_t[chapterCount];
    quint32 lastTime = 0;
    uint32_t i = 0;
    QListIterator<QVariant> it(data);
    while (it.hasNext()) {
      quint32 time = it.next().toUInt();
      if (!it.hasNext())
        break;

      QByteArray chapterTitle = it.next().toString().trimmed().toUtf8();
      if (i < chapterCount) {
        MP4Chapter_t* mp4Chapter = &chapterList[i];
        qstrncpy(mp4Chapter->title, chapterTitle.constData(),
                 sizeof(mp4Chapter->title) - 1);
        mp4Chapter->title[sizeof(mp4Chapter->title) - 1] = '\0';
      }
      if (i > 0 && i <= chapterCount) {
        chapterList[i - 1].duration = time - lastTime;
      }
      lastTime = time;
      ++i;
    }
  } else {
    chapterCount = 0;
    chapterList = nullptr;
  }
}

/**
 * Check if two chapters frames are equal.
 * @param f1 first chapters frame
 * @param f2 second chapters frame
 * @return true if equal.
 */
bool areMp4ChaptersFieldsEqual(const Frame& f1, const Frame& f2)
{
  return Frame::getField(f1, Frame::ID_Data) == Frame::getField(f2, Frame::ID_Data);
}

}

/**
 * Constructor.
 *
 * @param idx index in tagged file system model
 */
M4aFile::M4aFile(const QPersistentModelIndex& idx)
  : TaggedFile(idx), m_fileRead(false)
{
}

/**
 * Get key of tagged file format.
 * @return "Mp4v2Metadata".
 */
QString M4aFile::taggedFileKey() const
{
  return QLatin1String("Mp4v2Metadata");
}

/**
 * Read tags from file.
 *
 * @param force true to force reading even if tags were already read.
 */
void M4aFile::readTags(bool force)
{
  bool priorIsTagInformationRead = isTagInformationRead();
  if (force || !m_fileRead) {
    m_metadata.clear();
    m_extraFrames.clear();
    markTagUnchanged(Frame::Tag_2);
    m_fileRead = true;
    QByteArray fnIn =
#ifdef Q_OS_WIN32
        currentFilePath().toUtf8();
#else
        QFile::encodeName(currentFilePath());
#endif

    MP4FileHandle handle = MP4Read(fnIn);
    if (handle != MP4_INVALID_FILE_HANDLE) {
      m_fileInfo.read(handle);
#if MPEG4IP_MAJOR_MINOR_VERSION >= 0x0109
    MP4ItmfItemList* list = MP4ItmfGetItems(handle);
    if (list) {
      for (uint32_t i = 0; i < list->size; ++i) {
        MP4ItmfItem& item = list->elements[i];
        const char* key = nullptr;
        if (memcmp(item.code, "----", 4) == 0) {
          // free form tagfield
          if (item.name) {
            key = item.name;
          }
        } else {
          key = item.code;
        }
        if (key) {
          if (std::strcmp(key, "covr") == 0) {
            if (item.dataList.size > 0) {
              int i;
              MP4ItmfData* element;
              for (i = 0, element = item.dataList.elements;
                   i < static_cast<int>(item.dataList.size);
                   ++i, ++element) {
                QString mimeType, imgFormat;
                switch (element->typeCode) {
                case MP4_ITMF_BT_PNG:
                  mimeType = QLatin1String("image/png");
                  imgFormat = QLatin1String("PNG");
                  break;
                case MP4_ITMF_BT_BMP:
                  mimeType = QLatin1String("image/bmp");
                  imgFormat = QLatin1String("BMP");
                  break;
                case MP4_ITMF_BT_GIF:
                  mimeType = QLatin1String("image/gif");
                  imgFormat = QLatin1String("GIF");
                  break;
                case MP4_ITMF_BT_JPEG:
                default:
                  mimeType = QLatin1String("image/jpeg");
                  imgFormat = QLatin1String("JPG");
                }
                PictureFrame frame(
                      getValueByteArray(key, element->value, element->valueSize),
                      QLatin1String(""), PictureFrame::PT_CoverFront, mimeType,
                      Frame::TE_ISO8859_1, imgFormat);
                frame.setIndex(Frame::toNegativeIndex(i));
                frame.setExtendedType(Frame::ExtendedType(Frame::FT_Picture,
                                                          QLatin1String(key)));
                m_extraFrames.append(frame);
              }
            }
          } else {
            QByteArray ba;
            if (item.dataList.size > 0 &&
                item.dataList.elements[0].value &&
                item.dataList.elements[0].valueSize > 0) {
              ba = getValueByteArray(key, item.dataList.elements[0].value,
                  item.dataList.elements[0].valueSize);
            }
            m_metadata[QString::fromLatin1(key)] = ba;
          }
        }
      }
      MP4ItmfItemListFree(list);
    }

    MP4Chapter_t* chapterList = nullptr;
    uint32_t chapterCount = 0;
    MP4GetChapters(handle, &chapterList, &chapterCount, MP4ChapterTypeQt);
    if (chapterList) {
      Frame frame;
      mp4ChaptersToFrame(chapterList, chapterCount, frame);
      frame.setIndex(Frame::toNegativeIndex(m_extraFrames.size()));
      m_extraFrames.append(frame);
      MP4Free(chapterList);
    }
#elif defined HAVE_MP4V2_MP4GETMETADATABYINDEX_CHARPP_ARG
      static char notFreeFormStr[] = "NOFF";
      static char freeFormStr[] = "----";
      char* ppName;
      uint8_t* ppValue = 0;
      uint32_t pValueSize = 0;
      uint32_t index = 0;
      unsigned numEmptyEntries = 0;
      for (index = 0; index < 64; ++index) {
        ppName = notFreeFormStr;
        bool ok = MP4GetMetadataByIndex(handle, index,
                                        &ppName, &ppValue, &pValueSize);
        if (ok && ppName && memcmp(ppName, "----", 4) == 0) {
          // free form tagfield
          free(ppName);
          free(ppValue);
          ppName = freeFormStr;
          ppValue = 0;
          pValueSize = 0;
          ok = MP4GetMetadataByIndex(handle, index,
                                     &ppName, &ppValue, &pValueSize);
        }
        if (ok) {
          numEmptyEntries = 0;
          if (ppName) {
            QString key(ppName);
            QByteArray ba;
            if (ppValue && pValueSize > 0) {
              ba = getValueByteArray(ppName, ppValue, pValueSize);
            }
            m_metadata[key] = ba;
            free(ppName);
          }
          free(ppValue);
          ppName = 0;
          ppValue = 0;
          pValueSize = 0;
        } else {
          // There are iTunes files with invalid fields in between,
          // so we stop after 3 invalid indices.
          if (++numEmptyEntries >= 3) {
            break;
          }
        }
      }
#else
      const char* ppName = 0;
      uint8_t* ppValue = 0;
      uint32_t pValueSize = 0;
      uint32_t index = 0;
      unsigned numEmptyEntries = 0;
      for (index = 0; index < 64; ++index) {
        if (MP4GetMetadataByIndex(handle, index,
                                  &ppName, &ppValue, &pValueSize)) {
          numEmptyEntries = 0;
          if (ppName) {
            QString key(ppName);
            QByteArray ba;
            if (ppValue && pValueSize > 0) {
              ba = getValueByteArray(ppName, ppValue, pValueSize);
            }
            m_metadata[key] = ba;

            // If the field is free form, there are two memory leaks in mp4v2.
            // The first is not accessible, the second can be freed.
            if (isFreeFormMetadata(handle, ppName)) {
              free(const_cast<char*>(ppName));
            }
          }
          free(ppValue);
          ppName = 0;
          ppValue = 0;
          pValueSize = 0;
        } else {
          // There are iTunes files with invalid fields in between,
          // so we stop after 3 invalid indices.
          if (++numEmptyEntries >= 3) {
            break;
          }
        }
      }
#endif
      MP4Close(handle
#if MPEG4IP_MAJOR_MINOR_VERSION >= 0x0200
               , MP4_CLOSE_DO_NOT_COMPUTE_BITRATE
#endif
               );
    }
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
bool M4aFile::writeTags(bool force, bool* renamed, bool preserve)
{
  bool ok = true;
  QString fnStr(currentFilePath());
  if (isChanged() && !QFileInfo(fnStr).isWritable()) {
    revertChangedFilename();
    return false;
  }

  if (m_fileRead && (force || isTagChanged(Frame::Tag_2))) {
    QByteArray fn =
#ifdef Q_OS_WIN32
        fnStr.toUtf8();
#else
        QFile::encodeName(fnStr);
#endif

    // store time stamp if it has to be preserved
    quint64 actime = 0, modtime = 0;
    if (preserve) {
      getFileTimeStamps(fnStr, actime, modtime);
    }

    MP4FileHandle handle = MP4Modify(fn);
    if (handle != MP4_INVALID_FILE_HANDLE) {
#if MPEG4IP_MAJOR_MINOR_VERSION >= 0x0109
      MP4ItmfItemList* list = MP4ItmfGetItems(handle);
      if (list) {
        for (uint32_t i = 0; i < list->size; ++i) {
          MP4ItmfRemoveItem(handle, &list->elements[i]);
        }
        MP4ItmfItemListFree(list);
      }
      const MP4Tags* tags = MP4TagsAlloc();
#else
      // return code is not checked because it will fail if no metadata exists
      MP4MetadataDelete(handle);
#endif

      for (auto it = m_metadata.constBegin(); it != m_metadata.constEnd(); ++it) {
        const QByteArray& value = *it;
        if (!value.isEmpty()) {
          const QString& name = it.key();
          const QByteArray& str = value;
#if MPEG4IP_MAJOR_MINOR_VERSION >= 0x0109
          // clazy:excludeall=qlatin1string-non-ascii
          if (name == QLatin1String("\251nam")) {
            MP4TagsSetName(tags, str);
          } else if (name == QLatin1String("\251ART")) {
            MP4TagsSetArtist(tags, str);
          } else if (name == QLatin1String("\251wrt")) {
            MP4TagsSetComposer(tags, str);
          } else if (name == QLatin1String("\251cmt")) {
            MP4TagsSetComments(tags, str);
          } else if (name == QLatin1String("\251too")) {
            MP4TagsSetEncodingTool(tags, str);
          } else if (name == QLatin1String("\251day")) {
            MP4TagsSetReleaseDate(tags, str);
          } else if (name == QLatin1String("\251alb")) {
            MP4TagsSetAlbum(tags, str);
          } else if (name == QLatin1String("trkn")) {
            MP4TagTrack indexTotal;
            int slashPos = str.indexOf('/');
            if (slashPos != -1) {
              indexTotal.total = str.mid(slashPos + 1).toUShort();
              indexTotal.index = str.mid(0, slashPos).toUShort();
            } else {
              indexTotal.total = 0;
              indexTotal.index = str.toUShort();
            }
            MP4TagsSetTrack(tags, &indexTotal);
          } else if (name == QLatin1String("disk")) {
            MP4TagDisk indexTotal;
            int slashPos = str.indexOf('/');
            if (slashPos != -1) {
              indexTotal.total = str.mid(slashPos + 1).toUShort();
              indexTotal.index = str.mid(0, slashPos).toUShort();
            } else {
              indexTotal.total = 0;
              indexTotal.index = str.toUShort();
            }
            MP4TagsSetDisk(tags, &indexTotal);
          } else if (name == QLatin1String("\251gen") || name == QLatin1String("gnre")) {
            MP4TagsSetGenre(tags, str);
          } else if (name == QLatin1String("tmpo")) {
            uint16_t tempo = str.toUShort();
            MP4TagsSetTempo(tags, &tempo);
          } else if (name == QLatin1String("cpil")) {
            uint8_t cpl = str.toUShort();
            MP4TagsSetCompilation(tags, &cpl);
          } else if (name == QLatin1String("\251grp")) {
            MP4TagsSetGrouping(tags, str);
          } else if (name == QLatin1String("aART")) {
            MP4TagsSetAlbumArtist(tags, str);
          } else if (name == QLatin1String("pgap")) {
            uint8_t pgap = str.toUShort();
            MP4TagsSetGapless(tags, &pgap);
          } else if (name == QLatin1String("tvsh")) {
            MP4TagsSetTVShow(tags, str);
          } else if (name == QLatin1String("tvnn")) {
            MP4TagsSetTVNetwork(tags, str);
          } else if (name == QLatin1String("tven")) {
            MP4TagsSetTVEpisodeID(tags, str);
          } else if (name == QLatin1String("tvsn")) {
            uint32_t val = str.toULong();
            MP4TagsSetTVSeason(tags, &val);
          } else if (name == QLatin1String("tves")) {
            uint32_t val = str.toULong();
            MP4TagsSetTVEpisode(tags, &val);
          } else if (name == QLatin1String("desc")) {
            MP4TagsSetDescription(tags, str);
          } else if (name == QLatin1String("ldes")) {
            MP4TagsSetLongDescription(tags, str);
          } else if (name == QLatin1String("\251lyr")) {
            MP4TagsSetLyrics(tags, str);
          } else if (name == QLatin1String("sonm")) {
            MP4TagsSetSortName(tags, str);
          } else if (name == QLatin1String("soar")) {
            MP4TagsSetSortArtist(tags, str);
          } else if (name == QLatin1String("soaa")) {
            MP4TagsSetSortAlbumArtist(tags, str);
          } else if (name == QLatin1String("soal")) {
            MP4TagsSetSortAlbum(tags, str);
          } else if (name == QLatin1String("soco")) {
            MP4TagsSetSortComposer(tags, str);
          } else if (name == QLatin1String("sosn")) {
            MP4TagsSetSortTVShow(tags, str);
          } else if (name == QLatin1String("cprt")) {
            MP4TagsSetCopyright(tags, str);
          } else if (name == QLatin1String("\251enc")) {
            MP4TagsSetEncodedBy(tags, str);
          } else if (name == QLatin1String("purd")) {
            MP4TagsSetPurchaseDate(tags, str);
          } else if (name == QLatin1String("pcst")) {
            uint8_t val = str.toUShort();
            MP4TagsSetPodcast(tags, &val);
          } else if (name == QLatin1String("keyw")) {
            MP4TagsSetKeywords(tags, str);
          } else if (name == QLatin1String("catg")) {
            MP4TagsSetCategory(tags, str);
          } else if (name == QLatin1String("hdvd")) {
            uint8_t val = str.toUShort();
            MP4TagsSetHDVideo(tags, &val);
          } else if (name == QLatin1String("stik")) {
            uint8_t val = str.toUShort();
            MP4TagsSetMediaType(tags, &val);
          } else if (name == QLatin1String("rtng")) {
            uint8_t val = str.toUShort();
            MP4TagsSetContentRating(tags, &val);
          } else if (name == QLatin1String("apID")) {
            MP4TagsSetITunesAccount(tags, str);
          } else if (name == QLatin1String("akID")) {
            uint8_t val = str.toUShort();
            MP4TagsSetITunesAccountType(tags, &val);
          } else if (name == QLatin1String("sfID")) {
            uint32_t val = str.toULong();
            MP4TagsSetITunesCountry(tags, &val);
          } else if (name == QLatin1String("cnID")) {
            uint32_t val = str.toULong();
            MP4TagsSetContentID(tags, &val);
          } else if (name == QLatin1String("atID")) {
            uint32_t val = str.toULong();
            MP4TagsSetArtistID(tags, &val);
          } else if (name == QLatin1String("plID")) {
            uint64_t val = str.toULongLong();
            MP4TagsSetPlaylistID(tags, &val);
          } else if (name == QLatin1String("geID")) {
            uint32_t val = str.toULong();
            MP4TagsSetGenreID(tags, &val);
          } else if (name == QLatin1String("cmID")) {
            uint32_t val = str.toULong();
            MP4TagsSetComposerID(tags, &val);
          } else if (name == QLatin1String("xid ")) {
            MP4TagsSetXID(tags, str);
          } else {
            MP4ItmfItem* item;
            if (name.length() == 4 &&
                (name.at(0).toLatin1() == '\251' ||
                 (name.at(0) >= QLatin1Char('a') &&
                  name.at(0) <= QLatin1Char('z')))) {
              item = MP4ItmfItemAlloc(name.toLatin1().constData(), 1);
            } else {
              item = MP4ItmfItemAlloc("----", 1);
              item->mean = strdup("com.apple.iTunes");
              item->name = strdup(name.toUtf8().data());
            }

            MP4ItmfData& data = item->dataList.elements[0];
            data.typeCode = MP4_ITMF_BT_UTF8;
            data.valueSize = value.size();
            data.value = reinterpret_cast<uint8_t*>(malloc(data.valueSize));
            memcpy(data.value, value.data(), data.valueSize);

            MP4ItmfAddItem(handle, item);
            MP4ItmfItemFree(item);
          }
#else
          bool setOk;
          if (name == QLatin1String("\251nam")) {
            setOk = MP4SetMetadataName(handle, str);
          } else if (name == QLatin1String("\251ART")) {
            setOk = MP4SetMetadataArtist(handle, str);
          } else if (name == QLatin1String("\251wrt")) {
            setOk = MP4SetMetadataWriter(handle, str);
          } else if (name == QLatin1String("\251cmt")) {
            setOk = MP4SetMetadataComment(handle, str);
          } else if (name == QLatin1String("\251too")) {
            setOk = MP4SetMetadataTool(handle, str);
          } else if (name == QLatin1String("\251day")) {
            unsigned short year = str.toUShort();
            if (year > 0) {
              if (year < 1000) year += 2000;
              else if (year > 9999) year = 9999;
              setOk = MP4SetMetadataYear(handle, QByteArray().setNum(year));
              if (setOk) {
                if (year >= 0) {
                  setTextField(QLatin1String("\251day"),
                               year != 0 ? QString::number(year)
                                         : QLatin1String(""), Frame::FT_Date);
                }
              }
            } else {
              setOk = true;
            }
          } else if (name == QLatin1String("\251alb")) {
            setOk = MP4SetMetadataAlbum(handle, str);
          } else if (name == QLatin1String("trkn")) {
            uint16_t track = 0, totalTracks = 0;
            int slashPos = str.indexOf('/');
            if (slashPos != -1) {
              totalTracks = str.mid(slashPos + 1).toUShort();
              track = str.mid(0, slashPos).toUShort();
            } else {
              track = str.toUShort();
            }
            setOk = MP4SetMetadataTrack(handle, track, totalTracks);
          } else if (name == QLatin1String("disk")) {
            uint16_t disk = 0, totalDisks = 0;
            int slashPos = str.indexOf('/');
            if (slashPos != -1) {
              totalDisks = str.mid(slashPos + 1).toUShort();
              disk = str.mid(0, slashPos).toUShort();
            } else {
              disk = str.toUShort();
            }
            setOk = MP4SetMetadataDisk(handle, disk, totalDisks);
          } else if (name == QLatin1String("\251gen") || name == QLatin1String("gnre")) {
            setOk = MP4SetMetadataGenre(handle, str);
          } else if (name == QLatin1String("tmpo")) {
            uint16_t tempo = str.toUShort();
            setOk = MP4SetMetadataTempo(handle, tempo);
          } else if (name == QLatin1String("cpil")) {
            uint8_t cpl = str.toUShort();
            setOk = MP4SetMetadataCompilation(handle, cpl);
// While this works on Debian Etch with libmp4v2-dev 1.5.0.1-0.3 from
// www.debian-multimedia.org, linking on OpenSUSE 10.3 with
// libmp4v2-devel-1.5.0.1-6 from packman.links2linux.de fails with
// undefined reference to MP4SetMetadataGrouping. To avoid this,
// in the line below, 0x105 is replaced by 0x106.
#if MPEG4IP_MAJOR_MINOR_VERSION >= 0x0106
          } else if (name == QLatin1String("\251grp")) {
            setOk = MP4SetMetadataGrouping(handle, str);
#endif
#if MPEG4IP_MAJOR_MINOR_VERSION >= 0x0106
          } else if (name == QLatin1String("aART")) {
            setOk = MP4SetMetadataAlbumArtist(handle, str);
          } else if (name == QLatin1String("pgap")) {
            uint8_t pgap = str.toUShort();
            setOk = MP4SetMetadataPartOfGaplessAlbum(handle, pgap);
#endif
          } else {
            setOk = MP4SetMetadataFreeForm(
              handle, const_cast<char*>(name.toUtf8().data()),
              reinterpret_cast<uint8_t*>(const_cast<char*>(value.data())),
              value.size());
          }
          if (!setOk) {
            qDebug("MP4SetMetadata %s failed", name.toLatin1().data());
            ok = false;
          }
#endif
        }
      }

      bool hasChapters = false;
      const auto frames = m_extraFrames;
      for (const Frame& frame : frames) {
        if (frame.getType() == Frame::FT_Other &&
            frame.getName() == QLatin1String("Chapters")) {
          uint32_t chapterCount = 0;
          MP4Chapter_t* chapterList = nullptr;
          frameToMp4Chapters(frame, chapterList, chapterCount);
          MP4SetChapters(handle, chapterList, chapterCount, MP4ChapterTypeQt);
          hasChapters = true;
          delete [] chapterList;
        } else {
          QByteArray ba;
          if (PictureFrame::getData(frame, ba)) {
#if MPEG4IP_MAJOR_MINOR_VERSION >= 0x0109
            MP4TagArtwork artwork;
            artwork.data = ba.data();
            artwork.size = static_cast<uint32_t>(ba.size());
            artwork.type = MP4_ART_JPEG;
            QString mimeType;
            if (PictureFrame::getMimeType(frame, mimeType)) {
              if (mimeType == QLatin1String("image/png")) {
                artwork.type = MP4_ART_PNG;
              } else if (mimeType == QLatin1String("image/bmp")) {
                artwork.type = MP4_ART_BMP;
              } else if (mimeType == QLatin1String("image/gif")) {
                artwork.type = MP4_ART_GIF;
              }
            }
            MP4TagsAddArtwork(tags, &artwork);
#else
            MP4SetMetadataCoverArt(handle, reinterpret_cast<uint8_t*>(ba.data()),
                                   ba.size());
#endif
          }
        }
      }
      if (!hasChapters) {
        MP4DeleteChapters(handle);
      }

#if MPEG4IP_MAJOR_MINOR_VERSION >= 0x0109
      MP4TagsStore(tags, handle);
      MP4TagsFree(tags);
#endif

      MP4Close(handle
#if MPEG4IP_MAJOR_MINOR_VERSION >= 0x0200
               , MP4_CLOSE_DO_NOT_COMPUTE_BITRATE
#endif
               );
      if (ok) {
        // without this, old tags stay in the file marked as free
        MP4Optimize(fn);
        markTagUnchanged(Frame::Tag_2);
      }

      // restore time stamp
      if (actime || modtime) {
        setFileTimeStamps(fnStr, actime, modtime);
      }
    } else {
      qDebug("MP4Modify failed");
      ok = false;
    }
  }

  if (isFilenameChanged()) {
    if (!renameFile()) {
      return false;
    }
    markFilenameUnchanged();
    // link tags to new file name
    readTags(true);
    *renamed = true;
  }
  return ok;
}

/**
 * Free resources allocated when calling readTags().
 *
 * @param force true to force clearing even if the tags are modified
 */
void M4aFile::clearTags(bool force)
{
  if (!m_fileRead || (isChanged() && !force))
    return;

  bool priorIsTagInformationRead = isTagInformationRead();
  m_metadata.clear();
  m_extraFrames.clear();
  markTagUnchanged(Frame::Tag_2);
  m_fileRead = false;
  notifyModelDataChanged(priorIsTagInformationRead);
}

/**
 * Remove frames.
 *
 * @param tagNr tag number
 * @param flt filter specifying which frames to remove
 */
void M4aFile::deleteFrames(Frame::TagNumber tagNr, const FrameFilter& flt)
{
  if (tagNr != Frame::Tag_2)
    return;

  if (flt.areAllEnabled()) {
    m_metadata.clear();
    m_extraFrames.clear();
    markTagChanged(Frame::Tag_2, Frame::ExtendedType());
  } else {
    bool changed = false;
    for (auto it = m_metadata.begin(); it != m_metadata.end();) { // clazy:exclude=detaching-member
      QString name(it.key());
      Frame::Type type = getTypeForName(name);
      if (flt.isEnabled(type, name)) {
        it = m_metadata.erase(it);
        changed = true;
      } else {
        ++it;
      }
    }
    bool pictureEnabled = flt.isEnabled(Frame::FT_Picture);
    bool chaptersEnabled = flt.isEnabled(Frame::FT_Other,
                                         QLatin1String("Chapters"));
    if ((pictureEnabled || chaptersEnabled) && !m_extraFrames.isEmpty()) {
      for (auto it = m_extraFrames.begin(); it != m_extraFrames.end();) {
        Frame::Type type = it->getType();
        if ((pictureEnabled && type == Frame::FT_Picture) ||
            (chaptersEnabled && type == Frame::FT_Other &&
             it->getName() == QLatin1String("Chapters"))) {
          it = m_extraFrames.erase(it);
          changed = true;
        } else {
          ++it;
        }
      }
    }
    if (changed) {
      markTagChanged(Frame::Tag_2, Frame::ExtendedType());
    }
  }
}

/**
 * Get metadata field as string.
 *
 * @param name field name
 *
 * @return value as string, "" if not found,
 *         QString::null if the tags have not been read yet.
 */
QString M4aFile::getTextField(const QString& name) const
{
  if (m_fileRead) {
    auto it = m_metadata.constFind(name);
    if (it != m_metadata.constEnd()) {
      return QString::fromUtf8((*it).data(), (*it).size());
    }
    return QLatin1String("");
  }
  return QString();
}

/**
 * Set text field.
 * If value is null if the tags have not been read yet, nothing is changed.
 * If value is different from the current value, tag 2 is marked as changed.
 *
 * @param name name
 * @param value value, "" to remove, QString::null to do nothing
 * @param type frame type
 */
void M4aFile::setTextField(const QString& name, const QString& value,
                           const Frame::ExtendedType& type)
{
  if (m_fileRead && !value.isNull()) {
    QByteArray str = value.toUtf8();
    auto it = m_metadata.find(name); // clazy:exclude=detaching-member
    if (it != m_metadata.end()) {
      if (QString::fromUtf8((*it).data(), (*it).size()) != value) {
        *it = str;
        markTagChanged(Frame::Tag_2, type);
      }
    } else {
      m_metadata.insert(name, str);
      markTagChanged(Frame::Tag_2, type);
    }
  }
}

/**
 * Check if tag information has already been read.
 *
 * @return true if information is available,
 *         false if the tags have not been read yet, in which case
 *         hasTag() does not return meaningful information.
 */
bool M4aFile::isTagInformationRead() const
{
  return m_fileRead;
}

/**
 * Check if file has a tag.
 *
 * @param tagNr tag number
 * @return true if a V2 tag is available.
 * @see isTagInformationRead()
 */
bool M4aFile::hasTag(Frame::TagNumber tagNr) const
{
  return tagNr == Frame::Tag_2 && !m_metadata.empty();
}

/**
 * Get file extension including the dot.
 *
 * @return file extension ".m4a".
 */
QString M4aFile::getFileExtension() const
{
  return QLatin1String(".m4a");
}

/**
 * Get technical detail information.
 *
 * @param info the detail information is returned here
 */
void M4aFile::getDetailInfo(DetailInfo& info) const
{
  if (m_fileRead && m_fileInfo.valid) {
    info.valid = true;
    info.format = QLatin1String("MP4");
    info.bitrate = m_fileInfo.bitrate;
    info.sampleRate = m_fileInfo.sampleRate;
    info.channels = m_fileInfo.channels;
    info.duration = m_fileInfo.duration;
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
unsigned M4aFile::getDuration() const
{
  if (m_fileRead && m_fileInfo.valid) {
    return m_fileInfo.duration;
  }
  return 0;
}

/**
 * Get the format of tag.
 *
 * @param tagNr tag number
 * @return "Vorbis".
 */
QString M4aFile::getTagFormat(Frame::TagNumber tagNr) const
{
  return hasTag(tagNr) ? QLatin1String("MP4") : QString();
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
bool M4aFile::getFrame(Frame::TagNumber tagNr, Frame::Type type, Frame& frame) const
{
  if (type < Frame::FT_FirstFrame || type > Frame::FT_LastV1Frame ||
      tagNr > 1)
    return false;

  if (tagNr == Frame::Tag_1) {
    frame.setValue(QString());
  } else {
    if (type == Frame::FT_Genre) {
      QString str(getTextField(QLatin1String("\251gen")));
      frame.setValue(str.isEmpty() ? getTextField(QLatin1String("gnre")) : str);
    } else {
      frame.setValue(getTextField(getNameForType(type)));
    }
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
bool M4aFile::setFrame(Frame::TagNumber tagNr, const Frame& frame)
{
  if (tagNr == Frame::Tag_2) {
    if ((frame.getType() == Frame::FT_Picture) ||
        (frame.getType() == Frame::FT_Other &&
                frame.getName() == QLatin1String("Chapters"))) {
      int idx = Frame::fromNegativeIndex(frame.getIndex());
      if (idx >= 0 && idx < m_extraFrames.size()) {
        Frame newFrame(frame);
        if ((frame.getType() == Frame::FT_Picture &&
             PictureFrame::areFieldsEqual(m_extraFrames[idx], newFrame)) ||
            (frame.getType() == Frame::FT_Other &&
             frame.getName() == QLatin1String("Chapters") &&
             areMp4ChaptersFieldsEqual(m_extraFrames[idx], newFrame))) {
          m_extraFrames[idx].setValueChanged(false);
        } else {
          m_extraFrames[idx] = newFrame;
          markTagChanged(tagNr, frame.getExtendedType());
        }
        return true;
      } else {
        return false;
      }
    }
    QString name = fixUpTagKey(frame.getInternalName(), TT_Mp4);
    auto it = m_metadata.find(name); // clazy:exclude=detaching-member
    if (it != m_metadata.end()) {
      if (frame.getType() != Frame::FT_Picture) {
        QByteArray str = frame.getValue().toUtf8();
        if (*it != str) {
          *it = str;
          markTagChanged(Frame::Tag_2, frame.getExtendedType());
        }
      } else {
        if (PictureFrame::getData(frame, *it)) {
          markTagChanged(Frame::Tag_2,
                         Frame::ExtendedType(Frame::FT_Picture, name));
        }
      }
      return true;
    }
  }

  // Try the basic method
  Frame::Type type = frame.getType();
  if (type < Frame::FT_FirstFrame || type > Frame::FT_LastV1Frame ||
      tagNr > 1)
    return false;

  if (tagNr == Frame::Tag_2) {
    if (type == Frame::FT_Genre) {
      QString str = frame.getValue();
      QString oldStr(getTextField(QLatin1String("\251gen")));
      if (oldStr.isEmpty()) {
        oldStr = getTextField(QLatin1String("gnre"));
      }
      if (str != oldStr) {
        int genreNum = Genres::getNumber(str);
        if (genreNum != 255) {
          const QString genreName(QLatin1String("gnre"));
          setTextField(genreName, str,
                       Frame::ExtendedType(Frame::FT_Genre, genreName));
          m_metadata.remove(QLatin1String("\251gen"));
        } else {
          const QString genreName(QLatin1String("\251gen"));
          setTextField(genreName, str,
                       Frame::ExtendedType(Frame::FT_Genre, genreName));
          m_metadata.remove(QLatin1String("gnre"));
        }
      }
    } else if (type == Frame::FT_Track) {
      int numTracks;
      int num = splitNumberAndTotal(frame.getValue(), &numTracks);
      if (num >= 0) {
        QString str;
        if (num != 0) {
          str.setNum(num);
          if (numTracks == 0)
            numTracks = getTotalNumberOfTracksIfEnabled();
          if (numTracks > 0) {
            str += QLatin1Char('/');
            str += QString::number(numTracks);
          }
        } else {
          str = QLatin1String("");
        }
        const QString trackName(QLatin1String("trkn"));
        setTextField(trackName, str,
                     Frame::ExtendedType(Frame::FT_Track, trackName));
      }
    } else {
      const QString fieldName = getNameForType(type);
      setTextField(fieldName, frame.getValue(),
                   Frame::ExtendedType(type, fieldName));
    }
  }
  return true;
}

/**
 * Add a frame in the tags.
 *
 * @param tagNr tag number
 * @param frame frame to add
 *
 * @return true if ok.
 */
bool M4aFile::addFrame(Frame::TagNumber tagNr, Frame& frame)
{
  if (tagNr == Frame::Tag_2) {
    Frame::ExtendedType extendedType = frame.getExtendedType();
    Frame::Type type = extendedType.getType();
    if (type == Frame::FT_Picture) {
      if (frame.getFieldList().empty()) {
        PictureFrame::setFields(frame);
      }
      frame.setIndex(Frame::toNegativeIndex(m_extraFrames.size()));
      m_extraFrames.append(frame);
      markTagChanged(tagNr, extendedType);
      return true;
    }
    if (type == Frame::FT_Other &&
        frame.getName() == QLatin1String("Chapters")) {
      if (frame.getFieldList().empty()) {
        setMp4ChaptersFields(frame);
      }
      frame.setIndex(Frame::toNegativeIndex(m_extraFrames.size()));
      m_extraFrames.append(frame);
      markTagChanged(Frame::Tag_2, extendedType);
      return true;;
    }
    QString name;
    if (type != Frame::FT_Other) {
      name = getNameForType(type);
      if (!name.isEmpty()) {
        extendedType = Frame::ExtendedType(type, name);
        frame.setExtendedType(extendedType);
      }
    }
    name = fixUpTagKey(frame.getInternalName(), TT_Mp4);
    m_metadata[name] = frame.getValue().toUtf8();
    markTagChanged(Frame::Tag_2, extendedType);
    return true;
  }
  return false;
}

/**
 * Delete a frame in the tags.
 *
 * @param tagNr tag number
 * @param frame frame to delete.
 *
 * @return true if ok.
 */
bool M4aFile::deleteFrame(Frame::TagNumber tagNr, const Frame& frame)
{
  if (tagNr == Frame::Tag_2) {
    if ((frame.getType() == Frame::FT_Picture) ||
        (frame.getType() == Frame::FT_Other &&
                frame.getName() == QLatin1String("Chapters"))) {
      int idx = Frame::fromNegativeIndex(frame.getIndex());
      if (idx >= 0 && idx < m_extraFrames.size()) {
        m_extraFrames.removeAt(idx);
        while (idx < m_extraFrames.size()) {
          m_extraFrames[idx].setIndex(Frame::toNegativeIndex(idx));
          ++idx;
        }
        markTagChanged(tagNr, frame.getExtendedType());
        return true;
      }
    }
    QString name(frame.getInternalName());
    auto it = m_metadata.find(name); // clazy:exclude=detaching-member
    if (it != m_metadata.end()) {
      m_metadata.erase(it);
      markTagChanged(Frame::Tag_2, frame.getExtendedType());
      return true;
    }
  }

  // Try the superclass method
  return TaggedFile::deleteFrame(tagNr, frame);
}

/**
 * Get all frames in tag.
 *
 * @param tagNr tag number
 * @param frames frame collection to set.
 */
void M4aFile::getAllFrames(Frame::TagNumber tagNr, FrameCollection& frames)
{
  if (tagNr == Frame::Tag_2) {
    frames.clear();
    QString name;
    QString value;
    int i = 0;
    for (auto it = m_metadata.constBegin(); it != m_metadata.constEnd(); ++it) {
      name = it.key();
      Frame::Type type = getTypeForName(name);
      value = QString::fromUtf8((*it).data(), (*it).size());
      frames.insert(Frame(type, value, name, i++));
    }
    for (auto it = m_extraFrames.constBegin(); it != m_extraFrames.constEnd(); ++it) {
      frames.insert(*it);
    }
    frames.addMissingStandardFrames();
    return;
  }

  TaggedFile::getAllFrames(tagNr, frames);
}

/**
 * Get a list of frame IDs which can be added.
 * @param tagNr tag number
 * @return list with frame IDs.
 */
QStringList M4aFile::getFrameIds(Frame::TagNumber tagNr) const
{
  if (tagNr != Frame::Tag_2)
    return QStringList();

  static const Frame::Type types[] = {
    Frame::FT_Title,
    Frame::FT_Artist,
    Frame::FT_Album,
    Frame::FT_Comment,
    Frame::FT_Compilation,
    Frame::FT_Date,
    Frame::FT_Track,
    Frame::FT_Genre,
#if MPEG4IP_MAJOR_MINOR_VERSION >= 0x0106
    Frame::FT_AlbumArtist,
#endif
    Frame::FT_Bpm,
    Frame::FT_Composer,
#if MPEG4IP_MAJOR_MINOR_VERSION >= 0x0109
    Frame::FT_Copyright,
#endif
    Frame::FT_Description,
    Frame::FT_Disc,
    Frame::FT_EncodedBy,
#if MPEG4IP_MAJOR_MINOR_VERSION >= 0x0109
    Frame::FT_EncoderSettings,
#endif
#if MPEG4IP_MAJOR_MINOR_VERSION >= 0x0105
    Frame::FT_Grouping,
#endif
#if MPEG4IP_MAJOR_MINOR_VERSION >= 0x0109
    Frame::FT_Lyrics,
#endif
    Frame::FT_Picture,
    Frame::FT_Rating
#if MPEG4IP_MAJOR_MINOR_VERSION >= 0x0109
    , Frame::FT_SortAlbum,
    Frame::FT_SortAlbumArtist,
    Frame::FT_SortArtist,
    Frame::FT_SortComposer,
    Frame::FT_SortName
#endif
  };

  QStringList lst;
  for (auto type : types) {
    lst.append(Frame::ExtendedType(type, QLatin1String("")). // clazy:exclude=reserve-candidates
               getName());
  }
#if MPEG4IP_MAJOR_MINOR_VERSION >= 0x0106
  lst << QLatin1String("pgap");
#endif
#if MPEG4IP_MAJOR_MINOR_VERSION >= 0x0109
  lst << QLatin1String("akID") << QLatin1String("apID") << QLatin1String("atID") << QLatin1String("catg") << QLatin1String("cnID") <<
    QLatin1String("geID") << QLatin1String("hdvd") << QLatin1String("keyw") << QLatin1String("ldes") << QLatin1String("pcst") <<
    QLatin1String("plID") << QLatin1String("purd") << QLatin1String("rtng") << QLatin1String("sfID") <<
    QLatin1String("sosn") << QLatin1String("stik") << QLatin1String("tven") <<
    QLatin1String("tves") << QLatin1String("tvnn") << QLatin1String("tvsh") << QLatin1String("tvsn") <<
    QLatin1String("purl") << QLatin1String("egid") << QLatin1String("cmID") << QLatin1String("xid ");
#endif
  lst << QLatin1String("Chapters");
  return lst;
}


/**
 * Read information about an MPEG-4 file.
 * @param fn file name
 * @return true if ok.
 */
bool M4aFile::FileInfo::read(MP4FileHandle handle)
{
  valid = false;
  uint32_t numTracks = MP4GetNumberOfTracks(handle);
  for (uint32_t i = 0; i < numTracks; ++i) {
    MP4TrackId trackId = MP4FindTrackId(handle, i);
    const char* trackType = MP4GetTrackType(handle, trackId);
    if (std::strcmp(trackType, MP4_AUDIO_TRACK_TYPE) == 0) {
      valid = true;
      bitrate = (MP4GetTrackBitRate(handle, trackId) + 500) / 1000;
      sampleRate = MP4GetTrackTimeScale(handle, trackId);
      duration = MP4ConvertFromTrackDuration(
        handle, trackId,
        MP4GetTrackDuration(handle, trackId), MP4_MSECS_TIME_SCALE) / 1000;
#if MPEG4IP_MAJOR_MINOR_VERSION >= 0x0109
      channels = MP4GetTrackAudioChannels(handle, trackId);
#else
      channels = 2;
#endif
      break;
    }
  }
  return valid;
}
