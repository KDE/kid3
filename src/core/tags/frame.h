/**
 * \file frame.h
 * Generalized frame.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 25 Aug 2007
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

#pragma once

#include <QString>
#include <QVariant>
#include <QList>
#include <QSet>
#include <QHash>
#include <set>
#include "formatreplacer.h"
#include "framenotice.h"
#include "kid3api.h"

/** Generalized frame. */
class KID3_CORE_EXPORT Frame {
  Q_GADGET
  Q_ENUMS(Type)
  Q_ENUMS(FieldId)
  Q_ENUMS(TextEncoding)
  Q_ENUMS(PictureType)
  Q_ENUMS(TagVersion)
  Q_ENUMS(TagNumber)
public:
  /** Generalized frame types. */
  enum Type {
    FT_Title,
    FT_FirstFrame = FT_Title,
    FT_Artist,
    FT_Album,
    FT_Comment,
    FT_Date,
    FT_Track,
    FT_Genre,
    FT_LastV1Frame = FT_Genre,
    FT_AlbumArtist,
    FT_Arranger,
    FT_Author,
    FT_Bpm,
    FT_CatalogNumber,
    FT_Compilation,
    FT_Composer,
    FT_Conductor,
    FT_Copyright,
    FT_Disc,
    FT_EncodedBy,
    FT_EncoderSettings,
    FT_EncodingTime,
    FT_Grouping,
    FT_InitialKey,
    FT_Isrc,
    FT_Language,
    FT_Lyricist,
    FT_Lyrics,
    FT_Media,
    FT_Mood,
    FT_OriginalAlbum,
    FT_OriginalArtist,
    FT_OriginalDate,
    FT_Description,
    FT_Performer,
    FT_Picture,
    FT_Publisher,
    FT_ReleaseCountry,
    FT_Remixer,
    FT_SortAlbum,
    FT_SortAlbumArtist,
    FT_SortArtist,
    FT_SortComposer,
    FT_SortName,
    FT_Subtitle,
    FT_Website,
    FT_WWWAudioFile,
    FT_WWWAudioSource,
    FT_ReleaseDate,
    FT_Rating,
    FT_Work,
    FT_LastFrame = FT_Work,
    FT_Other,
    FT_UnknownFrame
  };

  /**
   * Types of fields in a frame, must be the same as id3lib's ID3_FieldID.
   **/
  enum FieldId {
    ID_NoField,
    ID_TextEnc,
    ID_Text,
    ID_Url,
    ID_Data,
    ID_Description,
    ID_Owner,
    ID_Email,
    ID_Rating,
    ID_Filename,
    ID_Language,
    ID_PictureType,
    ID_ImageFormat,
    ID_MimeType,
    ID_Counter,
    ID_Id,
    ID_VolumeAdj,
    ID_NumBits,
    ID_VolChgRight,
    ID_VolChgLeft,
    ID_PeakVolRight,
    ID_PeakVolLeft,
    ID_TimestampFormat,
    ID_ContentType,

    // These are additional fields used by TagLib
    ID_Price,
    ID_Date,
    ID_Seller,

    // Additional field for METADATA_BLOCK_PICTURE
    ID_ImageProperties,

    // Type of subframe in CTOC and CHAP frames
    ID_Subframe
  };

  /** Text encoding for fields of type ID_TextEnc. */
  enum TextEncoding {
    TE_ISO8859_1 = 0,
    TE_UTF16 = 1,
    TE_UTF16BE = 2,
    TE_UTF8 = 3
  };

  /** Picture type, compatible with ID3v2 and FLAC. */
  enum PictureType {
    PT_Other = 0,
    PT_Png32Icon = 1,
    PT_OtherIcon = 2,
    PT_CoverFront = 3,
    PT_CoverBack = 4,
    PT_LeafletPage = 5,
    PT_Media = 6,
    PT_LeadArtist = 7,
    PT_Artist = 8,
    PT_Conductor = 9,
    PT_Band = 10,
    PT_Composer = 11,
    PT_Lyricist = 12,
    PT_RecordingLocation = 13,
    PT_DuringRecording = 14,
    PT_DuringPerformance = 15,
    PT_Video = 16,
    PT_Fish = 17,
    PT_Illustration = 18,
    PT_ArtistLogo = 19,
    PT_PublisherLogo = 20
  };

