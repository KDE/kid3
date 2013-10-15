/**
 * \file frame.cpp
 * Generalized frame.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 25 Aug 2007
 *
 * Copyright (C) 2007-2013  Urs Fleisch
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

#include "frame.h"
#include <QMap>
#include <QRegExp>
#include <QCoreApplication>
#include "pictureframe.h"

namespace {

/**
 * Get name of frame from type.
 *
 * @param type type
 *
 * @return name.
 */
const char* getNameFromType(Frame::Type type)
{
  static const char* const names[] = {
    QT_TRANSLATE_NOOP("@default", "Title"),           // FT_Title,
    QT_TRANSLATE_NOOP("@default", "Artist"),          // FT_Artist,
    QT_TRANSLATE_NOOP("@default", "Album"),           // FT_Album,
    QT_TRANSLATE_NOOP("@default", "Comment"),         // FT_Comment,
    QT_TRANSLATE_NOOP("@default", "Date"),            // FT_Date,
    QT_TRANSLATE_NOOP("@default", "Track Number"),    // FT_Track,
    QT_TRANSLATE_NOOP("@default", "Genre"),           // FT_Genre,
                                  // FT_LastV1Frame = FT_Track,
    QT_TRANSLATE_NOOP("@default", "Album Artist"),    // FT_AlbumArtist
    QT_TRANSLATE_NOOP("@default", "Arranger"),        // FT_Arranger,
    QT_TRANSLATE_NOOP("@default", "Author"),          // FT_Author,
    QT_TRANSLATE_NOOP("@default", "BPM"),             // FT_Bpm,
    QT_TRANSLATE_NOOP("@default", "Catalog Number"),  // FT_CatalogNumber,
    QT_TRANSLATE_NOOP("@default", "Compilation"),     // FT_Compilation,
    QT_TRANSLATE_NOOP("@default", "Composer"),        // FT_Composer,
    QT_TRANSLATE_NOOP("@default", "Conductor"),       // FT_Conductor,
    QT_TRANSLATE_NOOP("@default", "Copyright"),       // FT_Copyright,
    QT_TRANSLATE_NOOP("@default", "Disc Number"),     // FT_Disc,
    QT_TRANSLATE_NOOP("@default", "Encoded-by"),      // FT_EncodedBy,
    QT_TRANSLATE_NOOP("@default", "Encoder Settings"), // FT_EncoderSettings,
    QT_TRANSLATE_NOOP("@default", "Encoding Time"),   // FT_EncodingTime,
    QT_TRANSLATE_NOOP("@default", "Grouping"),        // FT_Grouping,
    QT_TRANSLATE_NOOP("@default", "Initial Key"),     // FT_InitialKey,
    QT_TRANSLATE_NOOP("@default", "ISRC"),            // FT_Isrc,
    QT_TRANSLATE_NOOP("@default", "Language"),        // FT_Language,
    QT_TRANSLATE_NOOP("@default", "Lyricist"),        // FT_Lyricist,
    QT_TRANSLATE_NOOP("@default", "Lyrics"),          // FT_Lyrics,
    QT_TRANSLATE_NOOP("@default", "Media"),           // FT_Media,
    QT_TRANSLATE_NOOP("@default", "Mood"),            // FT_Mood,
    QT_TRANSLATE_NOOP("@default", "Original Album"),  // FT_OriginalAlbum,
    QT_TRANSLATE_NOOP("@default", "Original Artist"), // FT_OriginalArtist,
    QT_TRANSLATE_NOOP("@default", "Original Date"),   // FT_OriginalDate,
    QT_TRANSLATE_NOOP("@default", "Part"),            // FT_Part,
    QT_TRANSLATE_NOOP("@default", "Performer"),       // FT_Performer,
    QT_TRANSLATE_NOOP("@default", "Picture"),         // FT_Picture,
    QT_TRANSLATE_NOOP("@default", "Publisher"),       // FT_Publisher,
    QT_TRANSLATE_NOOP("@default", "Release Country"), // FT_ReleaseCountry,
    QT_TRANSLATE_NOOP("@default", "Remixer"),         // FT_Remixer,
    QT_TRANSLATE_NOOP("@default", "Sort Album"),      // FT_SortAlbum,
    QT_TRANSLATE_NOOP("@default", "Sort Album Artist"), // FT_SortAlbumArtist,
    QT_TRANSLATE_NOOP("@default", "Sort Artist"),     // FT_SortArtist,
    QT_TRANSLATE_NOOP("@default", "Sort Composer"),   // FT_SortComposer,
    QT_TRANSLATE_NOOP("@default", "Sort Name"),       // FT_SortName,
    QT_TRANSLATE_NOOP("@default", "Subtitle"),        // FT_Subtitle,
    QT_TRANSLATE_NOOP("@default", "Website"),         // FT_Website,
    QT_TRANSLATE_NOOP("@default", "WWW Audio File"),  // FT_WWWAudioFile,
    QT_TRANSLATE_NOOP("@default", "WWW Audio Source") // FT_WWWAudioSource,
                                  // FT_LastFrame = FT_WWWAudioSource
  };
  struct not_used { int array_size_check[
      sizeof(names) / sizeof(names[0]) == Frame::FT_LastFrame + 1
      ? 1 : -1 ]; };
  return type <= Frame::FT_LastFrame ? names[type] : "Unknown";
}

