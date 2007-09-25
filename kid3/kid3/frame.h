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
	FrameFilter() : m_enabledFrames(0) {}

	/**
	 * Destructor.
	 */
	~FrameFilter() {}

	/**
	 * If all frames are disabled enable all.
	 */
	void allDisabledToAllEnabled() { if (m_enabledFrames == 0) enableAll(); }

	/**
	 * enable all frames.
	 */
	void enableAll() { m_enabledFrames = FTM_AllFrames; }

  /**
   * Check if all fields are true.
   *
   * @return true if all fields are true.
   */
  bool areAllEnabled() const {
		return (m_enabledFrames & FTM_AllFrames) == FTM_AllFrames;
	}

	/**
	 * Check if frame is enabled.
	 * @param type frame type
	 * @return true if frame is enabled.
	 */
	bool isEnabled(Frame::Type type) {
		return type > Frame::FT_LastFrame || (m_enabledFrames & (1 << type)) != 0;
	}

	/**
	 * Enable or disable frame.
	 * @param type frame type
	 * @param en true to enable
	 */
	void enable(Frame::Type type, bool en = true) {
		if (type <= Frame::FT_LastFrame) {
			if (en) {
				m_enabledFrames |= (1 << type);
			} else {
				m_enabledFrames &= ~(1 << type);
			}
		}
	}

private:
	enum { FTM_AllFrames = (1 << (Frame::FT_LastFrame + 1)) - 1 };
	unsigned long m_enabledFrames;
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
	FrameCollection copyEnabledFrames(FrameFilter flt) const;
};

#endif // FRAME_H
