/**
 * \file frame.h
 * Generalized frame.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 25 Aug 2007
 *
 * Copyright (C) 2007  Urs Fleisch
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
#include <qstring.h>
#include <qvariant.h>
#if QT_VERSION >= 0x040000
#include <QList>
#else
#include <qvaluelist.h>
#endif
#include <set>

/** Generalized frame. */
class Frame {
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
		FT_Composer,
		FT_Conductor,
		FT_Copyright,
		FT_Disc,
		FT_EncodedBy,
		FT_Grouping,
		FT_Isrc,
		FT_Language,
		FT_Lyricist,
		FT_Lyrics,
		FT_Media,
		FT_OriginalAlbum,
		FT_OriginalArtist,
		FT_OriginalDate,
		FT_Part,
		FT_Performer,
		FT_Picture,
		FT_Publisher,
		FT_Remixer,
		FT_Subtitle,
		FT_Website,
		FT_LastFrame = FT_Website,
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
			ID_ContentType
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
#if QT_VERSION >= 0x040000
	typedef QList<Field> FieldList;
#else
	typedef QValueList<Field> FieldList;
#endif

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
		return m_type < rhs.m_type || (m_type == FT_Other && m_type == rhs.m_type && m_name < rhs.m_name);
	}

	/**
	 * Get type of frame.
	 * @return type.
	 */
	Type getType() const { return m_type; }

	/**
	 * Set type of frame.
	 * @param type type of frame
	 */
	void setType(Type type) { m_type = type; }

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
	 *
	 * @param internal true to get internal (non-general) name
	 *
	 * @return name.
	 */
	QString getName(bool internal = false) const;

	/**
	 * Set internal name of frame.
	 * @param name internal (non-general) name
	 */
	void setInternalName(const QString& name) { m_name = name; }

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

#ifdef DEBUG
	/**
	 * Dump contents of frame to debug console.
	 */
	void dump() const;
#endif

	/**
	 * Get name of frame from type.
	 *
	 * @param type type
	 *
	 * @return name.
	 */
	static const char* getNameFromType(Type type);

	/**
	 * Get type of frame from English name.
	 *
	 * @param name name, spaces and case are ignored
	 *
	 * @return type.
	 */
	static Type getTypeFromName(QString name);

	/**
	 * If a frame contains a string list as a value, it is stored in a single
	 * string, separated by this special separator character.
	 *
	 * @return separator character.
	 */
	static char stringListSeparator() { return '|'; }

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

	Type m_type;
	int m_index;
	bool m_valueChanged;
	QString m_value;
	QString m_name;
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
	class not_used { int num_frame_types_check[
			Frame::FT_LastFrame == 31
			? 1 : -1 ]; };
	enum { FTM_AllFrames = 0xffffffff };
	// if FTM_AllFrames is not 31, use
	// enum { FTM_AllFrames = (1 << (Frame::FT_LastFrame + 1)) - 1 };
	unsigned long m_enabledFrames;
	std::set<QString> m_disabledOtherFrames;
};

/** Collection of frames. */
class FrameCollection : public std::multiset<Frame> {
public:
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
	iterator findByName(const QString& name) const;

	/**
	 * Get value by type.
	 *
	 * @param type type
	 *
	 * @return value, QString::null if not found.
	 */
	QString getValue(Frame::Type type) const;

	/**
	 * Set value by type.
	 *
	 * @param type type
	 * @param value value, nothing is done if QString::null
	 */
	void setValue(Frame::Type type, const QString& value);

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

#ifdef DEBUG
	/**
	 * Dump contents of frame collection to debug console.
	 */
	void dump() const;
#endif
};


/**
 * Replaces frame format codes in a string.
 */
class FrameFormatReplacer : public FormatReplacer {
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