/**
 * Get type of frame from English name.
 *
 * @param name name, spaces and case are ignored
 *
 * @return type.
 */
Frame::Type getTypeFromName(QString name)
{
  static QMap<QString, int> strNumMap;
  if (strNumMap.empty()) {
    // first time initialization
    for (int i = 0; i <= Frame::FT_LastFrame; ++i) {
      Frame::Type type = static_cast<Frame::Type>(i);
      strNumMap.insert(QString::fromLatin1(getNameFromType(type)).
                       remove(QLatin1Char(' ')).toUpper(), type);
    }
  }
  QMap<QString, int>::const_iterator it =
    strNumMap.find(name.remove(QLatin1Char(' ')).toUpper());
  if (it != strNumMap.end()) {
    return static_cast<Frame::Type>(*it);
  }
  return Frame::FT_Other;
}

}

Frame::ExtendedType::ExtendedType(const QString& name) :
  m_type(getTypeFromName(name)), m_name(name)
{
}

Frame::ExtendedType::ExtendedType(Type type) :
  m_type(type), m_name(QString::fromLatin1(getNameFromType(type)))
{
}

/**
 * Get name of type.
 * @return name.
 */
QString Frame::ExtendedType::getName() const
{
  return m_type != FT_Other ? QString::fromLatin1(getNameFromType(m_type)) : m_name;
}

/**
 * Get translated name of type.
 * @return name.
 */
QString Frame::ExtendedType::getTranslatedName() const
{
  return m_type != FT_Other ? QCoreApplication::translate("@default", getNameFromType(m_type)) : m_name;
}


/**
 * Constructor.
 */
Frame::Frame() :
  m_index(-1), m_valueChanged(false)
{
}

/**
 * Constructor.
 */
Frame::Frame(Type type, const QString& value,
             const QString& name, int index) :
  m_extendedType(type, name), m_index(index), m_valueChanged(false),
  m_value(value)
{
}

/**
 * Constructor.
 * @param type  type and internal name
 * @param value value
 * @param index index inside tag, -1 if unknown
 */
Frame::Frame(const ExtendedType& type, const QString& value, int index) :
  m_extendedType(type), m_index(index), m_valueChanged(false), m_value(value)
{
}

/**
 * Destructor.
 */
Frame::~Frame()
{
}

/**
 * Set the value from a field in the field list.
 */
void Frame::setValueFromFieldList()
{
  if (!getFieldList().empty()) {
    QString text;
    for (Frame::FieldList::const_iterator fldIt = getFieldList().begin();
         fldIt != getFieldList().end();
         ++fldIt) {
      int id = (*fldIt).m_id;
      if (id == Frame::Field::ID_Text ||
          id == Frame::Field::ID_Description ||
          id == Frame::Field::ID_Url) {
        m_value = (*fldIt).m_value.toString();
        if (id == Frame::Field::ID_Text) {
          // highest priority, will not be overwritten
          break;
        }
      }
    }
  }
}