  /** Supported tags. */
  enum TagNumber {
    Tag_1,              /**< First tag */
    Tag_2,              /**< Second tag */
    Tag_3,              /**< Third tag */
    Tag_NumValues,      /**< Total number of tags */

    // Special uses of tags
    Tag_Id3v1 = Tag_1,  /**< Tag which can be ID3v1 tag */
    Tag_Id3v2 = Tag_2,  /**< Tag which can be ID3v2 tag */
    Tag_Picture = Tag_2 /**< Tag used for pictures */
  };

  /** Tag version contained in track data. */
  enum TagVersion {
    TagNone = 0, /**< Empty or imported and not from a tag */
    TagV1 = 1 << Tag_1,   /**< Tag 1 */
    TagV2 = 1 << Tag_2,   /**< Tag 2 */
    TagV3 = 1 << Tag_3,   /**< Tag 3 */
    /** Tag 1 and 2 or merged from tag 2 and tag 1 (where tag 2 is not set) */
    TagV2V1 = TagV1 | TagV2,
    TagVAll = TagV1 | TagV2 | TagV3 /**< All tags */
  };

  /**
   * Cast a mask of tag version bits to a TagVersion enum.
   * @param tagMask tag mask (bit 0 for tag 1, bit 1 for tag 2)
   * @return tag version mask.
   */
  static TagVersion tagVersionCast(int tagMask) {
    return static_cast<TagVersion>(tagMask & TagVAll);
  }

  /**
   * @brief Cast an integer to a tag number.
   * @param nr number
   * @return tag number, Tag_NumValues if invalid.
   */
  static TagNumber tagNumberCast(int nr) {
    return nr >= Tag_1 && nr < Tag_NumValues
        ? static_cast<TagNumber>(nr) : Tag_NumValues;
  }

  /**
   * Get a tag mask from a tag number.
   * @param tagNr tag number
   * @return tag version mask.
   */
  static TagVersion tagVersionFromNumber(TagNumber tagNr) {
    return tagNr < Tag_NumValues
        ? static_cast<TagVersion>(1 << tagNr) : TagNone;
  }

  /**
   * Get list of available tag versions with translated description.
   * @return tag version/description pairs.
   */
  static const QList<QPair<TagVersion, QString> > availableTagVersions();

  /**
   * Get highest priority tag number which is set in a tag mask.
   * @param tagMask tag mask with bits set for tags
   * @return tag number, Tag_NumValues if no tag is set.
   */
  static TagNumber tagNumberFromMask(TagVersion tagMask) {
    return (tagMask & TagV2)
        ? Tag_2 : (tagMask & TagV1)
          ? Tag_1 : (tagMask & TagV3)
            ? Tag_3 : Tag_NumValues;
  }

  /**
   * Get tag numbers which are set in a tag mask, ordered from highest
   * to lowest priority.
   * @param tagMask tag mask with bits set for tags
   * @return list of tag numbers.
   */
  static const QList<TagNumber> tagNumbersFromMask(TagVersion tagMask) {
    QList<TagNumber> result;
    if (tagMask & TagV2) result << Tag_2;
    if (tagMask & TagV1) result << Tag_1;
    if (tagMask & TagV3) result << Tag_3;
    return result;
  }

  /**
   * Get all tag numbers, ordered from highest to lowest priority.
   * @return list of tag numbers.
   */
  static const QList<TagNumber> allTagNumbers() {
    return tagNumbersFromMask(TagVAll);
  }

  /**
   * Get string representation for tag number.
   * @param tagNr tag number
   * @return "1" for Tag_1, "2" for Tag_2, ..., null if invalid.
   */
  static QString tagNumberToString(TagNumber tagNr);

