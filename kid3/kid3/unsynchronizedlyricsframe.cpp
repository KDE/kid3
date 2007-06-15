/**
 * \file unsynchronizedlyricsframe.cpp
 * Unsynchronized lyrics frame for TagLib.
 * Copy-pasted from CommentsFrame.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 01 Oct 2006
 */

#include "unsynchronizedlyricsframe.h"

#if defined HAVE_TAGLIB && !defined TAGLIB_SUPPORTS_USLT_FRAMES

class UnsynchronizedLyricsFrame::UnsynchronizedLyricsFramePrivate
{
public:
  UnsynchronizedLyricsFramePrivate() : textEncoding(TagLib::String::Latin1) {}
  TagLib::String::Type textEncoding;
  TagLib::ByteVector language;
  TagLib::String description;
  TagLib::String text;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

UnsynchronizedLyricsFrame::UnsynchronizedLyricsFrame(TagLib::String::Type encoding) : Frame("USLT")
{
  d = new UnsynchronizedLyricsFramePrivate;
  d->textEncoding = encoding;
}

UnsynchronizedLyricsFrame::UnsynchronizedLyricsFrame(const TagLib::ByteVector &data) : Frame(data)
{
  d = new UnsynchronizedLyricsFramePrivate;
  setData(data);
}

UnsynchronizedLyricsFrame::~UnsynchronizedLyricsFrame()
{
  delete d;
}

TagLib::String UnsynchronizedLyricsFrame::toString() const
{
  return d->text;
}

TagLib::ByteVector UnsynchronizedLyricsFrame::language() const
{
  return d->language;
}

TagLib::String UnsynchronizedLyricsFrame::description() const
{
  return d->description;
}

TagLib::String UnsynchronizedLyricsFrame::text() const
{
  return d->text;
}

void UnsynchronizedLyricsFrame::setLanguage(const TagLib::ByteVector &languageEncoding)
{
  d->language = languageEncoding.mid(0, 3);
}

void UnsynchronizedLyricsFrame::setDescription(const TagLib::String &s)
{
  d->description = s;
}

void UnsynchronizedLyricsFrame::setText(const TagLib::String &s)
{
  d->text = s;
}


TagLib::String::Type UnsynchronizedLyricsFrame::textEncoding() const
{
  return d->textEncoding;
}

void UnsynchronizedLyricsFrame::setTextEncoding(TagLib::String::Type encoding)
{
  d->textEncoding = encoding;
}

////////////////////////////////////////////////////////////////////////////////
// protected members
////////////////////////////////////////////////////////////////////////////////

void UnsynchronizedLyricsFrame::parseFields(const TagLib::ByteVector &data)
{
  if(data.size() < 5) {
    qDebug("An unsynchronized lyrics frame must contain at least 5 bytes.");
    return;
  }

  d->textEncoding = TagLib::String::Type(data[0]);
  d->language = data.mid(1, 3);

  int byteAlign = d->textEncoding == TagLib::String::Latin1 || d->textEncoding == TagLib::String::UTF8 ? 1 : 2;

  TagLib::ByteVectorList l = TagLib::ByteVectorList::split(data.mid(4), textDelimiter(d->textEncoding), byteAlign, 2);

  if(l.size() == 2) {
    d->description = TagLib::String(l.front(), d->textEncoding);
    d->text = TagLib::String(l.back(), d->textEncoding);
  }
}

TagLib::ByteVector UnsynchronizedLyricsFrame::renderFields() const
{
  TagLib::ByteVector v;

  v.append(char(d->textEncoding));
  v.append(d->language.size() == 3 ? d->language : "   ");
  v.append(d->description.data(d->textEncoding));
  v.append(textDelimiter(d->textEncoding));
  v.append(d->text.data(d->textEncoding));

  return v;
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

UnsynchronizedLyricsFrame::UnsynchronizedLyricsFrame(const TagLib::ByteVector &data, Header *h) : Frame(h)
{
  d = new UnsynchronizedLyricsFramePrivate();
  parseFields(fieldData(data));
}

#endif // HAVE_TAGLIB && !TAGLIB_SUPPORTS_USLT_FRAMES