/**
 * Set a field in the field list from the value.
 */
void Frame::setFieldListFromValue()
{
  if (!fieldList().empty()) {
    Frame::FieldList::iterator it = fieldList().end();
    for (Frame::FieldList::iterator fldIt = fieldList().begin();
         fldIt != fieldList().end();
         ++fldIt) {
      int id = (*fldIt).m_id;
      if (id == Frame::Field::ID_Text ||
          id == Frame::Field::ID_Description ||
          id == Frame::Field::ID_Url) {
        it = fldIt;
        if (id == Frame::Field::ID_Text) {
          // highest priority, will not be overwritten
          break;
        }
      }
    }
    if (it != fieldList().end()) {
      (*it).m_value = m_value;
    }
  }
}

/**
 * Convert string (e.g. "track/total number of tracks") to number.
 *
 * @param str string to convert
 * @param ok  if not 0, true is returned here if conversion is ok
 *
 * @return number in string ignoring total after slash.
 */
int Frame::numberWithoutTotal(const QString& str, bool* ok)
{
  int slashPos = str.indexOf(QLatin1Char('/'));
  return slashPos == -1 ?
    str.toInt(ok) :
    str.left(slashPos).toInt(ok);
}

/**
 * Get the value of a field.
 *
 * @param id field ID
 *
 * @return field value, invalid if field not found.
 */
QVariant Frame::getFieldValue(Field::Id id) const
{
  for (FieldList::const_iterator it = m_fieldList.begin();
       it != m_fieldList.end();
       ++it) {
    if ((*it).m_id == id) {
      return (*it).m_value;
    }
  }
  return QVariant();
}

/**
 * Set value as string and mark it as changed if it is changed.
 * This method will avoid setting "different" representations.
 * @param value value as string
 */
void Frame::setValueIfChanged(const QString& value)
{
  if (value != differentRepresentation()) {
    QString oldValue(getValue());
    if (value != oldValue && !(value.isEmpty() && oldValue.isEmpty())) {
      setValue(value);
      setValueChanged();
    }
  }
}

/**
 * Check if the fields in another frame are equal.
 *
 * @param other other frame
 *
 * @return true if equal.
 */
bool Frame::isEqual(const Frame& other) const
{
  if (getType() != other.getType() || getValue() != other.getValue())
    return false;

  const FieldList& otherFieldList = other.getFieldList();
  if (m_fieldList.size() != otherFieldList.size())
    return false;

  FieldList::const_iterator thisIt, otherIt;
  for (thisIt = m_fieldList.constBegin(), otherIt = otherFieldList.constBegin();
       thisIt != m_fieldList.constEnd() && otherIt != otherFieldList.constEnd();
       ++thisIt, ++otherIt) {
    if (thisIt->m_id != otherIt->m_id || thisIt->m_value != otherIt->m_value) {
      return false;
    }
  }

  return true;
}

#ifndef QT_NO_DEBUG
/**
 * Convert frame type to string.
 * @param type frame type
 * @return string representation of type.
 */
static const char* frameTypeToString(Frame::Type type)
{
  static const char* const typeStr[] = {
    "FT_Title",
    "FT_Artist",
    "FT_Album",
    "FT_Comment",
    "FT_Date",
    "FT_Track",
    "FT_Genre",
    "FT_AlbumArtist",
    "FT_Arranger",
    "FT_Author",
    "FT_Bpm",
    "FT_CatalogNumber",
    "FT_Compilation",
    "FT_Composer",
    "FT_Conductor",
    "FT_Copyright",
    "FT_Disc",
    "FT_EncodedBy",
    "FT_EncoderSettings",
    "FT_EncodingTime",
    "FT_Grouping",
    "FT_InitialKey",
    "FT_Isrc",
    "FT_Language",
    "FT_Lyricist",
    "FT_Lyrics",
    "FT_Media",
    "FT_Mood",
    "FT_OriginalAlbum",
    "FT_OriginalArtist",
    "FT_OriginalDate",
    "FT_Part",
    "FT_Performer",
    "FT_Picture",
    "FT_Publisher",
    "FT_ReleaseCountry",
    "FT_Remixer",
    "FT_SortAlbum",
    "FT_SortAlbumArtist",
    "FT_SortArtist",
    "FT_SortComposer",
    "FT_SortName",
    "FT_Subtitle",
    "FT_Website",
    "FT_WWWAudioFile",
    "FT_WWWAudioSource",
    "FT_Other",
    "FT_UnknownFrame"
  };
  return type >= 0 && (unsigned)type <= sizeof(typeStr) / sizeof(typeStr[0]) ?
    typeStr[type] : "ILLEGAL";
}