  /**
   * Get tag number from string representation.
   * @param str string representation
   * @return Tag_1 for "1", Tag_2 for "2", ..., Tag_NumValues if invalid.
   */
  static TagNumber tagNumberFromString(const QString& str);

/** for loop through all TagNumber values. */
#define FOR_ALL_TAGS(variable) \
  for (Frame::TagNumber variable = Frame::Tag_1; \
       variable < Frame::Tag_NumValues; \
       variable = static_cast<Frame::TagNumber>(variable + 1))

/** for loop through TagNumber values set in mask. */
#define FOR_TAGS_IN_MASK(variable, mask) \
  FOR_ALL_TAGS(variable) \
    if ((mask) & (1 << variable))

  /** Field in frame. */
  struct KID3_CORE_EXPORT Field {
    /**
     * Equality operator.
     * @param rhs right hand side to compare
     * @return true if this == rhs.
     */
    bool operator==(const Field& rhs) const {
      return m_id == rhs.m_id && m_value == rhs.m_value;
    }

    int m_id;         /**< type of field. */
    QVariant m_value; /**< value of field. */

    /**
     * Get a translated string for a field ID.
     *
     * @param type field ID type
     *
     * @return field ID type, null string if unknown.
     */
    static QString getFieldIdName(FieldId type);

    /**
     * List of field ID strings, NULL terminated.
     */
    static const char* const* getFieldIdNames();

    /**
     * Get field ID from field name.
     * @param fieldName name of field, can be English or translated
     * @return field ID, ID_NoField if not found.
     */
    static FieldId getFieldId(const QString& fieldName);

    /**
     * Get a translated string for a text encoding.
     *
     * @param type text encoding type
     *
     * @return text encoding type, null string if unknown.
     */
    static QString getTextEncodingName(TextEncoding type);

    /**
     * List of text encoding strings, NULL terminated.
     */
    static const char* const* getTextEncodingNames();

    /**
     * Get a translated string for a timestamp format.
     *
     * @param type timestamp format type
     *
     * @return timestamp format type, null string if unknown.
     */
    static QString getTimestampFormatName(int type);

    /**
     * List of timestamp format strings, NULL terminated.
     */
    static const char* const* getTimestampFormatNames();

    /**
     * Get a translated string for a content type.
     *
     * @param type content type
     *
     * @return content type, null string if unknown.
     */
    static QString getContentTypeName(int type);

    /**
     * List of content type strings, NULL terminated.
     */
    static const char* const* getContentTypeNames();

    /**
     * Compare two field lists in a tolerant way.
     * This function can be used instead of the standard QList equality
     * operator if the field lists can be from different tag formats, which
     * may not all support the same field types.
     * @param fl1 first field list
     * @param fl2 second field list
     * @return true if they are similar enough.
     */
    static bool fuzzyCompareFieldLists(const QList<Field>& fl1,
                                       const QList<Field>& fl2);

  };

  /** list of fields. */
  typedef QList<Field> FieldList;

  /**
   * Type and name of frame.
   */
  class KID3_CORE_EXPORT ExtendedType {
  public:
    /**
     * Constructor.
     */
    ExtendedType() : m_type(FT_UnknownFrame) {}

    /**
     * Constructor.
     * @param type type
     * @param name internal name
     */
    ExtendedType(Type type, const QString& name) : m_type(type), m_name(name) {}

    /**
     * Constructor.
     * @param name internal name
     */
    explicit ExtendedType(const QString& name);

    /**
     * Constructor.
     * @param type type
     */
    explicit ExtendedType(Type type);

    /**
     * Get name of type.
     * @return name.
     */
    QString getName() const;

    /**
     * Get translated name of type.
     * @return name.
     */
    QString getTranslatedName() const;

    /**
     * Get internal name of type.
     * @return name.
     */
    QString getInternalName() const { return m_name; }

