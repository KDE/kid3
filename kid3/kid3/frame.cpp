/**
 * \file frame.cpp
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

#include "frame.h"
#include "pictureframe.h"
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
	static const char* const names[] = {
		I18N_NOOP("Title"),           // FT_Title,
		I18N_NOOP("Artist"),          // FT_Artist,
		I18N_NOOP("Album"),           // FT_Album,
		I18N_NOOP("Comment"),         // FT_Comment,
		I18N_NOOP("Date"),            // FT_Date,
		I18N_NOOP("Track Number"),    // FT_Track,
		I18N_NOOP("Genre"),           // FT_Genre,
		                              // FT_LastV1Frame = FT_Track,
		I18N_NOOP("Album Artist"),    // FT_AlbumArtist
		I18N_NOOP("Arranger"),        // FT_Arranger,
		I18N_NOOP("Author"),          // FT_Author,
		I18N_NOOP("BPM"),             // FT_Bpm,
		I18N_NOOP("Composer"),        // FT_Composer,
		I18N_NOOP("Conductor"),       // FT_Conductor,
		I18N_NOOP("Copyright"),       // FT_Copyright,
		I18N_NOOP("Disc Number"),     // FT_Disc,
		I18N_NOOP("Encoded-by"),      // FT_EncodedBy,
		I18N_NOOP("Grouping"),        // FT_Grouping,
		I18N_NOOP("ISRC"),            // FT_Isrc,
		I18N_NOOP("Language"),        // FT_Language,
		I18N_NOOP("Lyricist"),        // FT_Lyricist,
		I18N_NOOP("Lyrics"),          // FT_Lyrics,
		I18N_NOOP("Media"),           // FT_Media,
		I18N_NOOP("Original Album"),  // FT_OriginalAlbum,
		I18N_NOOP("Original Artist"), // FT_OriginalArtist,
		I18N_NOOP("Original Date"),   // FT_OriginalDate,
		I18N_NOOP("Part"),            // FT_Part,
		I18N_NOOP("Performer"),       // FT_Performer,
		I18N_NOOP("Picture"),         // FT_Picture,
		I18N_NOOP("Publisher"),       // FT_Publisher,
		I18N_NOOP("Remixer"),         // FT_Remixer,
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
 * Get type of frame from English name.
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
			strNumMap.insert(QString(getNameFromType(type)).
											 remove(' ').QCM_toUpper(), type);
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
 * Convert string (e.g. "track/total number of tracks") to number.
 *
 * @param str string to convert
 * @param ok  if not 0, true is returned here if conversion is ok
 *
 * @return number in string ignoring total after slash.
 */
int Frame::numberWithoutTotal(const QString& str, bool* ok)
{
	int slashPos = str.QCM_indexOf("/");
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

#ifdef DEBUG
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
		"FT_Composer",
		"FT_Conductor",
		"FT_Copyright",
		"FT_Disc",
		"FT_EncodedBy",
		"FT_Grouping",
		"FT_Isrc",
		"FT_Language",
		"FT_Lyricist",
		"FT_Lyrics",
		"FT_Media",
		"FT_OriginalAlbum",
		"FT_OriginalArtist",
		"FT_OriginalDate",
		"FT_Part",
		"FT_Performer",
		"FT_Picture",
		"FT_Publisher",
		"FT_Remixer",
		"FT_Subtitle",
		"FT_Website",
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
		return QString("ByteArray of %1 bytes").arg(val.toByteArray().size());
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
				 m_name.QCM_latin1(), m_value.QCM_latin1(), frameTypeToString(m_type), m_index,
				 m_valueChanged);
	qDebug("  fields=");
	for (FieldList::const_iterator it = m_fieldList.begin();
			 it != m_fieldList.end();
			 ++it) {
		qDebug("  Field: id=%s, value=%s", fieldIdToString((*it).m_id),
					 variantToString((*it).m_value).QCM_latin1());
	}
}
#endif


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
 * Find a frame by name.
 *
 * @param name  the name of the frame to find, if the exact name is not
 *              found, a case-insensitive search for the first name
 *              starting with this string is performed
 *
 * @return iterator or end() if not found.
 */
