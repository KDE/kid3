/**
 * \file frame.cpp
 * Generalized frame.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 25 Aug 2007
 */

#include "frame.h"
#include <qmap.h>
#include "qtcompatmac.h"

/**
 * Constructor.
 */
Frame::Frame() :
	m_type(FT_UnknownFrame), m_index(-1), m_valueChanged(false)
{
}

/**
 * Constructor.
 */
Frame::Frame(Type type, const QString& value,
						 const QString& name, int index) :
	m_type(type), m_index(index), m_valueChanged(false),
	m_value(value), m_name(name)
{
}

/**
 * Destructor.
 */
Frame::~Frame()
{
}

/**
 * Get name of frame from type.
 *
 * @param type type
 *
 * @return name.
 */
const char* Frame::getNameFromType(Type type)
{
	// These names are also used to generate Vorbis field names
	// by taking the untranslated name in all uppercase letters
	// and removing spaces.
  static const char* const names[] = {
		I18N_NOOP("Title"),           // FT_Title,
		I18N_NOOP("Artist"),          // FT_Artist,
		I18N_NOOP("Album"),           // FT_Album,
		I18N_NOOP("Comment"),         // FT_Comment,
		I18N_NOOP("Date"),            // FT_Date,
		I18N_NOOP("Track Number"),    // FT_Track,
		I18N_NOOP("Genre"),           // FT_Genre,
		                              // FT_LastV1Frame = FT_Track,
		I18N_NOOP("Arranger"),        // FT_Arranger,
		I18N_NOOP("Author"),          // FT_Author,
		I18N_NOOP("BPM"),             // FT_Bpm,
		I18N_NOOP("Composer"),        // FT_Composer,
		I18N_NOOP("Conductor"),       // FT_Conductor,
		I18N_NOOP("Copyright"),       // FT_Copyright,
		I18N_NOOP("Disc Number"),     // FT_Disc,
		I18N_NOOP("Encoded-by"),      // FT_EncodedBy,
		I18N_NOOP("ISRC"),            // FT_Isrc,
		I18N_NOOP("Language"),        // FT_Language,
		I18N_NOOP("Lyricist"),        // FT_Lyricist,
		I18N_NOOP("Original Album"),  // FT_OriginalAlbum,
		I18N_NOOP("Original Artist"), // FT_OriginalArtist,
		I18N_NOOP("Original Date"),   // FT_OriginalDate,
		I18N_NOOP("Part"),            // FT_Part,
		I18N_NOOP("Performer"),       // FT_Performer,
		I18N_NOOP("Publisher"),       // FT_Publisher,
		I18N_NOOP("Subtitle"),        // FT_Subtitle,
		I18N_NOOP("Website")          // FT_Website,
		                              // FT_LastFrame = FT_Website
	};
	class not_used { int array_size_check[
			sizeof(names) / sizeof(names[0]) == FT_LastFrame + 1
			? 1 : -1 ]; };
	return type <= FT_LastFrame ? names[type] : "Unknown";
}

/**
 * Get type of frame from name.
 *
 * @param name name, spaces and case are ignored
 *
 * @return type.
 */
Frame::Type Frame::getTypeFromName(QString name)
{
	static QMap<QString, int> strNumMap;
	if (strNumMap.empty()) {
		// first time initialization
		for (int i = 0; i <= FT_LastFrame; ++i) {
			Type type = static_cast<Type>(i);
			strNumMap.insert(QString(getNameFromType(type)).remove(' ').QCM_toUpper(),
											 type);
		}
	}
	QMap<QString, int>::const_iterator it =
		strNumMap.find(name.remove(' ').QCM_toUpper());
	if (it != strNumMap.end()) {
		return static_cast<Type>(*it);
	}
	return FT_Other;
}

/**
 * Get name of frame.
 *
 * @param internal true to get internal (non-general) name
 *
 * @return name.
 */
QString Frame::getName(bool internal) const
{
	return !internal && m_type != FT_Other ?
		QString(getNameFromType(m_type)) : m_name;
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
 * Set values which are different inactive.
 *
 * @param others frames to compare
 */
void FrameCollection::filterDifferent(const FrameCollection& others)
{
	for (iterator it = begin(); it != end(); ++it) {
		Frame& frame = const_cast<Frame&>(*it);
		// This frame list is not tied to a specific file, so the
		// index is not valid.
		frame.setIndex(-1);
		if (!frame.isInactive()) {
			iterator othersIt = others.find(frame);
			if (othersIt == others.end() ||
					frame.getValue() != othersIt->getValue()) {
				frame.setInactive();
			}
		}
	}
}

/**
 * Add standard frames which are missing.
 */
void FrameCollection::addMissingStandardFrames()
{
	for (int i = Frame::FT_FirstFrame; i <= Frame::FT_LastV1Frame; ++i) {
		Frame frame(static_cast<Frame::Type>(i), QString::null, QString::null, -1);
		FrameCollection::const_iterator it = find(frame);
		if (it == end()) {
			insert(frame);
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
FrameCollection FrameCollection::copyEnabledFrames(FrameFilter flt) const
{
	FrameCollection frames;
	for (const_iterator it = begin();
			 it != end();
			 ++it) {
		if (flt.isEnabled(it->getType())) {
			Frame frame = *it;
			frame.setIndex(-1);
			frames.insert(frame);
		}
	}
	return frames;
}
