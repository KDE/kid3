/**
 * \file urllinkframe.h
 * URL link frame for TagLib.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 01 Oct 2006
 *
 * Copyright (C) 2006-2007  Urs Fleisch
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

#ifndef URLLINKFRAME_H
#define URLLINKFRAME_H

#include "taglibfile.h"

#if defined HAVE_TAGLIB && !defined TAGLIB_SUPPORTS_URLLINK_FRAMES

#include <taglib/id3v2frame.h>

namespace TagLib {
	namespace ID3v2 {
		class Frame;
		class FrameFactory;
	}
}

//! An ID3v2 URL link frame implementation.
class UrlLinkFrame : public TagLib::ID3v2::Frame {
	friend class TagLib::ID3v2::FrameFactory;

public:
	/*!
	 * This is a dual purpose constructor.  \a data can either be binary data
	 * that should be parsed or (at a minimum) the frame ID.
	 */
	explicit UrlLinkFrame(const TagLib::ByteVector &data);

	/*!
	 * Destroys this UrlLinkFrame instance.
	 */
	virtual ~UrlLinkFrame();

	/*!
	 * Returns the URL.
	 */
	virtual TagLib::String url() const;

	/*!
	 * Sets the URL to \a s.
	 */
	virtual void setUrl(const TagLib::String &s);

	// Reimplementations.

	virtual void setText(const TagLib::String &s);
	virtual TagLib::String toString() const;

protected:
	virtual void parseFields(const TagLib::ByteVector &data);
	virtual TagLib::ByteVector renderFields() const;

	/*!
	 * The constructor used by the FrameFactory.
	 */
	UrlLinkFrame(const TagLib::ByteVector &data, Header *h);

private:
	UrlLinkFrame(const UrlLinkFrame &);
	UrlLinkFrame &operator=(const UrlLinkFrame &);

	class UrlLinkFramePrivate;
	UrlLinkFramePrivate *d;
};

/*!
 * This is a specialization of URL link frames that allows for
 * user defined entries.  Each entry has a description in addition to the
 * normal list of fields that a URL link frame has.
 *
 * This description identifies the frame and must be unique.
 */
class UserUrlLinkFrame : public UrlLinkFrame {
	friend class TagLib::ID3v2::FrameFactory;

public:
	/*!
	 * Constructs an empty user defined URL link frame.  For this to be
	 * a useful frame both a description and text must be set.
	 */
	explicit UserUrlLinkFrame(TagLib::String::Type encoding = TagLib::String::Latin1);

	/*!
	 * This is a dual purpose constructor.  \a data can either be binary data
	 * that should be parsed or (at a minimum) the frame ID.
	 */
	explicit UserUrlLinkFrame(const TagLib::ByteVector &data);

	/*!
	 * Destroys this UserUrlLinkFrame instance.
	 */
	virtual ~UserUrlLinkFrame();

	// Reimplementations.

	virtual TagLib::String toString() const;

	/*!
	 * Returns the text encoding that will be used in rendering this frame.
	 * This defaults to the type that was either specified in the constructor
	 * or read from the frame when parsed.
	 *
	 * \see setTextEncoding()
	 * \see render()
	 */
	TagLib::String::Type textEncoding() const;

	/*!
	 * Sets the text encoding to be used when rendering this frame to
	 * \a encoding.
	 *
	 * \see textEncoding()
	 * \see render()
	 */
	void setTextEncoding(TagLib::String::Type encoding);

	/*!
	 * Returns the description for this frame.
	 */
	TagLib::String description() const;

	/*!
	 * Sets the description of the frame to \a s.  \a s must be unique.
	 */
	void setDescription(const TagLib::String &s);

protected:
	virtual void parseFields(const TagLib::ByteVector &data);
	virtual TagLib::ByteVector renderFields() const;

	/*!
	 * The constructor used by the FrameFactory.
	 */
	UserUrlLinkFrame(const TagLib::ByteVector &data, Header *h);

private:
	UserUrlLinkFrame(const UserUrlLinkFrame &);
	UserUrlLinkFrame &operator=(const UserUrlLinkFrame &);

	class UserUrlLinkFramePrivate;
	UserUrlLinkFramePrivate *d;
};

#endif // HAVE_TAGLIB && !TAGLIB_SUPPORTS_URLLINK_FRAMES

#endif // URLLINKFRAME_H