/**
 * Convert field id to string.
 * @param id field id
 * @return string representation of id.
 */
static const char* fieldIdToString(int id)
{
  static const char* const idStr[] = {
    "ID_NoField",
    "ID_TextEnc",
    "ID_Text",
    "ID_Url",
    "ID_Data",
    "ID_Description",
    "ID_Owner",
    "ID_Email",
    "ID_Rating",
    "ID_Filename",
    "ID_Language",
    "ID_PictureType",
    "ID_ImageFormat",
    "ID_MimeType",
    "ID_Counter",
    "ID_Id",
    "ID_VolumeAdj",
    "ID_NumBits",
    "ID_VolChgRight",
    "ID_VolChgLeft",
    "ID_PeakVolRight",
    "ID_PeakVolLeft",
    "ID_TimestampFormat",
    "ID_ContentType"
  };
  return id >= 0 && (unsigned)id <= sizeof(idStr) / sizeof(idStr[0]) ?
    idStr[id] : "ILLEGAL";
}

/**
 * Get string representation of variant.
 * @param val variant value
 * @return string representation.
 */
static QString variantToString(const QVariant& val)
{
  if (val.type() == QVariant::ByteArray) {
    return QString(QLatin1String("ByteArray of %1 bytes")).arg(val.toByteArray().size());
  } else {
    return val.toString();
  }
}

/**
 * Dump contents of frame to debug console.
 */
void Frame::dump() const
{
  qDebug("Frame: name=%s, value=%s, type=%s, index=%d, valueChanged=%u",
         getInternalName().toLatin1().data(), m_value.toLatin1().data(),
         frameTypeToString(getType()), m_index,
         m_valueChanged);
  qDebug("  fields=");
  for (FieldList::const_iterator it = m_fieldList.begin();
       it != m_fieldList.end();
       ++it) {
    qDebug("  Field: id=%s, value=%s", fieldIdToString((*it).m_id),
           variantToString((*it).m_value).toLatin1().data());
  }
}
#endif


/**
 * Bit mask containing the bits of all frame types which shall be used as
 * quick access frames.
 * This mask has to be handled like FrameFilter::m_enabledFrames.
 */
quint64 FrameCollection::s_quickAccessFrames =
    FrameCollection::DEFAULT_QUICK_ACCESS_FRAMES;

/**
 * Set values which are different inactive.
 *
 * @param others frames to compare, will be modified
 */
void FrameCollection::filterDifferent(FrameCollection& others)
{
  QByteArray frameData, othersData;
  for (iterator it = begin(); it != end(); ++it) {
    Frame& frame = const_cast<Frame&>(*it);
    // This frame list is not tied to a specific file, so the
    // index is not valid.
    frame.setIndex(-1);
    iterator othersIt = others.find(frame);
    if (othersIt == others.end() ||
        (frame.getType() != Frame::FT_Picture &&
         frame.getValue() != othersIt->getValue()) ||
        (frame.getType() == Frame::FT_Picture &&
         !(PictureFrame::getData(frame, frameData) &&
           PictureFrame::getData(*othersIt, othersData) &&
           frameData == othersData))) {
      frame.setDifferent();
    }
    while (othersIt != others.end() && !(frame < *othersIt)) {
      // Mark equal frames as already handled using index -2, used below.
      // This is probably faster than removing the frames.
      const_cast<Frame&>(*othersIt).setIndex(-2);
      ++othersIt;
    }
  }

  // Insert frames which are in others but not in this (not marked as already
  // handled by index -2) as different frames.
  for (iterator othersIt = others.begin();
       othersIt != others.end();
       ++othersIt) {
    if (othersIt->getIndex() != -2) {
      Frame& frame = const_cast<Frame&>(*othersIt);
      frame.setIndex(-1);
      frame.setDifferent();
      insert(frame);
    }
  }
}

