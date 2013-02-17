/**
 * \file frame.h
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

#ifndef FRAME_H
#define FRAME_H

#include "formatreplacer.h"
#include <QString>
#include <QVariant>
#include <QList>
#include <set>
#include "kid3api.h"

/** Generalized frame. */
class KID3_CORE_EXPORT Frame {
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
    FT_Part,
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
    FT_LastFrame = FT_WWWAudioSource,
    FT_Other,
    FT_UnknownFrame
  };

  /** Field in frame. */
  struct Field {
    /**
     * Types of fields in a frame, must be the same as id3lib's ID3_FieldID.
     **/
    enum Id {
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
      ID_Seller
    };

    /** Text encoding for fields of type ID_TextEnc. */
    enum TextEncoding {
      TE_ISO8859_1 = 0,
      TE_UTF16 = 1,
      TE_UTF16BE = 2,
      TE_UTF8 = 3
    };

    int m_id;         /**< type of field. */
    QVariant m_value; /**< value of field. */
  };

  /** list of fields. */
  typedef QList<Field> FieldList;

  /**
   * Type and name of frame.
   */
  class ExtendedType {
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
   * Destructor.
   */
  ~Frame();

  /**
   * Less than operator.
   * Needed for sorting in multiset.
   * @param rhs right hand side to compare
   * @return true if this < rhs.
   */
  bool operator<(const Frame& rhs) const {
    return m_extendedType < rhs.m_extendedType;
  }

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
  void setInactive() { m_value = QString::null; }

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
  QVariant getFieldValue(Field::Id id) const;

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
  static int numberWithoutTotal(const QString& str, bool* ok = 0);

private:
  friend class TaggedFile;

  /**
   * Get representation of different frames in multiple files.
   * @return "different" representation.
   */
  static QChar differentRepresentation() { return QChar(0x2260); }

  ExtendedType m_extendedType;
  int m_index;
  bool m_valueChanged;
  QString m_value;
  FieldList m_fieldList;
};

/** Filter to enable a subset of frame types. */
class FrameFilter {
public:
  /**
   * Constructor.
   * All frames are disabled
   */
  FrameFilter();

  /**
   * Destructor.
   */
  ~FrameFilter();

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
  static const quint64 DEFAULT_QUICK_ACCESS_FRAMES =
      (1ULL << Frame::FT_Title)   |
      (1ULL << Frame::FT_Artist)  |
      (1ULL << Frame::FT_Album)   |
      (1ULL << Frame::FT_Comment) |
      (1ULL << Frame::FT_Date)    |
      (1ULL << Frame::FT_Track)   |
      (1ULL << Frame::FT_Genre);

  /**
   * Constructor.
   */
  FrameCollection() {}

  /**
   * Destructor.
   */
  ~FrameCollection() {}

  /**
   * Set values which are different inactive.
   *
   * @param others frames to compare, will be modified!
   */
  void filterDifferent(FrameCollection& others);

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
   *
   * @return iterator or end() if not found.
   */
  const_iterator findByName(const QString& name) const;

  /**
   * Find a frame by type or name.
   *
   * @param type  type and name of the frame to find, if the exact name is not
   *              found, a case-insensitive search for the first name
   *              starting with this string is performed
   *
   * @return iterator or end() if not found.
   */
  const_iterator findByExtendedType(const Frame::ExtendedType& type) const;

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
  virtual ~FrameFormatReplacer();

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
  virtual QString getReplacement(const QString& code) const;

private:
  const FrameCollection& m_frames;
};

#endif // FRAME_H