    /**
     * Less than operator.
     * @param rhs right hand side to compare
     * @return true if this < rhs.
     */
    bool operator<(const ExtendedType& rhs) const {
      return m_type < rhs.m_type ||
             (m_type == FT_Other && m_type == rhs.m_type && m_name < rhs.m_name);
    }

    /**
     * Equality operator.
     * @param rhs right hand side to compare
     * @return true if this == rhs.
     */
    bool operator==(const ExtendedType& rhs) const {
      return m_type == rhs.m_type &&
             (m_type != FT_Other || m_name == rhs.m_name);
    }

    /**
     * Get type
     * @return type.
     */
    Type getType() const { return m_type; }

  private:
    friend class Frame;
    Type m_type;
    QString m_name;
  };


  /**
   * Constructor.
   */
  Frame();

  /**
   * Constructor.
   * @param type  type
   * @param value value
   * @param name  internal name
   * @param index index inside tag, -1 if unknown
   */
  Frame(Type type, const QString& value, const QString& name, int index);

  /**
   * Constructor.
   * @param type  type and internal name
   * @param value value
   * @param index index inside tag, -1 if unknown
   */
  Frame(const ExtendedType& type, const QString& value, int index);

  /**
   * Less than operator.
   * Needed for sorting in multiset.
   * @param rhs right hand side to compare
   * @return true if this < rhs.
   */
  bool operator<(const Frame& rhs) const {
    return m_extendedType < rhs.m_extendedType;
  }

#ifdef Q_CC_MSVC
  /**
   * Equality operator.
   * Needed when building with MSVC and BUILD_SHARED_LIBS.
   * @param rhs right hand side to compare
   * @return true if this == rhs.
   */
  bool operator==(const Frame& rhs) const {
    return m_extendedType == rhs.m_extendedType && m_value == rhs.m_value &&
      m_fieldList == rhs.m_fieldList;
  }
#endif

  /**
   * Get type of frame.
   * @return type.
   */
  Type getType() const { return m_extendedType.m_type; }

  /**
   * Set type of frame.
   * @param type type of frame
   */
  void setType(Type type) { m_extendedType.m_type = type; }

  /**
   * Get type and name of frame.
   * @return extended type.
   */
  ExtendedType getExtendedType() const { return m_extendedType; }

  /**
   * Set type and name of frame.
   * @param type extended type of frame
   */
  void setExtendedType(const ExtendedType& type) { m_extendedType = type; }

  /**
   * Get index of frame.
   * @return index.
   */
  int getIndex() const { return m_index; }

  /**
   * Set index of frame.
   * @param index index of frame
   */
  void setIndex(int index) { m_index = index; }

  /**
   * Get name of frame.
   * @return name.
   */
  QString getName() const { return m_extendedType.getName(); }

  /**
   * Get internal name of frame.
   * @return name.
   */
  QString getInternalName() const { return m_extendedType.getInternalName(); }

  /**
   * Get value as string.
   * @return value.
   */
  QString getValue() const { return m_value; }

  /**
   * Set value as string.
   * @param value value as string
   */
  void setValue(const QString& value) { m_value = value; }

  /**
   * Get value as integer.
   * @return value.
   */
  int getValueAsNumber() const;

  /**
   * Set value as integer.
   * @param n value as number
   */
  void setValueAsNumber(int n);

  /**
   * Check if value is empty.
   * @return true if empty.
   */
  bool isEmpty() const { return m_value.isEmpty(); }

  /**
   * Check if frame is inactive.
   * @return true if inactive.
   */
  bool isInactive() const { return m_value.isNull(); }

  /**
   * Set frame inactive.
   */
  void setInactive() { m_value = QString(); }

  /**
   * Check if frame represents different frames in multiple files.
   * @return true if different.
   */
  bool isDifferent() const { return m_value == differentRepresentation(); }

  /**
   * Represents different frames in multiple files.
   */
  void setDifferent() { m_value = differentRepresentation(); }