/**
 * Add standard frames which are missing.
 */
void FrameCollection::addMissingStandardFrames()
{
  quint64 mask;
  int i;
  for (i = Frame::FT_FirstFrame, mask = 1ULL;
       i <= Frame::FT_LastFrame;
       ++i, mask <<= 1) {
    if (s_quickAccessFrames & mask) {
      Frame frame(static_cast<Frame::Type>(i), QString::null, QString::null, -1);
      FrameCollection::const_iterator it = find(frame);
      if (it == end()) {
        insert(frame);
      }
    }
  }
}

/**
 * Copy enabled frames.
 *
 * @param flt filter with enabled frames
 *
 * @return copy with enabled frames.
 */
FrameCollection FrameCollection::copyEnabledFrames(const FrameFilter& flt) const
{
  FrameCollection frames;
  for (const_iterator it = begin();
       it != end();
       ++it) {
    if (flt.isEnabled(it->getType(), it->getName())) {
      Frame frame = *it;
      frame.setIndex(-1);
      frames.insert(frame);
    }
  }
  return frames;
}

/**
 * Remove all frames which are not enabled from the collection.
 *
 * @param flt filter with enabled frames
 */
void FrameCollection::removeDisabledFrames(const FrameFilter& flt)
{
  for (iterator it = begin();
       it != end();) {
    if (!flt.isEnabled(it->getType(), it->getName())) {
      erase(it++);
    } else {
      ++it;
    }
  }
}

/**
 * Copy frames which are empty or inactive from other frames.
 * This can be used to merge two frame collections.
 *
 * @param frames other frames
 */
void FrameCollection::merge(const FrameCollection& frames)
{
  for (const_iterator otherIt = frames.begin();
       otherIt != frames.end();
       ++otherIt) {
    iterator it = find(*otherIt);
    if (it != end()) {
      QString value(otherIt->getValue());
      Frame& frameFound = const_cast<Frame&>(*it);
      if (frameFound.getValue().isEmpty() && !value.isEmpty()) {
        frameFound.setValueIfChanged(value);
      }
    } else {
      Frame frame(*otherIt);
      frame.setIndex(-1);
      frame.setValueChanged(true);
      insert(frame);
    }
  }
}

/**
 * Check if the standard tags are empty or inactive.
 *
 * @return true if empty or inactive.
 */
bool FrameCollection::isEmptyOrInactive() const
{
  return
    getTitle().isEmpty() &&
    getArtist().isEmpty() &&
    getAlbum().isEmpty() &&
    getComment().isEmpty() &&
    getYear() <= 0 &&
    getTrack() <= 0 &&
    getGenre().isEmpty();
}

/**
 * Search for a frame only by name.
 *
 * @param name the name of the frame to find, a case-insensitive search for
 *             the first name starting with this string is performed
 *
 * @return iterator or end() if not found.
 */
FrameCollection::const_iterator FrameCollection::searchByName(
    const QString& name) const
{
  if (name.isEmpty())
    return end();

  const_iterator it;
  QString ucName = name.toUpper().remove(QLatin1Char('/'));
  int len = ucName.length();
  for (it = begin(); it != end(); ++it) {
    QString ucFrameName(it->getName().toUpper().remove(QLatin1Char('/')));
    if (ucName == ucFrameName.left(len)) {
      break;
    }
    int nlPos = ucFrameName.indexOf(QLatin1Char('\n'));
    if (nlPos > 0 && ucName == ucFrameName.mid(nlPos + 1, len)) {
      // Description in TXXX, WXXX, COMM, PRIV matches
      break;
    }
  }
  return it;
}

/**
 * Find a frame by name.
 *
 * @param name  the name of the frame to find, if the exact name is not
 *              found, a case-insensitive search for the first name
 *              starting with this string is performed
 *
 * @return iterator or end() if not found.
 */
