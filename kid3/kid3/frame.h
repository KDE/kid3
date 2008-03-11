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
		FT_Arranger,
		FT_Author,
		FT_Bpm,
		FT_Composer,
		FT_Conductor,
		FT_Copyright,
		FT_Disc,
		FT_EncodedBy,
		FT_Isrc,
		FT_Language,
		FT_Lyricist,
		FT_OriginalAlbum,
		FT_OriginalArtist,
		FT_OriginalDate,
		FT_Part,
		FT_Performer,
		FT_Picture,
		FT_Publisher,
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

private:
	friend class TaggedFile;
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
	enum { FTM_AllFrames = (1 << (Frame::FT_LastFrame + 1)) - 1 };
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
	 * @param others frames to compare
	 */
	void filterDifferent(const FrameCollection& others);

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
	 * Find a frame by name.
	 *
	 * @param name  the name of the frame to find, if the exact name is not
	 *              found, a case-insensitive search for the first name
	 *              starting with this string is performed
	 *
	 * @return iterator or end() if not found.
	 */
	iterator findByName(const QString& name) const;
};

#endif // FRAME_H