  /**
   * Get warning notice if frame is marked.
   * @return notice.
   */
  FrameNotice getNotice() const { return m_marked; }

  /**
   * Check if frame is marked.
   * @return true if marked.
   */
  bool isMarked() const { return m_marked; }

  /**
   * Mark frame.
   * @param notice warning notice
   */
  void setMarked(FrameNotice notice) { m_marked = notice; }

  /**
   * Check if value is changed.
   * @return true if changed.
   */
  bool isValueChanged() const { return m_valueChanged; }

  /**
   * Mark the value as changed.
   * @param changed true to mark as changed
   */
  void setValueChanged(bool changed = true) { m_valueChanged = changed; }

  /**
   * Set value as string and mark it as changed if it is changed.
   * This method will avoid setting "different" representations.
   * @param value value as string
   */
  void setValueIfChanged(const QString& value);

  /**
   * Set the value from a field in the field list.
   */
  void setValueFromFieldList();

  /**
   * Set a field in the field list from the value.
   */
  void setFieldListFromValue();

  /**
   * Get fields in the frame.
   * @return field list.
   */
  const FieldList& getFieldList() const { return m_fieldList; }

  /**
   * Set fields in the frame.
   * @param fields field list
   */
  void setFieldList(const FieldList& fields) { m_fieldList = fields; }

  /**
   * Get fields in the frame.
   * @return reference to field list.
   */
  FieldList& fieldList() { return m_fieldList; }

  /**
   * Get the value of a field.
   *
   * @param id field ID
   *
   * @return field value, invalid if field not found.
   */
  QVariant getFieldValue(FieldId id) const;

  /**
   * Check if the fields in another frame are equal.
   *
   * @param other other frame
   *
   * @return true if equal.
   */
  bool isEqual(const Frame& other) const;

#ifndef QT_NO_DEBUG
  /**
   * Dump contents of frame to debug console.
   */
  void dump() const;
#endif

  /**
   * If a frame contains a string list as a value, it is stored in a single
   * string, separated by this special separator character.
   *
   * @return separator character.
   */
  static QLatin1Char stringListSeparator() { return QLatin1Char('|'); }

  /**
   * Convert string (e.g. "track/total number of tracks") to number.
   *
   * @param str string to convert
   * @param ok  if not 0, true is returned here if conversion is ok
   *
   * @return number in string ignoring total after slash.
   */
  static int numberWithoutTotal(const QString& str, bool* ok = nullptr);

  /**
   * Get representation of different frames in multiple files.
   * @return "different" representation.
   */
  static QChar differentRepresentation() { return QChar(0x2260); }

  /**
   * Set value of a field.
   *
   * @param frame frame to set
   * @param id    field ID
   * @param value field value
   *
   * @return true if field found and set.
   */
  static bool setField(Frame& frame, FieldId id, const QVariant& value);

  /**
   * Set value of a field.
   *
   * @param frame frame to set
   * @param fieldName name of field, can be English or translated
   * @param value field value
   *
   * @return true if field found and set.
   */
  static bool setField(Frame& frame, const QString& fieldName,
                       const QVariant& value);

  /**
   * Get value of a field.
   *
   * @param frame frame to get
   * @param id    field ID
   *
   * @return field value, invalid if not found.
   */
  static QVariant getField(const Frame& frame, FieldId id);

  /**
   * Get value of a field.
   *
   * @param frame frame to get
   * @param fieldName name of field, can be English or translated
   *
   * @return field value, invalid if not found.
   */
  static QVariant getField(const Frame& frame, const QString& fieldName);

  /**
   * Get type of frame from English name.
   *
   * @param name name, spaces and case are ignored
   *
   * @return type.
   */
  static Type getTypeFromName(const QString& name);

  /**
   * Get a translated string for a frame type.
   *
   * @param type frame type
   *
   * @return frame type, null string if unknown.
   */
  static QString getFrameTypeName(Type type);

  /**
   * Get a display name for a frame name.
   * @param name frame name as returned by getName()
   * @return display name, transformed if necessary and translated.
   */
  static QString getDisplayName(const QString& name);

