/**
 * \file frame.h
 * Generalized frame.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 25 Aug 2007
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

		int m_id;
		QVariant m_value;
	};

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
	 */
	Frame(Type type, const QString& value, const QString& name, int index);

	/**
	 * Destructor.
	 */
	~Frame();

	/**
	 * Get type of frame.
	 * @return type.
	 */
	Type getType() const { return m_type; }

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
	void setValue(const QString& value);

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
	 * Check if frame is enabled.
	 * @return true if enabled.
	 */
	bool isEnabled() const { return m_enabled; }

	/**
	 * Enable or diable the frame.
	 * @param en true to enable.
	 */
	void setEnabled(bool enabled = true) { m_enabled = enabled; }

	/**
	 * Check if frame is truncated.
	 * @return true if truncated.
	 */
	bool isTruncated() const { return m_truncated; }

	/**
	 * Set truncation status of frame.
	 * @param truncated true if truncated.
	 */
	void setTruncated(bool truncated) { m_truncated = truncated; }

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
	 * Get type of frame from name.
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
	bool m_enabled;
	bool m_truncated;
	bool m_number;
	bool m_maxValue255;
	bool m_maxLength28;
	bool m_maxLength30;
	QString m_value;
	QString m_name;
	FieldList m_fieldList;
};

#if QT_VERSION >= 0x040000
typedef QList<Frame> FrameCollection;
#else
typedef QValueList<Frame> FrameCollection;
#endif

#endif // FRAME_H