FrameCollection::iterator FrameCollection::findByName(const QString& name) const
{
	Frame::Type type = Frame::getTypeFromName(name);
	Frame frame(type, "", name, -1);
	const_iterator it = find(frame);
	if (it == end()) {
		QString ucName = name.QCM_toUpper();
		int len = ucName.length();
		for (it = begin(); it != end(); ++it) {
			if (ucName == it->getName().QCM_toUpper().left(len)) {
				break;
			}
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
	Frame frame(type, "", "", -1);
	const_iterator it = find(frame);
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
		Frame frame(type, "", "", -1);
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
		QString str = value != 0 ? QString::number(value) : QString("");
		setValue(type, str);
	}
}

#ifdef DEBUG
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
		return (m_enabledFrames & (1 << type)) != 0;
	} else if (!name.isEmpty()) {
		std::set<QString>::iterator it = m_disabledOtherFrames.find(name);
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
			m_enabledFrames |= (1 << type);
		} else {
			m_enabledFrames &= ~(1 << type);
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
#if QT_VERSION >= 0x040000
		const char c = code[0].toLatin1();
#else
		const char c = code[0].latin1();
#endif
		for (unsigned i = 0; i < sizeof(shortToLong) / sizeof(shortToLong[0]); ++i) {
			if (shortToLong[i].shortCode == c) {
				name = shortToLong[i].longCode;
				break;
			}
		}
	} else if (code.length() > 1) {
		name = code;
	}

	if (!name.isNull()) {
		QString lcName(name.QCM_toLower());
		int fieldWidth = 2;
		if (lcName == "year") {
			name = "date";
		} else if (lcName == "tracknumber") {
			name = "track number";
		} else if (lcName.startsWith("track.") && lcName.length() == 7 &&
							 lcName[6] >= '0' && lcName[6] <= '9') {
#if QT_VERSION >= 0x040000
			fieldWidth = lcName[6].toLatin1() - '0';
#else
			fieldWidth = lcName[6].latin1() - '0';
#endif
			lcName = name = "track";
		}

		FrameCollection::iterator it = m_frames.findByName(name);
		if (it != m_frames.end()) {
			result = it->getValue();
			if (result.isNull()) {
				// code was found, but value is empty
				result = "";
			}
			if (it->getType() == Frame::FT_Picture && result.isEmpty()) {
				QVariant fieldValue = it->getFieldValue(Frame::Field::ID_Data);
				if (fieldValue.isValid() && fieldValue.toByteArray().size() > 0) {
					// If there is a picture without description, return "1", so that
					// an empty value indicates "no picture"
					result = "1";
				}				
			}
		}

		if (lcName == "track") {
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
	if (!onlyRows) str += "<table>\n";

	str += "<tr><td>%s</td><td>%{title}</td><td>";
	str += QCM_translate("Title");
	str += "</td></tr>\n";

	str += "<tr><td>%l</td><td>%{album}</td><td>";
	str += QCM_translate("Album");
	str += "</td></tr>\n";

	str += "<tr><td>%a</td><td>%{artist}</td><td>";
	str += QCM_translate("Artist");
	str += "</td></tr>\n";

	str += "<tr><td>%c</td><td>%{comment}</td><td>";
	str += QCM_translate("Comment");
	str += "</td></tr>\n";

	str += "<tr><td>%y</td><td>%{year}</td><td>";
	str += QCM_translate(I18N_NOOP("Year"));
	str += "</td></tr>\n";

	str += "<tr><td>%t</td><td>%{track}</td><td>";
	str += QCM_translate(I18N_NOOP("Track"));
	str += " &quot;01&quot;</td></tr>\n";

	str += "<tr><td>%t</td><td>%{track.3}</td><td>";
	str += QCM_translate(I18N_NOOP("Track"));
	str += " &quot;001&quot;</td></tr>\n";

	str += "<tr><td>%T</td><td>%{tracknumber}</td><td>";
	str += QCM_translate(I18N_NOOP("Track"));
	str += " &quot;1&quot;</td></tr>\n";

	str += "<tr><td>%g</td><td>%{genre}</td><td>";
	str += QCM_translate("Genre");
	str += "</td></tr>\n";

	if (!onlyRows) str += "</table>\n";
	return str;
}