  /**
   * Get a map with display names as keys and frame names as values.
   * @param names frame names as returned by getName()
   * @return mapping of display names to frame names.
   */
  static QMap<QString, QString> getDisplayNameMap(const QStringList& names);

  /**
   * Get the frame name for a translated display name.
   * @param name translated display name
   * @return English frame name for @a name if found, else @a name.
   */
  static QString getNameForTranslatedFrameName(const QString& name);

  /**
   * Convert frame index to a negative index used for a second collection.
   * This can be a collection containing picture frames, the mapping is
   * -1 -> -1, 0 -> -2, 1 -> -3, 2 -> -4, ...
   * @param index positive index, -1 is the unknown index
   * @return negative index <= -1.
   */
  static int toNegativeIndex(int index) { return -2 - index; }

  /**
   * Convert negative index used for a second collection to a frame index.
   * This can be used to get the index of a collection containing picture
   * frames, the mapping is
   * -1 -> -1, -2 -> 0, -3 -> 1, -4 -> 2, ...
   * @param negativeIndex negative index, -1 is the unknown index
   * @return frame negative >= -1.
   */
  static int fromNegativeIndex(int negativeIndex) { return -2 - negativeIndex; }

private:
  friend class TaggedFile;

  ExtendedType m_extendedType;
  int m_index;
  QString m_value;
  FieldList m_fieldList;
  FrameNotice m_marked;
  bool m_valueChanged;
};

/** Filter to enable a subset of frame types. */
class KID3_CORE_EXPORT FrameFilter {
public:
  /**
   * Constructor.
   * All frames are disabled
   */
  FrameFilter();

  /**
   * Enable all frames.
   */
  void enableAll();

  /**
   * Check if all fields are true.
   *
   * @return true if all fields are true.
   */
  bool areAllEnabled() const;

  /**
   * Check if frame is enabled.
   *
   * @param type frame type
   * @param name frame name
   *
   * @return true if frame is enabled.
   */
  bool isEnabled(Frame::Type type, const QString& name = QString()) const;

  /**
   * Enable or disable frame.
   *
   * @param type frame type
   * @param name frame name
   * @param en true to enable
   */
  void enable(Frame::Type type, const QString& name = QString(), bool en = true);

private:
  static const quint64 FTM_AllFrames = (1ULL << (Frame::FT_LastFrame + 1)) - 1;
  quint64 m_enabledFrames;
  std::set<QString> m_disabledOtherFrames;
};

/** Collection of frames. */
class KID3_CORE_EXPORT FrameCollection : public std::multiset<Frame> {
public:
  /**
   * Default value for quick access frames.
   */
  static constexpr quint64 DEFAULT_QUICK_ACCESS_FRAMES =
      (1ULL << Frame::FT_Title)   |
      (1ULL << Frame::FT_Artist)  |
      (1ULL << Frame::FT_Album)   |
      (1ULL << Frame::FT_Comment) |
      (1ULL << Frame::FT_Date)    |
      (1ULL << Frame::FT_Track)   |
      (1ULL << Frame::FT_Genre);

  /**
   * Set values which are different inactive.
   *
   * @param others frames to compare, will be modified!
   * @param differentValues optional storage for the different values
   */
  void filterDifferent(FrameCollection& others,
          QHash<Frame::ExtendedType, QSet<QString>>* differentValues = nullptr);

  /**
   * Add standard frames which are missing.
   */
  void addMissingStandardFrames();

  /**
   * Copy enabled frames.
   *
   * @param flt filter with enabled frames
   *
   * @return copy with enabled frames.
   */
  FrameCollection copyEnabledFrames(const FrameFilter& flt) const;

  /**
   * Remove all frames which are not enabled from the collection.
   *
   * @param flt filter with enabled frames
   */
  void removeDisabledFrames(const FrameFilter& flt);

  /**
   * Set the index of all frames to -1.
   */
  void setIndexesInvalid();

