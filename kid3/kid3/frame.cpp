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
			 it != end();
			 ++it) {
		if (!flt.isEnabled(it->getType(), it->getName())) {
			erase(it);
		}
	}
}


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