FrameCollection::const_iterator FrameCollection::findByName(const QString& name) const
{
  Frame frame(Frame::ExtendedType(name), QLatin1String(""), -1);
  const_iterator it = find(frame);
  if (it != end()) {
    return it;
  }
  return searchByName(name);
}

/**
 * Find a frame by type or name.
 *
 * @param type  type and name of the frame to find, if the exact name is not
 *              found, a case-insensitive search for the first name
 *              starting with this string is performed
 *
 * @return iterator or end() if not found.
 */
FrameCollection::const_iterator FrameCollection::findByExtendedType(
    const Frame::ExtendedType& type) const
{
  Frame frame(type, QLatin1String(""), -1);
  const_iterator it = find(frame);
  if (it == end()) {
    it = searchByName(frame.getInternalName());
  }
  return it;
}

/**
 * Find a frame by index.
 *
 * @param index the index in the frame, see \ref Frame::getIndex()
 *
 * @return iterator or end() if not found.
 */
FrameCollection::const_iterator FrameCollection::findByIndex(int index) const
{
  const_iterator it;
  for (it = begin(); it != end(); ++it) {
    if (it->getIndex() == index) {
      break;
    }
  }
  return it;
}

/**
 * Get value by type.
 *
 * @param type type
 *
 * @return value, QString::null if not found.
 */
QString FrameCollection::getValue(Frame::Type type) const
{
  Frame frame(type, QLatin1String(""), QLatin1String(""), -1);
  const_iterator it = find(frame);
  return it != end() ? it->getValue() : QString::null;
}

/**
 * Get value by type and name.
 *
 * @param type  type and name of the frame to find, if the exact name is not
 *              found, a case-insensitive search for the first name
 *              starting with this string is performed
 *
 * @return value, QString::null if not found.
 */
QString FrameCollection::getValue(const Frame::ExtendedType& type) const
{
  const_iterator it = findByExtendedType(type);
  return it != end() ? it->getValue() : QString::null;
}

/**
 * Set value by type.
 *
 * @param type type
 * @param value value, nothing is done if QString::null
 */
void FrameCollection::setValue(Frame::Type type, const QString& value)
{
  if (!value.isNull()) {
    Frame frame(type, QLatin1String(""), QLatin1String(""), -1);
    iterator it = find(frame);
    if (it != end()) {
      Frame& frameFound = const_cast<Frame&>(*it);
      frameFound.setValueIfChanged(value);
    } else {
      frame.setValueIfChanged(value);
      insert(frame);
    }
  }
}

/**
 * Set value by type and name.
 *
 * @param type  type and name of the frame to find, if the exact name is not
 *              found, a case-insensitive search for the first name
 *              starting with this string is performed
 * @param value value, nothing is done if QString::null
 */
void FrameCollection::setValue(const Frame::ExtendedType& type,
                               const QString& value)
{
  if (!value.isNull()) {
    Frame frame(type, QLatin1String(""), -1);
    const_iterator it = find(frame);
    if (it == end()) {
      it = searchByName(type.getInternalName());
    }
    if (it != end()) {
      Frame& frameFound = const_cast<Frame&>(*it);
      frameFound.setValueIfChanged(value);
    } else {
      frame.setValueIfChanged(value);
      insert(frame);
    }
  }
}

/**
 * Get integer value by type.
 *
 * @param type type
 *
 * @return value, 0 if empty, -1 if not found.
 */
int FrameCollection::getIntValue(Frame::Type type) const
{
  QString str = getValue(type);
  return str.isNull() ? -1 : str.toInt();
}

/**
 * Set integer value by type.
 *
 * @param type type
 * @param value value, 0 to set empty, nothing is done if -1
 */
void FrameCollection::setIntValue(Frame::Type type, int value)
{
  if (value != -1) {
    QString str = value != 0 ? QString::number(value) : QLatin1String("");
    setValue(type, str);
  }
}

/**
 * Compare the frames with another frame collection and mark the value as
 * changed on frames which are different.
 *
 * @param other other frame collection
 */
