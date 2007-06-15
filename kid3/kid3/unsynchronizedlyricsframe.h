/**
 * \file unsynchronizedlyricsframe.h
 * Unsynchronized lyrics frame for TagLib.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 01 Oct 2006
 */

#ifndef UNSYNCHRONIZEDLYRICSFRAME_H
#define UNSYNCHRONIZEDLYRICSFRAME_H

#include "taglibfile.h"

#if defined HAVE_TAGLIB && !defined TAGLIB_SUPPORTS_USLT_FRAMES

#include <taglib/id3v2frame.h>

namespace TagLib {
	namespace ID3v2 {
		class Frame;
		class FrameFactory;
	}
}

//! An implementation of ID3v2 unsynchronized lyrics.
class UnsynchronizedLyricsFrame : public TagLib::ID3v2::Frame
{
	friend class TagLib::ID3v2::FrameFactory;

public:
	/*!
	 * Construct an empty unsynchronized lyrics frame that will use the text encoding
	 * \a encoding.
	 */
	explicit UnsynchronizedLyricsFrame(TagLib::String::Type encoding = TagLib::String::Latin1);

	/*!
	 * Construct a unsynchronized lyrics frame based on the data in \a data.
	 */
	explicit UnsynchronizedLyricsFrame(const TagLib::ByteVector &data);

	/*!
	 * Destroys this UnsynchronizedLyricsFrame instance.
	 */
	virtual ~UnsynchronizedLyricsFrame();

	/*!
	 * Returns the text of this unsynchronized lyrics frame.
	 *
	 * \see text()
	 */
	virtual TagLib::String toString() const;

	/*!
	 * Returns the language encoding as a 3 byte encoding as specified by
	 * <a href="http://en.wikipedia.org/wiki/ISO_639">ISO-639-2</a>.
	 *
	 * \note Most taggers simply ignore this value.
	 *
	 * \see setLanguage()
	 */
	TagLib::ByteVector language() const;

	/*!
	 * Returns the description of this unsynchronized lyrics frame.
	 *
	 * \note Most taggers simply ignore this value.
	 *
	 * \see setDescription()
	 */
	TagLib::String description() const;

	/*!
	 * Returns the text of this unsynchronized lyrics frame.
	 *
	 * \see setText()
	 */
	TagLib::String text() const;

	/*!
	 * Set the language using the 3 byte language code from
	 * <a href="http://en.wikipedia.org/wiki/ISO_639">ISO-639-2</a> to
	 * \a languageCode.
	 *
	 * \see language()
	 */
	void setLanguage(const TagLib::ByteVector &languageCode);

	/*!
	 * Sets the description of the unsynchronized lyrics frame to \a s.
	 *
	 * \see decription()
	 */
	void setDescription(const TagLib::String &s);

	/*!
	 * Sets the text portion of the unsynchronized lyrics frame to \a s.
	 *
	 * \see text()
	 */
	virtual void setText(const TagLib::String &s);

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

protected:
	// Reimplementations.

	virtual void parseFields(const TagLib::ByteVector &data);
	virtual TagLib::ByteVector renderFields() const;

private:
	/*!
	 * The constructor used by the FrameFactory.
	 */
	UnsynchronizedLyricsFrame(const TagLib::ByteVector &data, Header *h);
	UnsynchronizedLyricsFrame(const UnsynchronizedLyricsFrame &);
	UnsynchronizedLyricsFrame &operator=(const UnsynchronizedLyricsFrame &);

	class UnsynchronizedLyricsFramePrivate;
	UnsynchronizedLyricsFramePrivate *d;
};

#endif // HAVE_TAGLIB && !TAGLIB_SUPPORTS_USLT_FRAMES

#endif // UNSYNCHRONIZEDLYRICSFRAME_H