  /**
   * Copy frames which are empty or inactive from other frames.
   * This can be used to merge two frame collections.
   *
   * @param frames other frames
   */
  void merge(const FrameCollection& frames);

  /**
   * Check if the standard tags are empty or inactive.
   *
   * @return true if empty or inactive.
   */
  bool isEmptyOrInactive() const;

  /**
   * Find a frame by name.
   *
   * @param name  the name of the frame to find, if the exact name is not
   *              found, a case-insensitive search for the first name
   *              starting with this string is performed
   * @param index 0 for first frame with @a name, 1 for second, etc.
   *
   * @return iterator or end() if not found.
   */
  const_iterator findByName(const QString& name, int index = 0) const;

  /**
   * Find a frame by type or name.
   *
   * @param type  type and name of the frame to find, if the exact name is not
   *              found, a case-insensitive search for the first name
   *              starting with this string is performed
   * @param index 0 for first frame with @a type, 1 for second, etc.
   *
   * @return iterator or end() if not found.
   */
  const_iterator findByExtendedType(const Frame::ExtendedType& type,
                                    int index = 0) const;

  /**
   * Find a frame by index.
   *
   * @param index the index in the frame, see \ref Frame::getIndex()
   *
   * @return iterator or end() if not found.
   */
  const_iterator findByIndex(int index) const;

  /**
   * Get value by type.
   *
   * @param type type
   *
   * @return value, QString::null if not found.
   */
  QString getValue(Frame::Type type) const;

  /**
   * Get value by type and name.
   *
   * @param type  type and name of the frame to find, if the exact name is not
   *              found, a case-insensitive search for the first name
   *              starting with this string is performed
   *
   * @return value, QString::null if not found.
   */
  QString getValue(const Frame::ExtendedType& type) const;

  /**
   * Set value by type.
   *
   * @param type type
   * @param value value, nothing is done if QString::null
   */
  void setValue(Frame::Type type, const QString& value);

  /**
   * Set value by type and name.
   *
   * @param type  type and name of the frame to find, if the exact name is not
   *              found, a case-insensitive search for the first name
   *              starting with this string is performed
   * @param value value, nothing is done if QString::null
   */
  void setValue(const Frame::ExtendedType& type, const QString& value);

  /**
   * Get integer value by type.
   *
   * @param type type
   *
   * @return value, 0 if empty, -1 if not found.
   */
  int getIntValue(Frame::Type type) const;

  /**
   * Set integer value by type.
   *
   * @param type type
   * @param value value, 0 to set empty, nothing is done if -1
   */
  void setIntValue(Frame::Type type, int value);

  /**
   * Get artist.
   *
   * @return artist, QString::null if not found.
   */
  QString getArtist() const { return getValue(Frame::FT_Artist); }

  /**
   * Set artist.
   *
   * @param artist artist, nothing is done if QString::null
   */
  void setArtist(const QString& artist) { setValue(Frame::FT_Artist, artist); }

  /**
   * Get album.
   *
   * @return album, QString::null if not found.
   */
  QString getAlbum() const { return getValue(Frame::FT_Album); }

  /**
   * Set album.
   *
   * @param album album, nothing is done if QString::null
   */
  void setAlbum(const QString& album) { setValue(Frame::FT_Album, album); }

  /**
   * Get title.
   *
   * @return title, QString::null if not found.
   */
  QString getTitle() const { return getValue(Frame::FT_Title); }

  /**
   * Set title.
   *
   * @param title title, nothing is done if QString::null
   */
  void setTitle(const QString& title) { setValue(Frame::FT_Title, title); }

  /**
   * Get comment.
   *
   * @return comment, QString::null if not found.
   */
  QString getComment() const { return getValue(Frame::FT_Comment); }

  /**
   * Set comment.
   *
   * @param comment comment, nothing is done if QString::null
   */
  void setComment(const QString& comment) { setValue(Frame::FT_Comment, comment); }