void FrameCollection::markChangedFrames(const FrameCollection& other)
{
  for (FrameCollection::iterator it = begin(); it != end(); ++it) {
    const_iterator otherIt = it->getIndex() != -1
        ? other.findByIndex(it->getIndex())
        : other.find(*it);
    Frame& frame = const_cast<Frame&>(*it);
    frame.setValueChanged(!(otherIt != other.end() && otherIt->isEqual(*it)));
  }
}

#ifndef QT_NO_DEBUG
/**
 * Dump contents of frame collection to debug console.
 */
void FrameCollection::dump() const
{
  qDebug("FrameCollection:");
  for (const_iterator it = begin();
       it != end();
       ++it) {
    (*it).dump();
  }
}
#endif


/**
 * Constructor.
 * All frames are disabled
 */
FrameFilter::FrameFilter() : m_enabledFrames(0) {}

/**
 * Destructor.
 */
FrameFilter::~FrameFilter() {}

/**
 * Enable all frames.
 */
void FrameFilter::enableAll()
{
  m_enabledFrames = FTM_AllFrames;
  m_disabledOtherFrames.clear();
}

/**
 * Check if all fields are true.
 *
 * @return true if all fields are true.
 */
bool FrameFilter::areAllEnabled() const
{
  return (m_enabledFrames & FTM_AllFrames) == FTM_AllFrames &&
    m_disabledOtherFrames.empty();
}

/**
 * Check if frame is enabled.
 *
 * @param type frame type
 * @param name frame name
 *
 * @return true if frame is enabled.
 */
bool FrameFilter::isEnabled(Frame::Type type, const QString& name) const
{
  if (type <= Frame::FT_LastFrame) {
    return (m_enabledFrames & (1ULL << type)) != 0;
  } else if (!name.isEmpty()) {
    std::set<QString>::const_iterator it = m_disabledOtherFrames.find(name);
    return it == m_disabledOtherFrames.end();
  } else {
    return true;
  }
}

/**
 * Enable or disable frame.
 *
 * @param type frame type
 * @param name frame name
 * @param en true to enable
 */
void FrameFilter::enable(Frame::Type type, const QString& name, bool en)
{
  if (type <= Frame::FT_LastFrame) {
    if (en) {
      m_enabledFrames |= (1ULL << type);
    } else {
      m_enabledFrames &= ~(1ULL << type);
    }
  } else if (!name.isEmpty()) {
    if (en) {
      std::set<QString>::iterator it = m_disabledOtherFrames.find(name);
      if (it != m_disabledOtherFrames.end()) {
        m_disabledOtherFrames.erase(it);
      }
    } else {
      m_disabledOtherFrames.insert(name);
    }
  }
}


/**
 * Constructor.
 *
 * @param frames frame collection
 * @param str    string with format codes
 */
FrameFormatReplacer::FrameFormatReplacer(
  const FrameCollection& frames, const QString& str) :
  FormatReplacer(str), m_frames(frames) {}

/**
 * Destructor.
 */
FrameFormatReplacer::~FrameFormatReplacer() {}

/**
 * Replace a format code (one character %c or multiple characters %{chars}).
 * Supported format fields:
 * %s title (song)
 * %l album
 * %a artist
 * %c comment
 * %y year
 * %t track, two digits, i.e. leading zero if < 10
 * %T track, without leading zeroes
 * %g genre
 *
 * @param code format code
 *
 * @return replacement string,
 *         QString::null if code not found.
 */
QString FrameFormatReplacer::getReplacement(const QString& code) const
{
  QString result;
  QString name;

  if (code.length() == 1) {
    static const struct {
      char shortCode;
      const char* longCode;
    } shortToLong[] = {
      { 's', "title" },
      { 'l', "album" },
      { 'a', "artist" },
      { 'c', "comment" },
      { 'y', "year" },
      { 't', "track" },
      { 'T', "tracknumber" },
      { 'g', "genre" }
    };
    const char c = code[0].toLatin1();
    for (unsigned i = 0; i < sizeof(shortToLong) / sizeof(shortToLong[0]); ++i) {
      if (shortToLong[i].shortCode == c) {
        name = QString::fromLatin1(shortToLong[i].longCode);
        break;
      }
    }
  } else if (code.length() > 1) {
    name = code;
  }

  if (!name.isNull()) {
    QString lcName(name.toLower());
    int fieldWidth = lcName == QLatin1String("track") ? 2 : -1;
    if (lcName == QLatin1String("year")) {
      name = QLatin1String("date");
    } else if (lcName == QLatin1String("tracknumber")) {
      name = QLatin1String("track number");
    }
    int len = lcName.length();
    if (len > 2 && lcName.at(len - 2) == QLatin1Char('.') &&
        lcName.at(len - 1) >= QLatin1Char('0') && lcName.at(len - 1) <= QLatin1Char('9')) {
      fieldWidth = lcName.at(len - 1).toLatin1() - '0';
      lcName.truncate(len - 2);
      name.truncate(len - 2);
    }

    FrameCollection::const_iterator it = m_frames.findByName(name);
    if (it != m_frames.end()) {
      result = it->getValue();
      if (result.isNull()) {
        // code was found, but value is empty
        result = QLatin1String("");
      }
      if (it->getType() == Frame::FT_Picture && result.isEmpty()) {
        QVariant fieldValue = it->getFieldValue(Frame::Field::ID_Data);
        if (fieldValue.isValid() && fieldValue.toByteArray().size() > 0) {
          // If there is a picture without description, return "1", so that
          // an empty value indicates "no picture"
          result = QLatin1String("1");
        }
      }
    }

    if (lcName == QLatin1String("year")) {
      QRegExp yearRe(QLatin1String("^\\d{4}-\\d{2}"));
      if (yearRe.indexIn(result) == 0) {
        result.truncate(4);
      }
    }

    if (fieldWidth > 0) {
      bool ok;
      int nr = Frame::numberWithoutTotal(result, &ok);
      if (ok) {
        result.sprintf("%0*d", fieldWidth, nr);
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
QString FrameFormatReplacer::getToolTip(bool onlyRows)
{
  QString str;
  if (!onlyRows) str += QLatin1String("<table>\n");

  str += QLatin1String("<tr><td>%s</td><td>%{title}</td><td>");
  str += QCoreApplication::translate("@default", "Title");
  str += QLatin1String("</td></tr>\n");

  str += QLatin1String("<tr><td>%l</td><td>%{album}</td><td>");
  str += QCoreApplication::translate("@default", "Album");
  str += QLatin1String("</td></tr>\n");

  str += QLatin1String("<tr><td>%a</td><td>%{artist}</td><td>");
  str += QCoreApplication::translate("@default", "Artist");
  str += QLatin1String("</td></tr>\n");

  str += QLatin1String("<tr><td>%c</td><td>%{comment}</td><td>");
  str += QCoreApplication::translate("@default", "Comment");
  str += QLatin1String("</td></tr>\n");

  str += QLatin1String("<tr><td>%y</td><td>%{year}</td><td>");
  str += QCoreApplication::translate("@default", QT_TRANSLATE_NOOP("@default", "Year"));
  str += QLatin1String("</td></tr>\n");

  str += QLatin1String("<tr><td>%t</td><td>%{track}</td><td>");
  str += QCoreApplication::translate("@default", QT_TRANSLATE_NOOP("@default", "Track"));
  str += QLatin1String(" &quot;01&quot;</td></tr>\n");

  str += QLatin1String("<tr><td>%t</td><td>%{track.3}</td><td>");
  str += QCoreApplication::translate("@default", QT_TRANSLATE_NOOP("@default", "Track"));
  str += QLatin1String(" &quot;001&quot;</td></tr>\n");

  str += QLatin1String("<tr><td>%T</td><td>%{tracknumber}</td><td>");
  str += QCoreApplication::translate("@default", QT_TRANSLATE_NOOP("@default", "Track"));
  str += QLatin1String(" &quot;1&quot;</td></tr>\n");

  str += QLatin1String("<tr><td>%g</td><td>%{genre}</td><td>");
  str += QCoreApplication::translate("@default", "Genre");
  str += QLatin1String("</td></tr>\n");

  if (!onlyRows) str += QLatin1String("</table>\n");
  return str;
}