  /**
   * Get genre.
   *
   * @return genre, QString::null if not found.
   */
  QString getGenre() const { return getValue(Frame::FT_Genre); }

  /**
   * Set genre.
   *
   * @param genre genre, nothing is done if QString::null
   */
  void setGenre(const QString& genre) { setValue(Frame::FT_Genre, genre); }

  /**
   * Get track.
   *
   * @return track, -1 if not found.
   */
  int getTrack() const { return getIntValue(Frame::FT_Track); }

  /**
   * Set track.
   *
   * @param track track, nothing is done if -1
   */
  void setTrack(int track) { setIntValue(Frame::FT_Track, track); }

  /**
   * Get year.
   *
   * @return year, -1 if not found.
   */
  int getYear() const { return getIntValue(Frame::FT_Date); }

  /**
   * Set year.
   *
   * @param year year, nothing is done if -1
   */
  void setYear(int year) { setIntValue(Frame::FT_Date, year); }

  /**
   * Compare the frames with another frame collection and mark the value as
   * changed on frames which are different.
   *
   * @param other other frame collection
   */
  void markChangedFrames(const FrameCollection& other);

#ifndef QT_NO_DEBUG
  /**
   * Dump contents of frame collection to debug console.
   */
  void dump() const;
#endif

  /**
   * Set mask containing the bits of all frame types which shall be used as
   * quick access frames.
   * @param mask bit mask with bits for quick access frames set, default is
   * DEFAULT_QUICK_ACCESS_FRAMES.
   */
  static void setQuickAccessFrames(quint64 mask) {
    s_quickAccessFrames = mask;
  }

  /**
   * Get mask containing the bits of all frame types which shall be used as
   * quick access frames.
   * @return mask bit mask with bits for quick access frames set.
   */
  static quint64 getQuickAccessFrames() {
    return s_quickAccessFrames;
  }

  /**
   * Create a frame collection from a list of subframe fields.
   *
   * The given subframe fields must start with a Frame::ID_Subframe field with
   * the frame name as its value, followed by the fields of the frame. More
   * subframes may follow.
   *
   * @param begin iterator to begin of subframes
   * @param end iterator after end of subframes
   *
   * @return frames constructed from subframe fields.
   */
  static FrameCollection fromSubframes(Frame::FieldList::const_iterator begin,
                                       Frame::FieldList::const_iterator end);

private:
  /**
   * Search for a frame only by name.
   *
   * @param name the name of the frame to find, a case-insensitive search for
   *             the first name starting with this string is performed
   *
   * @return iterator or end() if not found.
   */
  const_iterator searchByName(const QString& name) const;

  /**
   * Bit mask containing the bits of all frame types which shall be used as
   * quick access frames.
   * This mask has to be handled like FrameFilter::m_enabledFrames.
   */
  static quint64 s_quickAccessFrames;
};


/**
 * Replaces frame format codes in a string.
 */
class KID3_CORE_EXPORT FrameFormatReplacer : public FormatReplacer {
public:
  /**
   * Constructor.
   *
   * @param frames frame collection
   * @param str    string with format codes
   */
  explicit FrameFormatReplacer(
    const FrameCollection& frames, const QString& str = QString());

  /**
   * Destructor.
   */
  virtual ~FrameFormatReplacer() override = default;

  FrameFormatReplacer(const FrameFormatReplacer& other) = delete;
  FrameFormatReplacer &operator=(const FrameFormatReplacer& other) = delete;

  /**
   * Get help text for supported format codes.
   *
   * @param onlyRows if true only the tr elements are returned,
   *                 not the surrounding table
   *
   * @return help text.
   */
  static QString getToolTip(bool onlyRows = false);

protected:
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
  virtual QString getReplacement(const QString& code) const override;

private:
  const FrameCollection& m_frames;
};

/** Hash function to use Frame::ExtendedType as a QHash key. */
inline uint qHash(const Frame::ExtendedType& key) {
  return qHash(key.getType()) ^ qHash(key.getInternalName());
}
